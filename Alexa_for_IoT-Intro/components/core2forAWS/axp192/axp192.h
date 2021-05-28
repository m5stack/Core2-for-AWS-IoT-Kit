/**
 * @file axp192.h
 * @brief Functions for the AXP192 Power Management Unit (PMU)
 */

#pragma once
#include "stdint.h"

#define AXP192_DC_VOLT_STEP  25
#define AXP192_DC_VOLT_MIN   700
#define AXP192_DC_VOLT_MAX   3500

#define AXP192_LDO_VOLT_STEP 100
#define AXP192_LDO_VOLT_MIN  1800
#define AXP192_LDO_VOLT_MAX  3300

#define AXP192_VOFF_VOLT_STEP 100
#define AXP192_VOFF_VOLT_MIN  2600
#define AXP192_VOFF_VOLT_MAX  3300

#define AXP192_LDO23_DC123_EXT_CTL_REG 0x12
#define AXP192_DC1_EN_BIT   (0)
#define AXP192_DC3_EN_BIT   (1)
#define AXP192_LDO2_EN_BIT  (2)
#define AXP192_LDO3_EN_BIT  (3)
#define AXP192_DC2_EN_BIT   (4)
#define AXP192_EXT_EN_BIT   (6)

#define AXP192_DC1_VOLT_REG         0x26
#define AXP192_DC2_VOLT_REG         0x23
#define AXP192_DC3_VOLT_REG         0x27
#define AXP192_LDO23_VOLT_REG       0x28
#define AXP192_VBUS_IPSOUT_CTL_REG  0x30 // not support yet
#define AXP192_VOFF_VOLT_REG        0x31 // PWRON short press in here
#define AXP192_POWEROFF_REG         0x32 // CHGLED in here
#define AXP192_CHG_CTL1_REG         0x33 
#define AXP192_CHG_CTL2_REG         0x34
#define AXP192_SPARE_CHG_CTL_REG    0x35
#define AXP192_PEK_CTL_REG          0x36
#define AXP192_CHG_BOOL_REG         0x01

#define AXP192_ADC1_ENABLE_REG      0x82
#define BAT_VOLT_BIT        (7)
#define BAT_CURRENT_BIT     (6)
#define ACIN_VOLT_BIT       (5)
#define ACIN_CURRENT_BIT    (4)
#define VBUS_VOLT_BIT       (3)
#define VBUS_CURRENT_BIT    (2)
#define APS_VOLT_BIT        (1)
#define TS_BIT              (0)

#define AXP192_ACIN_ADC_VOLTAGE_REG         0x56
#define AXP192_ACIN_ADC_CURRENT_REG         0x58

#define AXP192_VBUS_ADC_VOLTAGE_REG         0x5A
#define AXP192_VBUS_ADC_CURRENT_REG         0x5C

#define AXP192_BAT_ADC_VOLTAGE_REG          0x78
#define AXP192_BAT_ADC_CURRENT_IN_REG       0x7A
#define AXP192_BAT_ADC_CURRENT_OUT_REG      0x7C

#define AXP192_GPIO0_CTL_REG                0x90                   
#define AXP192_GPIO0_VOLT_REG               0x91                   
#define AXP192_GPIO1_CTL_REG                0x92                   
#define AXP192_GPIO2_CTL_REG                0x93
#define AXP192_GPIO34_CTL_REG               0x95

#define AXP192_GPIO34_STATE_REG             0x96
#define AXP192_GPIO012_STATE_REG            0x94

/**
 * @brief List of available charging voltages.
 */
/* @[declare_axp192_chargevolt] */
typedef enum {
    CHARGE_VOLT_4100mV = 0b0000, /**< @brief Charge at 4.10v. */
    CHARGE_VOLT_4150mV = 0b0001, /**< @brief Charge at 4.15v. */
    CHARGE_VOLT_4200mV = 0b0010, /**< @brief Charge at 4.20v. */   
    CHARGE_VOLT_4360mV = 0b0011, /**< @brief Charge at 4.36v. */
} Axp192_ChargeVolt_t;
/* @[declare_axp192_chargevolt] */

/**
 * @brief List of available charging current rates.
 */
