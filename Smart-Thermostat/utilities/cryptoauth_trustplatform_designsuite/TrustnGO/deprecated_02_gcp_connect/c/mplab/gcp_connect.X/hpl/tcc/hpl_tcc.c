/**
 * \file
 *
 * \brief SAM TCC
 *
 * Copyright (c) 2014-2019 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */

#include <compiler.h>
#include <hpl_pwm.h>
#include <hpl_tcc.h>
#include <hpl_tcc_config.h>
#include <hpl_timer.h>
#include <utils.h>
#include <utils_assert.h>

/**
 * \brief TCC configuration type
 */
struct tcc_cfg {
	void *                 hw; /*!< instance of TCC */
	IRQn_Type              irq;
	hri_tcc_ctrla_reg_t    ctrl_a;
	hri_tcc_ctrlbset_reg_t ctrl_b;
	hri_tcc_dbgctrl_reg_t  dbg_ctrl;
	hri_tcc_evctrl_reg_t   event_ctrl;
	hri_tcc_cc_reg_t       cc0;
	hri_tcc_cc_reg_t       cc1;
	hri_tcc_cc_reg_t       cc2;
	hri_tcc_cc_reg_t       cc3;
	hri_tcc_per_reg_t      per;
};

/**
 * \internal Retrieve configuration
 *
 * \param[in] hw The pointer of TCC base address
 *
 * \return The configuration
 */
static struct tcc_cfg *_get_tcc_cfg(void *hw);

/**
 * \brief Array of TCC configurations
 */
static struct tcc_cfg _cfgs[1] = {
    {(void *)TCC0,
     TCC0_IRQn,
     CONF_TCC0_CTRLA,
     CONF_TCC0_CTRLB,
     CONF_TCC0_DBGCTRL,
     CONF_TCC0_EVCTRL,
     CONF_TCC0_CC0,
     CONF_TCC0_CC1,
     CONF_TCC0_CC2,
     CONF_TCC0_CC3,
     CONF_TCC0_PER},
};

static struct _timer_device *_tcc0_dev = NULL;

/**
 * \brief Init irq param with the given tcc hardware instance
 */
static void _tcc_init_irq_param(const void *const hw, void *dev)
{
	if (hw == TCC0) {
		_tcc0_dev = (struct _timer_device *)dev;
	}
}
/**
 * \brief Initialize TCC
 */
int32_t _timer_init(struct _timer_device *const device, void *const hw)
{
	struct tcc_cfg *cfg = _get_tcc_cfg(hw);
	if (cfg == NULL) {
		return ERR_NOT_FOUND;
	}

	device->hw = hw;

	if (!hri_tcc_is_syncing(hw, TCC_SYNCBUSY_SWRST)) {
		if (hri_tcc_get_CTRLA_reg(hw, TCC_CTRLA_ENABLE)) {
			hri_tcc_clear_CTRLA_ENABLE_bit(hw);
			hri_tcc_wait_for_sync(hw, TCC_SYNCBUSY_ENABLE);
		}
		hri_tcc_write_CTRLA_reg(hw, TCC_CTRLA_SWRST);
	}
	hri_tcc_wait_for_sync(hw, TCC_SYNCBUSY_SWRST);

	hri_tcc_write_CTRLA_reg(hw, cfg->ctrl_a);
	hri_tcc_set_CTRLB_reg(hw, cfg->ctrl_b);
	hri_tcc_write_DBGCTRL_reg(hw, cfg->dbg_ctrl);
	hri_tcc_write_EVCTRL_reg(hw, cfg->event_ctrl);
	hri_tcc_write_PER_reg(hw, cfg->per);
	hri_tcc_set_INTEN_OVF_bit(hw);

	_tcc_init_irq_param(hw, (void *)device);
	NVIC_DisableIRQ((IRQn_Type)cfg->irq);
	NVIC_ClearPendingIRQ((IRQn_Type)cfg->irq);
	NVIC_EnableIRQ((IRQn_Type)cfg->irq);

	return ERR_NONE;
}
/**
 * \brief De-initialize TCC
 */
void _timer_deinit(struct _timer_device *const device)
{
	void *const     hw  = device->hw;
	struct tcc_cfg *cfg = _get_tcc_cfg(hw);
	if (cfg != NULL) {
		NVIC_DisableIRQ((IRQn_Type)cfg->irq);
		hri_tcc_clear_CTRLA_ENABLE_bit(hw);
		hri_tcc_set_CTRLA_SWRST_bit(hw);
	}
}
/**
 * \brief Start hardware timer
 */
void _timer_start(struct _timer_device *const device)
{
	hri_tcc_set_CTRLA_ENABLE_bit(device->hw);
}
/**
 * \brief Stop hardware timer
 */
void _timer_stop(struct _timer_device *const device)
{
	hri_tcc_clear_CTRLA_ENABLE_bit(device->hw);
}
/**
 * \brief Set timer period
 */
void _timer_set_period(struct _timer_device *const device, const uint32_t clock_cycles)
{
	hri_tcc_write_PER_reg(device->hw, clock_cycles);
}
/**
 * \brief Retrieve timer period
 */
uint32_t _timer_get_period(const struct _timer_device *const device)
{
	return hri_tcc_read_PER_reg(device->hw);
}
/**
 * \brief Check if timer is running
 */
bool _timer_is_started(const struct _timer_device *const device)
{
	return hri_tcc_get_CTRLA_ENABLE_bit(device->hw);
}

/**
 * \brief Retrieve timer helper functions
 */
struct _timer_hpl_interface *_tcc_get_timer(void)
{
	return NULL;
}

/**
 * \brief Retrieve pwm helper functions
 */
struct _pwm_hpl_interface *_tcc_get_pwm(void)
{
	return NULL;
}
/**
 * \brief Set timer IRQ
 *
 * \param[in] hw The pointer to hardware instance
 */
void _timer_set_irq(struct _timer_device *const device)
{
	void *const     hw  = device->hw;
	struct tcc_cfg *cfg = _get_tcc_cfg(hw);
	if (cfg != NULL) {
		_irq_set((IRQn_Type)cfg->irq);
	}
}
/**
 * \internal TCC interrupt handler
 *
 * \param[in] instance TCC instance number
 */
static void tcc_interrupt_handler(struct _timer_device *device)
{
	void *const hw = device->hw;

	if (hri_tcc_get_interrupt_OVF_bit(hw)) {
		hri_tcc_clear_interrupt_OVF_bit(hw);
		device->timer_cb.period_expired(device);
	}
}

/**
 * \brief TCC interrupt handler
 */
void TCC0_Handler(void)
{
	tcc_interrupt_handler(_tcc0_dev);
}

static struct tcc_cfg *_get_tcc_cfg(void *hw)
{
	uint8_t i;

	for (i = 0; i < ARRAY_SIZE(_cfgs); i++) {
		if (_cfgs[i].hw == hw) {
			return &(_cfgs[i]);
		}
	}
	return NULL;
}
