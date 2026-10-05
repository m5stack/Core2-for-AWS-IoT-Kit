/* SPDX-FileCopyrightText: 2022 Rashed Talukder
 * SPDX-FileCopyrightText: 2022 M5Stack
 * SPDX-License-Identifier: MIT */

#pragma once
#include <stdint.h>
#include "esp_err.h"
#include "esp_event.h"

typedef struct esp_netif_obj esp_netif_t;
typedef struct { uint32_t addr; } esp_ip4_addr_t;
typedef struct { struct { esp_ip4_addr_t ip; } ip_info; } ip_event_got_ip_t;
enum { IP_EVENT_STA_GOT_IP = 0 };
extern esp_event_base_t const IP_EVENT;
#define IPSTR "%u"
#define IP2STR(address) ((unsigned int)(address)->addr)

esp_err_t esp_netif_init(void);
esp_netif_t *esp_netif_create_default_wifi_sta(void);
void esp_netif_destroy(esp_netif_t *netif);
esp_err_t esp_wifi_clear_default_wifi_driver_and_handlers(void *netif);