/* @[declare_axp192_chargecurrent] */
typedef enum {
    CHARGE_Current_100mA = 0b0000, /**< @brief Charge at 100mA. */
    CHARGE_Current_190mA,          /**< @brief Charge at 190mA. */
    CHARGE_Current_280mA,          /**< @brief Charge at 280mA. */
    CHARGE_Current_360mA,          /**< @brief Charge at 360mA. */
    CHARGE_Current_450mA,          /**< @brief Charge at 450mA. */
    CHARGE_Current_550mA,          /**< @brief Charge at 550mA. */
    CHARGE_Current_630mA,          /**< @brief Charge at 630mA. */
    CHARGE_Current_700mA,          /**< @brief Charge at 700mA. */
    CHARGE_Current_780mA,          /**< @brief Charge at 780mA. */
    CHARGE_Current_880mA,          /**< @brief Charge at 880mA. */
    CHARGE_Current_960mA,          /**< @brief Charge at 960mA. */
    CHARGE_Current_1000mA,         /**< @brief Charge at 1000mA. */
    CHARGE_Current_1080mA,         /**< @brief Charge at 1080mA. */
    CHARGE_Current_1160mA,         /**< @brief Charge at 1160mA. */
    CHARGE_Current_1240mA,         /**< @brief Charge at 1240mA. */
    CHARGE_Current_1320mA,         /**< @brief Charge at 1320mA. */
} Axp192_ChargeCurrent_t;
/* @[declare_axp192_chargecurrent] */

typedef enum {
    SPARE_CHARGE_VOLT_3100mV = 0x00,
    SPARE_CHARGE_VOLT_3000mV = 0x01,    
    SPARE_CHARGE_VOLT_2500mV = 0x03,    
} Axp192_SpareChargeVolt_t;

typedef enum {
    SPARE_CHARGE_Current_50uA = 0x00,
    SPARE_CHARGE_Current_100uA = 0x01,    
    SPARE_CHARGE_Current_200uA = 0x02,    
    SPARE_CHARGE_Current_400uA = 0x03,    
} Axp192_SpareChargeCurrent_t;

/**
 * @brief List of possible durations the power button must
 * be held to power on the Core2 for AWS IoT EduKit.
 */
/* @[declare_axp192_startuptime] */
typedef enum {
    STARTUP_128mS = 0x00, /**< @brief Power button hold for 128ms to turn on. */
    STARTUP_512mS = 0x01, /**< @brief Power button hold for 512ms to turn on. */
    STARTUP_1S = 0x02, /**< @brief Power button hold for 1s to turn on. */
    STARTUP_2S = 0x03, /**< @brief Power button hold for 2s to turn on. */
} Axp192_StartupTime_t;
/* @[declare_axp192_startuptime] */

/**
 * @brief List of possible durations the power button must 
 * be held to power off the Core2 for AWS IoT EduKit.
 */
/* @[declare_axp192_powerofftime] */
typedef enum {
    POWEROFF_4S = 0x00, /**< @brief Power button hold for 4s to turn off. */
    POWEROFF_6S = 0x01, /**< @brief Power button hold for 6s to turn off. */
    POWEROFF_8S = 0x02, /**< @brief Power button hold for 8s to turn off. */
    POWEROFF_10S = 0x03, /**< @brief Power button hold for 10s to turn off. */
} Axp192_PoweroffTime_t;
/* @[declare_axp192_powerofftime] */

/**
 * @brief Initializes the AXP192 over I2C.
 * 
 * It is a wrapper to Axp192_I2CInit().
 */
/* @[declare_axp192_init] */
void Axp192_Init();
/* @[declare_axp192_init] */

/**
 * @brief Extends the DC voltage range of the Low-Dropout
 * regulator (LDO) on the AXP192.
 * 
 * @param[in] value Desired voltage of the LDO.
 */
/* @[declare_axp192_enableldodcext] */
void Axp192_EnableLDODCExt(uint8_t value);
/* @[declare_axp192_enableldodcext] */

/**
 * @brief Enables or disables extending the Low-Dropout 
 * (LDO) voltage on the AXP192.
 * 
 * @param[in] state Desired state of LDO extention.
 * 1 to enable, 0 to disable.
 */
/* @[declare_axp192_enableexten] */
void Axp192_EnableExten(uint8_t state);
/* @[declare_axp192_enableexten] */

