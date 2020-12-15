#pragma once

#include "stdio.h"

#define ATECC608A_I2C_PORT I2C_NUM_1

ATCA_STATUS Atecc608a_Init();

ATCA_STATUS Atecc608a_GetSerialString(char * sn);
