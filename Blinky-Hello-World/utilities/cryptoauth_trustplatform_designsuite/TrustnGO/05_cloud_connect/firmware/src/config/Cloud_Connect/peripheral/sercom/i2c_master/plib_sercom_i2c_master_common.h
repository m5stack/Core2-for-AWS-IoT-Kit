/*******************************************************************************
  Serial Communication Interface Inter-Integrated Circuit (SERCOM I2C) Library
  Instance Header File

  Company
    Microchip Technology Inc.

  File Name
    plib_sercom_i2c_master.h

  Summary
    SERCOM I2C peripheral library interface.

  Description
    This file defines the interface to the SERCOM I2C peripheral library. This
    library provides access to and control of the associated peripheral
    instance.

  Remarks:

*******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries.
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

#ifndef PLIB_SERCOM_I2C_MASTER_H
#define PLIB_SERCOM_I2C_MASTER_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "device.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* SERCOM I2C Transfer type

  Summary:
    List of transfer direction.

  Description:
    This enum defines the I2C transfer direction.

  Remarks:
    None.
*/

enum
{
    I2C_TRANSFER_WRITE = 0,
    I2C_TRANSFER_READ  = 1,
};

// *****************************************************************************
/* SERCOM I2C Error.

  Summary:
    Defines the possible errors that the SERCOM I2C peripheral can generate.

  Description:
    This enum defines the possible error the SERCOM I2C peripheral can generate.
    An error of this type is returned by the SERCOMx_I2C_ErrorGet() function.

  Remarks:
    None.
*/

typedef enum
{
    /* No error has occurred. */
    SERCOM_I2C_ERROR_NONE,

    /* A bus transaction was NAK'ed */
    SERCOM_I2C_ERROR_NAK,

    /* A bus error has occurred. */
    SERCOM_I2C_ERROR_BUS,

} SERCOM_I2C_ERROR;

// *****************************************************************************
/* SERCOM I2C State.

   Summary:
    SERCOM I2C PLib Task State.

   Description:
    This data type defines the SERCOM I2C PLib Task State.

   Remarks:
    None.
*/

typedef enum
{
    /* SERCOM PLib Task Error State */
    SERCOM_I2C_STATE_ERROR = -1,

    /* SERCOM PLib Task Idle State */
    SERCOM_I2C_STATE_IDLE,

    /* SERCOM PLib Task Address Send State */
    SERCOM_I2C_STATE_ADDR_SEND,

    SERCOM_I2C_REINITIATE_TRANSFER,
    /* SERCOM PLib Task Read Transfer State */
    SERCOM_I2C_STATE_TRANSFER_READ,

    /* SERCOM PLib Task Write Transfer State */
    SERCOM_I2C_STATE_TRANSFER_WRITE,

    /* SERCOM PLib Task High Speed Slave Address Send State */
    SERCOM_I2C_STATE_TRANSFER_ADDR_HS,

    /* SERCOM PLib Task Transfer Done State */
    SERCOM_I2C_STATE_TRANSFER_DONE,

} SERCOM_I2C_STATE;

// *****************************************************************************
/* SERCOM I2C Callback

   Summary:
    SERCOM I2C Callback Function Pointer.

   Description:
    This data type defines the SERCOM I2C Callback Function Pointer.

   Remarks:
    None.
*/

typedef void (*SERCOM_I2C_CALLBACK)
(
    /*Transfer context*/
    uintptr_t contextHandle

);

// *****************************************************************************
/* SERCOM I2C PLib Instance Object

   Summary:
    SERCOM I2C PLib Object structure.

   Description:
    This data structure defines the SERCOM I2C PLib Instance Object.

   Remarks:
    None.
*/

typedef struct
{
    bool                        isHighSpeed;

    bool                        txMasterCode;

    bool                        transferDir;

    uint16_t                    address;

    uint8_t                     masterCode;

    uint8_t*                    writeBuffer;

    uint8_t*                    readBuffer;

    size_t                      writeSize;

    size_t                      readSize;

    size_t                      writeCount;

    size_t                      readCount;

    /* State */
    volatile SERCOM_I2C_STATE   state;

    /* Transfer status */
    volatile SERCOM_I2C_ERROR   error;

    /* Transfer Event Callback */
    SERCOM_I2C_CALLBACK         callback;

    /* Transfer context */
    uintptr_t context;

} SERCOM_I2C_OBJ;

// *****************************************************************************
/* Transaction Request Block

   Summary:
    Transaction Request Block Structure.

   Description:
    This data structure defines the Transaction Request Block.

   Remarks:
    None.
*/

typedef struct
{
    /* SERCOM I2C Clock Speed */
    uint32_t clkSpeed;

} SERCOM_I2C_TRANSFER_SETUP;

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif
// DOM-IGNORE-END

#endif /* PLIB_SERCOM_I2C_MASTER_H */