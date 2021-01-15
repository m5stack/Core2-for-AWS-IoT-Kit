#pragma once
#include "stdint.h"

void Speaker_Init();

void Speaker_WriteBuff(uint8_t* buff, uint32_t len, uint32_t timeout);

void Speaker_Deinit();