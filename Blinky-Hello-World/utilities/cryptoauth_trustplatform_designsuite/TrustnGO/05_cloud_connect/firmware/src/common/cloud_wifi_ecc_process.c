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

#include "cloud_wifi_config.h"

#ifndef CLOUD_CONFIG_GCP

#include "cloud_wifi_ecc_process.h"
#include "drv/driver/m2m_ssl.h"
#include <stdio.h>
#include "definitions.h"
#include <stdint.h>
#include "cryptoauthlib.h"
#include "ecc_types.h"
#include "atcacert/atcacert_client.h"
#include "atcacert/atcacert_pem.h"
#include "drv/driver/m2m_periph.h"
#include "drv/driver/m2m_types.h"
#include "drv/driver/m2m_wifi.h"
#include "../drv/common/nm_common.h"
#include "tng/tng_atcacert_client.h"
#include "tng/tng_atca.h"
#include "tng/tngtls_cert_def_1_signer.h"

static uint16_t g_ecdh_key_slot[] = { 2 };
//! Index into the ECDH private key slots array
static uint32_t g_ecdh_key_slot_index = 0;
uint8_t subject_key_id[20];


#define MAX_TNG_CERT_DEVICE_SIZE    546
#define MAX_TLS_CERT_LENGTH         1024
#define SIGNER_CERT_MAX_LEN         (g_tngtls_cert_def_1_signer.cert_template_size + 8) // Need some space in case the cert changes size by a few bytes
#define SIGNER_PUBLIC_KEY_MAX_LEN   64
#define DEVICE_CERT_MAX_LEN         (MAX_TNG_CERT_DEVICE_SIZE + 8)                      // Need some space in case the cert changes size by a few bytes
#define CERT_SN_MAX_LEN             32
#define TLS_SRV_ECDSA_CHAIN_FILE    "ECDSA.lst"
#define INIT_CERT_BUFFER_LEN        (MAX_TLS_CERT_LENGTH*sizeof(uint32_t) - TLS_FILE_NAME_MAX*2 - SIGNER_CERT_MAX_LEN - DEVICE_CERT_MAX_LEN)


static const char* bin2hex(const void* data, size_t data_size)
{
    static char buf[256];
    static char hex[] = "0123456789abcdef";
    const uint8_t* data8 = data;

    if (data_size * 2 > sizeof(buf) - 1)
    {
        return "[buf too small]";
    }

    for (size_t i = 0; i < data_size; i++)
    {
        buf[i * 2 + 0] = hex[(*data8) >> 4];
        buf[i * 2 + 1] = hex[(*data8) & 0xF];
        data8++;
    }
    buf[data_size * 2] = 0;

    return buf;
}

static int8_t ecdh_derive_client_shared_secret(tstrECPoint *server_public_key,
                                               uint8_t *    ecdh_shared_secret,
                                               tstrECPoint *client_public_key)
{
    int8_t status = M2M_ERR_FAIL;
    uint8_t ecdh_mode;
    uint16_t key_id;

    if ((g_ecdh_key_slot_index < 0) ||
        (g_ecdh_key_slot_index >= (sizeof(g_ecdh_key_slot) / sizeof(g_ecdh_key_slot[0]))))
    {
        g_ecdh_key_slot_index = 0;
    }

    if (_gDevice->mIface->mIfaceCFG->devtype == ATECC608A)
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
    if (atcab_genkey(key_id, client_public_key->X) == ATCA_SUCCESS)
    {
        client_public_key->u16Size = 32;
        //do the ecdh from the private key in tempkey, results put in ecdh_shared_secret
        if (atcab_ecdh_base(ecdh_mode, key_id, server_public_key->X, ecdh_shared_secret, NULL) == ATCA_SUCCESS)
        {
            status = M2M_SUCCESS;
        }
    }

    return status;
}

static int8_t ecdh_derive_key_pair(tstrECPoint *server_public_key)
{
    int8_t status = M2M_ERR_FAIL;

    if ((g_ecdh_key_slot_index < 0) ||
        (g_ecdh_key_slot_index >= (sizeof(g_ecdh_key_slot) / sizeof(g_ecdh_key_slot[0]))))
    {
        g_ecdh_key_slot_index = 0;
    }

    if ( (status = atcab_genkey(g_ecdh_key_slot[g_ecdh_key_slot_index], server_public_key->X) ) == ATCA_SUCCESS)
    {
        server_public_key->u16Size      = 32;
        server_public_key->u16PrivKeyID = g_ecdh_key_slot[g_ecdh_key_slot_index];

        g_ecdh_key_slot_index++;

        status = M2M_SUCCESS;
    }

    return status;
}

