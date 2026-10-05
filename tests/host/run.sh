#!/bin/sh
# SPDX-FileCopyrightText: 2022 Rashed Talukder
# SPDX-FileCopyrightText: 2022 M5Stack
# SPDX-License-Identifier: MIT

set -eu
cd "$(dirname "$0")"
build_dir=$(mktemp -d "${TMPDIR:-/tmp}/core2-bsp-tests.XXXXXX")
trap 'rm -rf "$build_dir"' EXIT HUP INT TERM

for bsp_enabled in 0 1; do
    bsp_flag=
    if [ "$bsp_enabled" -eq 1 ]; then
        bsp_flag=-DCONFIG_SOFTWARE_BSP_SUPPORT=1
    fi
    "${CC:-cc}" -std=c11 -Wall -Wextra -Werror -g \
        -fsanitize=address,undefined -fno-omit-frame-pointer \
        -Imocks/include $bsp_flag \
        -DCONFIG_SOFTWARE_DISPLAY_SUPPORT=1 -DCONFIG_SOFTWARE_BUTTON_SUPPORT=1 \
        -DCONFIG_SOFTWARE_MOTION_SUPPORT=1 -DCONFIG_SOFTWARE_RTC_SUPPORT=1 \
        -DCONFIG_SOFTWARE_CRYPTO_SUPPORT=1 -DCONFIG_SOFTWARE_RGB_LED_SUPPORT=1 \
        -DCONFIG_SOFTWARE_WIFI_SUPPORT=1 \
        test_init.c -o "$build_dir/test_init_$bsp_enabled"
    "$build_dir/test_init_$bsp_enabled"
done

"${CC:-cc}" -std=c11 -Wall -Wextra -Werror -Wno-unused-variable -g \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    -Imocks/include -I../../lib/common/include test_i2c.c -o "$build_dir/test_i2c"
"$build_dir/test_i2c"

"${CC:-cc}" -std=c11 -Wall -Wextra -Werror -g \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    -Imocks/include -I../../lib/audio/include test_audio.c -o "$build_dir/test_audio"
"$build_dir/test_audio"

"${CC:-cc}" -std=c11 -Wall -Wextra -Werror -g \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    -Imocks/include -I../../lib/rgb_led/include test_rgb.c -o "$build_dir/test_rgb"
"$build_dir/test_rgb"

"${CC:-cc}" -std=c11 -Wall -Wextra -Werror -g \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    -Imocks/include -I../../lib/common/include -I../../lib/expports/include \
    test_expports.c -o "$build_dir/test_expports"
"$build_dir/test_expports"

"${CC:-cc}" -std=c11 -Wall -Wextra -Werror -g \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    -Imocks/include -I../../lib/common/include -I../../lib/motion/include \
    test_motion.c -o "$build_dir/test_motion"
"$build_dir/test_motion"

"${CC:-cc}" -std=c11 -Wall -Wextra -Werror -Wno-unused-variable -g \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    -Imocks/include -I../../lib/common/include -I../../lib/motion/include \
    test_bmi270.c ../../lib/motion/bmi270_config.c -o "$build_dir/test_bmi270"
"$build_dir/test_bmi270"

"${CC:-cc}" -std=c11 -Wall -Wextra -Werror -g \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    -Imocks/include -I../../lib/common/include -I../../lib/board -I../../lib/board/include \
    test_board.c -o "$build_dir/test_board"
"$build_dir/test_board"

"${CC:-cc}" -std=c11 -Wall -Wextra -Werror -g \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    -Imocks/include test_sd.c -o "$build_dir/test_sd"
"$build_dir/test_sd"

"${CC:-cc}" -std=c11 -D_DARWIN_C_SOURCE -D_POSIX_C_SOURCE=200809L \
    -Wall -Wextra -Werror -g -fsanitize=address,undefined -fno-omit-frame-pointer \
    -Imocks/include -I../../lib/common/include -I../../lib/rtc/include \
    test_rtc.c -o "$build_dir/test_rtc"
"$build_dir/test_rtc"

"${CC:-cc}" -std=c11 -Wall -Wextra -Werror -g \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    -I../../lib/power/include -Imocks/include -I../../lib/common/include \
    test_power.c ../../lib/power/axp192.c -o "$build_dir/test_power"
"$build_dir/test_power"

"${CC:-cc}" -std=c11 -Wall -Wextra -Werror -Wno-unused-variable -Wno-unused-parameter -g \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    -Imocks/wifi -Imocks/include -I../../lib/wifi/include \
    -DCONFIG_CORE2FORAWS_WIFI_PROVISIONING=1 -DCONFIG_CORE2FORAWS_WIFI_RECONNECT_MAX_BACKOFF_S=32 \
    test_wifi.c -o "$build_dir/test_wifi"
"$build_dir/test_wifi"