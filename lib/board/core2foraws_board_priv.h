/*
 * Core2 for AWS IoT Kit BSP v3.0.0
 * Copyright (C) 2022 Rashed Talukder.  All Rights Reserved.
 * Copyright (C) 2022 M5Stack. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef _CORE2FORAWS_BOARD_PRIV_H_
#define _CORE2FORAWS_BOARD_PRIV_H_

#include "core2foraws_board.h"

/* Called by the display driver once it has identified the panel. */
void core2foraws_board_lcd_set( core2foraws_board_lcd_t lcd );

#endif
