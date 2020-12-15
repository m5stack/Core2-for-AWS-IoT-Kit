/*******************************************************************************
  WINC1500 Peripherals Application Interface

  File Name:
    m2m_periph.c

  Summary:
    WINC1500 Peripherals Application Interface

  Description:
    WINC1500 Peripherals Application Interface
 *******************************************************************************/

//DOM-IGNORE-BEGIN
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

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
INCLUDES
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

#include "m2m_periph.h"
#include "nmasic.h"
#include "m2m_hif.h"

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
MACROS
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
#define GPIO_OP_DIR     0
#define GPIO_OP_SET     1
#define GPIO_OP_GET     2
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
DATA TYPES
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
STATIC FUNCTIONS
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
static int8_t get_gpio_idx(uint8_t u8GpioNum)
{
    if(u8GpioNum >= M2M_PERIPH_GPIO_MAX) return -1;
    if(u8GpioNum == M2M_PERIPH_GPIO15) { return 15;
    } else if(u8GpioNum == M2M_PERIPH_GPIO16) { return 16;
    } else if(u8GpioNum == M2M_PERIPH_GPIO18) { return 18;
    } else if(u8GpioNum == M2M_PERIPH_GPIO3) { return 3;
    } else if(u8GpioNum == M2M_PERIPH_GPIO4) { return 4;
    } else if(u8GpioNum == M2M_PERIPH_GPIO5) { return 5;
    } else if(u8GpioNum == M2M_PERIPH_GPIO6) { return 6;
    } else {
        return -2;
    }
}
/*
 * GPIO read/write skeleton with wakeup/sleep capability.
 */
static int8_t gpio_ioctl(uint8_t op, uint8_t u8GpioNum, uint8_t u8InVal, uint8_t * pu8OutVal)
{
    int8_t ret, gpio;

    ret = hif_chip_wake();
    if(ret != M2M_SUCCESS) goto _EXIT;

    gpio = get_gpio_idx(u8GpioNum);
    if(gpio < 0) goto _EXIT1;

    if(op == GPIO_OP_DIR) {
        ret = set_gpio_dir((uint8_t)gpio, u8InVal);
    } else if(op == GPIO_OP_SET) {
        ret = set_gpio_val((uint8_t)gpio, u8InVal);
    } else if(op == GPIO_OP_GET) {
        ret = get_gpio_val((uint8_t)gpio, pu8OutVal);
    }
    if(ret != M2M_SUCCESS) goto _EXIT1;

_EXIT1:
    ret = hif_chip_sleep();
_EXIT:
    return ret;
}
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
FUNCTION IMPLEMENTATION
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/


int8_t m2m_periph_init(tstrPerphInitParam * param)
{
    return M2M_SUCCESS;
}

int8_t m2m_periph_gpio_set_dir(uint8_t u8GpioNum, uint8_t u8GpioDir)
{
    return gpio_ioctl(GPIO_OP_DIR, u8GpioNum, u8GpioDir, NULL);
}

int8_t m2m_periph_gpio_set_val(uint8_t u8GpioNum, uint8_t u8GpioVal)
{
    return gpio_ioctl(GPIO_OP_SET, u8GpioNum, u8GpioVal, NULL);
}

int8_t m2m_periph_gpio_get_val(uint8_t u8GpioNum, uint8_t * pu8GpioVal)
{
    return gpio_ioctl(GPIO_OP_GET, u8GpioNum, 0, pu8GpioVal);
}

int8_t m2m_periph_pullup_ctrl(uint32_t pinmask, uint8_t enable)
{
    return pullup_ctrl(pinmask, enable);
}

//DOM-IGNORE-END
