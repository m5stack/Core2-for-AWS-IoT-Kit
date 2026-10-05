/*
 * AXP192 Power Management Unit driver for the Core2 for AWS IoT Kit BSP.
 *
 * Written from scratch based on the AXP192 datasheet.
 * No third-party source code was referenced or copied.
 * Copyright (C) 2022 Rashed Talukder.
 * Copyright (C) 2022 M5Stack. All Rights Reserved.
 *
 * ADC sensitivity values used in this file (from AXP192 datasheet §6):
 *   ACIN / VBUS voltage : 1.7  mV / LSB   (12-bit, registers 0x56/0x5A)
 *   ACIN current        : 0.625 mA / LSB  (12-bit, register  0x58)
 *   VBUS current        : 0.375 mA / LSB  (12-bit, register  0x5C)
 *   Internal temp       : 0.1  °C / LSB,  offset –144.7 °C (12-bit, 0x5E)
 *   TS input            : 0.8  mV / LSB   (12-bit, register  0x62)
 *   GPIO 0-3 voltage    : 0.5  mV / LSB   (12-bit, registers 0x64–0x6A)
 *   Battery voltage     : 1.1  mV / LSB   (12-bit, registers 0x78–0x79)
 *   Charge current      : 0.5  mA / LSB   (13-bit, registers 0x7A–0x7B)
 *   Discharge current   : 0.5  mA / LSB   (13-bit, registers 0x7C–0x7D)
 *   APS voltage         : 1.4  mV / LSB   (12-bit, registers 0x7E–0x7F)
 *   Battery power       : 1.1 mV × 0.5 mA / LSB (24-bit, registers 0x70–0x72)
 *
 * 12-bit ADC format : value = (HIGH_BYTE << 4) | (LOW_BYTE & 0x0F)
 * 13-bit ADC format : value = (HIGH_BYTE << 5) | (LOW_BYTE & 0x1F)
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdint.h>

#include "axp192.h"

/* ------------------------------------------------------------------ */
/* Static helpers for multi-byte ADC reads                            */
/* ------------------------------------------------------------------ */

/**
 * @brief Read the coulomb counter registers and compute mAh.
 *
 * The AXP192 maintains separate 32-bit charge and discharge coulomb
 * accumulators.  The net capacity is:
 *     mAh = 65536 * 0.5 mA * (charge - discharge) / 3600 / sample_rate
 * With the default 25 Hz ADC rate this simplifies to:
 *     mAh = 32768 * (charge - discharge) / 3600 / 25
 */
static axp192_err_t read_coulomb_counter(const axp192_t *axp, float *buffer)
{
    uint8_t tmp[4];
    int32_t charge_counts, discharge_counts;
    axp192_err_t status;

    status = axp->read(axp->handle, AXP192_ADDRESS,
                       AXP192_CHARGE_COULOMB, tmp, sizeof(charge_counts));
    if (AXP192_OK != status) {
        return status;
    }
    charge_counts = (int32_t)(((uint32_t)tmp[0] << 24) |
                              ((uint32_t)tmp[1] << 16) |
                              ((uint32_t)tmp[2] << 8)  | tmp[3]);

    status = axp->read(axp->handle, AXP192_ADDRESS,
                       AXP192_DISCHARGE_COULOMB, tmp, sizeof(discharge_counts));
    if (AXP192_OK != status) {
        return status;
    }
    discharge_counts = (int32_t)(((uint32_t)tmp[0] << 24) |
                                 ((uint32_t)tmp[1] << 16) |
                                 ((uint32_t)tmp[2] << 8)  | tmp[3]);

    *buffer = 32768.0f * (charge_counts - discharge_counts) / 3600.0f / 25.0f;
    return AXP192_OK;
}

/**
 * @brief Read the 24-bit battery power register.
 *
 * Sensitivity: 1.1 mV * 0.5 mA per LSB.
 */
static axp192_err_t read_battery_power(const axp192_t *axp, float *buffer)
{
    uint8_t tmp[3];
    const float sensitivity = 1.1f * 0.5f / 1000.0f;

    axp192_err_t status = axp->read(axp->handle, AXP192_ADDRESS,
                                    AXP192_BATTERY_POWER, tmp, 3);
    if (AXP192_OK != status) {
        return status;
    }

    uint32_t raw = ((uint32_t)tmp[0] << 16) |
                   ((uint32_t)tmp[1] << 8)  | tmp[2];
    *buffer = raw * sensitivity;
    return AXP192_OK;
}

/**
 * @brief Read a two-byte ADC register and convert to engineering units.
 *
 * Most AXP192 ADC registers are 12-bit (high byte << 4 | low nibble).
 * Battery current registers are 13-bit (high byte << 5 | low 5 bits).
 *
 * Each register type has its own sensitivity (mV/LSB, mA/LSB, etc.)
 * documented in the AXP192 datasheet.
 */
