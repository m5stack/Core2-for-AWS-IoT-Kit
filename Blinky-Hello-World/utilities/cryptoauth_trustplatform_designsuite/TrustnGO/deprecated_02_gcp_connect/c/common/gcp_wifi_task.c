/**
 * \file
 * \brief GCP WiFi Functions
 *
 * \copyright (c) 2015-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip software
 * and any derivatives exclusively with Microchip products. It is your
 * responsibility to comply with third party license terms applicable to your
 * use of third party software (including open source software) that may
 * accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
 * PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT,
 * SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE
 * OF ANY KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF
 * MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE
 * FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL
 * LIABILITY ON ALL CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED
 * THE AMOUNT OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR
 * THIS SOFTWARE.
 */

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "atcacert/atcacert_client.h"
#include "atcacert/atcacert_host_hw.h"
#include "gcp_status.h"
#include "gcp_wifi_task.h"
#include "common/include/nm_common.h"
#include "cryptoauthlib.h"
#include "driver/include/m2m_periph.h"
#include "driver/include/m2m_ssl.h"
#include "driver/include/m2m_types.h"
#include "driver/include/m2m_wifi.h"
#include "MQTTClient.h"
#include "parson.h"
#include "driver_init.h"
#include "hex_dump.h"
#include "time_utils.h"
#include "config.h"

#define PUBLISH_INTERVAL	5000
#define GOOGLE_HOST_ENDPOINT   "mqtt.googleapis.com"
#define GOOGLE_PORT                    (443)

/** Maximum Allowed SSID Length */
#define WIFI_MAX_SSID_SIZE  32

/** Maximum Allowed Password Length */
#define WIFI_MAX_PASS_SIZE  64


ATCAIfaceCfg cfg_ateccx08a_gcp = {
	.iface_type             = ATCA_I2C_IFACE,
	.devtype                = ATECC608A,
	.atcai2c.slave_address  = 0x6A,
	.atcai2c.bus            = 2,
	.atcai2c.baud           = 400000,
	.wake_delay             = 1500,
	.rx_retries             = 20
};

enum oled1_pushbutton_id
{
	OLED1_PUSHBUTTON_ID_UNKNOWN = 0,
	OLED1_PUSHBUTTON_ID_1       = 1,
	OLED1_PUSHBUTTON_ID_2       = 2,
	OLED1_PUSHBUTTON_ID_3       = 3
};

enum oled1_led_state
{
	OLED1_LED_ON     = 0,
	OLED1_LED_OFF    = 1,
	OLED1_LED_TOGGLE = 2
};

static struct demo_button_state g_demo_button_state;
enum oled1_pushbutton_id g_selected_pushbutton;
static bool g_demo_led_state;


#define MQTT_BUFFER_SIZE            (1024)
#define MQTT_COMMAND_TIMEOUT_MS     (2000)
#define MQTT_YEILD_TIMEOUT_MS       (500)
#define MQTT_KEEP_ALIVE_INTERVAL_S  (900) // GCP will disconnect after 30min unless kept alive with a PING message


// Global variables

//! The current state of the GCP WIFI task
static enum gcp_iot_state g_gcp_wifi_state = GCP_STATE_WINC1500_INIT;


//! The GCP TLS connection
static struct socket_connection g_socket_connection;
static uint8_t g_host_ip_address[4];
static bool g_is_connected = false;

static MQTTClient g_mqtt_client;
static Network    g_mqtt_network;

static uint8_t  g_rx_buffer[MQTT_BUFFER_SIZE];
static uint32_t g_rx_buffer_length = 0;
static uint32_t g_rx_buffer_location = 0;

static uint8_t  g_mqtt_rx_buffer[MQTT_BUFFER_SIZE];
static uint8_t  g_mqtt_tx_buffer[MQTT_BUFFER_SIZE];


static char g_mqtt_update_topic_name[257];
static char g_mqtt_update_delta_topic_name[257];

static enum wifi_status g_wifi_status = WIFI_STATUS_UNKNOWN;
static uint32_t g_tx_size = 0;

typedef struct {
    int code;
    const char* name;
} ErrorInfo;

