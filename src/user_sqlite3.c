#include "string.h"
#include "user_sqlite3.h"
#include "user_utils.h"
#include "user_fatfs.h"
#include "sqlite3.h"
#include "sdfat_fns.h"
#include "vfs.h"
#include "safe_str_lib.h"
#include "assert.h"

#define USER_DEBUG(fmt, ...)
#define CACHEBLOCKSZ                        (64)
#define DEFAULT_MAXNAMESIZE                 (32)

typedef struct st_linkedlist {
    uint16_t blockid;
    struct st_linkedlist *next;
    uint8_t data[CACHEBLOCKSZ];
} linkedlist_t, *pLinkedList_t;

typedef struct st_filecache {
    uint32_t size;
    linkedlist_t *list;
} filecache_t, *pFileCache_t;

typedef struct UserSqlite3File {
    sqlite3_file base;
    vfs_file *fd;
    filecache_t *cache;
    char name[DEFAULT_MAXNAMESIZE];
} UserSqlite3File;

int UserClose(sqlite3_file*);
int UserLock(sqlite3_file *, int);
int UserUnlock(sqlite3_file*, int);
int UserSync(sqlite3_file*, int);
int UserOpen(sqlite3_vfs*, const char *, sqlite3_file *, int, int*);
int UserRead(sqlite3_file*, void*, int, sqlite3_int64);
int UserWrite(sqlite3_file*, const void*, int, sqlite3_int64);
int UserTruncate(sqlite3_file*, sqlite3_int64);
int UserDelete(sqlite3_vfs*, const char *, int);
int UserFileSize(sqlite3_file*, sqlite3_int64*);
int UserAccess(sqlite3_vfs*, const char*, int, int*);
int UserFullPathname(sqlite3_vfs*, const char *, int, char*);
int UserCheckReservedLock(sqlite3_file*, int *);
int UserFileControl(sqlite3_file *, int, void*);
int UserSectorSize(sqlite3_file*);
int UserDeviceCharacteristics(sqlite3_file*);
void* UserDlOpen(sqlite3_vfs*, const char *);
void UserDlError(sqlite3_vfs*, int, char*);
void (*UserDlSym(sqlite3_vfs*, void*, const char*))(void);
void UserDlClose(sqlite3_vfs*, void*);
int UserRandomness(sqlite3_vfs*, int, char*);
int UserSleep(sqlite3_vfs*, int);
int UserCurrentTime(sqlite3_vfs*, double*);
int UserFileClose(sqlite3_file*);
int UserFileRead(sqlite3_file*, void*, int, sqlite3_int64);
int UserFileWrite(sqlite3_file*, const void*, int, sqlite3_int64);
int UserFileFileSize(sqlite3_file*, sqlite3_int64*);
int UserFileSync(sqlite3_file*, int);

static sqlite3_vfs g_userVfs = {
    1,          // iVersion
    sizeof(UserSqlite3File),   // szOsFile
    FS_OBJ_NAME_LEN,    // mxPathname
    NULL,               // pNext
    "mh1903",          // name
    0,                  // pAppData
    UserOpen,           // xOpen
    UserDelete,         // xDelete
    UserAccess,         // xAccess
    UserFullPathname,   // xFullPathname
    UserDlOpen,         // xDlOpen
    UserDlError,        // xDlError
    UserDlSym,          // xDlSym
    UserDlClose,        // xDlClose
    UserRandomness,     // xRandomness
    UserSleep,          // xSleep
    UserCurrentTime,    // xCurrentTime
    0                   // xGetLastError
};

const sqlite3_io_methods g_fileIoMethods = {
    1,
    UserClose,
    UserRead,
    UserWrite,
    UserTruncate,
    UserSync,
    UserFileSize,
    UserLock,
    UserUnlock,
    UserCheckReservedLock,
    UserFileControl,
    UserSectorSize,
    UserDeviceCharacteristics
};

const sqlite3_io_methods g_fileMemMethods = {
    1,
    UserFileClose,
    UserFileRead,
    UserFileWrite,
    UserTruncate,
    UserFileSync,
    UserFileFileSize,
    UserLock,
    UserUnlock,
    UserCheckReservedLock,
    UserFileControl,
    UserSectorSize,
    UserDeviceCharacteristics
};

static sqlite3 *g_db;

