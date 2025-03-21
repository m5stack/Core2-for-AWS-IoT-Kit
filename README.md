# M5Stack Core2 for AWS IoT Kit Code Repository
This is the accompanying code repository for microcontroller tutorials presented in the [AWS IoT Kit](https://aws-iot-kit-docs.m5stack.com) program using the [M5Stack Core2 for AWS IoT Kit](https://m5stack.com/products/m5stack-core2-esp32-iot-development-kit-for-aws-iot-kit) reference Hardware.

Each of the folders in this repository contains a separate project as described below. All projects are compatible with [ESP-IDF v4.2](https://www.espressif.com/en/products/sdks/esp-idf) or [PlatformIO](https://platformio.org/) and runs on Espressif's port of the [FreeRTOS kernel](https://www.freertos.org/). Please ensure that your installation of PlatformIO is updated to the latest version of PlatformIO Core (5.1+) using the command `pio upgrade`. Follow the [AWS IoT Kit — Getting Started](https://aws-iot-kit-docs.m5stack.com/en/getting-started/) tutorial for instructions on how to setup your environment.

For Arduino, UIFlow, or MicroPython content and code, please view the official [M5Stack Docs](https://docs.m5stack.com/#/).

##!!!SECURITY WARNING!!!
- [CVE-2019-16128](https://nvd.nist.gov/vuln/detail/CVE-2019-16128)
- [ATECC608A](https://census-labs.com/news/2020/10/21/microchip-cryptoauthlib-atcab_sign_base-buffer-overflow/)
- [cryptography@37.0.4](https://security.snyk.io/package/pip/cryptography/37.0.4)

** DO NOT USE THIS SOFTWARE OR HARDWARE IN PRODUCTION **

With those warnings in place I, [Bryan McWhirt](https://github.com/bmmcwhirt) have modified the code to force cryptography 37.0.4 to be used. Us this at your own risk. I am not entirely sure it's legal for M5 to still be selling this in the United States, but I'm no lawyer. I will be contacting Digikey about a refund. This will allow you to finish the Blinky-Hello-World tutorial.

## Included Projects
### Core2 for AWS IoT Kit Hardware Features Demo
This project is a demo of the hardware features available on the M5Stack Core2 for AWS IoT Kit reference hardware. It uses at least one available API of each hardware feature in the board support package (BSP). The BSP drivers are located in the **/components/core2forAWS/** directory. There is also a ported version of Espressif's ESP-CRYPTOAUTHLIB to be used with the BSP for the Microchip ATECC608 Trust&GO secure element to function.

## Core2 for AWS IoT Kit Factory Firmware
This project is the factory firmware that comes loaded with the device. It contains basic functionality and can be used to restore the device to factory state.

### Getting Started
This project is used in the [AWS IoT Kit — Getting started](https://aws-iot-kit-docs.m5stack.com/en/getting-started/) tutorial. It contains a port of [ESP RainMaker](https://rainmaker.espressif.com/). It is a quick end-to-end demonstration of a cloud connected smart home application. Please follow the tutorial for usage.

### Cloud Connected Blinky Hello World
This project is used in the [AWS IoT Kit — Cloud Connected Blink](https://aws-iot-kit-docs.m5stack.com/en/blinky-hello-world/) tutorial. It is a blinky LED demo that uses the on-board secure element for provisioning the device to AWS IoT and a FreeRTOS task to flash the on-board SK6812 LED bars. This example uses the [Espressif ported AWS IoT Device SDK for Embedded C](https://github.com/espressif/esp-aws-iot). Please follow the tutorial for usage.

### Smart Thermostat
This project is used in the [AWS IoT Kit — Smart Thermostat](https://aws-iot-kit-docs.m5stack.com/en/smart-thermostat/) and [AWS IoT Kit — Smart Spaces](https://aws-iot-kit-docs.m5stack.com/en/smart-spaces/) tutorials. It is a demonstration that uses AWS IoT device shadow feature to provide an example of a smart thermostat and how to update device states through the device shadow feature. This example uses the [Espressif ported AWS IoT Device SDK for Embedded C](https://github.com/espressif/esp-aws-iot). Please follow the tutorial for usage.

### Alexa for IoT-Intro (Beta)
This project is used in the [AWS IoT Kit — Intro to Alexa for IoT](https://aws-iot-kit-docs.m5stack.com/en/intro-to-alexa-for-iot/) tutorial. It contains several Alexa for AWS IoT (AIA) features including english "Alexa" wake word detection, smart home device, audio player and others. Please follow the tutorial for usage. This is preview software based on the [ESP-VA-SDK](https://github.com/espressif/esp-va-sdk), and is not a stable port.

## Support
To get support with AWS IoT Kit, post your question in the [content repo's discussions](https://github.com/m5stack/aws-iot-kit-tutorials/discussions).
For issues with the AWS IoT Kit this repo, please [submit an issue](https://github.com/m5stack/Core2-for-AWS-IoT-Kit/issues) to this repository.