#define NEW_SOCKET_ERROR(err) {err, #err}
static const ErrorInfo g_socket_error_info[] =
{
    NEW_SOCKET_ERROR(SOCK_ERR_NO_ERROR),
    NEW_SOCKET_ERROR(SOCK_ERR_INVALID_ADDRESS),
    NEW_SOCKET_ERROR(SOCK_ERR_ADDR_ALREADY_IN_USE),
    NEW_SOCKET_ERROR(SOCK_ERR_MAX_TCP_SOCK),
    NEW_SOCKET_ERROR(SOCK_ERR_MAX_UDP_SOCK),
    NEW_SOCKET_ERROR(SOCK_ERR_INVALID_ARG),
    NEW_SOCKET_ERROR(SOCK_ERR_MAX_LISTEN_SOCK),
    NEW_SOCKET_ERROR(SOCK_ERR_INVALID),
    NEW_SOCKET_ERROR(SOCK_ERR_ADDR_IS_REQUIRED),
    NEW_SOCKET_ERROR(SOCK_ERR_CONN_ABORTED),
    NEW_SOCKET_ERROR(SOCK_ERR_TIMEOUT),
    NEW_SOCKET_ERROR(SOCK_ERR_BUFFER_FULL),
};

void console_print_message(const char *message)
{
	printf("%s\r\n", message);	
}
void console_print_success_message(const char *message)
{
    printf("SUCCESS:  %s\r\n", message);
}
void console_print_error_message(const char *message)
{
    printf("ERROR:    %s\r\n", message);
}
void console_print_hex_dump(const void *buffer, size_t length)
{
	print_hex_dump(buffer, length, true, true, 16);
}


static const char* get_socket_error_name(int error_code)
{
    for (size_t i = 0; i < sizeof(g_socket_error_info) / sizeof(g_socket_error_info[0]); i++)
        if (error_code == g_socket_error_info[i].code)
            return g_socket_error_info[i].name;
    return "UNKNOWN";
}
 


static void gcp_wifi_callback(uint8 u8MsgType, void *pvMsg)
{
    tstrM2mWifiStateChanged *wifi_state_changed = NULL;
    tstrM2MIPConfig *ip_config = NULL;
    tstrSystemTime *system_time = NULL;
    uint8 *ip_address = NULL;
    char message[256];
    
    switch (u8MsgType)
    {
    case M2M_WIFI_RESP_CON_STATE_CHANGED:
        wifi_state_changed = (tstrM2mWifiStateChanged*)pvMsg;
        
        switch (wifi_state_changed->u8CurrState)
        {
        case M2M_WIFI_CONNECTED:
            console_print_message("WINC1500 WIFI: Connected to the WIFI access point.");
            break;
        
        case M2M_WIFI_DISCONNECTED:
            if (wifi_state_changed->u8CurrState == M2M_WIFI_CONNECTED)
            {
                // Set the state to disconnect from the GCPIoT
                g_gcp_wifi_state = GCP_STATE_GCP_DISCONNECT;
            }
            else
            {
                console_print_message("WINC1500 WIFI: Disconnected from the WIFI access point.");

                // Set the state to disconnect from the GCPIoT
                g_gcp_wifi_state = GCP_STATE_WIFI_CONFIGURE;
            }
            break;
        
        default:
            memset(&message[0], 0, sizeof(message));
            sprintf(&message[0], "WINC1500 WIFI: Unknown connection status: %d",
                    wifi_state_changed->u8ErrCode);
            console_print_error_message(message);
            break;
        }

        break;
        
    case M2M_WIFI_REQ_DHCP_CONF:
        ip_config = (tstrM2MIPConfig*)pvMsg;
        ip_address = (uint8*)&ip_config->u32StaticIP;

        memset(&message[0], 0, sizeof(message));
        sprintf(&message[0], "WINC1500 WIFI: Device IP Address: %u.%u.%u.%u",
                ip_address[0], ip_address[1], ip_address[3], ip_address[4]);
        console_print_message(message);

		gethostbyname((uint8_t*)GOOGLE_HOST_ENDPOINT);
        break;
        
    case M2M_WIFI_RESP_GET_SYS_TIME:
        system_time = (tstrSystemTime*)pvMsg;
        memset(&message[0], 0, sizeof(message));
        sprintf(&message[0], "WINC1500 WIFI: Device Time:       %02d/%02d/%02d %02d:%02d:%02d",
                system_time->u16Year, system_time->u8Month, system_time->u8Day,
                system_time->u8Hour, system_time->u8Minute, system_time->u8Second);
		time_utils_set(system_time->u16Year, system_time->u8Month, system_time->u8Day, system_time->u8Hour, system_time->u8Minute, system_time->u8Second);
        console_print_message(message);
        break;
        
    default:
        printf("%s: unhandled message %d\r\n", __FUNCTION__, (int)u8MsgType);
        // Do nothing
        break;
    }
}

static void gcp_wifi_ssl_callback(uint8 u8MsgType, void *pvMsg)
{
 

}

