#pragma once

void FT6336U_Init();
void FT6336U_UpdateTask();
void FT6336U_GetTouch(uint16_t* x, uint16_t* y, bool* press_down);
bool FT6336U_WasPressed();
uint16_t FT6336U_GetPressPosX();
uint16_t FT6336U_GetPressPosY();
