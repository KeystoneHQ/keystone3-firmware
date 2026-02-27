#ifndef _DRV_POWER_H
#define _DRV_POWER_H

typedef enum {
    POWER_TYPE_VCC33,
} PowerType;

void PowerInit(void);

void OpenPower(PowerType powerType);
void ClosePower(PowerType powerType);
void PowerTest(int argc, char *argv[]);

#endif