static void gcp_wifi_socket_handler(SOCKET sock, uint8 u8Msg, void *pvMsg)
{
    tstrSocketConnectMsg *socket_connect_message = NULL;
    tstrSocketRecvMsg *socket_receive_message = NULL;
    sint16 *bytes_sent = NULL;
    
    // Check for the WINC1500 WIFI socket events
    switch (u8Msg)
    {
    case SOCKET_MSG_CONNECT:
        socket_connect_message = (tstrSocketConnectMsg*)pvMsg;
        if (socket_connect_message != NULL)
        {
            if (socket_connect_message->s8Error == SOCK_ERR_NO_ERROR)
            {
                // Set the state to connected to the GCP IoT
                g_gcp_wifi_state = GCP_STATE_GCP_CONNECTED;
            }
            else
            {
                // An error has occurred
                printf("SOCKET_MSG_CONNECT error %s(%d)\r\n", get_socket_error_name(socket_connect_message->s8Error), socket_connect_message->s8Error);
                
                // Set the state to disconnect from the GCP IoT
                g_gcp_wifi_state = GCP_STATE_GCP_DISCONNECT;
            }
        }
        break;
    
    case SOCKET_MSG_RECV:
    case SOCKET_MSG_RECVFROM:
        socket_receive_message = (tstrSocketRecvMsg*)pvMsg;
        if (socket_receive_message != NULL)
        {
            if (socket_receive_message->s16BufferSize >= 0)
            {
                g_rx_buffer_length += socket_receive_message->s16BufferSize;

                // The message was received
                if (socket_receive_message->u16RemainingSize == 0)
                {
                    g_wifi_status = WIFI_STATUS_MESSAGE_RECEIVED;
                }
                //printf("%s: SOCKET_MSG_RECV %d\r\n", __FUNCTION__, (int)socket_receive_message->s16BufferSize);
            }
            else
            {
                if (socket_receive_message->s16BufferSize == SOCK_ERR_TIMEOUT)
                {
                    // A timeout has occurred
                    g_wifi_status = WIFI_STATUS_TIMEOUT;
                }
                else
                {
                    // An error has occurred
                    g_wifi_status = WIFI_STATUS_ERROR;

                    // Set the state to disconnect from the GCP IoT
                    g_gcp_wifi_state = GCP_STATE_WIFI_DISCONNECT;
                }
            }
        }
        break;
        
    case SOCKET_MSG_SEND:
        bytes_sent = (sint16*)pvMsg;
        
        if (*bytes_sent <= 0 || *bytes_sent > (int32_t)g_tx_size)
        {
            // Seen an odd instance where bytes_sent is way more than the requested bytes sent.
            // This happens when we're expecting an error, so were assuming this is an error
            // condition.
            g_wifi_status = WIFI_STATUS_ERROR;

            // Set the state to disconnect from the GCP IoT
            g_gcp_wifi_state = GCP_STATE_WIFI_DISCONNECT;
        }
        else if (*bytes_sent > 0)
        {
            // The message was sent
            g_wifi_status = WIFI_STATUS_MESSAGE_SENT;
        }
        //printf("%s: SOCKET_MSG_SEND %d\r\n", __FUNCTION__, (int)*bytes_sent);
    break;

    default:
        printf("%s: unhandled message %d\r\n", __FUNCTION__, (int)u8Msg);
        // Do nothing
        break;
    }
}