uint32_t linkedlist_store(linkedlist_t **leaf, uint32_t offset, uint32_t len, const uint8_t *data)
{
    const uint8_t blank[CACHEBLOCKSZ] = { 0 };
    uint16_t blockid = offset / CACHEBLOCKSZ;
    linkedlist_t *block;

    if (!memcmp(data, blank, CACHEBLOCKSZ))
        return len;

    block = *leaf;
    if (!block || (block->blockid != blockid)) {
        block = (linkedlist_t *) sqlite3_malloc(sizeof(linkedlist_t));
        if (!block)
            return SQLITE_NOMEM;

        memset(block->data, 0, CACHEBLOCKSZ);
        block->blockid = blockid;
    }

    if (!*leaf) {
        *leaf = block;
        block->next = NULL;
    } else if (block != *leaf) {
        if (block->blockid > (*leaf)->blockid) {
            block->next = (*leaf)->next;
            (*leaf)->next = block;
        } else {
            block->next = (*leaf);
            (*leaf) = block;
        }
    }

    memcpy(block->data + offset % CACHEBLOCKSZ, data, len);

    return len;
}

uint32_t filecache_pull(pFileCache_t cache, uint32_t offset, uint32_t len, uint8_t *data)
{
    uint16_t i;
    float blocks;
    uint32_t r = 0;

    blocks = (offset % CACHEBLOCKSZ + len) / (float) CACHEBLOCKSZ;
    if (blocks == 0.0)
        return 0;
    if (!cache->list)
        return 0;

    if ((blocks - (int) blocks) > 0.0)
        blocks = blocks + 1.0;

    for (i = 0; i < (uint16_t) blocks; i++) {
        uint16_t round;
        float relablock;
        linkedlist_t *leaf;
        uint32_t relaoffset, relalen;
        uint8_t * reladata = (uint8_t*) data;

        relalen = len - r;

        reladata = reladata + r;
        relaoffset = offset + r;

        round = CACHEBLOCKSZ - relaoffset % CACHEBLOCKSZ;
        if (relalen > round) relalen = round;

        for (leaf = cache->list; leaf && leaf->next; leaf = leaf->next) {
            if ((leaf->next->blockid * CACHEBLOCKSZ) > relaoffset)
                break;
        }

        relablock = relaoffset / ((float)CACHEBLOCKSZ) - leaf->blockid;

        if ((relablock >= 0) && (relablock < 1))
            memcpy(data + r, leaf->data + (relaoffset % CACHEBLOCKSZ), relalen);

        r = r + relalen;
    }

    return 0;
}

uint32_t filecache_push(pFileCache_t cache, uint32_t offset, uint32_t len, const uint8_t *data)
{
    uint16_t i;
    float blocks;
    uint32_t r = 0;
    uint8_t updateroot = 0x1;

    blocks = (offset % CACHEBLOCKSZ + len) / (float) CACHEBLOCKSZ;

    if (blocks == 0.0)
        return 0;

    if ((blocks - (int) blocks) > 0.0)
        blocks = blocks + 1.0;

    for (i = 0; i < (uint16_t) blocks; i++) {
        uint16_t round;
        uint32_t localr;
        linkedlist_t *leaf;
        uint32_t relaoffset, relalen;
        uint8_t * reladata = (uint8_t*) data;

        relalen = len - r;

        reladata = reladata + r;
        relaoffset = offset + r;

        round = CACHEBLOCKSZ - relaoffset % CACHEBLOCKSZ;
        if (relalen > round) relalen = round;

        for (leaf = cache->list; leaf && leaf->next; leaf = leaf->next) {
            if ((leaf->next->blockid * CACHEBLOCKSZ) > relaoffset)
                break;
            updateroot = 0x0;
        }

        localr = linkedlist_store(&leaf, relaoffset, (relalen > CACHEBLOCKSZ) ? CACHEBLOCKSZ : relalen, reladata);
        if (localr == SQLITE_NOMEM)
            return SQLITE_NOMEM;

        r = r + localr;

        if (updateroot & 0x1)
            cache->list = leaf;
    }

    if (offset + len > cache->size)
        cache->size = offset + len;

    return r;
}

void filecache_free(pFileCache_t cache)
{
    pLinkedList_t ll = cache->list, next;

    while (ll != NULL) {
        next = ll->next;
        sqlite3_free(ll);
        ll = next;
    }
}

int UserFileClose(sqlite3_file *id)
{
    UserSqlite3File *file = (UserSqlite3File*) id;

    filecache_free(file->cache);
    sqlite3_free(file->cache);

    USER_DEBUG("UserFileClose: %s OK\n", file->name);
    return SQLITE_OK;
}