static axp192_err_t axp192_read_adc(const axp192_t *axp, uint8_t reg,
                                    float *buffer)
{
    uint8_t tmp[2];
    float sensitivity = 1.0f;
    float offset = 0.0f;
    axp192_err_t status;

    switch (reg) {
    case AXP192_ACIN_VOLTAGE:
    case AXP192_VBUS_VOLTAGE:
        sensitivity = 1.7f / 1000.0f;       /* 1.7 mV / LSB */
        break;
    case AXP192_ACIN_CURRENT:
        sensitivity = 0.625f / 1000.0f;     /* 0.625 mA / LSB */
        break;
    case AXP192_VBUS_CURRENT:
        sensitivity = 0.375f / 1000.0f;     /* 0.375 mA / LSB */
        break;
    case AXP192_TEMP:
        sensitivity = 0.1f;                 /* 0.1 °C / LSB */
        offset = -144.7f;                   /* 0x000 = −144.7 °C */
        break;
    case AXP192_TS_INPUT:
        sensitivity = 0.8f / 1000.0f;       /* 0.8 mV / LSB */
        break;
    case AXP192_BATTERY_POWER:
        return read_battery_power(axp, buffer);
    case AXP192_BATTERY_VOLTAGE:
        sensitivity = 1.1f / 1000.0f;       /* 1.1 mV / LSB */
        break;
    case AXP192_CHARGE_CURRENT:
    case AXP192_DISCHARGE_CURRENT:
        sensitivity = 0.5f / 1000.0f;       /* 0.5 mA / LSB */
        break;
    case AXP192_GPIO0_VOLTAGE:
    case AXP192_GPIO1_VOLTAGE:
    case AXP192_GPIO2_VOLTAGE:
    case AXP192_GPIO3_VOLTAGE:
        sensitivity = 0.5f / 1000.0f;       /* 0.5 mV / LSB */
        break;
    case AXP192_APS_VOLTAGE:
        sensitivity = 1.4f / 1000.0f;       /* 1.4 mV / LSB */
        break;
    case AXP192_COULOMB_COUNTER:
        return read_coulomb_counter(axp, buffer);
    }

    status = axp->read(axp->handle, AXP192_ADDRESS, reg, tmp, 2);
    if (AXP192_OK != status) {
        return status;
    }

    uint16_t raw;
    if (reg == AXP192_CHARGE_CURRENT || reg == AXP192_DISCHARGE_CURRENT) {
        /* 13-bit format for battery current registers */
        raw = ((uint16_t)tmp[0] << 5) | (tmp[1] & 0x1f);
    } else {
        /* 12-bit format for voltage and other ADC registers */
        raw = ((uint16_t)tmp[0] << 4) | (tmp[1] & 0x0f);
    }

    *buffer = (raw * sensitivity) + offset;
    return AXP192_OK;
}

/* ------------------------------------------------------------------ */
/* Public API                                                         */
/* ------------------------------------------------------------------ */

axp192_err_t axp192_read(const axp192_t *axp, uint8_t reg, void *buffer)
{
    switch (reg) {
    /* ADC registers — convert to float with engineering units. */
    case AXP192_ACIN_VOLTAGE:
    case AXP192_VBUS_VOLTAGE:
    case AXP192_ACIN_CURRENT:
    case AXP192_VBUS_CURRENT:
    case AXP192_TEMP:
    case AXP192_TS_INPUT:
    case AXP192_BATTERY_POWER:
    case AXP192_BATTERY_VOLTAGE:
    case AXP192_CHARGE_CURRENT:
    case AXP192_DISCHARGE_CURRENT:
    case AXP192_APS_VOLTAGE:
    case AXP192_GPIO0_VOLTAGE:
    case AXP192_GPIO1_VOLTAGE:
    case AXP192_GPIO2_VOLTAGE:
    case AXP192_GPIO3_VOLTAGE:
    case AXP192_COULOMB_COUNTER:
        return axp192_read_adc(axp, reg, buffer);

    default:
        /* All other registers — return raw byte. */
        return axp->read(axp->handle, AXP192_ADDRESS, reg, buffer, 1);
    }
}

axp192_err_t axp192_write(const axp192_t *axp, uint8_t reg,
                           const uint8_t *buffer)
{
    switch (reg) {
    /* ADC registers are read-only. */
    case AXP192_ACIN_VOLTAGE:
    case AXP192_VBUS_VOLTAGE:
    case AXP192_ACIN_CURRENT:
    case AXP192_VBUS_CURRENT:
    case AXP192_TEMP:
    case AXP192_TS_INPUT:
    case AXP192_BATTERY_POWER:
    case AXP192_BATTERY_VOLTAGE:
    case AXP192_CHARGE_CURRENT:
    case AXP192_DISCHARGE_CURRENT:
    case AXP192_APS_VOLTAGE:
    case AXP192_GPIO0_VOLTAGE:
    case AXP192_GPIO1_VOLTAGE:
    case AXP192_GPIO2_VOLTAGE:
    case AXP192_GPIO3_VOLTAGE:
    case AXP192_COULOMB_COUNTER:
        return AXP192_ERROR_ENOTSUP;

    default:
        return axp->write(axp->handle, AXP192_ADDRESS, reg, buffer, 1);
    }
}
