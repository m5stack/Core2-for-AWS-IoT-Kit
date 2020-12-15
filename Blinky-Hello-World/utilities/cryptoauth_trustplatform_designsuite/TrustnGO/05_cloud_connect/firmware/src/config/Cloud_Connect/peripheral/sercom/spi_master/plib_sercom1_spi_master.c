/*******************************************************************************
  SERIAL COMMUNICATION SERIAL PERIPHERAL INTERFACE(SERCOM1_SPI) PLIB

  Company
    Microchip Technology Inc.

  File Name
    plib_sercom1_spi_master.c

  Summary
    SERCOM1_SPI Master PLIB Implementation File.

  Description
    This file defines the interface to the SERCOM SPI peripheral library.
    This library provides access to and control of the associated
    peripheral instance.

  Remarks:
    None.

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

#include "plib_sercom1_spi_master.h"

// *****************************************************************************
// *****************************************************************************
// Section: MACROS Definitions
// *****************************************************************************
// *****************************************************************************


/* SERCOM1 clk freq value for the baud calculation */
#define SERCOM1_Frequency      (uint32_t) (48000000UL)

/* SERCOM1 SPI baud value for 1000000 Hz baud rate */
#define SERCOM1_SPIM_BAUD_VALUE         (23U)

/*Global object to save SPI Exchange related data  */
SPI_OBJECT sercom1SPIObj;

// *****************************************************************************
// *****************************************************************************
// Section: SERCOM1_SPI Implementation
// *****************************************************************************
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Function:
    void SERCOM1_SPI_Initialize(void);

  Summary:
    Initializes instance SERCOM1 of the SERCOM module operating in SPI mode.

  Description:
    This function initializes instance SERCOM1 of SERCOM module operating in SPI mode.
    This function should be called before any other library function. The SERCOM
    module will be configured as per the MHC settings.

  Remarks:
    Refer plib_sercom1_spi.h file for more information.
*/

void SERCOM1_SPI_Initialize(void)
{
    /* Instantiate the SERCOM1 SPI object */
    sercom1SPIObj.callback = NULL ;
    sercom1SPIObj.transferIsBusy = false ;
	sercom1SPIObj.txSize = 0;
	sercom1SPIObj.rxSize = 0;

    /* Selection of the Character Size and Receiver Enable */
    SERCOM1_REGS->SPIM.SERCOM_CTRLB = SERCOM_SPIM_CTRLB_CHSIZE_8_BIT | SERCOM_SPIM_CTRLB_RXEN_Msk | SERCOM_SPIM_CTRLB_MSSEN_Msk;

    /* Wait for synchronization */
    while(SERCOM1_REGS->SPIM.SERCOM_SYNCBUSY);

    /* Selection of the Baud Value */
    SERCOM1_REGS->SPIM.SERCOM_BAUD = SERCOM_SPIM_BAUD_BAUD(SERCOM1_SPIM_BAUD_VALUE);

    /* Configure Data Out Pin Out , Master Mode,
     * Data In and Pin Out,Data Order and Standby mode if configured
     * and Selection of the Clock Phase and Polarity and Enable the SPI Module
     */
    SERCOM1_REGS->SPIM.SERCOM_CTRLA = SERCOM_SPIM_CTRLA_MODE_SPI_MASTER | SERCOM_SPIM_CTRLA_DOPO_PAD1 | SERCOM_SPIM_CTRLA_DIPO_PAD0 | SERCOM_SPIM_CTRLA_CPOL_IDLE_LOW | SERCOM_SPIM_CTRLA_CPHA_LEADING_EDGE | SERCOM_SPIM_CTRLA_DORD_MSB | SERCOM_SPIM_CTRLA_ENABLE_Msk ;

    /* Wait for synchronization */
    while(SERCOM1_REGS->SPIM.SERCOM_SYNCBUSY);
}

