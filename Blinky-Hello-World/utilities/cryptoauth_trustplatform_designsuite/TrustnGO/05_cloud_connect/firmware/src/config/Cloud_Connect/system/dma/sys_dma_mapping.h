/*******************************************************************************
  DMA System Service Mapping File

  Company:
    Microchip Technology Inc.

  File Name:
    sys_dma_mapping.h

  Summary:
    DMA System Service mapping file.

  Description:
    This header file contains the mapping of the APIs defined in the API header
    to either the function implementations or macro implementation or the
    specific variant implementation.
*******************************************************************************/

//DOM-IGNORE-BEGIN
/******************************************************************************
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
//DOM-IGNORE-END

#ifndef SYS_DMA_MAPPING_H
#define SYS_DMA_MAPPING_H


// *****************************************************************************
// *****************************************************************************
// Section: DMA System Service Mapping
// *****************************************************************************
// *****************************************************************************

#include "peripheral/dmac/plib_dmac.h"

#define SYS_DMA_ChannelCallbackRegister(channel, eventHandler, context)  DMAC_ChannelCallbackRegister((DMAC_CHANNEL)channel, (DMAC_CHANNEL_CALLBACK)eventHandler, context)

#define SYS_DMA_ChannelTransfer(channel, srcAddr, destAddr, blockSize)  DMAC_ChannelTransfer((DMAC_CHANNEL)channel, srcAddr, destAddr, blockSize)

#define SYS_DMA_ChannelIsBusy(channel)  DMAC_ChannelIsBusy((DMAC_CHANNEL)channel)

#define SYS_DMA_ChannelDisable(channel)  DMAC_ChannelDisable((DMAC_CHANNEL)channel)

#endif // SYS_DMA_MAPPING_H
