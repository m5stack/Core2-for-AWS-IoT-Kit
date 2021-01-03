// Copyright 2018 Espressif Systems (Shanghai) PTE LTD
// All rights reserved.

#pragma once

#include "sdkconfig.h"

void app_aws_iot_set_thing_name(char *thing_name);
char *app_aws_iot_get_thing_name();
void app_aws_iot_callback();
