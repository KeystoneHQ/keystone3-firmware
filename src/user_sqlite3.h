#ifndef _USER_SQLITE3_H
#define _USER_SQLITE3_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

void Sqlite3Test(int argc, char *argv[]);
void UserSqlite3Init(void);
bool GetEnsName(const char *addr, char *name);
bool GetDBContract(const char* address, const char *selector, const uint32_t chainId, char *functionABIJson, char *contractName);

#endif
