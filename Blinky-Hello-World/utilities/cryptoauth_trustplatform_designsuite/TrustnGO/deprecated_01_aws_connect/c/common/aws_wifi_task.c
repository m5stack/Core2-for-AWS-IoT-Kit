/**
 * \file
 * \brief AWS WIFI FreeRTOS Task Functions
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

#include "atcacert/atcacert_pem.h"
#include "atcacert/atcacert_host_hw.h"
#include "aws_status.h"
#include "aws_wifi_task.h"
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

#include "atcacert/atcacert_client.h"
#include "tngtls_cert_def_1_signer.h"
#include "tng_atca.h"

#define MAIN_WLAN_SSID      "xxxxxxxxxx"
#define MAIN_WLAN_AUTH      M2M_WIFI_SEC_WPA_PSK
#define MAIN_WLAN_PSK       "xxxxxxxxxx"

#define AWS_HOST_ENDPOINT   "xxxxxxxxxx"


ATCAIfaceCfg cfg_ateccx08a_aws = {
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

#define AWS_PORT                    (8883)

#define MAX_TNG_CERT_DEVICE_SIZE	546
#define MAX_TLS_CERT_LENGTH			1024
#define SIGNER_CERT_MAX_LEN 		(g_tngtls_cert_def_1_signer.cert_template_size + 8) // Need some space in case the cert changes size by a few bytes
#define SIGNER_PUBLIC_KEY_MAX_LEN 	64
#define DEVICE_CERT_MAX_LEN			(MAX_TNG_CERT_DEVICE_SIZE + 8) // Need some space in case the cert changes size by a few bytes
#define CERT_SN_MAX_LEN				32
#define TLS_SRV_ECDSA_CHAIN_FILE	"ECDSA.lst"
#define INIT_CERT_BUFFER_LEN        (MAX_TLS_CERT_LENGTH*sizeof(uint32) - TLS_FILE_NAME_MAX*2 - SIGNER_CERT_MAX_LEN - DEVICE_CERT_MAX_LEN)

#define MQTT_BUFFER_SIZE            (1024)
#define MQTT_COMMAND_TIMEOUT_MS     (2000)
#define MQTT_YEILD_TIMEOUT_MS       (500)
#define MQTT_KEEP_ALIVE_INTERVAL_S  (900) // AWS will disconnect after 30min unless kept alive with a PING message


// Global variables

//! The current state of the AWS WIFI task
static enum aws_iot_state g_aws_wifi_state = AWS_STATE_WINC1500_INIT;
//! Array of private key slots to rotate through the ECDH calculations
static uint16 g_ecdh_key_slot[] = {2};
//! Index into the ECDH private key slots array
static uint32 g_ecdh_key_slot_index = 0;

//! The AWS TLS connection
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

// AWS has a limit of 128 bytes for the MQTT client ID and thing name
// See http://docs.aws.amazon.com/general/latest/gr/aws_service_limits.html#limits_iot
static char g_mqtt_client_id[129];
static char g_thing_name[129];
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

extern void set_winc_spi_descriptor(struct spi_m_sync_descriptor *spi_inst);

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

static sint8 ecdh_derive_client_shared_secret(tstrECPoint *server_public_key,
                                              uint8 *ecdh_shared_secret,
                                              tstrECPoint *client_public_key)
{
    sint8 status = M2M_ERR_FAIL;
    uint8_t ecdh_mode;
    uint16_t key_id;

    if ((g_ecdh_key_slot_index < 0) ||
        (g_ecdh_key_slot_index >= (sizeof(g_ecdh_key_slot) / sizeof(g_ecdh_key_slot[0]))))
    {
        g_ecdh_key_slot_index = 0;
    }

    if(_gDevice->mIface->mIfaceCFG->devtype == ATECC608A)
    {
        //do special ecdh functions for the 608, keep ephemeral keys in SRAM
        ecdh_mode = ECDH_MODE_SOURCE_TEMPKEY | ECDH_MODE_COPY_OUTPUT_BUFFER;
        key_id = GENKEY_PRIVATE_TO_TEMPKEY;
    }
    else
    {
        //specializations for the 508, use an EEPROM key slot
        ecdh_mode = ECDH_PREFIX_MODE;
        key_id = g_ecdh_key_slot[g_ecdh_key_slot_index];
        g_ecdh_key_slot_index++;
    }

    //generate an ephemeral key
    //TODO - add loop to make sure we get an acceptable private key
    if(atcab_genkey(key_id, client_public_key->X) == ATCA_SUCCESS)
    {
        client_public_key->u16Size = 32;
        //do the ecdh from the private key in tempkey, results put in ecdh_shared_secret
        if(atcab_ecdh_base(ecdh_mode, key_id, server_public_key->X, ecdh_shared_secret, NULL) == ATCA_SUCCESS)
        {
            status = M2M_SUCCESS;
        }
    }

    return status;
}

static sint8 ecdh_derive_key_pair(tstrECPoint *server_public_key)
{
    sint8 status = M2M_ERR_FAIL;

    if ((g_ecdh_key_slot_index < 0) ||
        (g_ecdh_key_slot_index >= (sizeof(g_ecdh_key_slot) / sizeof(g_ecdh_key_slot[0]))))
    {
        g_ecdh_key_slot_index = 0;
    }

    if( (status = atcab_genkey(g_ecdh_key_slot[g_ecdh_key_slot_index], server_public_key->X) ) == ATCA_SUCCESS)
    {
        server_public_key->u16Size      = 32;
        server_public_key->u16PrivKeyID = g_ecdh_key_slot[g_ecdh_key_slot_index];

        g_ecdh_key_slot_index++;

        status = M2M_SUCCESS;
    }

    return status;
}

static sint8 ecdsa_process_sign_verify_request(uint32 number_of_signatures)
{
    sint8 status = M2M_ERR_FAIL;
    tstrECPoint	Key;
    uint32 index = 0;
    uint8 signature[80];
    uint8 hash[80] = {0};
    uint16 curve_type = 0;

    for(index = 0; index < number_of_signatures; index++)
    {
        status = m2m_ssl_retrieve_cert(&curve_type, hash, signature, &Key);

        if (status != M2M_SUCCESS)
        {
            M2M_ERR("m2m_ssl_retrieve_cert() failed with ret=%d", status);
            return status;
        }

        if(curve_type == EC_SECP256R1)
        {
            bool is_verified = false;

            status = atcab_verify_extern(hash, signature, Key.X, &is_verified);
            if(status == ATCA_SUCCESS)
            {
                status = (is_verified == true) ? M2M_SUCCESS : M2M_ERR_FAIL;
                if(is_verified == false)
                {
                    M2M_INFO("ECDSA SigVerif FAILED\n");
                }
            }
            else
            {
                status = M2M_ERR_FAIL;
            }

            if(status != M2M_SUCCESS)
            {
                m2m_ssl_stop_processing_certs();
                break;
            }
        }
    }

    return status;
}

static sint8 ecdsa_process_sign_gen_request(tstrEcdsaSignReqInfo *sign_request,
                                            uint8 *signature,
                                            uint16 *signature_size)
{
    sint8 status = M2M_ERR_FAIL;
    uint8 hash[32];
	uint16_t device_key_slot=0;

    status = m2m_ssl_retrieve_hash(hash, sign_request->u16HashSz);
    if (status != M2M_SUCCESS)
    {
        M2M_ERR("m2m_ssl_retrieve_hash() failed with ret=%d", status);
        return status;
    }

    if(sign_request->u16CurveType == EC_SECP256R1)
    {
        *signature_size = 64;
        status = atcab_sign(device_key_slot, hash, signature);
    }

    return status;
}

static sint8 ecdh_derive_server_shared_secret(uint16 private_key_id,
                                              tstrECPoint *client_public_key,
                                              uint8 *ecdh_shared_secret)
{
    uint16 key_slot	= private_key_id;
    sint8 status = M2M_ERR_FAIL;
    uint8 atca_status = ATCA_STATUS_UNKNOWN;

    atca_status = atcab_ecdh(key_slot, client_public_key->X, ecdh_shared_secret);
    if(atca_status == ATCA_SUCCESS)
    {
        status = M2M_SUCCESS;
    }
    else
    {
        M2M_INFO("__SLOT = %u, Err = %X\n", key_slot, atca_status);
    }

    return status;
}


static void ecc_process_request(tstrEccReqInfo *ecc_request)
{
    tstrEccReqInfo ecc_response;
	uint8 signature[80];
	uint16 response_data_size = 0;
	uint8 *response_data_buffer = NULL;

    ecc_response.u16Status = 1;

    switch (ecc_request->u16REQ)
    {
	case ECC_REQ_CLIENT_ECDH:
    	ecc_response.u16Status = ecdh_derive_client_shared_secret(&(ecc_request->strEcdhREQ.strPubKey),
                                                                  ecc_response.strEcdhREQ.au8Key,
                                                                  &ecc_response.strEcdhREQ.strPubKey);
	    break;

	case ECC_REQ_GEN_KEY:
	    ecc_response.u16Status = ecdh_derive_key_pair(&ecc_response.strEcdhREQ.strPubKey);
	    break;

	case ECC_REQ_SERVER_ECDH:
	    ecc_response.u16Status = ecdh_derive_server_shared_secret(ecc_request->strEcdhREQ.strPubKey.u16PrivKeyID,
                                                                  &(ecc_request->strEcdhREQ.strPubKey),
                                                                  ecc_response.strEcdhREQ.au8Key);
	    break;

	case ECC_REQ_SIGN_VERIFY:
	    ecc_response.u16Status = ecdsa_process_sign_verify_request(ecc_request->strEcdsaVerifyREQ.u32nSig);
	    break;

	case ECC_REQ_SIGN_GEN:
	    ecc_response.u16Status = ecdsa_process_sign_gen_request(&(ecc_request->strEcdsaSignREQ), signature,
                                                                &response_data_size);
	    response_data_buffer = signature;
    	break;

    default:
        // Do nothing
        break;
    }

    ecc_response.u16REQ      = ecc_request->u16REQ;
    ecc_response.u32UserData = ecc_request->u32UserData;
    ecc_response.u32SeqNo    = ecc_request->u32SeqNo;

	m2m_ssl_ecc_process_done();
	m2m_ssl_handshake_rsp(&ecc_response, response_data_buffer, response_data_size);
}

static size_t winc_certs_get_total_files_size(const tstrTlsSrvSecHdr* header)
{
    uint8 *pBuffer = (uint8*) header;
    uint16 count = 0;

    while ((*pBuffer) == 0xFF)
    {

        if(count == INIT_CERT_BUFFER_LEN)
        break;
        count++;
        pBuffer++;
    }

    if(count == INIT_CERT_BUFFER_LEN)
    return sizeof(*header); // Buffer is empty, no files

    return header->u32NextWriteAddr;
}

static const char* bin2hex(const void* data, size_t data_size)
{
    static char buf[256];
    static char hex[] = "0123456789abcdef";
    const uint8_t* data8 = data;

    if (data_size*2 > sizeof(buf)-1)
    return "[buf too small]";

    for (size_t i = 0; i < data_size; i++)
    {
        buf[i*2 + 0] = hex[(*data8) >> 4];
        buf[i*2 + 1] = hex[(*data8) & 0xF];
        data8++;
    }
    buf[data_size*2] = 0;

    return buf;
}

static sint8 winc_certs_append_file_buf(uint32* buffer32, uint32 buffer_size,
                                        const char* file_name, uint32 file_size,
                                        const uint8* file_data)
{
    tstrTlsSrvSecHdr* header = (tstrTlsSrvSecHdr*)buffer32;
    tstrTlsSrvSecFileEntry* file_entry = NULL;
    uint16 str_size = m2m_strlen((uint8*)file_name) + 1;
    uint16 count = 0;
    uint8 *pBuffer = (uint8*)buffer32;

    while ((*pBuffer) == 0xFF)
    {

        if(count == INIT_CERT_BUFFER_LEN)
        break;
        count++;
        pBuffer++;
    }

    if(count == INIT_CERT_BUFFER_LEN)
    {
        // The WINC will need to add the reference start pattern to the header
        header->u32nEntries = 0; // No certs
        // The WINC will need to add the offset of the flash were the certificates are stored to this address
        header->u32NextWriteAddr = sizeof(*header); // Next cert will be written after the header
    }

    if (header->u32nEntries >= sizeof(header->astrEntries)/sizeof(header->astrEntries[0]))
    return M2M_ERR_FAIL; // Already at max number of files

    if ((header->u32NextWriteAddr + file_size) > buffer_size)
    return M2M_ERR_FAIL; // Not enough space in buffer for new file

    file_entry = &header->astrEntries[header->u32nEntries];
    header->u32nEntries++;

    if (str_size > sizeof(file_entry->acFileName))
    return M2M_ERR_FAIL; // File name too long
    m2m_memcpy((uint8*)file_entry->acFileName, (uint8*)file_name, str_size);

    file_entry->u32FileSize = file_size;
    file_entry->u32FileAddr = header->u32NextWriteAddr;
    header->u32NextWriteAddr += file_size;

    // Use memmove to accommodate optimizations where the file data is temporarily stored
    // in buffer32
    memmove(((uint8*)buffer32) + (file_entry->u32FileAddr), (uint8*)file_data, file_size);

    return M2M_SUCCESS;
}

static sint8 ecc_transfer_certificates(uint8_t subject_key_id[20])
{
	sint8 status = M2M_SUCCESS;
	int atca_status = ATCACERT_E_SUCCESS;
	uint8_t *signer_cert = NULL;
	size_t signer_cert_size;
	uint8_t signer_public_key[SIGNER_PUBLIC_KEY_MAX_LEN];
	uint8_t *device_cert = NULL;
	size_t device_cert_size;
	uint8_t cert_sn[CERT_SN_MAX_LEN];
	size_t cert_sn_size;
	uint8_t *file_list = NULL;
	char *device_cert_filename = NULL;
	char *signer_cert_filename = NULL;
	uint32 sector_buffer[MAX_TLS_CERT_LENGTH];
	char pem_cert[1024];
	size_t pem_cert_size;
	const atcacert_def_t* cert_def = NULL;
    do
    {
	    // Clear cert chain buffer
	    memset(sector_buffer, 0xFF, sizeof(sector_buffer));

	    // Use the end of the sector buffer to temporarily hold the data to save RAM
	    file_list   = ((uint8_t*)sector_buffer) + (sizeof(sector_buffer) - TLS_FILE_NAME_MAX*2);
	    signer_cert = file_list - SIGNER_CERT_MAX_LEN;
	    device_cert = signer_cert - DEVICE_CERT_MAX_LEN;

	    // Init the file list
	    memset(file_list, 0, TLS_FILE_NAME_MAX*2);
	    device_cert_filename = (char*)&file_list[0];
	    signer_cert_filename = (char*)&file_list[TLS_FILE_NAME_MAX];

	    // Uncompress the signer certificate from the ATECCx08A device
	    signer_cert_size = SIGNER_CERT_MAX_LEN;
	    atca_status = atcacert_read_cert(&g_tngtls_cert_def_1_signer, NULL,
			signer_cert, &signer_cert_size);
	    if (atca_status != ATCACERT_E_SUCCESS)
        {
            // Break the do/while loop
            break;
        }
		pem_cert_size = sizeof(pem_cert);
		atcacert_encode_pem_cert(signer_cert, signer_cert_size, pem_cert, &pem_cert_size);
		printf("Signer Cert : \r\n%s\r\n", pem_cert);

	    // Get the signer's public key from its certificate
	    atca_status = atcacert_get_subj_public_key(&g_tngtls_cert_def_1_signer, signer_cert,
			signer_cert_size, signer_public_key);
	    if (atca_status != ATCACERT_E_SUCCESS)
        {
            // Break the do/while loop
            break;
        }
	    atca_status = tng_get_device_cert_def(&cert_def);
	    if (atca_status != ATCA_SUCCESS)
	    {
		    break;
	    }

	    // Uncompress the device certificate from the ATECCx08A device.
	    device_cert_size = DEVICE_CERT_MAX_LEN;
	    atca_status = atcacert_read_cert(cert_def, signer_public_key,
			device_cert, &device_cert_size);
	    if (atca_status != ATCACERT_E_SUCCESS)
        {
            // Break the do/while loop
            break;
        }
		pem_cert_size = sizeof(pem_cert);
		atcacert_encode_pem_cert(device_cert, device_cert_size, pem_cert, &pem_cert_size);
		printf("Device Cert : \r\n%s\r\n", pem_cert);

        if (subject_key_id)
        {
            atca_status = atcacert_get_subj_key_id(cert_def, device_cert,
				device_cert_size, subject_key_id);
            if (atca_status != ATCACERT_E_SUCCESS)
            {
                // Break the do/while loop
                break;
            }
        }

	    // Get the device certificate SN for the filename
	    cert_sn_size = sizeof(cert_sn);
	    atca_status = atcacert_get_cert_sn(cert_def, device_cert,
			device_cert_size, cert_sn, &cert_sn_size);
	    if (atca_status != ATCACERT_E_SUCCESS)
        {
            // Break the do/while loop
            break;
        }

	    // Build the device certificate filename
	    strcpy(device_cert_filename, "CERT_");
	    strcat(device_cert_filename, bin2hex(cert_sn, cert_sn_size));

	    // Add the DER device certificate the TLS certs buffer
	    status = winc_certs_append_file_buf(sector_buffer, sizeof(sector_buffer),
			device_cert_filename, device_cert_size, device_cert);
	    if (status != M2M_SUCCESS)
        {
            // Break the do/while loop
            break;
        }

	    device_cert = NULL; // Make sure we don't use this now that it has moved

	    // Get the signer certificate SN for the filename
	    cert_sn_size = sizeof(cert_sn);
	    atca_status = atcacert_get_cert_sn(&g_tngtls_cert_def_1_signer, signer_cert,
			signer_cert_size, cert_sn, &cert_sn_size);
	    if (atca_status != ATCACERT_E_SUCCESS)
        {
            // Break the do/while loop
            break;
        }


	    // Build the signer certificate filename
	    strcpy(signer_cert_filename, "CERT_");
	    strcat(signer_cert_filename, bin2hex(cert_sn, cert_sn_size));

	    // Add the DER signer certificate the TLS certs buffer
	    status = winc_certs_append_file_buf(sector_buffer, sizeof(sector_buffer),
			signer_cert_filename, signer_cert_size, signer_cert);
	    if (status != M2M_SUCCESS)
        {
            // Break the do/while loop
            break;
        }

	    // Add the cert chain list file to the TLS certs buffer
	    status = winc_certs_append_file_buf(sector_buffer, sizeof(sector_buffer),
			TLS_SRV_ECDSA_CHAIN_FILE, TLS_FILE_NAME_MAX*2, file_list);
	    if (status != M2M_SUCCESS)
        {
            // Break the do/while loop
            break;
        }

	    file_list = NULL;
	    signer_cert_filename = NULL;
	    device_cert_filename = NULL;

	    // Update the TLS cert chain on the WINC.
	    status = m2m_ssl_send_certs_to_winc((uint8 *)sector_buffer,
			(uint32)winc_certs_get_total_files_size((tstrTlsSrvSecHdr*)sector_buffer));
        if (status != M2M_SUCCESS)
        {
            // Break the do/while loop
            break;
        }
    } while (false);

	if (atca_status)
	{
    	M2M_ERR("eccSendCertsToWINC() failed with ret=%d", atca_status);
    	status =  M2M_ERR_FAIL;
	}

	return status;
}

static void aws_wifi_callback(uint8 u8MsgType, void *pvMsg)
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
                // Set the state to disconnect from the AWS IoT
                g_aws_wifi_state = AWS_STATE_AWS_DISCONNECT;
            }
            else
            {
                console_print_message("WINC1500 WIFI: Disconnected from the WIFI access point.");

                // Set the state to disconnect from the AWS IoT
                g_aws_wifi_state = AWS_STATE_WIFI_CONFIGURE;
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

		gethostbyname((uint8_t*)AWS_HOST_ENDPOINT);
        break;

    case M2M_WIFI_RESP_GET_SYS_TIME:
        system_time = (tstrSystemTime*)pvMsg;
        memset(&message[0], 0, sizeof(message));
        sprintf(&message[0], "WINC1500 WIFI: Device Time:       %02d/%02d/%02d %02d:%02d:%02d",
                system_time->u16Year, system_time->u8Month, system_time->u8Day,
                system_time->u8Hour, system_time->u8Minute, system_time->u8Second);
        console_print_message(message);
        break;

    default:
        printf("%s: unhandled message %d\r\n", __FUNCTION__, (int)u8MsgType);
        // Do nothing
        break;
    }
}

static void aws_wifi_ssl_callback(uint8 u8MsgType, void *pvMsg)
{
    tstrEccReqInfo *ecc_request = NULL;

    switch (u8MsgType)
    {
    case M2M_SSL_REQ_ECC:
        ecc_request = (tstrEccReqInfo*)pvMsg;
        ecc_process_request(ecc_request);
        break;

    case M2M_SSL_RESP_SET_CS_LIST:
    default:
        // Do nothing
        break;
    }
}

static void aws_wifi_socket_handler(SOCKET sock, uint8 u8Msg, void *pvMsg)
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
                // Set the state to connected to the AWS IoT
                g_aws_wifi_state = AWS_STATE_AWS_CONNECTED;
            }
            else
            {
                // An error has occurred
                printf("SOCKET_MSG_CONNECT error %s(%d)\r\n", get_socket_error_name(socket_connect_message->s8Error), socket_connect_message->s8Error);

                // Set the state to disconnect from the AWS IoT
                g_aws_wifi_state = AWS_STATE_AWS_DISCONNECT;
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

                    // Set the state to disconnect from the AWS IoT
                    g_aws_wifi_state = AWS_STATE_WIFI_DISCONNECT;
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

            // Set the state to disconnect from the AWS IoT
            g_aws_wifi_state = AWS_STATE_WIFI_DISCONNECT;
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

static void aws_wifi_dns_resolve_handler(uint8 *pu8DomainName, uint32 u32ServerIP)
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

                // Set the state to disconnect from the AWS IoT
                g_aws_wifi_state = AWS_STATE_AWS_DISCONNECT;

                // Break the do/while loop
                break;
            }

            // Set the socket address information
            socket_address.sin_family      = AF_INET;
            socket_address.sin_addr.s_addr = _htonl((uint32)((g_host_ip_address[0] << 24) |
                                                             (g_host_ip_address[1] << 16) |
                                                             (g_host_ip_address[2] << 8)  |
                                                             g_host_ip_address[3]));
            socket_address.sin_port        = _htons(AWS_PORT);

            setsockopt(new_socket, SOL_SSL_SOCKET, SO_SSL_ENABLE_SESSION_CACHING,
                       &ssl_caching_enabled, sizeof(ssl_caching_enabled));

			// TESTING ONLY
			setsockopt(new_socket, SOL_SSL_SOCKET, SO_SSL_BYPASS_X509_VERIF,
                       &ssl_caching_enabled, sizeof(ssl_caching_enabled));

			// Send the SNI
            setsockopt(new_socket, SOL_SSL_SOCKET, SO_SSL_SNI,
                       AWS_HOST_ENDPOINT, sizeof(AWS_HOST_ENDPOINT) + 1);

            // Connect to the AWS IoT server
            status = connect(new_socket, (struct sockaddr*)&socket_address,
                             sizeof(socket_address));
            if (status != SOCK_ERR_NO_ERROR)
            {
                memset(&message[0], 0, sizeof(message));
                sprintf(&message[0], "WINC1500 WIFI: Failed to connect to AWS Iot.");
                console_print_error_message(message);

                // Close the socket
                close(new_socket);

                // Set the state to disconnect from the AWS IoT
                g_aws_wifi_state = AWS_STATE_AWS_DISCONNECT;

                // Break the do/while loop
                break;
            }

            // Save the new socket connection information
            g_socket_connection.socket    = new_socket;
            g_socket_connection.address   = socket_address.sin_addr.s_addr;
            g_socket_connection.port      = AWS_PORT;
        } while (false);
    }
    else
    {
        // An error has occurred
        console_print_error_message("WINC1500 DNS lookup failed.");

        // Set the state to disconnect from the AWS IoT
        g_aws_wifi_state = AWS_STATE_AWS_DISCONNECT;
    }
}
static void aws_mqtt_shadow_update_delta_callback(MessageData *data)
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

    // Report the new LED states
    aws_wifi_publish_shadow_update_message(g_demo_button_state);
}

static void aws_wifi_disable_pullups(void)
{
    uint32 pin_mask =
    (
        M2M_PERIPH_PULLUP_DIS_HOST_WAKEUP     |
        M2M_PERIPH_PULLUP_DIS_SD_CMD_SPI_SCK  |
        M2M_PERIPH_PULLUP_DIS_SD_DAT0_SPI_TXD
    );

    m2m_periph_pullup_ctrl(pin_mask, 0);
}

static unsigned short aws_wifi_get_message_id(void)
{
    static uint16_t message_id = 0;

    message_id++;

    if (message_id == (USHRT_MAX - 1))
    {
        message_id = 1;
    }

    return message_id;
}
static sint8 aws_wifi_init(void)
{
    sint8 wifi_status = M2M_SUCCESS;
    tstrWifiInitParam wifi_paramaters;

    do
    {
        // Reset the global Demo Button states
        memset(&g_demo_button_state, 0, sizeof(g_demo_button_state));

        // Register the AWS WIFI socket callbacks
        registerSocketCallback(aws_wifi_socket_handler, aws_wifi_dns_resolve_handler);

        // Set the AWS WIFI configuration attributes
        m2m_memset((uint8*)&wifi_paramaters, 0, sizeof(wifi_paramaters));
        wifi_paramaters.pfAppWifiCb = aws_wifi_callback;

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
        wifi_status = m2m_ssl_init(aws_wifi_ssl_callback);
        if (wifi_status != M2M_SUCCESS)
        {
            // Break the do/while loop
            break;
        }

        // Set the active WINC1500 TLS cipher suites
         wifi_status = m2m_ssl_set_active_ciphersuites(SSL_ECC_ONLY_CIPHERS);
        if (wifi_status != M2M_SUCCESS)
        {
            // Break the do/while loop
            break;
        }

        // Reset the socket connection information
        memset(&g_socket_connection, 0, sizeof(g_socket_connection));

        // Initialize the WINC1500 WIFI socket
        socketInit();
        aws_wifi_disable_pullups();


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
 * \brief Sets the current AWS WIFI state of the AWS WIFI task
 */