static int8_t ecdsa_process_sign_verify_request(uint32_t number_of_signatures)
{
    int8_t status = M2M_ERR_FAIL;
    tstrECPoint Key;
    uint32_t index = 0;
    uint8_t signature[80];
    uint8_t hash[80] = { 0 };
    uint16_t curve_type = 0;

    for (index = 0; index < number_of_signatures; index++)
    {
        status = m2m_ssl_retrieve_cert(&curve_type, hash, signature, &Key);

        if (status != M2M_SUCCESS)
        {
            M2M_ERR("m2m_ssl_retrieve_cert() failed with ret=%d", status);
            return status;
        }

        if (curve_type == EC_SECP256R1)
        {
            bool is_verified = false;

            status = atcab_verify_extern(hash, signature, Key.X, &is_verified);
            if (status == ATCA_SUCCESS)
            {
                status = (is_verified == true) ? M2M_SUCCESS : M2M_ERR_FAIL;
                if (is_verified == false)
                {
                    M2M_INFO("ECDSA SigVerif FAILED\n");
                }
            }
            else
            {
                status = M2M_ERR_FAIL;
            }

            if (status != M2M_SUCCESS)
            {
                m2m_ssl_stop_processing_certs();
                break;
            }
        }
    }

    return status;
}

static int8_t ecdsa_process_sign_gen_request(tstrEcdsaSignReqInfo *sign_request,
                                             uint8_t *             signature,
                                             uint16_t *            signature_size)
{
    int8_t status = M2M_ERR_FAIL;
    uint8_t hash[32];
    uint16_t device_key_slot = 0;

    status = m2m_ssl_retrieve_hash(hash, sign_request->u16HashSz);
    if (status != M2M_SUCCESS)
    {
        M2M_ERR("m2m_ssl_retrieve_hash() failed with ret=%d", status);
        return status;
    }

    if (sign_request->u16CurveType == EC_SECP256R1)
    {
        *signature_size = 64;
        status = atcab_sign(device_key_slot, hash, signature);
    }

    return status;
}

static int8_t ecdh_derive_server_shared_secret(uint16_t     private_key_id,
                                               tstrECPoint *client_public_key,
                                               uint8_t *    ecdh_shared_secret)
{
    uint16_t key_slot   = private_key_id;
    int8_t status = M2M_ERR_FAIL;
    uint8_t atca_status = ATCA_STATUS_UNKNOWN;

    atca_status = atcab_ecdh(key_slot, client_public_key->X, ecdh_shared_secret);
    if (atca_status == ATCA_SUCCESS)
    {
        status = M2M_SUCCESS;
    }
    else
    {
        M2M_INFO("__SLOT = %u, Err = %X\n", key_slot, atca_status);
    }

    return status;
}


void ecc_process_request(tstrEccReqInfo *ecc_request)
{
    tstrEccReqInfo ecc_response;
    uint8_t signature[80];
    uint16_t response_data_size = 0;
    uint8_t *response_data_buffer = NULL;

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
    uint8_t *pBuffer = (uint8_t*)header;
    uint16_t count = 0;

    while ((*pBuffer) == 0xFF)
    {

        if (count == INIT_CERT_BUFFER_LEN)
        {
            break;
        }
        count++;
        pBuffer++;
    }

    if (count == INIT_CERT_BUFFER_LEN)
    {
        return sizeof(*header); // Buffer is empty, no files

    }
    return header->u32NextWriteAddr;
}


static int8_t winc_certs_append_file_buf(uint32_t* buffer32, uint32_t buffer_size,
                                         const char* file_name, uint32_t file_size,
                                         const uint8_t* file_data)
{
    tstrTlsSrvSecHdr* header = (tstrTlsSrvSecHdr*)buffer32;
    tstrTlsSrvSecFileEntry* file_entry = NULL;
    uint16_t str_size = (uint8_t)strlen((char*)file_name) + 1;
    uint16_t count = 0;
    uint8_t *pBuffer = (uint8_t*)buffer32;

    while ((*pBuffer) == 0xFF)
    {

        if (count == INIT_CERT_BUFFER_LEN)
        {
            break;
        }
        count++;
        pBuffer++;
    }

    if (count == INIT_CERT_BUFFER_LEN)
    {
        // The WINC will need to add the reference start pattern to the header
        header->u32nEntries = 0;                    // No certs
        // The WINC will need to add the offset of the flash were the certificates are stored to this address
        header->u32NextWriteAddr = sizeof(*header); // Next cert will be written after the header
    }

    if (header->u32nEntries >= sizeof(header->astrEntries) / sizeof(header->astrEntries[0]))
    {
        return M2M_ERR_FAIL; // Already at max number of files

    }
    if ((header->u32NextWriteAddr + file_size) > buffer_size)
    {
        return M2M_ERR_FAIL; // Not enough space in buffer for new file

    }
    file_entry = &header->astrEntries[header->u32nEntries];
    header->u32nEntries++;

    if (str_size > sizeof(file_entry->acFileName))
    {
        return M2M_ERR_FAIL; // File name too long
    }
    memcpy((uint8_t*)file_entry->acFileName, (uint8_t*)file_name, str_size);

    file_entry->u32FileSize = file_size;
    file_entry->u32FileAddr = header->u32NextWriteAddr;
    header->u32NextWriteAddr += file_size;

    // Use memmove to accommodate optimizations where the file data is temporarily stored
    // in buffer32
    memmove(((uint8_t*)buffer32) + (file_entry->u32FileAddr), (uint8_t*)file_data, file_size);

    return M2M_SUCCESS;
}