int UserFileRead(sqlite3_file *id, void *buffer, int amount, sqlite3_int64 offset)
{
    int32_t ofst;
    UserSqlite3File *file = (UserSqlite3File*) id;
    USER_DEBUG("memread\n");
    ofst = (int32_t)(offset & 0x7FFFFFFF);

    filecache_pull(file->cache, ofst, amount, (uint8_t *) buffer);

    USER_DEBUG("UserFileRead: %s [%ld] [%d] OK\n", file->name, ofst, amount);
    return SQLITE_OK;
}

int UserFileWrite(sqlite3_file *id, const void *buffer, int amount, sqlite3_int64 offset)
{
    int32_t ofst;
    UserSqlite3File *file = (UserSqlite3File*) id;

    ofst = (int32_t)(offset & 0x7FFFFFFF);

    filecache_push(file->cache, ofst, amount, (const uint8_t *) buffer);

    USER_DEBUG("UserFileWrite: %s [%ld] [%d] OK\n", file->name, ofst, amount);
    return SQLITE_OK;
}

int UserFileSync(sqlite3_file *id, int flags)
{
    USER_DEBUG("UserFileSync: %s OK\n", file->name);
    return  SQLITE_OK;
}

int UserFileFileSize(sqlite3_file *id, sqlite3_int64 *size)
{
    UserSqlite3File *file = (UserSqlite3File*) id;

    *size = 0LL | file->cache->size;
    USER_DEBUG("UserFileFileSize: %s [%d] OK\n", file->name, file->cache->size);
    return SQLITE_OK;
}

int UserOpen(sqlite3_vfs * vfs, const char * path, sqlite3_file * file, int flags, int * outflags)
{
    const char *mode = "r";
    UserSqlite3File *p = (UserSqlite3File*) file;

    if (path == NULL) return SQLITE_IOERR;

    USER_DEBUG("UserOpen: 1o %s %s\n", path, "r");
    memset(p, 0, sizeof(UserSqlite3File));

    strncpy(p->name, path, DEFAULT_MAXNAMESIZE);
    p->name[DEFAULT_MAXNAMESIZE - 1] = '\0';

    if (flags & SQLITE_OPEN_MAIN_JOURNAL) {
        p->fd = 0;
        p->cache = (filecache_t *) sqlite3_malloc(sizeof(filecache_t));
        if (! p->cache)
            return SQLITE_NOMEM;
        memset(p->cache, 0, sizeof(filecache_t));

        p->base.pMethods = &g_fileMemMethods;
        return SQLITE_OK;
    }

    vfs_file *filep = vfs_open(path, mode);
    p->fd = filep;
    if (p->fd <= 0) {
        return SQLITE_CANTOPEN;
    }

    p->base.pMethods = &g_fileIoMethods;
    USER_DEBUG("UserOpen: %s %d OK\n", p->name, p->fd);
    return SQLITE_OK;
}

int UserClose(sqlite3_file *id)
{
    UserSqlite3File *file = (UserSqlite3File*) id;

    int rc = sdfat_close(file->fd);
    USER_DEBUG("UserClose: %s %d %d\n", file->name, file->fd, rc);
    return rc ? SQLITE_IOERR_CLOSE : SQLITE_OK;
}

int UserRead(sqlite3_file *id, void *buffer, int amount, sqlite3_int64 offset)
{
    size_t nRead;
    int32_t iofst;
    UserSqlite3File *file = (UserSqlite3File*) id;

    iofst = (int32_t)(offset & 0x7FFFFFFF);

    vfs_lseek(file->fd, iofst);

    nRead = vfs_read(file->fd, buffer, amount);
    if (nRead == amount) {
        return SQLITE_OK;
    } else if (nRead >= 0) {
        USER_DEBUG("read failed\n");
        return SQLITE_IOERR_SHORT_READ;
    }

    return SQLITE_IOERR_READ;
}

int UserWrite(sqlite3_file *id, const void *buffer, int amount, sqlite3_int64 offset)
{
    return SQLITE_OK;
}

int UserTruncate(sqlite3_file *id, sqlite3_int64 bytes)
{
    USER_DEBUG("UserTruncate:\n");
    return 0 ? SQLITE_IOERR_TRUNCATE : SQLITE_OK;
}

int UserDelete(sqlite3_vfs * vfs, const char * path, int syncDir)
{
    USER_DEBUG("UserDelete: %s OK\n", path);
    return SQLITE_OK;
}

int UserFileSize(sqlite3_file *id, sqlite3_int64 *size)
{
    UserSqlite3File *file = (UserSqlite3File*) id;
    *size = 0LL | vfs_size(file->fd);
    return SQLITE_OK;
}