void aws_wifi_set_state(enum aws_iot_state state)
{
    g_aws_wifi_state = state;
}

/**
 * \brief Gets the current AWS WIFI task state.
 */
enum aws_iot_state aws_wifi_get_state(void)
{
    return g_aws_wifi_state;
}

int aws_wifi_read_data(uint8_t *read_buffer, uint32_t read_length,
                       uint32_t timeout_ms)
{
    int status = SUCCESS;

    if (g_is_connected == false || g_aws_wifi_state <= AWS_STATE_WIFI_DISCONNECT)
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

int aws_wifi_send_data(uint8_t *send_buffer, uint32_t send_length,
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


void aws_wifi_publish_shadow_update_message(struct demo_button_state state)
{
    int mqtt_status = FAILURE;
    MQTTMessage message;
    char json_message[256];
    JSON_Value *update_message_value = NULL;
    JSON_Object *update_message_object = NULL;

    do
    {
        // Only publish message when in the reporting state
        if (g_mqtt_client.isconnected != 1)
            break;

        // Create the Button update message
        update_message_value   = json_value_init_object();
        update_message_object  = json_value_get_object(update_message_value);

        //json_object_dotset_string(update_message_object, "state.reported.button1",
            //((state.button_1 == 1) ? "down" : "up"));
        //json_object_dotset_string(update_message_object, "state.reported.button2",
            //((state.button_2 == 1) ? "down" : "up"));
        //json_object_dotset_string(update_message_object, "state.reported.button3",
            //((state.button_3 == 1) ? "down" : "up"));
        json_object_dotset_string(update_message_object, "state.reported.led1",
            ((g_demo_led_state == 0) ? "on" : "off"));
        //json_object_dotset_string(update_message_object, "state.reported.led2",
            //(gpio_get_pin_level(LED0) ? "on" : "off"));
        //json_object_dotset_string(update_message_object, "state.reported.led3",
            //(gpio_get_pin_level(LED0) ? "on" : "off"));

        message.qos      = QOS0;
        message.retained = 0;
        message.dup      = 0;
        message.id       = aws_wifi_get_message_id();

        memset(json_message, 0, sizeof(json_message));
        json_serialize_to_buffer(update_message_value, json_message, sizeof(json_message));
        message.payload = json_message;
        message.payloadlen = (json_serialization_size(update_message_value) - 1);

        console_print_message("Publishing MQTT Shadow Update Message:");
        console_print_hex_dump(message.payload, message.payloadlen);

        mqtt_status = MQTTPublish(&g_mqtt_client, g_mqtt_update_topic_name, &message);
        if (mqtt_status != SUCCESS)
        {
            // The AWS IoT Demo failed to publish the MQTT LED update message
            aws_iot_set_status(AWS_STATE_AWS_REPORTING,
                                AWS_STATUS_AWS_REPORT_FAILURE,
                                "The AWS IoT Demo failed to publish the MQTT shadow update message.");

            console_print_message("\r\n");
            console_print_error_message("The AWS IoT Demo failed to publish the MQTT shadow update message.");
        }
    } while (false);

    // Free allocated memory
    json_value_free(update_message_value);
}

ATCA_STATUS init_cal_for_device(void)
{
	ATCA_STATUS status;
	uint8_t ser_num[9];
	char displayStr[40] = {0};
	size_t displen = sizeof(displayStr);
	bool is_config_locked, is_data_locked;

	do
	{
		if(ATCA_SUCCESS != (status = atcab_init(&cfg_ateccx08a_aws)))
			break;

		displen = sizeof(displayStr);
		if(ATCA_SUCCESS != (status = atcab_read_serial_number(ser_num)))
			break;
		if(ATCA_SUCCESS != (status = atcab_bin2hex_(ser_num, 9, displayStr, &displen, false, false, false)))
			break;
		printf("\r\n\r\n\r\nDevice Serial Number : %s\r\n\r\n", displayStr);

		if(ATCA_SUCCESS != (status = atcab_is_locked(LOCK_ZONE_CONFIG, &is_config_locked)))
			break;
		if(ATCA_SUCCESS != (status = atcab_is_locked(LOCK_ZONE_DATA, &is_data_locked)))
			break;

		printf("Config Zone: %s\r\n", is_config_locked ? "Locked" : "Unlocked");
		printf("Data Zone: %s\r\n", is_data_locked ? "Locked" : "Unlocked");
		if(!is_config_locked || !is_data_locked)
		{
			printf("To run demo, both zones should be locked!\r\n");
			status = ATCA_GEN_FAIL;
		}

	} while (0);

	return status;
}

int config_mqtt_client_id(char *buf, uint8_t buf_size)
{
	uint8_t serial_num[9];
	size_t hex_size = buf_size;
	ATCA_STATUS rv ;

	rv = atcab_read_serial_number(serial_num);
	if(ATCA_SUCCESS != rv)
	{
		return rv;
	}
	rv = atcab_bin2hex_(serial_num, sizeof(serial_num), buf, &hex_size, false, false, false);

	if(ATCA_SUCCESS != rv)
	{
		return rv;
	}

	return ATCA_SUCCESS;
}

void aws_wifi_task(void *params)
{
    ATCA_STATUS atca_status = ATCA_STATUS_UNKNOWN;
    sint8 wifi_status = M2M_SUCCESS;
    int mqtt_status = FAILURE;
    MQTTPacket_connectData mqtt_options = MQTTPacket_connectData_initializer;
    char message[256];
    uint8_t subject_key_id[20];

	if(ATCA_SUCCESS != (atca_status = init_cal_for_device()))
		return;

    do
    {
        // The state machine for the AWS WIFI task
        switch (g_aws_wifi_state)
        {
        case AWS_STATE_WINC1500_INIT:
            /**
             * Initialize the AWS IoT Zero Touch Demo AWS WIFI task
             *
             * This portion of the state machine should never be
             * called more than once
             */
            // Initialize the AWS WINC1500 WIFI
            wifi_status = aws_wifi_init();
            if (wifi_status == M2M_SUCCESS)
            {
                // Set the current state
                aws_iot_set_status(AWS_STATE_WIFI_CONFIGURE, AWS_STATUS_SUCCESS,
					"The AWS IoT Zero Touch Demo WINC1500 WIFI configure was successful.");

                // Set the next AWS WIFI state
                g_aws_wifi_state = AWS_STATE_WIFI_CONFIGURE;
            }
            else
            {
                // Set the current state
                aws_iot_set_status(AWS_STATE_ATECCx08A_INIT, AWS_STATUS_ATECCx08A_INIT_FAILURE,
					"The AWS IoT Zero Touch Demo WINC1500 WIFI init was not successful.");

                console_print_error_message("An WINC1500 WIFI initialization error has occurred.");
                console_print_error_message("Stopping the AWS IoT demo.");

                // An error has occurred during initialization.  Stop the demo.
                g_aws_wifi_state = AWS_STATE_UNKNOWN;
            }
            break;

        case AWS_STATE_WIFI_CONFIGURE:
            // Transfer the ATECCx08A certificates to the WINC1500
            wifi_status= ecc_transfer_certificates(subject_key_id);
            if (wifi_status == M2M_SUCCESS)
            {
                // Convert the TNG device serial number to a hex string to use as the MQTT client ID
				(void)config_mqtt_client_id(g_mqtt_client_id,sizeof(g_mqtt_client_id));

                // Make the thing name the same as the MQTT client ID
                memcpy(g_thing_name, g_mqtt_client_id, min(sizeof(g_thing_name), sizeof(g_mqtt_client_id)));
                g_thing_name[sizeof(g_thing_name)-1] = 0; // Ensure a terminating null

                // Initialize the AWS MQTT update topic name
                memset(&g_mqtt_update_topic_name[0], 0, sizeof(g_mqtt_update_topic_name));
                sprintf(&g_mqtt_update_topic_name[0], "$aws/things/%s/shadow/update",g_mqtt_client_id);

                // Initialize the AWS MQTT update delta topic name
                memset(&g_mqtt_update_delta_topic_name[0], 0, sizeof(g_mqtt_update_delta_topic_name));
                sprintf(&g_mqtt_update_delta_topic_name[0], "$aws/things/%s/shadow/update/delta",g_mqtt_client_id);

                // Set the current state
                aws_iot_set_status(AWS_STATE_AWS_CONNECT, AWS_STATUS_SUCCESS,
					"The AWS IoT Zero Touch Demo WINC1500 WIFI connect was successful.");

                // Set the next AWS WIFI state
                g_aws_wifi_state = AWS_STATE_AWS_CONNECT;
            }
            else
            {
                // Set the current state
                aws_iot_set_status(AWS_STATE_WIFI_CONFIGURE, AWS_STATUS_ATECCx08A_INIT_FAILURE,
					"The AWS IoT Zero Touch Demo WINC1500 WIFI configure was not successful.");

                console_print_error_message("An WINC1500 WIFI configure error has occurred.");
                console_print_error_message("Stopping the AWS IoT demo.");

                // An error has occurred during initialization.  Stop the demo.
                g_aws_wifi_state = AWS_STATE_UNKNOWN;
            }
            break;

        case AWS_STATE_AWS_CONNECT:
            do
            {
                // Start the WINC1500 WIFI connect process
                do
                {
                    memset(&message[0], 0, sizeof(message));
                    sprintf(message, "\r\nAttempting to connect to AWS IoT ...\r\n  SSID:     %s\r\n  Password: %s",
                            MAIN_WLAN_SSID, MAIN_WLAN_PSK);
                    console_print_message(message);

                    if (strlen(MAIN_WLAN_PSK) > 0)
                    {
                        wifi_status = m2m_wifi_connect(MAIN_WLAN_SSID, sizeof(MAIN_WLAN_SSID),
							M2M_WIFI_SEC_WPA_PSK, MAIN_WLAN_PSK, M2M_WIFI_CH_ALL);
                    }
                    else
                    {
                        // Zero-length password used to indicate an open wifi ap
                        wifi_status = m2m_wifi_connect(MAIN_WLAN_SSID, sizeof(MAIN_WLAN_SSID),
							M2M_WIFI_SEC_OPEN, MAIN_WLAN_PSK, M2M_WIFI_CH_ALL);
                    }
                    if (wifi_status == M2M_SUCCESS)
                    {
                        // Set the next AWS WIFI state
                        g_aws_wifi_state = AWS_STATE_AWS_CONNECTING;
                    }
                    else
                    {
                        // Delay the AWS WIFI task connection attempts
                        delay_us(500);
                    }
                } while (wifi_status != M2M_SUCCESS);
            } while (false);
            break;

        case AWS_STATE_AWS_CONNECTING:
            // Waiting for the AWS IoT connection to complete
            break;

        case AWS_STATE_AWS_CONNECTED:
            // The AWS Zero Touch Demo is connect to AWS IoT
            console_print_success_message("AWS Zero Touch Demo: Connected to AWS IoT.");
            g_is_connected = true;

            do
            {
                // Send the MQTT Connect message
                mqtt_options.keepAliveInterval = MQTT_KEEP_ALIVE_INTERVAL_S;
                mqtt_options.cleansession = 1;
                mqtt_options.clientID.cstring = g_mqtt_client_id;

                mqtt_status = MQTTConnect(&g_mqtt_client, &mqtt_options);
                if (mqtt_status != SUCCESS)
                {
                    // The AWS IoT Demo failed to retrieve the device serial number
                    aws_iot_set_status(AWS_STATE_AWS_SUBSCRIPTION, AWS_STATUS_AWS_SUBSCRIPTION_FAILURE,
						"The AWS IoT Demo failed to connect with the MQTT connect message.");

                    console_print_message("\r\n");
                    console_print_error_message("The AWS IoT Demo failed to connect with the MQTT connect message.");

                    // Set the state to start the AWS WIFI Disconnect process
                    if (g_aws_wifi_state > AWS_STATE_WIFI_DISCONNECT)
                        g_aws_wifi_state = AWS_STATE_AWS_DISCONNECT;

                    // Break the do/while loop
                    break;
                }

                // Subscribe to the AWS IoT update delta topic message
                mqtt_status = MQTTSubscribe(&g_mqtt_client, g_mqtt_update_delta_topic_name, QOS0, &aws_mqtt_shadow_update_delta_callback);
                if (mqtt_status != SUCCESS)
                {
                    // The AWS IoT Demo failed to retrieve the device serial number
                    aws_iot_set_status(AWS_STATE_AWS_SUBSCRIPTION, AWS_STATUS_AWS_SUBSCRIPTION_FAILURE,
						"The AWS IoT Demo failed to subscribe to the MQTT update topic subscription.");

                    console_print_message("\r\n");
                    console_print_error_message("The AWS IoT Demo failed to subscribe to the MQTT update topic subscription.");

                    // Set the state to start the AWS WIFI Disconnect process
                    if (g_aws_wifi_state > AWS_STATE_WIFI_DISCONNECT)
                        g_aws_wifi_state = AWS_STATE_AWS_DISCONNECT;

                    // Break the do/while loop
                    break;
                }

                console_print_message("\r\n");
                console_print_success_message("Subscribed to the MQTT update topic subscription:");
                console_print_success_message(g_mqtt_update_delta_topic_name);
                console_print_message("\r\n");

                // Flash the processing LED to show the AWS IoT Demo has connected to AWS IoT
                //led_flash_processing_led(5);

                // Publish initial button state
                aws_wifi_publish_shadow_update_message(g_demo_button_state);

                // Set the state to AWS WIFI Reporting process
                g_aws_wifi_state = AWS_STATE_AWS_REPORTING;
            } while (false);
            break;

        case AWS_STATE_AWS_REPORTING:
            // Sending/receiving topic update messages to/from AWS IoT
            if (g_selected_pushbutton != OLED1_PUSHBUTTON_ID_UNKNOWN)
            {
                switch (g_selected_pushbutton)
                {
                case OLED1_PUSHBUTTON_ID_1:
                    g_demo_button_state.button_1 = !g_demo_button_state.button_1;
                    break;

                case OLED1_PUSHBUTTON_ID_2:
                    g_demo_button_state.button_2 = !g_demo_button_state.button_2;
                    break;

                case OLED1_PUSHBUTTON_ID_3:
                    g_demo_button_state.button_3 = !g_demo_button_state.button_3;
                    break;

                default:
                    // Do nothing
                    break;
                }

                // Publish the button update message
                aws_wifi_publish_shadow_update_message(g_demo_button_state);

                // Reset the selected pushbutton
                g_selected_pushbutton = OLED1_PUSHBUTTON_ID_UNKNOWN;
            }
            else
            {
                // Wait for incoming update messages
                mqtt_status = MQTTYield(&g_mqtt_client, MQTT_YEILD_TIMEOUT_MS);
                if (mqtt_status != SUCCESS)
                {
                    // The AWS IoT Demo failed to retrieve the device serial number
                    aws_iot_set_status(AWS_STATE_AWS_REPORTING, AWS_STATUS_AWS_REPORT_FAILURE,
						"The AWS IoT Demo failed to publish the MQTT LED update message.");

                    console_print_message("\r\n");
                    console_print_error_message("The AWS IoT Demo failed to publish the MQTT LED update message.");
                }

                // If an error occurred in the WIFI connection, make sure to disconnect properly
                if (g_wifi_status == WIFI_STATUS_ERROR)
                {
                    g_is_connected = false;
                }
            }
            break;

        case AWS_STATE_WIFI_DISCONNECT:
            // The AWS Zero Touch Demo is disconnected from access point
            g_is_connected = false;

            // Disconnect from the WINC1500 WIFI
            m2m_wifi_disconnect();

            // Close the socket
            close(g_socket_connection.socket);

            console_print_success_message("AWS Zero Touch Demo: Disconnected from WIFI access point.");

            // Set the state to start the AWS WIFI Configure process
            g_aws_wifi_state = AWS_STATE_WIFI_CONFIGURE;
            break;

        case AWS_STATE_AWS_DISCONNECT:
            // The AWS Zero Touch Demo is disconnected from AWS IoT

            // Disconnect from AWS IoT
            if (g_is_connected == true)
            {
                // Unsubscribe to the AWS IoT update delta topic message
                mqtt_status = MQTTUnsubscribe(&g_mqtt_client, g_mqtt_update_delta_topic_name);
                if (mqtt_status != SUCCESS)
                {
                    // The AWS IoT Demo failed to unsubscribe from the MQTT subscription
                    aws_iot_set_status(AWS_STATE_AWS_DISCONNECT, AWS_STATUS_AWS_SUBSCRIPTION_FAILURE,
                        "The AWS IoT Demo failed to unsubscribe to the MQTT update topic subscription.");

                    console_print_message("\r\n");
                    console_print_error_message("The AWS IoT Demo failed to unsubscribe to the MQTT update topic subscription.");
                }

                // Disconnect from AWS IoT
                mqtt_status = MQTTDisconnect(&g_mqtt_client);
                if (mqtt_status != SUCCESS)
                {
                    // The AWS IoT Demo failed to disconnect from AWS IoT
                    aws_iot_set_status(AWS_STATE_AWS_DISCONNECT, AWS_STATUS_AWS_SUBSCRIPTION_FAILURE,
						"The AWS IoT Demo failed to disconnect with the MQTT disconnect message.");

                    console_print_message("\r\n");
                    console_print_error_message("The AWS IoT Demo failed to disconnect with the MQTT disconnect message.");
                }
            }

            // Set the state to start the AWS WIFI Disconnect process
            g_aws_wifi_state = AWS_STATE_WIFI_DISCONNECT;
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