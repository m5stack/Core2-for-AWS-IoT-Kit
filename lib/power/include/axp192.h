/*
 * AXP192 Power Management Unit driver for the Core2 for AWS IoT Kit BSP.
 *
 * Written from scratch based on the AXP192 datasheet.
 * No third-party source code was referenced or copied.
 * Copyright (C) 2022 Rashed Talukder.
 * Copyright (C) 2022 M5Stack. All Rights Reserved.
 *
 * Hardware connections (from schema.yml for Core2 for AWS IoT Kit):
 *   - Internal I²C bus: SDA=GPIO21, SCL=GPIO22; the device is clocked at
 *     400 kHz (datasheet fast mode).
 *   - 7-bit I²C address: 0x34.  (Some datasheet versions list "0x68", which
 *     is the 8-bit *write* address; ESP-IDF uses 7-bit addresses, so 0x34.)
 *   - DCDC1  → ESP32 core power at 3.35 V.
 *   - DCDC3  → LCD backlight, adjustable 2.2–3.3 V.
 *   - LDO2   → LCD logic + SD card at 3.3 V.
 *   - LDO3   → Vibration motor.
 *   - GPIO0 (LDOIO0) → N_VBUSEN, controls the 5 V expansion bus.
 *   - GPIO1  → Green LED (NMOS open-drain, active LOW).
 *   - GPIO2  → Speaker amplifier enable (NMOS open-drain).
 *   - GPIO4  → LCD / touch controller hardware reset (via GPIO43_FUNCTION_CONTROL).
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef _AXP192_H
#define _AXP192_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* ------------------------------------------------------------------ */
/* I2C address                                                        */
/* ------------------------------------------------------------------ */

/*
 * 7-bit I²C address used by ESP-IDF's I2C master driver.
 * The AXP192 datasheet sometimes lists 0x68, which is the 8-bit write
 * address (0x34 << 1).  Always use 0x34 with ESP-IDF.
 */
#define AXP192_ADDRESS                  (0x34)

/* ------------------------------------------------------------------ */
/* Power control registers                                            */
/* ------------------------------------------------------------------ */
#define AXP192_POWER_STATUS             (0x00)
#define AXP192_CHARGE_STATUS            (0x01)
#define AXP192_OTG_VBUS_STATUS          (0x04)
#define AXP192_DATA_BUFFER0             (0x06)
#define AXP192_DATA_BUFFER1             (0x07)
#define AXP192_DATA_BUFFER2             (0x08)
#define AXP192_DATA_BUFFER3             (0x09)
#define AXP192_DATA_BUFFER4             (0x0a)
#define AXP192_DATA_BUFFER5             (0x0b)

/* Output control: bit 2 = EXTEN, bit 0 = DCDC2 */
#define AXP192_EXTEN_DCDC2_CONTROL      (0x10)

/* Power output control: bit 6 = EXTEN, 4 = DCDC2, 3 = LDO3,
 *                       2 = LDO2, 1 = DCDC3, 0 = DCDC1 */
#define AXP192_DCDC13_LDO23_CONTROL     (0x12)

#define AXP192_DCDC2_VOLTAGE            (0x23)
#define AXP192_DCDC2_SLOPE              (0x25)
#define AXP192_DCDC1_VOLTAGE            (0x26)
#define AXP192_DCDC3_VOLTAGE            (0x27)

/* Output voltage: bits [7:4] = LDO2, bits [3:0] = LDO3 */
#define AXP192_LDO23_VOLTAGE            (0x28)