int UserSync(sqlite3_file *id, int flags)
{
    UserSqlite3File *file = (UserSqlite3File*) id;

    int rc = vfs_flush(file->fd);

    return rc ? SQLITE_IOERR_FSYNC : SQLITE_OK;
}

int UserAccess(sqlite3_vfs * vfs, const char * path, int flags, int * result)
{
    // struct vfs_stat st;
    // int32_t rc = vfs_stat(path, &st);
    // *result = (rc != VFS_RES_ERR);

    USER_DEBUG("UserAccess: %d\n", *result);
    return SQLITE_OK;
}

int UserFullPathname(sqlite3_vfs * vfs, const char * path, int len, char * fullpath)
{
    // struct vfs_stat st;
    // int32_t rc = vfs_stat(path, &st);
    // if (rc == VFS_RES_OK) {
    //     strncpy(fullpath, st.name, len);
    // } else {
    //     strncpy(fullpath, path, len);
    // }

    strncpy(fullpath, path, len);
    fullpath[ len - 1 ] = '\0';

    USER_DEBUG("UserFullPathname: %s\n", fullpath);
    return SQLITE_OK;
}

int UserLock(sqlite3_file *id, int lock_type)
{
    return SQLITE_OK;
}

int UserUnlock(sqlite3_file *id, int lock_type)
{
    USER_DEBUG("UserUnlock:\n");
    return SQLITE_OK;
}

int UserCheckReservedLock(sqlite3_file *id, int *result)
{
    *result = 0;

    USER_DEBUG("UserCheckReservedLock:\n");
    return SQLITE_OK;
}

int UserFileControl(sqlite3_file *id, int op, void *arg)
{
    USER_DEBUG("UserFileControl:\n");
    return SQLITE_OK;
}

int UserSectorSize(sqlite3_file *id)
{
    USER_DEBUG("UserSectorSize:\n");
#define SPI_FLASH_SEC_SIZE 4096
    return SPI_FLASH_SEC_SIZE;
}

int UserDeviceCharacteristics(sqlite3_file *id)
{
    USER_DEBUG("UserDeviceCharacteristics:\n");
    return 0;
}

void * UserDlOpen(sqlite3_vfs * vfs, const char * path)
{
    USER_DEBUG("UserDlOpen:\n");
    return NULL;
}

void UserDlError(sqlite3_vfs * vfs, int len, char * errmsg)
{
    USER_DEBUG("UserDlError:\n");
    return;
}

void (* UserDlSym(sqlite3_vfs * vfs, void * handle, const char * symbol))(void)
{
    USER_DEBUG("UserDlSym:\n");
    return NULL;
}

void UserDlClose(sqlite3_vfs * vfs, void * handle)
{
    USER_DEBUG("UserDlClose:\n");
    return;
}

int UserRandomness(sqlite3_vfs * vfs, int len, char * buffer)
{
    // int rc = os_get_random((unsigned char *) buffer, len);
    USER_DEBUG("UserRandomness: %d\n", rc);
    return SQLITE_OK;
}

int UserSleep(sqlite3_vfs * vfs, int microseconds)
{
    USER_DEBUG("UserSleep:\n");
    return SQLITE_OK;
}

int UserCurrentTime(sqlite3_vfs * vfs, double * result)
{
    *result = 2440587.5;
    USER_DEBUG("UserCurrentTime: %g\n", *result);
    return SQLITE_OK;
}

int sqlite3_os_init(void)
{
    sqlite3_vfs_register(&g_userVfs, 1);
    return SQLITE_OK;
}

int sqlite3_os_end(void)
{
    return SQLITE_OK;
}

int OpenDb(char *filename, sqlite3 **db)
{
    int rc = sqlite3_open(filename, db);
    if (rc) {
        USER_DEBUG("Can't open database: %s\n", sqlite3_errmsg(*db));
        return rc;
    } else {
        USER_DEBUG("Opened database successfully\n");
    }
    return rc;
}