/**
 * @brief Enables or disables Low-Dropout (LDO) 2 on 
 * the AXP192.
 * 
 * @param[in] state Desired state of LDO 2.
 * 1 to enable, 0 to disable.
 */
/* @[declare_axp192_enableldo2] */
void Axp192_EnableLDO2(uint8_t state);
/* @[declare_axp192_enableldo2] */

/**
 * @brief Enables or disables Low-Dropout (LDO) 3 on 
 * the AXP192.
 * 
 * @param[in] state Desired state of LDO 3.
 * 1 to enable, 0 to disable.
 */
/* @[declare_axp192_enableldo3] */
void Axp192_EnableLDO3(uint8_t state);
/* @[declare_axp192_enableldo3] */

/**
 * @brief Enables or disables DC/DC 1 Buck Boost 
 * converter on the AXP192.
 * 
 * @param[in] state Desired state of DC/DC 1.
 * 1 to enable, 0 to disable.
 */
/* @[declare_axp192_enabledcdc1] */
void Axp192_EnableDCDC1(uint8_t state);
/* @[declare_axp192_enabledcdc1] */

/**
 * @brief Enables or disables DC/DC 2 Buck Boost 
 * converter on the AXP192.
 * 
 * @param[in] state Desired state of DC/DC 2.
 * 1 to enable, 0 to disable.
 */
/* @[declare_axp192_enabledcdc2] */
void Axp192_EnableDCDC2(uint8_t state);
/* @[declare_axp192_enabledcdc2] */

/**
 * @brief Enables or disables DC/DC 3 Buck Boost 
 * converter.
 * 
 * @param[in] state Desired state of DC/DC 3.
 * 1 to enable, 0 to disable.
 */
/* @[declare_axp192_enabledcdc3] */
void Axp192_EnableDCDC3(uint8_t state);
/* @[declare_axp192_enabledcdc3] */

/**
 * @brief Set both LDO 2 and LDO 3 voltage on the 
 * AXP192.
 * 
 * @param[in] ldo2_voltage Desired voltage of LDO 2.
 * @param[in] ldo3_voltage Desired voltage of LDO 3.
 */
/* @[declare_axp192_setldo23volt] */
void Axp192_SetLDO23Volt(uint16_t ldo2_voltage, uint16_t ldo3_voltage);
/* @[declare_axp192_setldo23volt] */

/**
 * @brief Set the LDO 2 voltage on the AXP192.
 * 
 * @param[in] voltage Desired voltage of LDO 2.
 */
/* @[declare_axp192_setldo2volt] */
void Axp192_SetLDO2Volt(uint16_t voltage);
/* @[declare_axp192_setldo2volt] */

/**
 * @brief Set the Low-Dropout (LDO) 3 voltage on 
 * the AXP192.
 * 
 * @param[in] voltage Desired voltage of LDO 3.
 */
/* @[declare_axp192_setldo3volt] */
void Axp192_SetLDO3Volt(uint16_t voltage);
/* @[declare_axp192_setldo3volt] */

/**
 * @brief Set the DC/DC 1 Buck Boost converter 
 * voltage on the AXP192.
 * 
 * @param[in] voltage Desired voltage of DC/DC 1.
 */
/* @[declare_axp192_setdcdc1volt] */
void Axp192_SetDCDC1Volt(uint16_t voltage);
/* @[declare_axp192_setdcdc1volt] */

/**
 * @brief Set the DC/DC 2 Buck Boost converter 
 * voltage on the AXP192.
 * 
 * @param[in] voltage Desired voltage of DC/DC 2.
 */
/* @[declare_axp192_setdcdc2volt] */
void Axp192_SetDCDC2Volt(uint16_t voltage);
/* @[declare_axp192_setdcdc2volt] */

/**
 * @brief Set the DC/DC 3 Buck Boost converter 
 * voltage on the AXP192.
 * 
 * @param[in] voltage Desired voltage of DC/DC 3.
 */
/* @[declare_axp192_setdcdc3volt] */
void Axp192_SetDCDC3Volt(uint16_t voltage);
/* @[declare_axp192_setdcdc3volt] */

/**
 * @brief Set the battery low-voltage to cut-off 
 * on the AXP192.
 * 
 * @note This value should not be set lower than the
 * 3.0v default to avoid damaging the built-in battery.
 * 
 * @param[in] voltage Low battery cut-off voltage.
 */