static void gcp_wifi_dns_resolve_handler(uint8 *pu8DomainName, uint32 u32ServerIP)
{
    sint8 status = SOCK_ERR_INVALID_ARG;
    SOCKET new_socket = SOCK_ERR_INVALID;
    struct sockaddr_in socket_address;
    int ssl_caching_enabled = 1;
    char message[128];
    
    if (u32ServerIP != 0)
    {
        // Save the Host IP Address
        g_host_ip_address[0] = u32ServerIP & 0xFF;
        g_host_ip_address[1] = (u32ServerIP >> 8) & 0xFF;
        g_host_ip_address[2] = (u32ServerIP >> 16) & 0xFF;
        g_host_ip_address[3] = (u32ServerIP >> 24) & 0xFF;
        
        sprintf(&message[0], "WINC1500 WIFI: DNS lookup:\r\n  Host:       %s\r\n  IP Address: %u.%u.%u.%u",
                (char*)pu8DomainName, g_host_ip_address[0], g_host_ip_address[1],
                g_host_ip_address[2], g_host_ip_address[3]);
        console_print_message(message);

        do
        {
            // Create the socket
            new_socket = socket(AF_INET, SOCK_STREAM, 1);
            if (new_socket < 0)
            {
                console_print_error_message("Failed to create the socket.");
                
                // Set the state to disconnect from the GCP IoT
                g_gcp_wifi_state = GCP_STATE_GCP_DISCONNECT;
                
                // Break the do/while loop
                break;
            }
            
            // Set the socket address information
            socket_address.sin_family      = AF_INET;
            socket_address.sin_addr.s_addr = _htonl((uint32)((g_host_ip_address[0] << 24) |
                                                             (g_host_ip_address[1] << 16) |
                                                             (g_host_ip_address[2] << 8)  |
                                                             g_host_ip_address[3]));
            socket_address.sin_port        = _htons(GOOGLE_PORT);
            
            setsockopt(new_socket, SOL_SSL_SOCKET, SO_SSL_ENABLE_SESSION_CACHING,
                       &ssl_caching_enabled, sizeof(ssl_caching_enabled));
            

            // Connect to the GCP IoT server
            status = connect(new_socket, (struct sockaddr*)&socket_address,
                             sizeof(socket_address));
            if (status != SOCK_ERR_NO_ERROR)
            {
                memset(&message[0], 0, sizeof(message));
                sprintf(&message[0], "WINC1500 WIFI: Failed to connect to GCP Iot.");
                console_print_error_message(message);
                
                // Close the socket
                close(new_socket);
                
                // Set the state to disconnect from the GCP IoT
                g_gcp_wifi_state = GCP_STATE_GCP_DISCONNECT;
                
                // Break the do/while loop
                break;
            }

            // Save the new socket connection information
            g_socket_connection.socket    = new_socket;
            g_socket_connection.address   = socket_address.sin_addr.s_addr;
            g_socket_connection.port      = GOOGLE_PORT;
        } while (false);
    }
    else
    {
        // An error has occurred                
        console_print_error_message("WINC1500 DNS lookup failed.");
                
        // Set the state to disconnect from the GCP IoT
        g_gcp_wifi_state = GCP_STATE_GCP_DISCONNECT;
    }
}
static void gcp_mqtt_subscribe_callback(MessageData *data)
{
    JSON_Value *delta_message_value = NULL;
    JSON_Object *delta_message_object = NULL;
    JSON_Object *led_state_object = NULL;
    const char *led_status = NULL;

    do 
    {
        // Parse the LED update message
        delta_message_value   = json_parse_string((char*)data->message->payload);
        delta_message_object  = json_value_get_object(delta_message_value);
        
        led_state_object = json_object_get_object(delta_message_object, "state");
        if (led_state_object == NULL)
        {
            // Break the do/while loop
            break;
        }

        // Print the received MQTT LED update message
        console_print_message("\r\n");
        console_print_message("Received MQTT Shadow Update Delta Message:");
        console_print_hex_dump(data->message->payload, data->message->payloadlen);
        console_print_message("\r\n");

        led_status = json_object_get_string(led_state_object, "led1");
        if (led_status != NULL)
		{
			g_demo_led_state = (strcmp(led_status, "on") == 0) ? 0 : 1;
	        gpio_set_pin_level(LED0,  g_demo_led_state);			
        }
		//led_status = json_object_get_string(led_state_object, "led2");
        //led_status = json_object_get_string(led_state_object, "led3");
    } while (false);

    // Free allocated memory
    json_value_free(delta_message_value);

    
}

static void gcp_wifi_disable_pullups(void)
{
    uint32 pin_mask = 
    (
        M2M_PERIPH_PULLUP_DIS_HOST_WAKEUP     |
        M2M_PERIPH_PULLUP_DIS_SD_CMD_SPI_SCK  |
        M2M_PERIPH_PULLUP_DIS_SD_DAT0_SPI_TXD
    );
    
    m2m_periph_pullup_ctrl(pin_mask, 0);
}