// *****************************************************************************
/* Function:
    bool SERCOM1_SPI_TransferSetup(SPI_TRANSFER_SETUP *setup,
                                                uint32_t spiSourceClock);

 Summary:
    Configure SERCOM SPI operational parameters at run time.

  Description:
    This function allows the application to change the SERCOM SPI operational
    parameter at run time. The application can thus override the MHC defined
    configuration for these parameters. The parameter are specified via the
    SPI_TRANSFER_SETUP type setup parameter. Each member of this parameter
    should be initialized to the desired value.

    The application may feel need to call this function in situation where
    multiple SPI slaves, each with different operation parameters, are connected
    to one SPI master. This function can thus be used to setup the SPI Master to
    meet the communication needs of the slave.

    Calling this function will affect any ongoing communication. The application
    must thus ensure that there is no on-going communication on the SPI before
    calling this function.

  Remarks:
    Refer plib_sercom1_spi.h file for more information.
*/

bool SERCOM1_SPI_TransferSetup(SPI_TRANSFER_SETUP *setup, uint32_t spiSourceClock)
{
    uint32_t baudValue = 0;

    bool statusValue = false;

    if(spiSourceClock == 0)
    {
        /* Fetch Master Clock Frequency directly */
        spiSourceClock = SERCOM1_Frequency;
    }

    /* Disable the SPI Module */
    SERCOM1_REGS->SPIM.SERCOM_CTRLA &= ~(SERCOM_SPIM_CTRLA_ENABLE_Msk);

    /* Wait for synchronization */
    while(SERCOM1_REGS->SPIM.SERCOM_SYNCBUSY);

    if(setup != NULL)
    {
        baudValue = (spiSourceClock/(2*(setup->clockFrequency))) - 1;

        if((baudValue > 0) & (baudValue <= 255))
        {
            /* Selection of the Clock Polarity and Clock Phase */
			SERCOM1_REGS->SPIM.SERCOM_CTRLA &= ~(SERCOM_SPIM_CTRLA_CPOL_Msk | SERCOM_SPIM_CTRLA_CPHA_Msk);
            SERCOM1_REGS->SPIM.SERCOM_CTRLA |= (uint32_t)setup->clockPolarity | (uint32_t)setup->clockPhase;

            /* Selection of the Baud Value */
            SERCOM1_REGS->SPIM.SERCOM_BAUD = baudValue;

            /* Selection of the Character Size */
			SERCOM1_REGS->SPIM.SERCOM_CTRLB &= ~SERCOM_SPIM_CTRLB_CHSIZE_Msk;
            SERCOM1_REGS->SPIM.SERCOM_CTRLB |= setup->dataBits;

            /* Wait for synchronization */
            while(SERCOM1_REGS->SPIM.SERCOM_SYNCBUSY);

            statusValue = true;
        }
    }

    /* Enabling the SPI Module */
    SERCOM1_REGS->SPIM.SERCOM_CTRLA |= SERCOM_SPIM_CTRLA_ENABLE_Msk;

    /* Wait for synchronization */
    while(SERCOM1_REGS->SPIM.SERCOM_SYNCBUSY);

    return statusValue;
}


// *****************************************************************************
/* Function:
    void SERCOM1_SPI_CallbackRegister(const SERCOM_SPI_CALLBACK* callBack,
                                                    uintptr_t context);

  Summary:
    Allows application to register callback with PLIB.

  Description:
    This function allows application to register an event handling function
    for the PLIB to call back when requested data exchange operation has
    completed or any error has occurred.
    The callback should be registered before the client performs exchange
    operation.
    At any point if application wants to stop the callback, it can use this
    function with "callBack" value as NULL.

  Remarks:
    Refer plib_sercom1_spi.h file for more information.
*/

void SERCOM1_SPI_CallbackRegister(SERCOM_SPI_CALLBACK callBack, uintptr_t context )
{
    sercom1SPIObj.callback = callBack;

    sercom1SPIObj.context = context;
}

// *****************************************************************************
/* Function:
    bool SERCOM1_SPI_IsBusy(void);

  Summary:
    Returns transfer status of SERCOM SERCOM1SPI.

  Description:
    This function ture if the SERCOM SERCOM1SPI module is busy with a transfer. The
    application can use the function to check if SERCOM SERCOM1SPI module is busy
    before calling any of the data transfer functions. The library does not
    allow a data transfer operation if another transfer operation is already in
    progress.

    This function can be used as an alternative to the callback function when
    the library is operating interrupt mode. The allow the application to
    implement a synchronous interface to the library.

  Remarks:
    Refer plib_sercom1_spi.h file for more information.
*/