/* @[declare_axp192_setv0ffvolt] */
void Axp192_SetVoffVolt(uint16_t voltage);
/* @[declare_axp192_setv0ffvolt] */

/**
 * @brief Enable sleep and short wake mode on AXP192.
 */
/* @[declare_axp192_enablepwronshortwake] */
void Axp192_EnablePWRONShortWake();
/* @[declare_axp192_enablepwronshortwake] */

/**
 * @brief Gets VBUS voltage on the AXP192.
 *
 * @return The voltage from the VBUS power source.
 */
/* @[declare_axp192_getvbusvolt] */
float Axp192_GetVbusVolt();
/* @[declare_axp192_getvbusvolt] */

/**
 * @brief Gets ACIN voltage on the AXP192.
 *
 * @return The voltage of the ACIN power source.
 */
/* @[declare_axp192_getacinvolt] */
float Axp192_GetAcinVolt();
/* @[declare_axp192_getacinvolt] */

/**
 * @brief Gets battery voltage on the AXP192.
 *
 * @return The voltage of the battery power source.
 */
/* @[declare_axp192_getbatvolt] */
float Axp192_GetBatVolt();
/* @[declare_axp192_getbatvolt] */

/**
 * @brief Gets VBUS current flowing through the AXP192.
 *
 * @return The current usage of the VBUS power source.
 */
/* @[declare_axp192_getvbuscurrent] */
float Axp192_GetVbusCurrent();
/* @[declare_axp192_getvbuscurrent] */

/**
 * @brief Gets ACIN current flowing through the AXP192.
 *
 * @return The current usage of ACIN power source.
 */
/* @[declare_axp192_getacincurrent] */
float Axp192_GetAcinCurrent();
/* @[declare_axp192_getacincurrent] */

/**
 * @brief Gets battery current flowing through the AXP192.
 *
 * @return The current usage of battery power source.
 */
/* @[declare_axp192_getbatcurrent] */
float Axp192_GetBatCurrent();
/* @[declare_axp192_getbatcurrent] */

/**
 * @brief Enables or disables the battery charging circuit 
 * on the AXP192.
 * 
 * @param[in] state Desired state of the battery charging 
 * circuit. 1 to enable, 0 to disable.
 */
/* @[declare_axp192_enablecharge] */
void Axp192_EnableCharge(uint16_t state);
/* @[declare_axp192_enablecharge] */

/**
 * @brief Set the battery charge voltage on the AXP192.
 * 
 * @note This is the voltage applied to the battery for 
 * charging. This is not the voltage of the battery. 
 * Default is 4.2v.
 * 
 * @param[in] volt Desired voltage to charge the battery. 
 */
/* @[declare_axp192_setchargevoltage] */
void Axp192_SetChargeVoltage(Axp192_ChargeVolt_t volt);
/* @[declare_axp192_setchargevoltage] */

/**
 * @brief Set the battery charge current on the AXP192.
 * 
 * @note This is the current applied to the battery for 
 * charging. This is not the current of the battery. 
 * Default is 100mA.
 * 
 * @param[in] cur Desired current to charge the battery. 
 */
/* @[declare_axp192_setchargecurrent] */
void Axp192_SetChargeCurrent(Axp192_ChargeCurrent_t cur);
/* @[declare_axp192_setchargecurrent] */

void Axp192_SetSpareBatCharge(uint8_t enable, Axp192_SpareChargeVolt_t volt, Axp192_SpareChargeCurrent_t current);

void Axp192_GetApsVoltage();

void Axp192_GetInternalTemp();

/**
 * @brief Enables or disables the ADC on the AXP192. 
 * 
 * @param[in] value Desired value of the ADC.
 */
/* @[declare_axp192_setadc1enable] */
void Axp192_SetAdc1Enable(uint8_t value);
/* @[declare_axp192_setadc1enable] */

void Axp192_SetAdc2Enable();

void Axp192_IsBatIn();

void Axp192_IsCharging();

/**
 * @brief Powers down the device.
 * 
 * @note Since the microcontroller and all the peripherals 
 * are powered by the AXP192, calling this function 
 * powers down the device. 
 */
/* @[declare_axp192_poweroff] */
void Axp192_PowerOff();
/* @[declare_axp192_poweroff] */

