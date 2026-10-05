# SPDX-FileCopyrightText: 2022 Rashed Talukder
# SPDX-FileCopyrightText: 2022 M5Stack
# SPDX-License-Identifier: MIT

idf_component_register( SRCS "smoke.c"
                        REQUIRES Core2-for-AWS-IoT-Kit nvs_flash )