static int callback(void *output, int nCol, char **argv, char **azColName)
{
    int i;
    int maxLen = 0;
    if (nCol == 1) {
        maxLen = SQL_BUFF_MAX_SIZE;
    } else if (nCol == 2) {
        maxLen = SQL_ABI_BUFF_MAX_SIZE;
    } else {
        USER_DEBUG("callback nCol error\n");
        return 0;
    }
    char **data = (char**)output;
    for (i = 0; i < nCol; i++) {
        USER_DEBUG("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
        snprintf(data[i], maxLen, "%s", argv[i] ? argv[i] : "NULL");
    }

    USER_DEBUG("\n");
    return 0;
}

char *zErrMsg = 0;
int db_exec(sqlite3 *db, const char *sql, char *result1)
{
    USER_DEBUG("sql: %s\n", sql);
    char *result[1] = {result1};
    int rc = sqlite3_exec(db, sql, callback, (void*)result, &zErrMsg);
    if (rc != SQLITE_OK) {
        printf("SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        sqlite3_free(zErrMsg);
    }
    return rc;
}

int db_exec_2(sqlite3 *db, const char *sql, char *result1, char* result2)
{
    USER_DEBUG("sql: %s\n", sql);
    char *result[2] = {result1, result2};
    int rc = sqlite3_exec(db, sql, callback, (void*)result, &zErrMsg);
    if (rc != SQLITE_OK) {
        printf("SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        sqlite3_free(zErrMsg);
    }
    return rc;
}

void SqliteTest(void)
{
    sqlite3 *db1;
    int ret = 0;
    if (OpenDb("0:ens.db", &db1))
        return;

    char buf[32];
    ret = db_exec(db1, "Select name from ens where addr = '0xf7a87f8e9136df87f8691c39edfd924e4944f35d'", buf);
    if (ret != SQLITE_OK) {
        sqlite3_close(db1);
        return;
    }
    sqlite3_close(db1);
}

bool GetEnsName(const char *addr, char *name)
{
    assert(strnlen_s(addr, SQL_ADDR_MAX_LEN) == SQL_ADDR_MAX_LEN - 1);
    sqlite3 *db;
    if (OpenDb(ENS_DB_FILE_PATH, &db)) {
        return NULL;
    }

    char sqlBuf[SQL_BUFF_MAX_SIZE] = {0};
    char lowAddr[SQL_BUFF_MAX_SIZE] = {0};
    strcpy(lowAddr, addr);
    snprintf(sqlBuf, SQL_BUFF_MAX_SIZE, "Select name from ens where addr = '%s'", strlwr(lowAddr));
    int ret = db_exec(db, sqlBuf, name);
    if (ret != SQLITE_OK) {
        sqlite3_close(db);
        return false;
    }

    sqlite3_close(db);
    return strnlen_s(name, SQL_ENS_NAME_MAX_LEN) ? true : false;
}

bool GetDBContract(const char* address, const char *selector, const uint32_t chainId, char *functionABIJson, char *contractName)
{
    assert(strnlen_s(address, SQL_ADDR_MAX_LEN) == SQL_ADDR_MAX_LEN - 1);
    sqlite3 *db;
    char index = address[2]; // [0,f]

    char contractDBPath[128] = {0};
    sprintf(contractDBPath, "0:contracts/%u_%c_contracts.db", chainId, index);
    if (OpenDb(contractDBPath, &db)) {
        return NULL;
    }

    char sqlBuf[SQL_BUFF_MAX_SIZE] = {0};
    snprintf(sqlBuf, SQL_BUFF_MAX_SIZE, "Select functionABI, name from contracts where selectorId = '%s' and address = '%s'", selector, address);
    int ret = db_exec_2(db, sqlBuf, functionABIJson, contractName);
    if (ret != SQLITE_OK) {
        sqlite3_close(db);
        return false;
    }

    sqlite3_close(db);
    return strnlen_s(functionABIJson, SQL_ABI_BUFF_MAX_SIZE) ? true : false;
}

void sqlite_open_close(void)
{
    sqlite3 *db;
    if (OpenDb("0:ens.db", &db))
        return;
    sqlite3_close(db);
}

void sqlite_open(void)
{
    sqlite3 *db;
    if (OpenDb("0:ens.db", &db))
        return;
    g_db = db;
}

void UserSqlite3Init(void)
{
    sqlite3_initialize();
}

void Sqlite3Test(int argc, char *argv[])
{
    if (strcmp(argv[0], "init") == 0) {
        VALUE_CHECK(argc, 1);
        sqlite3_initialize();
    } else if (strcmp(argv[0], "test") == 0) {
        VALUE_CHECK(argc, 1);
        SqliteTest();
    } else if (strcmp(argv[0], "ens") == 0) {
        VALUE_CHECK(argc, 2);
        char name[SQL_ENS_NAME_MAX_LEN] = {0};
        bool isexist = GetEnsName(argv[1], name);
        if (isexist == true) {
            printf("addr %s found name %s\n", argv[1], name);
        } else {
            printf("addr %s not found name\n", argv[1]);
        }
    } else if (strcmp(argv[0], "open_close") == 0) {
        VALUE_CHECK(argc, 1);
        sqlite_open_close();
    } else if (strcmp(argv[0], "open")) {
        VALUE_CHECK(argc, 1);
        sqlite_open();
    }
}