static unsigned short gcp_wifi_get_message_id(void)
{
    static uint16_t message_id = 0;

    message_id++;
    
    if (message_id == (USHRT_MAX - 1))
    {
        message_id = 1;
    }

    return message_id;
}
static sint8 gcp_wifi_init(void)
{
    sint8 wifi_status = M2M_SUCCESS;
    tstrWifiInitParam wifi_paramaters;
    
    do 
    {
        // Reset the global Demo Button states
        memset(&g_demo_button_state, 0, sizeof(g_demo_button_state));
        
        // Register the GCP WIFI socket callbacks
        registerSocketCallback(gcp_wifi_socket_handler, gcp_wifi_dns_resolve_handler);
    
        // Set the GCP WIFI configuration attributes
        m2m_memset((uint8*)&wifi_paramaters, 0, sizeof(wifi_paramaters));
        wifi_paramaters.pfAppWifiCb = gcp_wifi_callback;
    
        // Initialize the WINC1500 WIFI module
		set_winc_spi_descriptor(&SPI_INSTANCE);
        nm_bsp_init();
        wifi_status = m2m_wifi_init(&wifi_paramaters);
        if (wifi_status != M2M_SUCCESS)
        {
            // Break the do/while loop
            break;
        }

        // Initialize the WINC1500 SSL module
        wifi_status = m2m_ssl_init(gcp_wifi_ssl_callback);
        if (wifi_status != M2M_SUCCESS)
        {
            // Break the do/while loop
            break;
        }

        // Set the active WINC1500 TLS cipher suites
         wifi_status = m2m_ssl_set_active_ciphersuites(SSL_ENABLE_ALL_SUITES);
        if (wifi_status != M2M_SUCCESS)
        {
            // Break the do/while loop
            break;
        }

        // Reset the socket connection information
        memset(&g_socket_connection, 0, sizeof(g_socket_connection));

        // Initialize the WINC1500 WIFI socket
        socketInit();      
        gcp_wifi_disable_pullups();
        
        
        // Initialize the MQTT library
        g_mqtt_network.mqttread  = &mqtt_packet_read;
        g_mqtt_network.mqttwrite = &mqtt_packet_write;
        
        MQTTClientInit(&g_mqtt_client, &g_mqtt_network, MQTT_COMMAND_TIMEOUT_MS, 
                       g_mqtt_tx_buffer, sizeof(g_mqtt_tx_buffer),
                       g_mqtt_rx_buffer, sizeof(g_mqtt_rx_buffer));
    } while (false);
    
    return wifi_status;
}

/**
 * \brief Sets the current GCP WIFI state of the GCP WIFI task
 */
void gcp_wifi_set_state(enum gcp_iot_state state)
{
    g_gcp_wifi_state = state;
}

/**
 * \brief Gets the current GCP WIFI task state.
 */
enum gcp_iot_state gcp_wifi_get_state(void)
{
    return g_gcp_wifi_state;
}

int gcp_wifi_read_data(uint8_t *read_buffer, uint32_t read_length, 
                       uint32_t timeout_ms)
{
    int status = SUCCESS;
    
    if (g_is_connected == false || g_gcp_wifi_state <= GCP_STATE_WIFI_DISCONNECT)
    {
        return FAILURE;
    }
    
    if (g_rx_buffer_length >= read_length)
    {
        status = SUCCESS;
        
        // Get the data from the existing received buffer
        memcpy(&read_buffer[0], &g_rx_buffer[g_rx_buffer_location], read_length);

        g_rx_buffer_location += read_length;
        g_rx_buffer_length -= read_length;
    }
    else
    {
        // Reset the message buffer information
        g_wifi_status = WIFI_STATUS_UNKNOWN;
        g_rx_buffer_location = 0;
        g_rx_buffer_length = 0;

        memset(&g_rx_buffer[0], 0, sizeof(g_rx_buffer));

        // Receive the incoming message
        g_wifi_status = recv(g_socket_connection.socket, g_rx_buffer, sizeof(g_rx_buffer), timeout_ms);

        do
        {
            // Wait until the incoming message or error was received
            m2m_wifi_handle_events(NULL);

            if (g_wifi_status == WIFI_STATUS_TIMEOUT)
            {
                status = FAILURE;
                
                // Break the do/while loop
                break;
            }
            else if (g_wifi_status == WIFI_STATUS_ERROR)
            {
                status = FAILURE;
                
                // Break the do/while loop
                break;
            }

            if (g_wifi_status == WIFI_STATUS_MESSAGE_RECEIVED)
            {
                status = SUCCESS;

                memcpy(&read_buffer[0], &g_rx_buffer[0], read_length);

                g_rx_buffer_location += read_length;
                g_rx_buffer_length -= read_length;
            }            
        } while (g_wifi_status != WIFI_STATUS_MESSAGE_RECEIVED);
    }
            
    return ((status == SUCCESS) ? (int)read_length : status);
}

int gcp_wifi_send_data(uint8_t *send_buffer, uint32_t send_length, 
                       uint32_t timeout_ms)
{
    int status = SUCCESS;
    
    if (g_is_connected == false)
    {
        return FAILURE;
    }
    
    g_wifi_status = send(g_socket_connection.socket, send_buffer, send_length, 0);
    g_tx_size = send_length;

    do
    {
        // Wait until the outgoing message was sent
        m2m_wifi_handle_events(NULL);    

        if (g_wifi_status == WIFI_STATUS_ERROR)
        {
            status = FAILURE;
                
            // Break the do/while loop
            break;
        }
    } while (g_wifi_status != WIFI_STATUS_MESSAGE_SENT);
    
    return ((status == SUCCESS) ? (int)send_length : status);
}


