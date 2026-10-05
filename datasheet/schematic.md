# M5Stack Core2 for AWS - Hardware Reference (LLM Driver / BSP Specification)

## Source and Variant Scope

This reference describes the **M5Stack Core2 for AWS IoT Kit**: a Core2 main
unit plus the AWS-specific M5GO Bottom connected through M5Bus. It does not
describe the standard Core2 by itself.

Wiring was verified on 2026-07-31 against two checked-in official M5Stack
assets:

* `CORE2_V1.0_SCH_page_01.png` - Core2 V1.0 main-board schematic page,
  SHA-256 `6c305db1571c5d0b2bf6bf88a747719ac07a7be236002d15e44fc5974aa50204`
* `core2_for_aws_sch_01.webp` - Core2 for AWS add-on schematic,
  SHA-256 `feed9faed66efe07cc0154b96b20f4f8065edaffc9781a93fd10b215f30eed63`

Both files are byte-for-byte identical to the assets linked by the official
[M5Stack Core2 for AWS documentation](https://docs.m5stack.com/en/core/core2_for_aws).
The schematic images control physical wiring claims. Current product-page facts
corroborate those claims but do not silently replace dated schematic revisions.

## Hardware Revisions

Verified on 2026-10-05 against the official
[Core2 For AWS v1.3 documentation](https://docs.m5stack.com/en/products/sku/K010-AWS-V13)
(SKU K010-AWS-V13) and two more checked-in assets:

* `M5GO_Bottom2_AWS_V1.3_SCH.pdf` (and its `_page_01.png` render) - M5GO
  Bottom2 v1.3 AWS add-on schematic dated 2026-01-30, SHA-256
  `10e007d004efaa888ee524af118f184d6eab7b1b8faef53bd2847cacfc6766ef`
* `CORE2_V1.0_SCH.pdf` - vector PDF of the same Core2 V1.0 main-board page,
  SHA-256 `4a59d16291c7de9d9deda31fe154c8b5ac3c8f92209f6f8206d72f97afeea94f`

The main board is unchanged; v1.3 swaps the add-on board.

| Part | Core2 for AWS (K010-AWS) | Core2 for AWS v1.3 (K010-AWS-V13) | Software impact |
| ---- | ------------------------ | --------------------------------- | --------------- |
| IMU (add-on U1, I2C 0x68) | MPU6886 | BMI270 | Different driver; detected at run time |
| PDM mic (add-on U2) | SPM1423HM4H-B | LMD4737T261 | Same pinout and L/R=GND; PDM clock 1.3-3.25 MHz suits both |
| U1 decoupling C4 | 100 nF | 2.2 uF | None |
| M5Bus pin 2 (GPIO35) on add-on J4 | Connected | Not connected | None; the add-on does not use it |
| USB-UART (main board) | CP2104 (some CH9102F) | CH9102F | Host driver only |
| LCD controller | ILI9342C, or ILI9342E from 2026-08-07 | Same | Different init list; detected at run time |

Notes:

* The v1.3 product page still lists an SPM1423 microphone. Its add-on schematic
  shows U2 as an LMD4737T261 with SELECT tied to GND. The schematic controls.
* Both IMUs report in the same board frame (X+ screen right, Y+ screen top,
  Z+ out of the screen). Measured by gravity on 2026-10-05 with the board
  upright and on its side; this matches the official IMU orientation images
  for both revisions.
* No microphone or USB bridge identity is readable, so only the IMU identifies
  the kit model. v1.0/v1.1 builds of K010-AWS (RTC coin cell removed 2023-02,
  CH9102F introduced 2021-07) are indistinguishable in software.

## Board Overview

The **M5Stack Core2 for AWS** is an ESP32-based embedded system with integrated display, touch panel, audio amplifier, RTC, power management, and expansion bus.

Primary components:

| Component      | Function                    |
| -------------- | --------------------------- |
| ESP32-D0WDQ6   | Main MCU                    |
| AXP192         | Power Management Unit       |
| ILI9342C/E     | LCD controller              |
| FT6336U        | Capacitive touch controller |
| MPU6886 / BMI270 | IMU sensor (v1.3: BMI270)  |
| BM8563         | RTC                         |
| NS4168         | Speaker amplifier           |
| SY7088         | 5V boost converter          |
| ATECC608       | Secure element              |
| SPM1423 / LMD4737T261 | Microphone (v1.3: LMD4737T261) |
| CP2104 / CH9102F | USB-UART bridge           |
| XM25QH128      | SPI Flash                   |
| ESPSRAM64H     | PSRAM                       |
| TF card socket | microSD storage             |
| 1027 DC motor  | vibration motor             |
| SK6812 ×10     | RGB LED chain (add-on)      |
| TP4057         | Li-ion charger (add-on)     |

---

# MCU

## ESP32-D0WDQ6

### Power rails

| Rail       | Voltage       | Usage   |
| ---------- | ------------- | ------- |
| MCU_VDD    | 3.3V          | core IO |
| VDDA       | analog supply |         |
| VDD3P3_CPU | CPU           |         |
| VDD3P3_RTC | RTC domain    |         |
| VDD_SDIO   | SPI flash     |         |

---

## ESP32 Pin Map

| GPIO   | Function               |
| ------ | ---------------------- |
| GPIO0  | Boot / I2S LRCK / mic CLK |
| GPIO1  | UART TX0               |
| GPIO2  | speaker I2S data       |
| GPIO3  | UART RX0               |
| GPIO4  | SD card                |
| GPIO5  | LCD CS                 |
| GPIO12 | speaker BCLK           |
| GPIO13 | UART2 RX               |
| GPIO14 | UART2 TX               |
| GPIO15 | LCD DC                 |
| GPIO16 | PSRAM                  |
| GPIO17 | PSRAM                  |
| GPIO18 | SPI SCK                |
| GPIO19 | M5Bus GPIO             |
| GPIO21 | I2C SDA                |
| GPIO22 | I2C SCL                |
| GPIO23 | SPI MOSI               |
| GPIO25 | DAC / add-on RGB LED data (SK6812) |
| GPIO26 | DAC / add-on GPIO socket |
| GPIO27 | general IO             |
| GPIO32 | external I2C SDA       |
| GPIO33 | external I2C SCL       |
| GPIO34 | ADC / mic data (PDM)   |
| GPIO35 | ADC                    |
| GPIO36 | ADC                    |
| GPIO38 | SPI MISO               |
| GPIO39 | touch interrupt        |

---

# System Buses

## Internal I2C Bus

Pins:

```
SDA = GPIO21
SCL = GPIO22
```

Pull-ups present on board.

**All on-board I2C peripherals share this bus**, including devices physically located on the add-on board (MPU6886, ATECC608). These reach GPIO21/GPIO22 through M5Bus pins 17/18.

Devices on this bus:

| Device   | Board     | Function         |
| -------- | --------- | ---------------- |
| AXP192   | Core      | PMU              |
| FT6336U  | Core      | Touch controller |
| BM8563   | Core      | RTC              |
| MPU6886 / BMI270 | Add-on | IMU          |
| ATECC608 | Add-on    | Secure element   |

---

## External I2C Bus (Port A)

```
SDA = GPIO32
SCL = GPIO33
```

Available through the external **Port A** (red) Grove connector. **Used only for plug-in "unit" accessories**, not by any on-board peripheral.

---

## SPI Bus (Primary)

```
MOSI = GPIO23
MISO = GPIO38
SCK  = GPIO18
```

Note: GPIO19 appears on M5Bus pin 22 but is **not connected** to the primary SPI bus.

Devices:

| Device  | CS    |
| ------- | ----- |
| LCD     | GPIO5 |
| microSD | GPIO4 |

---

## SPI Flash

Chip: **XM25QH128**

Pins:

| Signal | ESP32  |
| ------ | ------ |
| CS     | GPIO11 |
| SCLK   | GPIO6  |
| MOSI   | GPIO7  |
| MISO   | GPIO8  |
| WP     | GPIO9  |
| HOLD   | GPIO10 |

---

## PSRAM

Chip: **ESPSRAM64H**

Pins:

| Signal | ESP32    |
| ------ | -------- |
| CS     | GPIO16   |
| CLK    | GPIO17   |
| SIO0   | GPIO8    |
| SIO1   | GPIO7    |
| SIO2   | GPIO10   |
| SIO3   | GPIO9    |

---

# Display

Controller: **ILI9342C** or **ILI9342E** (units built from 2026-08-07)

Interface: **SPI**

Signals:

| Signal | ESP32           |
| ------ | --------------- |
| MOSI   | GPIO23          |
| MISO   | GPIO38          |
| SCK    | GPIO18          |
| CS     | GPIO5           |
| DC     | GPIO15          |
| RST    | AXP192 GPIO4    |
| BL     | AXP192 DCDC3    |

Power:

```
PERI_VDD (3.3V)
```

Note: Display reset is controlled via the AXP192 PMU's GPIO4 (NMOS open-drain), not an ESP32 GPIO. Backlight brightness is controlled by the AXP192 DCDC3 rail voltage (2.2V–3.3V).

Note: The schematic routes the LCD module's MISO pin to GPIO38, but register
reads only answer on the bidirectional SDA line (GPIO23, 3-wire SPI). Measured
2026-10-05 on both kit models: reads on GPIO38 return FFh, reads on GPIO23
return the ILI9342C Read ID4 value 93h 42h. GPIO38 still carries SD card MISO.

---

# Touch Panel

Controller: **FT6336U**

Interface: **I2C**

Pins:

| Signal | ESP32           |
| ------ | --------------- |
| SDA    | GPIO21          |
| SCL    | GPIO22          |
| INT    | GPIO39          |
| RST    | AXP192 GPIO4    |

Note: Touch reset is shared with the display reset line, both controlled via AXP192 GPIO4.

---

# IMU

Chip: **MPU6886** (Core2 for AWS) or **BMI270** (Core2 for AWS v1.3), located on add-on board as U1. Both use 7-bit address `0x68` (SDO/AD0 pulled to GND by R5); INT pins are not routed.

Interface: **I2C** (internal bus via M5Bus pins 17/18)

Pins:

| Signal | ESP32  |
| ------ | ------ |
| SDA    | GPIO21 |
| SCL    | GPIO22 |

---

# RTC

Chip: **BM8563**

Interface: **I2C**

Pins:

| Signal | ESP32  |
| ------ | ------ |
| SDA    | GPIO21 |
| SCL    | GPIO22 |
| INT    | not connected |

Clock crystal:

```
32.768 kHz
```

Backup battery present.

---

# Secure Element

Chip: **ATECC608** (located on add-on board), at 7-bit I2C address `0x35`.

Interface: **I2C** (internal bus via M5Bus pins 17/18)

Pins:

| Signal | ESP32  |
| ------ | ------ |
| SDA    | GPIO21 |
| SCL    | GPIO22 |

---

# Audio System

## Microphone

Device: **SPM1423** (Core2 for AWS) or **LMD4737T261** (Core2 for AWS v1.3), located on add-on board as U2 with SELECT/L-R tied to GND

Interface:

```
PDM
```

Signals:

| Signal | ESP32  |
| ------ | ------ |
| DATA   | GPIO34 |
| CLK    | GPIO0  |

Note: PDM interface (not standard I2S). DATA/CLK reach the add-on board through M5Bus pins 26/24.

---

## Speaker Amplifier

Chip: **NS4168**

Interface: **I2S**

Pins:

| Signal | ESP32  |
| ------ | ------ |
| LRCK   | GPIO0  |
| BCLK   | GPIO12 |
| DATA   | GPIO2  |
| CTRL   | AXP192 GPIO2 |

Speaker output to onboard speaker.

Note: The amplifier enable (CTRL / SPK_EN) is driven by AXP192 GPIO2, not an ESP32 GPIO.

---

# Vibration Motor

Device: **1027 DC Motor**

Control:

```
VIB_MOTOR
```

Driven from an AXP192 rail (LDO3), not an ESP32 GPIO.

---

# Storage

## microSD Card

Interface: **SPI**

Pins:

| Signal | ESP32  |
| ------ | ------ |
| CS     | GPIO4  |
| MOSI   | GPIO23 |
| MISO   | GPIO38 |
| SCK    | GPIO18 |

Power:

```
PERI_VDD
```

---

# USB Interface

Bridge chip:

```
CP2104 (CH9102F on later units and on v1.3)
```

Connections:

| Signal | ESP32 |
| ------ | ----- |
| TX     | GPIO3 |
| RX     | GPIO1 |

USB-C connector provides:

```
USB_5V
```

---

# Power System

## PMU

Chip: **AXP192**

Functions:

* battery charging
* power rails
* power monitoring
* voltage regulators

Interface:

```
I2C
```

Connected to:

```
GPIO21 / GPIO22
```

Main rails generated:

| Rail     | Usage        |
| -------- | ------------ |
| IPS_BUS  | system power |
| MCU_VDD  | MCU 3.3V     |
| PERI_VDD | peripherals  |
| RTC_VDD  | RTC          |
| SYS_VBAT | battery      |

---

# Boost Converter

Chip:

```
SY7088
```

Function:

```
3.3V → 5V boost
```

Enable pin:

```
BST_EN
```

Used for:

```
BUS_5V
```

---

# Expansion Bus (M5Bus)

Connector exposes ESP32 signals.

## Pinout

| Pin | Signal |
| --- | ------ |
| 1   | GND    |
| 2   | GPIO35 |
| 3   | GND    |
| 4   | GPIO36 |
| 5   | GND    |
| 6   | EN     |
| 7   | GPIO23 |
| 8   | GPIO25 |
| 9   | GPIO38 |
| 10  | GPIO26 |
| 11  | GPIO18 |
| 12  | 3.3V   |
| 13  | GPIO3  |
| 14  | GPIO1  |
| 15  | GPIO13 |
| 16  | GPIO14 |
| 17  | GPIO21 |
| 18  | GPIO22 |
| 19  | GPIO32 |
| 20  | GPIO33 |
| 21  | GPIO27 |
| 22  | GPIO19 |
| 23  | GPIO2  |
| 24  | GPIO0  |
| 25  | HPWR   |
| 26  | GPIO34 |
| 27  | HPWR   |
| 28  | 5V     |
| 29  | HPWR   |
| 30  | BAT    |

---

# External Ports

The M5Stack Core2 main board exposes a **single** Grove-compatible port for external "unit" accessories.

## Port A (Red) — I2C

Connector J4.

| Pin | Signal          | GPIO   |
| --- | --------------- | ------ |
| 1   | GND             | —      |
| 2   | 5V              | —      |
| 3   | EXT_I2C_SDA     | GPIO32 |
| 4   | EXT_I2C_SCL     | GPIO33 |

Used only for external I2C "unit" accessories. **Not used by any on-board peripheral** — all on-board I2C devices use the internal bus (GPIO21/GPIO22).

> **Note:** Port B and Port C do **not** exist on the Core2 main board. The DAC (GPIO26), ADC (GPIO36), and UART2 (GPIO13 RX / GPIO14 TX) signals are only routed over the M5Bus and are broken out on the add-on board's GPIO socket (J2) and UART socket (J1).

---

# External Battery

Connector J5.

Pins:

| Pin | Signal   |
| --- | -------- |
| 1   | GND      |
| 2   | SYS_VBAT |

---

# Add-on Board (Core2 for AWS)

The add-on board mates with the core board through the 30-pin M5Bus.

## RGB LED Chain

Device: **SK6812** × 10 (chain, GRB)

Data path:

```
GPIO25 (M5Bus pin 8, net "RGB")
  → R8 (1k) → Q1 (SS8550) base, R7 (4.7k) pull-up to +3.3V
  → SK6812 data input → LED1 … LED10
```

Power: +5V (M5Bus pin 28).

> **Note:** The RGB LED data line is **GPIO25** (also the DAC2 pin — mutually exclusive uses). GPIO26 (M5Bus pin 10) is *not* used for the LEDs; it passes through to the add-on GPIO socket J2.

## Expansion Sockets

| Socket | Ref | Pin 1 | Pin 2 | Pin 3 | Pin 4 |
| ------ | --- | ----- | ----- | ----- | ----- |
| UART   | J1  | GPIO13 (RX) | GPIO14 (TX) | +5V | GND |
| GPIO   | J2  | GPIO36 (ADC) | GPIO26 (DAC) | +5V | GND |
| I2C    | J3  | GND   | +5V (VIN) | GPIO21 (SDA) | GPIO22 (SCL) |

The J3 I2C socket is on the **internal** I2C bus (GPIO21/GPIO22) with 4.7k pull-ups (R2/R3) to +3.3V.

## Battery Charger

Chip: **TP4057** (U3)

```
VIN → charger → BAT+ (returns to core SYS_VBAT via M5Bus pin 30)
PROG resistor R9 = 2k
Status LED D1 (1615RG)
```

---

# Buttons

## Reset button

Signal:

```
MCU_RST
```

Connected to ESP32 reset.

---

## Power button

Signal:

```
PWR_KEY
```

Connected to PMU.

---

# LEDs

## System LED

Signal:

```
SYS_LED
```

Connected to ESP32.

---

# Important Power Rails

| Rail     | Voltage      |
| -------- | ------------ |
| USB_5V   | USB input    |
| BUS_5V   | boosted 5V   |
| IPS_BUS  | system power |
| MCU_VDD  | 3.3V         |
| PERI_VDD | peripherals  |
| RTC_VDD  | RTC          |
| SYS_VBAT | battery      |

---

# Driver Implementation Notes

### I2C devices (all on internal bus)

Drivers required:

```
AXP192    (core board)
BM8563    (core board)
FT6336U   (core board)
MPU6886   (add-on board; BMI270 on v1.3)
ATECC608  (add-on board)
```

Bus (internal I2C — shared by all on-board peripherals):

```
GPIO21 SDA
GPIO22 SCL
```

---

### SPI devices

Drivers required:

```
ILI9342C or ILI9342E LCD
microSD
```

Bus:

```
MOSI GPIO23
MISO GPIO38
SCK GPIO18
```

---

### Audio drivers

```
I2S microphone
I2S speaker amplifier
```

---

### Power management

PMU driver required for:

```
battery level
voltage rails
power control
```

---

### Expansion bus

External "unit" modules connect through the on-board Grove port:

- **Port A** (Red) — I2C (GPIO32/GPIO33)

The DAC/ADC (GPIO26/GPIO36) and UART2 (GPIO13/GPIO14) signals are not on an on-board Grove port; they reach the add-on board's GPIO socket (J2) and UART socket (J1) over the M5Bus.

The **M5Bus** 30-pin connector joins the core board to the add-on board, carrying power and GPIO signals including the internal I2C bus.
