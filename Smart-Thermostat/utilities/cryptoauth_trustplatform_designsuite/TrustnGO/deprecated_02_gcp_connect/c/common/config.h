/**
 * \file
 * \brief  Example Configuration
 *
 * \copyright (c) 2017 Microchip Technology Inc. and its subsidiaries.
 *            You may use this software and any derivatives exclusively with
 *            Microchip products.
 *
 * \page License
 * 
 * (c) 2017 Microchip Technology Inc. and its subsidiaries. You may use this
 * software and any derivatives exclusively with Microchip products.
 * 
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.
 * 
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIPS TOTAL LIABILITY ON ALL CLAIMS IN
 * ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 * 
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
 * TERMS.
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include "atmel_start.h"


/** Period used to update the timer/counter callbacks */
#define TIMER_UPDATE_PERIOD     (100)

#define WIFI_UPDATE_PERIOD      TIMER_UPDATE_PERIOD
#define CLIENT_UPDATE_PERIOD    TIMER_UPDATE_PERIOD

/* Define if json library will be used */
#define CONFIG_USE_JSON_LIB

/* Define for General Debug */
#define CONFIG_DEBUG

/* Define for WIFI Manager Debug */
#define CONFIG_WIFI_DEBUG

/* Define for Client Manager Debug */
#define CONFIG_CLIENT_DEBUG

/* Define if the connection parameters are in code */
#define CONFIG_USE_STATIC_CONFIG

/* Define if simulating the data */
#define CONFIG_SENSOR_SIMULATOR

  
/** \brief Check if the configuration has been loaded */
bool config_ready(void);

/** \brief Configure the crypto device being used */
void config_crypto(void);


/* Functions to retrieve configuration data */
int config_get_ssid(char* buf, size_t buflen);
int config_get_password(char* buf, size_t buflen);
int config_get_client_id(char* buf, size_t buflen);
int config_get_client_username(char* buf, size_t buflen);
int config_get_client_password(char* buf, size_t buflen);
int config_get_client_pub_topic(char* buf, size_t buflen);
int config_get_client_sub_topic(char* buf, size_t buflen);
int config_get_host_info(char* buf, size_t buflen, uint16_t * port);
int config_print_public_key(void);
int config_set_gcp_thing_id(void);

#ifdef CONFIG_DEBUG
#define DEBUG_PRINTF(f, ...)  printf(f, ##__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)  __NOP()
#endif

#endif /* CONFIG_H_ */