void gcp_wifi_publish_message(struct demo_button_state state)
{
    int mqtt_status = FAILURE;
    MQTTMessage message;
    char json_message[256];
    uint32_t ts = time_utils_get_utc();
	char* led_status;

    do
    {
        // Only publish message when in the reporting state
        if (g_mqtt_client.isconnected != 1)
            break; 
			
	      
		gpio_toggle_pin_level(LED0);
		led_status = (gpio_get_pin_level(LED0) == 0) ? "ON" : "OFF";
	
			       	
       snprintf(json_message, sizeof(json_message), "{ \"timestamp\": %lu, \"Led_Status\": \"%s\"}", ts, led_status );


            
        message.qos      = QOS0;
        message.retained = 0;
        message.dup      = 0;
        message.id       = gcp_wifi_get_message_id();
                
		message.payload = (void*)json_message;
		message.payloadlen = strlen(json_message);

        console_print_message("Publishing MQTT Shadow Update Message:");
        console_print_hex_dump(message.payload, message.payloadlen);
		
		
		config_get_client_pub_topic(g_mqtt_update_topic_name,sizeof(g_mqtt_update_topic_name));
        mqtt_status = MQTTPublish(&g_mqtt_client, g_mqtt_update_topic_name, &message);
        if (mqtt_status != SUCCESS)
        {
            // The GCP IoT Demo failed to publish the MQTT LED update message
            gcp_iot_set_status(GCP_STATE_GCP_REPORTING,
                                GCP_STATUS_GCP_REPORT_FAILURE,
                                "The GCP IoT Demo failed to publish the MQTT shadow update message.");
                    
            console_print_message("\r\n");
            console_print_error_message("The GCP IoT Demo failed to publish the MQTT shadow update message.");
        }
    } while (false);


}

static uint32_t publish_update_message;

/* Helper functions */
static bool client_counter_finished()
{

	return (0 == publish_update_message);
}

static void client_counter_set(uint32_t val)
{
	/* Convert to loop time*/
	publish_update_message = val / CLIENT_UPDATE_PERIOD;

}

/* Must be called on the CLIENT_UPDATE_PERIOD */
void client_timer_update(void)
{
	if(publish_update_message)
	{
		publish_update_message--;
	}
}