#define AXP192_VBUS_IPSOUT_CHANNEL      (0x30)
#define AXP192_SHUTDOWN_VOLTAGE         (0x31)
#define AXP192_SHUTDOWN_BATTERY_CHGLED_CONTROL (0x32)
#define AXP192_CHARGE_CONTROL_1         (0x33)
#define AXP192_CHARGE_CONTROL_2         (0x34)
#define AXP192_BATTERY_CHARGE_CONTROL   (0x35)
#define AXP192_PEK                      (0x36)
#define AXP192_DCDC_FREQUENCY           (0x37)
#define AXP192_BATTERY_CHARGE_LOW_TEMP  (0x38)
#define AXP192_BATTERY_CHARGE_HIGH_TEMP (0x39)
#define AXP192_APS_LOW_POWER1           (0x3A)
#define AXP192_APS_LOW_POWER2           (0x3B)
#define AXP192_BATTERY_DISCHARGE_LOW_TEMP  (0x3c)
#define AXP192_BATTERY_DISCHARGE_HIGH_TEMP (0x3d)
#define AXP192_DCDC_MODE                (0x80)
#define AXP192_ADC_ENABLE_1             (0x82)
#define AXP192_ADC_ENABLE_2             (0x83)
#define AXP192_ADC_RATE_TS_PIN          (0x84)
#define AXP192_GPIO30_INPUT_RANGE       (0x85)
#define AXP192_GPIO0_ADC_IRQ_RISING     (0x86)
#define AXP192_GPIO0_ADC_IRQ_FALLING    (0x87)
#define AXP192_TIMER_CONTROL            (0x8a)
#define AXP192_VBUS_MONITOR             (0x8b)
#define AXP192_TEMP_SHUTDOWN_CONTROL    (0x8f)

/* ------------------------------------------------------------------ */
/* GPIO control registers                                             */
/* ------------------------------------------------------------------ */
#define AXP192_GPIO0_CONTROL            (0x90)
#define AXP192_GPIO0_LDOIO0_VOLTAGE     (0x91)
#define AXP192_GPIO1_CONTROL            (0x92)
#define AXP192_GPIO2_CONTROL            (0x93)
#define AXP192_GPIO20_SIGNAL_STATUS     (0x94)
#define AXP192_GPIO43_FUNCTION_CONTROL  (0x95)
#define AXP192_GPIO43_SIGNAL_STATUS     (0x96)
#define AXP192_GPIO20_PULLDOWN_CONTROL  (0x97)
#define AXP192_PWM1_FREQUENCY           (0x98)
#define AXP192_PWM1_DUTY_CYCLE_1        (0x99)
#define AXP192_PWM1_DUTY_CYCLE_2        (0x9a)
#define AXP192_PWM2_FREQUENCY           (0x9b)
#define AXP192_PWM2_DUTY_CYCLE_1        (0x9c)
#define AXP192_PWM2_DUTY_CYCLE_2        (0x9d)
#define AXP192_N_RSTO_GPIO5_CONTROL     (0x9e)

/* ------------------------------------------------------------------ */
/* Interrupt control registers                                        */
/* ------------------------------------------------------------------ */
#define AXP192_ENABLE_CONTROL_1         (0x40)
#define AXP192_ENABLE_CONTROL_2         (0x41)
#define AXP192_ENABLE_CONTROL_3         (0x42)
#define AXP192_ENABLE_CONTROL_4         (0x43)
#define AXP192_ENABLE_CONTROL_5         (0x4a)
#define AXP192_IRQ_STATUS_1             (0x44)
#define AXP192_IRQ_STATUS_2             (0x45)
#define AXP192_IRQ_STATUS_3             (0x46)
#define AXP192_IRQ_STATUS_4             (0x47)
#define AXP192_IRQ_STATUS_5             (0x4d)

/* ------------------------------------------------------------------ */
/* ADC data registers                                                 */
/* ------------------------------------------------------------------ */
#define AXP192_ACIN_VOLTAGE             (0x56)
#define AXP192_ACIN_CURRENT             (0x58)
#define AXP192_VBUS_VOLTAGE             (0x5a)
#define AXP192_VBUS_CURRENT             (0x5c)
#define AXP192_TEMP                     (0x5e)
#define AXP192_TS_INPUT                 (0x62)
#define AXP192_GPIO0_VOLTAGE            (0x64)
#define AXP192_GPIO1_VOLTAGE            (0x66)
#define AXP192_GPIO2_VOLTAGE            (0x68)
#define AXP192_GPIO3_VOLTAGE            (0x6a)
#define AXP192_BATTERY_POWER            (0x70)
#define AXP192_BATTERY_VOLTAGE          (0x78)
#define AXP192_CHARGE_CURRENT           (0x7a)
#define AXP192_DISCHARGE_CURRENT        (0x7c)
#define AXP192_APS_VOLTAGE              (0x7e)
#define AXP192_CHARGE_COULOMB           (0xb0)
#define AXP192_DISCHARGE_COULOMB        (0xb4)
#define AXP192_COULOMB_COUNTER_CONTROL  (0xb8)

