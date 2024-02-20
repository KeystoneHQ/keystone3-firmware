#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int32_t SimulatorSaveAccountSecret(uint8_t accountIndex, const AccountSecret_t *accountSecret, const char *password);
int32_t SimulatorLoadAccountSecret(uint8_t accountIndex, AccountSecret_t *accountSecret, const char *password);
uint8_t SimulatorGetAccountNum(void);
int32_t SimulatorVerifyPassword(uint8_t *accountIndex, const char *password);
int32_t SimulatorVerifyCurrentPassword(uint8_t accountIndex, const char *password);
