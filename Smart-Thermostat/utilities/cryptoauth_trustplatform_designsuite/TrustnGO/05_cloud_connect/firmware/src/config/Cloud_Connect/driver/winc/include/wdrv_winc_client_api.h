/*******************************************************************************
  WINC Driver Client API Header File

  Company:
    Microchip Technology Inc.

  File Name:
    wdrv_winc_client_api.h

  Summary:
    WINC wireless driver client API header file.

  Description:
    This file pulls together the elements which make up the client API
      assoc       - Current association.
      bssfind     - BSS scan functionality.
      custie      - Custom IE management for Soft-AP.
      nvm         - Off line access the WINC SPI flash.
      powersave   - Power save control.
      socket      - TCP/IP sockets.
      softap      - Soft-AP mode.
      sta         - Infrastructure stations mode.
      systtime    - System time.
      ssl         - SSL configuration and management.
      ble         - BLE control (WINC3400)

 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2019 Microchip Technology Inc. and its subsidiaries.
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
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/
// DOM-IGNORE-END

#ifndef _WDRV_WINC_CLIENT_API_H
#define _WDRV_WINC_CLIENT_API_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include "configuration.h"

#include "wdrv_winc.h"
#include "wdrv_winc_bssfind.h"
#include "wdrv_winc_powersave.h"
#include "wdrv_winc_assoc.h"
#include "wdrv_winc_systime.h"
#include "wdrv_winc_softap.h"
#include "wdrv_winc_sta.h"
#include "wdrv_winc_common.h"
#include "wdrv_winc_custie.h"
#ifdef WDRV_WINC_NETWORK_MODE_SOCKET
#include "wdrv_winc_socket.h"
#include "wdrv_winc_ssl.h"
#endif
#ifdef WDRV_WINC_DEVICE_HOST_FILE_DOWNLOAD
#include "wdrv_winc_host_file.h"
#endif
#ifdef WDRV_WINC_ENABLE_BLE
#include "wdrv_winc_ble.h"
#endif
#include "wdrv_winc_nvm.h"

#endif /* _WDRV_WINC_CLIENT_API_H */
