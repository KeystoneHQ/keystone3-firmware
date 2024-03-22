#ifndef _USER_SQLITE3_H
#define _USER_SQLITE3_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#define ENS_DB_FILE_PATH                    "0:ens.db"
#define SQL_BUFF_MAX_SIZE                   (256)
#define SQL_ABI_BUFF_MAX_SIZE               (2048)
#define SQL_ENS_NAME_MAX_LEN                (64)
#define SQL_ADDR_MAX_LEN                    (42 + 1)

void Sqlite3Test(int argc, char *argv[]);
void UserSqlite3Init(void);
bool GetEnsName(const char *addr, char *name);
bool GetDBContract(const char* address, const char *selector, const uint32_t chainId, char *functionABIJson, char *contractName);

#endif
