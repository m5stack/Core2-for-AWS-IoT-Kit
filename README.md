# M5Stack Core2 for AWS IoT EduKit Code Repository
This is the accompanying code repository for microcontroller tutorials presented in the [AWS IoT EduKit](https://edukit.workshop.aws) program using the [M5Stack Core2 for AWS IoT EduKit](https://m5stack.com/products/m5stack-core2-esp32-iot-development-kit-for-aws-iot-edukit) reference Hardware.

Each of the folders in this repository contains a separate project as described below. All projects are compatible with [ESP-IDF v4.2](https://www.espressif.com/en/products/sdks/esp-idf) or [PlatformIO](https://platformio.org/) and runs on Espressif's port of the [FreeRTOS kernel](https://www.freertos.org/). Please ensure that your installation of PlatformIO is updated to the latest version of PlatformIO Core (5.1+) using the command `pio upgrade`. Follow the [AWS IoT EduKit — Getting Started](https://edukit.workshop.aws/en/getting-started.html) tutorial for instructions on how to setup your environment.

For Arduino, UIFlow, or MicroPython content and code, please view the official [M5Stack Docs](https://docs.m5stack.com/#/).

## Included Projects
### Core2 for AWS IoT EduKit Hardware Features Demo
This project is a demo of the hardware features available on the M5Stack Core2 for AWS IoT EduKit reference hardware. It uses at least one available API of each hardware feature in the board support package (BSP). The BSP drivers are located in the **/components/core2forAWS/** directory. There is also a ported version of Espressif's ESP-CRYPTOAUTHLIB to be used with the BSP for the Microchip ATECC608 Trust&GO secure element to function.

## Core2 for AWS IoT EduKit Factory Firmware
This project is the factory firmware that comes loaded with the device. It contains basic functionality and can be used to restore the device to factory state.

### Getting Started
This project is used in the [AWS IoT EduKit — Getting started](https://edukit.workshop.aws/en/getting-started.html) tutorial. It contains a port of [ESP RainMaker](https://rainmaker.espressif.com/). It is a quick end-to-end demonstration of a cloud connected smart home application. Please follow the tutorial for usage.

### Cloud Connected Blinky Hello World
This project is used in the [AWS IoT EduKit — Cloud Connected Blink](https://edukit.workshop.aws/en/blinky-hello-world.html) tutorial. It is a blinky LED demo that uses the on-board secure element for provisioning the device to AWS IoT and a FreeRTOS task to flash the on-board SK6812 LED bars. This example uses the [Espressif ported AWS IoT Device SDK for Embedded C](https://github.com/espressif/esp-aws-iot). Please follow the tutorial for usage.

### Smart Thermostat
This project is used in the [AWS IoT EduKit — Smart Thermostat](https://edukit.workshop.aws/en/smart-thermostat.html) and [AWS IoT EduKit — Smart Spaces](https://edukit.workshop.aws/en/smart-spaces.html) tutorials. It is a demonstration that uses AWS IoT device shadow feature to provide an example of a smart thermostat and how to update device states through the device shadow feature. This example uses the [Espressif ported AWS IoT Device SDK for Embedded C](https://github.com/espressif/esp-aws-iot). Please follow the tutorial for usage.

### Alexa for IoT-Intro (Beta)
This project is used in the [AWS IoT EduKit — Intro to Alexa for IoT](https://edukit.workshop.aws/en/intro-to-alexa-for-iot.html) tutorial. It contains several Alexa for AWS IoT (AIA) features including english "Alexa" wake word detection, smart home device, audio player and others. Please follow the tutorial for usage. This is preview software based on the [ESP-VA-SDK](https://github.com/espressif/esp-va-sdk), and is not a stable port.

## Support
To get support with AWS IoT EduKit, post your question in the [content repo's discussions](https://github.com/aws-samples/aws-iot-edukit-tutorials/discussions).
For issues with the AWS IoT EduKit this repo, please [submit an issue](https://github.com/m5stack/Core2-for-AWS-IoT-EduKit/issues) to this repository.