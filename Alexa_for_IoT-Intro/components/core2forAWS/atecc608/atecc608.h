#pragma once

#include "stdio.h"

#define ATECC608_I2C_PORT I2C_NUM_1

ATCA_STATUS Atecc608_Init();

ATCA_STATUS Atecc608_GetSerialString(char * sn);