void gcp_wifi_task(void *params)
{
    ATCA_STATUS atca_status = ATCA_STATUS_UNKNOWN;
    sint8 wifi_status = M2M_SUCCESS;
    int mqtt_status = FAILURE;
    MQTTPacket_connectData mqtt_options = MQTTPacket_connectData_initializer;
    char message[256];
    char ssid[WIFI_MAX_SSID_SIZE];
    char pass[WIFI_MAX_PASS_SIZE];
	
	if(ATCA_SUCCESS != (atca_status = config_print_public_key()))
		return;
	
    do 
    {
        // The state machine for the GCP WIFI task
        switch (g_gcp_wifi_state)
        {
        case GCP_STATE_WINC1500_INIT:
            /**
             * Initialize the GCP IoT Zero Touch Demo GCP WIFI task
             *
             * This portion of the state machine should never be
             * called more than once
             */
            // Initialize the GCP WINC1500 WIFI
            wifi_status = gcp_wifi_init();
            if (wifi_status == M2M_SUCCESS)
            {
                // Set the current state
                gcp_iot_set_status(GCP_STATE_WIFI_CONFIGURE, GCP_STATUS_SUCCESS, 
					"The GCP IoT Zero Touch Demo WINC1500 WIFI configure was successful.");
                
                // Set the next GCP WIFI state
                g_gcp_wifi_state = GCP_STATE_WIFI_CONFIGURE;                    
            }
            else
            {
                // Set the current state
                gcp_iot_set_status(GCP_STATE_ATECCx08A_INIT, GCP_STATUS_ATECCx08A_INIT_FAILURE,
					"The GCP IoT Zero Touch Demo WINC1500 WIFI init was not successful.");

                console_print_error_message("An WINC1500 WIFI initialization error has occurred.");
                console_print_error_message("Stopping the GCP IoT demo.");

                // An error has occurred during initialization.  Stop the demo.
                g_gcp_wifi_state = GCP_STATE_UNKNOWN;
            }             
            break;
            
        case GCP_STATE_WIFI_CONFIGURE:
       
                // Set the current state
                gcp_iot_set_status(GCP_STATE_GCP_CONNECT, GCP_STATUS_SUCCESS,
					"The GCP IoT Zero Touch Demo WINC1500 WIFI connect was successful.");
                    
                // Set the next GCP WIFI state
                g_gcp_wifi_state = GCP_STATE_GCP_CONNECT;
         
            break;
            
        case GCP_STATE_GCP_CONNECT:
            do 
            {
                // Start the WINC1500 WIFI connect process
                do
                {
					/* Get the WIFI SSID */
					config_get_ssid(ssid, sizeof(ssid));
					
					/* Get the WIFI Password */
					config_get_password(pass, sizeof(pass));
				
                    memset(&message[0], 0, sizeof(message));
                    sprintf(message, "\r\nAttempting to connect to GCP IoT ...\r\n  SSID:     %s\r\n  Password: %s",
                            ssid, pass);
                    console_print_message(message);
                    
                    if (strlen(pass) > 0)
                    {
                        wifi_status = m2m_wifi_connect(ssid, sizeof(ssid), 
							M2M_WIFI_SEC_WPA_PSK, pass, M2M_WIFI_CH_ALL);
                    }
                    else
                    {
                        // Zero-length password used to indicate an open wifi ap
                        wifi_status = m2m_wifi_connect(ssid, sizeof(ssid), 
							M2M_WIFI_SEC_OPEN, pass, M2M_WIFI_CH_ALL);
                    }
                    if (wifi_status == M2M_SUCCESS)
                    {
                        // Set the next GCP WIFI state
                        g_gcp_wifi_state = GCP_STATE_GCP_CONNECTING;
                    }
                    else
                    {
                        // Delay the GCP WIFI task connection attempts
                        delay_us(500);
                    }
                } while (wifi_status != M2M_SUCCESS);
            } while (false);            
            break;

        case GCP_STATE_GCP_CONNECTING:
            // Waiting for the GCP IoT connection to complete
            break;

        case GCP_STATE_GCP_CONNECTED:
            // The GCP Zero Touch Demo is connect to GCP IoT
            console_print_success_message("GCP Demo: Connected to GCP IoT.");
            g_is_connected = true;
			uint8_t buf[1024];
			 size_t buf_bytes_remaining = 1024;

            do 
            {
                // Send the MQTT Connect message
                mqtt_options.keepAliveInterval = MQTT_KEEP_ALIVE_INTERVAL_S;
                mqtt_options.cleansession = 1;
				
				config_set_gcp_thing_id();
				
				 /* Client ID String */
				 mqtt_options.clientID.cstring = (char*)&buf[0];
				 config_get_client_id(mqtt_options.clientID.cstring, buf_bytes_remaining);
				

				 /* Username String */
				 mqtt_options.username.cstring = mqtt_options.clientID.cstring + strlen(mqtt_options.clientID.cstring) + 1;
				 buf_bytes_remaining -= (mqtt_options.username.cstring - mqtt_options.clientID.cstring);
				 config_get_client_username(mqtt_options.username.cstring, buf_bytes_remaining);
				

				 /* Password String */
				 mqtt_options.password.cstring = mqtt_options.username.cstring + strlen(mqtt_options.username.cstring) + 1;
				 buf_bytes_remaining -= (mqtt_options.password.cstring - mqtt_options.username.cstring);
				 config_get_client_password(mqtt_options.password.cstring, buf_bytes_remaining);
				
            
                mqtt_status = MQTTConnect(&g_mqtt_client, &mqtt_options);
                if (mqtt_status != SUCCESS)
                {
                    // The GCP IoT Demo failed to retrieve the device serial number
                    gcp_iot_set_status(GCP_STATE_GCP_SUBSCRIPTION, GCP_STATUS_GCP_SUBSCRIPTION_FAILURE,
						"The GCP IoT Demo failed to connect with the MQTT connect message.");
                
                    console_print_message("\r\n");
                    console_print_error_message("The GCP IoT Demo failed to connect with the MQTT connect message.");
                
                    // Set the state to start the GCP WIFI Disconnect process
                    if (g_gcp_wifi_state > GCP_STATE_WIFI_DISCONNECT)
                        g_gcp_wifi_state = GCP_STATE_GCP_DISCONNECT;

                    // Break the do/while loop
                    break;
                }
            
			     config_get_client_sub_topic(g_mqtt_update_delta_topic_name, sizeof(g_mqtt_update_delta_topic_name));
                // Subscribe to the GCP IoT update delta topic message
                mqtt_status = MQTTSubscribe(&g_mqtt_client, g_mqtt_update_delta_topic_name, QOS0, &gcp_mqtt_subscribe_callback);
                if (mqtt_status != SUCCESS)
                {
                    // The GCP IoT Demo failed to retrieve the device serial number
                    gcp_iot_set_status(GCP_STATE_GCP_SUBSCRIPTION, GCP_STATUS_GCP_SUBSCRIPTION_FAILURE, 
						"The GCP IoT Demo failed to subscribe to the MQTT update topic subscription.");
                
                    console_print_message("\r\n");
                    console_print_error_message("The GCP IoT Demo failed to subscribe to the MQTT update topic subscription.");
                
                    // Set the state to start the GCP WIFI Disconnect process
                    if (g_gcp_wifi_state > GCP_STATE_WIFI_DISCONNECT)
                        g_gcp_wifi_state = GCP_STATE_GCP_DISCONNECT;

                    // Break the do/while loop
                    break;
                }
            
                console_print_message("\r\n");
                console_print_success_message("Subscribed to the MQTT update topic subscription:");
                console_print_success_message(g_mqtt_update_delta_topic_name);
                console_print_message("\r\n");
      

                // Set the state to GCP WIFI Reporting process
                g_gcp_wifi_state = GCP_STATE_GCP_REPORTING;
            } while (false);
            break;
            
        case GCP_STATE_GCP_REPORTING:
            // Sending/receiving topic update messages to/from GCP IoT
            if (client_counter_finished())
            {            
                client_counter_set(PUBLISH_INTERVAL);
                // Publish the button update message
                gcp_wifi_publish_message(g_demo_button_state);
                
            }
            else
            {
                // Wait for incoming update messages
                mqtt_status = MQTTYield(&g_mqtt_client, MQTT_YEILD_TIMEOUT_MS);
                if (mqtt_status != SUCCESS)
                {
                    // The GCP IoT Demo failed to retrieve the device serial number
                    gcp_iot_set_status(GCP_STATE_GCP_REPORTING, GCP_STATUS_GCP_REPORT_FAILURE, 
						"The GCP IoT Demo failed to publish the MQTT LED update message.");
                    
                    console_print_message("\r\n");
                    console_print_error_message("The GCP IoT Demo failed to publish the MQTT LED update message.");
                }
                
                // If an error occurred in the WIFI connection, make sure to disconnect properly
                if (g_wifi_status == WIFI_STATUS_ERROR)
                {
                    g_is_connected = false;
                }
            }
            break;

        case GCP_STATE_WIFI_DISCONNECT:
            // The GCP Zero Touch Demo is disconnected from access point
            g_is_connected = false;

            // Disconnect from the WINC1500 WIFI
            m2m_wifi_disconnect();
            
            // Close the socket
            close(g_socket_connection.socket);
                        
            console_print_success_message("GCP Demo: Disconnected from WIFI access point.");
            
            // Set the state to start the GCP WIFI Configure process
            g_gcp_wifi_state = GCP_STATE_WIFI_CONFIGURE;
            break;
                        
        case GCP_STATE_GCP_DISCONNECT:       
            // The GCP Zero Touch Demo is disconnected from GCP IoT

            // Disconnect from GCP IoT
            if (g_is_connected == true)
            {
                // Unsubscribe to the GCP IoT update delta topic message
                mqtt_status = MQTTUnsubscribe(&g_mqtt_client, g_mqtt_update_delta_topic_name);
                if (mqtt_status != SUCCESS)
                {
                    // The GCP IoT Demo failed to unsubscribe from the MQTT subscription
                    gcp_iot_set_status(GCP_STATE_GCP_DISCONNECT, GCP_STATUS_GCP_SUBSCRIPTION_FAILURE,
                        "The GCP IoT Demo failed to unsubscribe to the MQTT update topic subscription.");
                
                    console_print_message("\r\n");
                    console_print_error_message("The GCP IoT Demo failed to unsubscribe to the MQTT update topic subscription.");
                }
                
                // Disconnect from GCP IoT
                mqtt_status = MQTTDisconnect(&g_mqtt_client);
                if (mqtt_status != SUCCESS)
                {
                    // The GCP IoT Demo failed to disconnect from GCP IoT
                    gcp_iot_set_status(GCP_STATE_GCP_DISCONNECT, GCP_STATUS_GCP_SUBSCRIPTION_FAILURE,
						"The GCP IoT Demo failed to disconnect with the MQTT disconnect message.");
                    
                    console_print_message("\r\n");
                    console_print_error_message("The GCP IoT Demo failed to disconnect with the MQTT disconnect message.");
                }
            }
            
            // Set the state to start the GCP WIFI Disconnect process
            g_gcp_wifi_state = GCP_STATE_WIFI_DISCONNECT;
            break;
                           
        default:
            // Do nothing
            break;
        }        

        // Handle WINC1500 pending events
        m2m_wifi_handle_events(NULL);
		delay_us(500);
    } while (true);
}