int8_t ecc_transfer_certificates()
{
    int8_t status = M2M_SUCCESS;
    int atca_status = ATCACERT_E_SUCCESS;
    uint8_t *signer_cert = NULL;
    size_t signer_cert_size;
    uint8_t *device_cert = NULL;
    size_t device_cert_size;
    uint8_t cert_sn[CERT_SN_MAX_LEN];
    size_t cert_sn_size;
    uint8_t *file_list = NULL;
    char *device_cert_filename = NULL;
    char *signer_cert_filename = NULL;
    uint32_t sector_buffer[MAX_TLS_CERT_LENGTH];
    char pem_cert[1024];
    size_t pem_cert_size;
    const atcacert_def_t* cert_def = NULL;

    do
    {
        // Clear cert chain buffer
        memset(sector_buffer, 0xFF, sizeof(sector_buffer));

        // Use the end of the sector buffer to temporarily hold the data to save RAM
        file_list   = ((uint8_t*)sector_buffer) + (sizeof(sector_buffer) - TLS_FILE_NAME_MAX * 2);
        signer_cert = file_list - SIGNER_CERT_MAX_LEN;
        device_cert = signer_cert - DEVICE_CERT_MAX_LEN;

        // Init the file list
        memset(file_list, 0, TLS_FILE_NAME_MAX * 2);
        device_cert_filename = (char*)&file_list[0];
        signer_cert_filename = (char*)&file_list[TLS_FILE_NAME_MAX];

        // Uncompress the signer certificate from the ATECCx08A device
        signer_cert_size = SIGNER_CERT_MAX_LEN;
        atca_status = tng_atcacert_read_signer_cert(signer_cert, &signer_cert_size);
        if (atca_status != ATCACERT_E_SUCCESS)
        {
            // Break the do/while loop
            break;
        }
        pem_cert_size = sizeof(pem_cert);
        atcacert_encode_pem_cert(signer_cert, signer_cert_size, pem_cert, &pem_cert_size);
        APP_DebugPrintf("Signer Cert : \r\n%s\r\n", pem_cert);


        // Uncompress the device certificate from the ATECCx08A device.
        device_cert_size = DEVICE_CERT_MAX_LEN;
        atca_status = tng_atcacert_read_device_cert(device_cert, &device_cert_size, NULL);
        if (atca_status != ATCACERT_E_SUCCESS)
        {
            // Break the do/while loop
            break;
        }
        pem_cert_size = sizeof(pem_cert);
        atcacert_encode_pem_cert(device_cert, device_cert_size, pem_cert, &pem_cert_size);
        APP_DebugPrintf("Device Cert : \r\n%s\r\n", pem_cert);


        // Get the device certificate SN for the filename

        atca_status = tng_get_device_cert_def(&cert_def);
        if (atca_status != ATCA_SUCCESS)
        {
            break;
        }
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
                                            TLS_SRV_ECDSA_CHAIN_FILE, TLS_FILE_NAME_MAX * 2, file_list);
        if (status != M2M_SUCCESS)
        {
            // Break the do/while loop
            break;
        }

        file_list = NULL;
        signer_cert_filename = NULL;
        device_cert_filename = NULL;

        // Update the TLS cert chain on the WINC.
        status = m2m_ssl_send_certs_to_winc((uint8_t*)sector_buffer,
                                            (uint32_t)winc_certs_get_total_files_size((tstrTlsSrvSecHdr*)sector_buffer));
        if (status != M2M_SUCCESS)
        {
            // Break the do/while loop
            break;
        }
    }
    while (false);

    if (atca_status)
    {
        M2M_ERR("eccSendCertsToWINC() failed with ret=%d", atca_status);
        status =  M2M_ERR_FAIL;
    }

    return status;
}

static void winc_wifi_ssl_callback(uint8_t u8MsgType, void *pvMsg)
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

int8_t transfer_ecc_certs_to_winc(void)
{
    int8_t ret_value;

    do
    {
        if ((ret_value = ecc_transfer_certificates()) != M2M_SUCCESS)
        {
            break;
        }

        if ((ret_value = m2m_ssl_init(winc_wifi_ssl_callback)) != M2M_SUCCESS)
        {
            break;
        }
    }
    while (0);

    return ret_value;
}
#endif