bool SERCOM1_SPI_IsBusy(void)
{
	if ((sercom1SPIObj.txSize == 0) && (sercom1SPIObj.rxSize == 0))
	{
		/* This means no transfer has been requested yet; hence SPI is not busy. */
		return false;
	}
	else
	{
		return ((sercom1SPIObj.transferIsBusy == true) || ((SERCOM1_REGS->SPIM.SERCOM_INTFLAG & SERCOM_SPIM_INTFLAG_TXC_Msk) == 0));
	}
}

// *****************************************************************************
/* Function:
    bool SERCOM1_SPI_WriteRead (void* pTransmitData, size_t txSize
                                        void* pReceiveData, size_t rxSize);

  Summary:
    Write and Read data on SERCOM SERCOM1 SPI peripheral.

  Description:
    This function transmits "txSize" number of bytes and receives "rxSize"
    number of bytes on SERCOM SERCOM1 SPI module. Data pointed by pTransmitData is
    transmitted and received data is saved in the location pointed by
    pReceiveData. The function will transfer the maximum of "txSize" or "rxSize"
    data units towards completion.

    When "Interrupt Mode" option is unchecked in MHC, this function will be
    blocking in nature.  In this mode, the function will not return until all
    the requested data is transferred.  The function returns true after
    transferring all the data.  This indicates that the operation has been
    completed.

    When "Interrupt Mode" option is selected in MHC, the function will be
    non-blocking in nature.  The function returns immediately. The data transfer
    process continues in the peripheral interrupt.  The application specified
    transmit and receive buffer  are ownerd by the library until the data
    transfer is complete and should not be modified by the application till the
    transfer is complete.  Only one transfer is allowed at any time. The
    Application can use a callback function or a polling function to check for
    completion of the transfer. If a callback is required, this should be
    registered prior to calling the SERCOM1_SPI_WriteRead() function. The
    application can use the SERCOM1_SPI_IsBusy() to poll for completion.

  Remarks:
    Refer plib_sercom1_spi.h file for more information.
*/