/* Virtual register used to request the computed coulomb counter */
#define AXP192_COULOMB_COUNTER          (0xff)

/* ------------------------------------------------------------------ */
/* Power status register (REG 0x00) bit masks                         */
/* ------------------------------------------------------------------ */
#define AXP192_STATUS_ACIN_PRESENT      (1u << 7)
#define AXP192_STATUS_ACIN_AVAILABLE    (1u << 6)
#define AXP192_STATUS_VBUS_PRESENT      (1u << 5)
#define AXP192_STATUS_VBUS_AVAILABLE    (1u << 4)
#define AXP192_STATUS_VBUS_ABOVE_VHOLD  (1u << 3)
#define AXP192_STATUS_BAT_DIRECTION     (1u << 2)
#define AXP192_STATUS_ACIN_VBUS_SHORT   (1u << 1)
#define AXP192_STATUS_BOOT_BY_POWER_IN  (1u << 0)

/* ------------------------------------------------------------------ */
/* Charge status register (REG 0x01) bit masks                        */
/* ------------------------------------------------------------------ */
#define AXP192_CHG_STATUS_BAT_PRESENT   (1u << 7)
#define AXP192_CHG_STATUS_CHARGING      (1u << 6)
#define AXP192_CHG_STATUS_FINISHED      (1u << 5)
#define AXP192_CHG_STATUS_TIMEOUT       (1u << 4)
#define AXP192_CHG_STATUS_BAT_ACTIVATE  (1u << 3)
#define AXP192_CHG_STATUS_CUR_LOW       (1u << 2)

/* ------------------------------------------------------------------ */
/* VBUS / IPSOUT control (REG 0x30) bit masks                         */
/* ------------------------------------------------------------------ */
#define AXP192_VBUS_CTL_INPUT_EN        (1u << 7)
#define AXP192_VBUS_CTL_PASS_THROUGH    (1u << 6)
#define AXP192_VBUS_CTL_VHOLD_MASK      (0x7u << 3)
#define AXP192_VBUS_CTL_VHOLD_SHIFT     3
#define AXP192_VBUS_CTL_CUR_LIMIT_EN    (1u << 1)
#define AXP192_VBUS_CTL_CUR_LIMIT_SEL   (1u << 0)

/* ------------------------------------------------------------------ */
/* Shutdown control (REG 0x31) bit masks                              */
/* ------------------------------------------------------------------ */
#define AXP192_VOFF_MASK                (0x7u << 0)
#define AXP192_SLEEP_RESTORE_EN         (1u << 3)

/* ------------------------------------------------------------------ */
/* Power off control (REG 0x32) bit masks                             */
/* ------------------------------------------------------------------ */
#define AXP192_POWER_OFF_REQUEST        (1u << 7)

/* ------------------------------------------------------------------ */
/* Charge control 1 (REG 0x33) bit masks                              */
/* ------------------------------------------------------------------ */
#define AXP192_CHG1_ENABLE              (1u << 7)
#define AXP192_CHG1_TARGET_VOLT_MASK    (0x3u << 5)
#define AXP192_CHG1_TARGET_VOLT_SHIFT   5
#define AXP192_CHG1_END_CUR             (1u << 4)
#define AXP192_CHG1_CURRENT_MASK        (0xFu << 0)
#define AXP192_CHG1_CURRENT_SHIFT       0

/* ------------------------------------------------------------------ */
/* Backup battery charge control (REG 0x35) bit masks                 */
/* ------------------------------------------------------------------ */
#define AXP192_BACKUP_CHG_ENABLE        (1u << 7)
#define AXP192_BACKUP_VOLT_MASK         (0x3u << 5)
#define AXP192_BACKUP_VOLT_SHIFT        5
#define AXP192_BACKUP_CUR_MASK          (0x3u << 0)
#define AXP192_BACKUP_CUR_SHIFT         0

/* ------------------------------------------------------------------ */
/* DCDC mode register (REG 0x80) bit masks                            */
/* ------------------------------------------------------------------ */
#define AXP192_DCDC_MODE_DCDC1          (1u << 3)
#define AXP192_DCDC_MODE_DCDC2          (1u << 2)
#define AXP192_DCDC_MODE_DCDC3          (1u << 1)

