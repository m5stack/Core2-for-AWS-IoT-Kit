/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file or main.c
 * to avoid loosing it when reconfiguring.
 */

#include "atmel_start.h"
#include "string.h"
#include "winc_init.h"
#include "driver/source/nmasic.h"

extern void set_winc_spi_descriptor(struct spi_m_sync_descriptor *spi_inst);

/*
 * \brief Initialize Wifi library
 */
void wifi_init(tstrWifiInitParam *params)
{
	tstrWifiInitParam param;
	int8_t            ret;

	set_winc_spi_descriptor(&SPI_INSTANCE);
	/* Initialize the BSP. */
	nm_bsp_init();
	/* Initialize Wi-Fi parameters structure. */
	memset((uint8_t *)&param, 0, sizeof(tstrWifiInitParam));
	/* Initialize Wi-Fi driver with data and status callbacks. */
	param.pfAppWifiCb = params->pfAppWifiCb;
	ret               = m2m_wifi_init(&param);
	if (M2M_SUCCESS != ret) {
		while (1) {
		}
	}
}