bool SERCOM1_SPI_WriteRead (void* pTransmitData, size_t txSize, void* pReceiveData, size_t rxSize)
{
    bool isRequestAccepted = false;
    uint32_t dummyData = 0;

    /* Verify the request */
    if((((txSize > 0) && (pTransmitData != NULL)) || ((rxSize > 0) && (pReceiveData != NULL))) && (sercom1SPIObj.transferIsBusy == false))
    {
        if((SERCOM1_REGS->SPIM.SERCOM_CTRLB & SERCOM_SPIM_CTRLB_CHSIZE_Msk) == SPI_DATA_BITS_9)
        {
            /* For 9-bit transmission, the txSize and rxSize must be an even number. */
            if(((txSize > 0) && (txSize & 0x01)) || ((rxSize > 0) && (rxSize & 0x01)))
            {
                return isRequestAccepted;
            }
        }

        isRequestAccepted = true;
        sercom1SPIObj.txBuffer = pTransmitData;
        sercom1SPIObj.rxBuffer = pReceiveData;
        sercom1SPIObj.rxCount = 0;
        sercom1SPIObj.txCount = 0;
        sercom1SPIObj.dummySize = 0;

        if(pTransmitData != NULL)
        {
            sercom1SPIObj.txSize = txSize;
        }
        else
        {
            sercom1SPIObj.txSize = 0;
        }

        if(pReceiveData != NULL)
        {
            sercom1SPIObj.rxSize = rxSize;
        }
        else
        {
            sercom1SPIObj.rxSize = 0;
        }

        sercom1SPIObj.transferIsBusy = true;

        /* Flush out any unread data in SPI read buffer */
        while(SERCOM1_REGS->SPIM.SERCOM_INTFLAG & SERCOM_SPIM_INTFLAG_RXC_Msk)
        {
            dummyData = SERCOM1_REGS->SPIM.SERCOM_DATA;
            (void)dummyData;
        }

        SERCOM1_REGS->SPIM.SERCOM_STATUS |= SERCOM_SPIM_STATUS_BUFOVF_Msk;

        SERCOM1_REGS->SPIM.SERCOM_INTFLAG |= SERCOM_SPIM_INTFLAG_ERROR_Msk;

        if(sercom1SPIObj.rxSize > sercom1SPIObj.txSize)
        {
            sercom1SPIObj.dummySize = sercom1SPIObj.rxSize - sercom1SPIObj.txSize;
        }

        /* Start the first write here itself, rest will happen in ISR context */
        if((SERCOM1_REGS->SPIM.SERCOM_CTRLB & SERCOM_SPIM_CTRLB_CHSIZE_Msk) == SPI_DATA_BITS_8)
        {
            if(sercom1SPIObj.txCount < sercom1SPIObj.txSize)
            {
                SERCOM1_REGS->SPIM.SERCOM_DATA = *((uint8_t*)sercom1SPIObj.txBuffer);

                sercom1SPIObj.txCount++;
            }
            else if(sercom1SPIObj.dummySize > 0)
            {
                SERCOM1_REGS->SPIM.SERCOM_DATA = 0xFF;

                sercom1SPIObj.dummySize--;
            }
        }
        else
        {
            sercom1SPIObj.txSize >>= 1;
            sercom1SPIObj.dummySize >>= 1;
            sercom1SPIObj.rxSize >>= 1;

            if(sercom1SPIObj.txCount < sercom1SPIObj.txSize)
            {
                SERCOM1_REGS->SPIM.SERCOM_DATA = *((uint16_t*)sercom1SPIObj.txBuffer) & SERCOM_SPIM_DATA_Msk;

                sercom1SPIObj.txCount++;
            }
            else if(sercom1SPIObj.dummySize > 0)
            {
                SERCOM1_REGS->SPIM.SERCOM_DATA = 0xFFFF & SERCOM_SPIM_DATA_Msk;

                sercom1SPIObj.dummySize--;
            }
        }

        if(rxSize > 0)
        {
            /* Enable ReceiveComplete  */
            SERCOM1_REGS->SPIM.SERCOM_INTENSET = SERCOM_SPIM_INTENSET_RXC_Msk;
        }
        else
        {
            /* Enable the DataRegisterEmpty  */
            SERCOM1_REGS->SPIM.SERCOM_INTENSET = SERCOM_SPIM_INTENSET_DRE_Msk;
        }
    }

    return isRequestAccepted;
}

bool SERCOM1_SPI_Write(void* pTransmitData, size_t txSize)
{
    return SERCOM1_SPI_WriteRead(pTransmitData, txSize, NULL, 0);
}

bool SERCOM1_SPI_Read(void* pReceiveData, size_t rxSize)
{
    return SERCOM1_SPI_WriteRead(NULL, 0, pReceiveData, rxSize);
}

// *****************************************************************************
/* Function:
    void SERCOM1_SPI_InterruptHandler(void);

  Summary:
    Handler that handles the SPI interrupts

  Description:
    This Function is called from the handler to handle the exchange based on the
    Interrupts.

  Remarks:
    Refer plib_sercom1_spi.h file for more information.
*/