/**
 * @brief Sets how long the power button needs to be held 
 * to power off all components connected to the AXP192.
 * 
 * @note The default is @ref POWEROFF_6S (6 seconds).
 * 
 * @param[in] time Desired time to hold the power button 
 * to turn off power. 
 */
/* @[declare_axp192_setpresspowerofftime] */
void Axp192_SetPressPoweroffTime(Axp192_PoweroffTime_t time);
/* @[declare_axp192_setpresspowerofftime] */

/**
 * @brief Sets how long the power button needs to be held 
 * to power on all components connected to the AXP192.
 * 
 * Default is @ref STARTUP_128mS (128 milliseconds).
 * 
 * @param[in] time Desired time to hold the power button 
 * to turn on power. 
 */
/* @[declare_axp192_setpressstartuptime] */
void Axp192_SetPressStartupTime(Axp192_StartupTime_t time);
/* @[declare_axp192_setpressstartuptime] */

void Axp192_WriteDataStash();

void Axp192_ReadDataStash();

/**
 * @brief Sets the mode of GPIO 4 on the AXP192 for
 * charging control.
 * 
 * @param[in] mode Desired mode of GPIO 4.
 * 0 for NMOS open-drain ouput, 1 for universal input. 
 * Default value for the Core2 for AWS 
 * is 1, and not meant to be changed. 
 */
/* @[declare_axp192_setgpio4mode] */
void Axp192_SetGPIO4Mode(uint8_t mode);
/* @[declare_axp192_setgpio4mode] */

/**
 * @brief Sets the signal status level of GPIO 4.
 * 
 * @param[in] level Desired signal status level 
 * of GPIO 4.
 */
/* @[declare_axp192_setgpio4level] */
void Axp192_SetGPIO4Level(uint8_t level);
/* @[declare_axp192_setgpio4level] */

/**
 * @brief Sets the mode of GPIO 2 on the AXP192.
 * 
 * @param[in] mode Desired mode of GPIO 2.
 * 0 for PWM, 1 for ADC input. 
 * Default value for the Core2 for AWS 
 * is 1, and not meant to be changed. 
 */
/* @[declare_axp192_setgpio2mode] */
void Axp192_SetGPIO2Mode(uint8_t mode);
/* @[declare_axp192_setgpio2mode] */

/**
 * @brief Sets the average voltage level of GPIO 2 
 * on the AXP192 if the mode is set to PWM.
 * 
 * @param[in] level Desired average voltage level 
 * of GPIO 2.
 */
/* @[declare_axp192_setgpio2level] */
void Axp192_SetGPIO2Level(uint8_t level);
/* @[declare_axp192_setgpio2level] */

/**
 * @brief Sets the mode of GPIO 0 on the AXP192.
 * 
 * @param[in] mode Desired mode of GPIO 0.
 * 0 for Low-Dropout, 1 for ADC input. 
 * Default value for the Core2 for AWS 
 * is 0, and not meant to be changed. 
 */
/* @[declare_axp192_setgpio0mode] */
void Axp192_SetGPIO0Mode(uint8_t mode);
/* @[declare_axp192_setgpio0mode] */

/**
 * @brief Sets the voltage of GPIO 0 on the AXP192.
 * 
 * @param[in] volt Desired voltage of GPIO 0.
 * Default value for the Core2 for AWS is 3300mV.
 */
/* @[declare_axp192_setgpio0volt] */
void Axp192_SetGPIO0Volt(uint16_t volt);
/* @[declare_axp192_setgpio0volt] */

/**
 * @brief Sets the mode of GPIO 1 on the AXP192.
 * 
 * @param[in] mode Desired mode of GPIO 1.
 * 0 for PWM, 1 for ADC input. 
 * Default value for the Core2 for AWS 
 * is 1, and not meant to be changed. 
 */
/* @[declare_axp192_setgpio1mode] */
void Axp192_SetGPIO1Mode(uint8_t mode);
/* @[declare_axp192_setgpio1mode] */

/**
 * @brief Sets the average voltage level of GPIO 1 
 * on the AXP192 if the mode is set to PWM.
 * 
 * @param[in] level Desired average voltage level 
 * of GPIO 1.
 */
/* @[declare_axp192_setgpio1level] */
void Axp192_SetGPIO1Level(uint8_t level);
/* @[declare_axp192_setgpio1level] */