/* ------------------------------------------------------------------ */
/* GPIO control mode mask (REG 0x90 / 0x92 / 0x93)                    */
/* ------------------------------------------------------------------ */
#define AXP192_GPIO_MODE_MASK           0x07u

/* ------------------------------------------------------------------ */
/* Charge target voltage selection                                    */
/* ------------------------------------------------------------------ */
typedef enum {
    AXP192_CHG_VOLT_4V10 = 0,
    AXP192_CHG_VOLT_4V15 = 1,
    AXP192_CHG_VOLT_4V20 = 2,
    AXP192_CHG_VOLT_4V36 = 3
} axp192_chg_voltage_t;

/* ------------------------------------------------------------------ */
/* Backup battery voltage selection (REG 0x35 bits [6:5])             */
/* ------------------------------------------------------------------ */
typedef enum {
    AXP192_BACKUP_VOLT_3V1  = 0,
    AXP192_BACKUP_VOLT_3V0  = 1,
    AXP192_BACKUP_VOLT_3V0b = 2, /* REG35H[6:5]=10 is also 3.0V per datasheet */
    AXP192_BACKUP_VOLT_2V5  = 3
} axp192_backup_voltage_t;

/* ------------------------------------------------------------------ */
/* Backup battery current selection (REG 0x35 bits [1:0])             */
/* ------------------------------------------------------------------ */
typedef enum {
    AXP192_BACKUP_CUR_50UA  = 0,
    AXP192_BACKUP_CUR_100UA = 1,
    AXP192_BACKUP_CUR_200UA = 2,
    AXP192_BACKUP_CUR_400UA = 3
} axp192_backup_current_t;

/* ------------------------------------------------------------------ */
/* VBUS current limit selection (REG 0x30 bit [0])                    */
/* ------------------------------------------------------------------ */
typedef enum {
    AXP192_VBUS_CUR_100MA = 0,
    AXP192_VBUS_CUR_500MA = 1
} axp192_vbus_cur_limit_t;

/* ------------------------------------------------------------------ */
/* Error codes                                                        */
/* ------------------------------------------------------------------ */
#define AXP192_OK                       (0)
#define AXP192_ERROR_NOTTY              (-1)
#define AXP192_ERROR_ENOTSUP            (-95)

/* ------------------------------------------------------------------ */
/* Driver handle                                                      */
/* ------------------------------------------------------------------ */

/**
 * @brief AXP192 driver handle.
 *
 * The caller must supply platform-specific I2C read/write callbacks and
 * an opaque handle that is forwarded to those callbacks untouched.
 */
typedef struct {
    int32_t (* read)(void *handle, uint8_t address, uint8_t reg,
                     uint8_t *buffer, uint16_t size);
    int32_t (* write)(void *handle, uint8_t address, uint8_t reg,
                      const uint8_t *buffer, uint16_t size);
    void *handle;
} axp192_t;

typedef int32_t axp192_err_t;

/* ------------------------------------------------------------------ */
/* Public API                                                         */
/* ------------------------------------------------------------------ */

/**
 * @brief Read a register or ADC value from the AXP192.
 *
 * For ADC registers (voltages, currents, temperature, coulomb counter)
 * the raw value is converted to engineering units (volts, amps, °C, …)
 * and stored in @p buffer as a @c float.
 *
 * For all other registers a single raw byte is stored in @p buffer.
 *
 * @param[in]  axp    Driver handle with platform I2C callbacks.
 * @param[in]  reg    Register address (use the AXP192_* defines).
 * @param[out] buffer Destination — float* for ADC regs, uint8_t* otherwise.
 * @return AXP192_OK on success, or a negative error code.
 */
axp192_err_t axp192_read(const axp192_t *axp, uint8_t reg, void *buffer);

/**
 * @brief Write a single byte to an AXP192 register.
 *
 * Writes to read-only ADC registers are rejected with AXP192_ERROR_ENOTSUP.
 *
 * @param[in] axp    Driver handle with platform I2C callbacks.
 * @param[in] reg    Register address (use the AXP192_* defines).
 * @param[in] buffer Pointer to the byte to write.
 * @return AXP192_OK on success, or a negative error code.
 */
axp192_err_t axp192_write(const axp192_t *axp, uint8_t reg,
                           const uint8_t *buffer);

#ifdef __cplusplus
}
#endif
#endif /* _AXP192_H */