void SERCOM1_SPI_InterruptHandler(void)
{
    uint32_t dataBits = 0;
    uint32_t receivedData = 0;
    static bool isLastByteTransferInProgress = false;

    if(SERCOM1_REGS->SPIM.SERCOM_INTENSET != 0)
    {
        dataBits = SERCOM1_REGS->SPIM.SERCOM_CTRLB & SERCOM_SPIM_CTRLB_CHSIZE_Msk;

        if((SERCOM1_REGS->SPIM.SERCOM_INTFLAG & SERCOM_SPIM_INTFLAG_RXC_Msk) == SERCOM_SPIM_INTFLAG_RXC_Msk)
        {
            receivedData =  SERCOM1_REGS->SPIM.SERCOM_DATA;

            if(sercom1SPIObj.rxCount < sercom1SPIObj.rxSize)
            {
                if(dataBits == SPI_DATA_BITS_8)
                {
                    ((uint8_t*)sercom1SPIObj.rxBuffer)[sercom1SPIObj.rxCount++] = receivedData;
                }
                else
                {
                    ((uint16_t*)sercom1SPIObj.rxBuffer)[sercom1SPIObj.rxCount++] = receivedData;
                }
            }
        }

        /* If there are more words to be transmitted, then transmit them here and keep track of the count */
        if((SERCOM1_REGS->SPIM.SERCOM_INTFLAG & SERCOM_SPIM_INTFLAG_DRE_Msk) == SERCOM_SPIM_INTFLAG_DRE_Msk)
        {
            /* Disable the DRE interrupt. This will be enabled back if more than
             * one byte is pending to be transmitted */
            SERCOM1_REGS->SPIM.SERCOM_INTENCLR = SERCOM_SPIM_INTENCLR_DRE_Msk;

            if(dataBits == SPI_DATA_BITS_8)
            {
                if(sercom1SPIObj.txCount < sercom1SPIObj.txSize)
                {
                    SERCOM1_REGS->SPIM.SERCOM_DATA = ((uint8_t*)sercom1SPIObj.txBuffer)[sercom1SPIObj.txCount++];
                }
                else if(sercom1SPIObj.dummySize > 0)
                {
                    SERCOM1_REGS->SPIM.SERCOM_DATA = 0xFF;

                    sercom1SPIObj.dummySize--;
                }
            }
            else
            {
                if(sercom1SPIObj.txCount < sercom1SPIObj.txSize)
                {
                    SERCOM1_REGS->SPIM.SERCOM_DATA = ((uint16_t*)sercom1SPIObj.txBuffer)[sercom1SPIObj.txCount++];
                }
                else if(sercom1SPIObj.dummySize > 0)
                {
                    SERCOM1_REGS->SPIM.SERCOM_DATA = 0xFFFF;

                    sercom1SPIObj.dummySize--;
                }
            }

            if((sercom1SPIObj.txCount == sercom1SPIObj.txSize) && (sercom1SPIObj.dummySize == 0))
            {
                 /* At higher baud rates, the data in the shift register can be
                 * shifted out and TXC flag can get set resulting in a
                 * callback been given to the application with the SPI interrupt
                 * pending with the application. This will then result in the
                 * interrupt handler being called again with nothing to transmit.
                 * To avoid this, a software flag is set, but
                 * the TXC interrupt is not enabled until the very end.
                 */

                isLastByteTransferInProgress = true;
            }
            else if(sercom1SPIObj.rxCount == sercom1SPIObj.rxSize)
            {
                SERCOM1_REGS->SPIM.SERCOM_INTENSET = SERCOM_SPIM_INTENSET_DRE_Msk;

                SERCOM1_REGS->SPIM.SERCOM_INTENCLR = SERCOM_SPIM_INTENCLR_RXC_Msk;
            }
        }

        if(((SERCOM1_REGS->SPIM.SERCOM_INTFLAG & SERCOM_SPIM_INTFLAG_TXC_Msk) == SERCOM_SPIM_INTFLAG_TXC_Msk) && (isLastByteTransferInProgress == true))
        {
            if(sercom1SPIObj.rxCount == sercom1SPIObj.rxSize)
            {
                sercom1SPIObj.transferIsBusy = false;

                /* Disable the Data Register empty and Receive Complete Interrupt flags */
                SERCOM1_REGS->SPIM.SERCOM_INTENCLR = SERCOM_SPIM_INTENCLR_DRE_Msk | SERCOM_SPIM_INTENCLR_RXC_Msk | SERCOM_SPIM_INTENSET_TXC_Msk;

                isLastByteTransferInProgress = false;

                if(sercom1SPIObj.callback != NULL)
                {
                    sercom1SPIObj.callback(sercom1SPIObj.context);
                }
            }
        }

        if(isLastByteTransferInProgress == true)
        {
            /* For the last byte transfer, the DRE interrupt is already disabled.
             * Enable TXC interrupt to ensure no data is present in the shift
             * register before application callback is called.
             */
            SERCOM1_REGS->SPIM.SERCOM_INTENSET = SERCOM_SPIM_INTENSET_TXC_Msk;
        }
    }
}
