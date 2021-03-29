# Table Of Contents

- [Table Of Contents](#table-of-contents)
- [0. Important Note](#0-important-note)
- [1. Introduction](#1-introduction)
  - [1.1 Solution Architecture](#11-solution-architecture)
  - [1.2 The Software](#12-the-software)
  - [1.3 The Hardware](#13-the-hardware)
- [2. Development Setup](#2-development-setup)
- [3. AWS IoT Configuration](#3-aws-iot-configuration)
  - [3.1 Using Espressif's AWS Account](#31-using-espressifs-aws-account)
  - [3.2 Using Your AWS Account](#32-using-your-aws-account)
- [4. Device Provisioning](#4-device-provisioning)
  - [4.1 Configuration Steps](#41-configuration-steps)
  - [4.2 Additional Device Settings](#42-additional-device-settings)
- [5. Device Performance](#5-device-performance)
  - [5.1 CPU and Memory usage](#51-cpu-and-memory-usage)
  - [5.2 User Perceived Latency (UPL)](#52-user-perceived-latency-upl)
- [6. Test Case Specific Instructions](#6-test-case-specific-instructions)
  - [6.1 Wi-Fi AP Change (Test Case 1.7)](#61-wi-fi-ap-change-test-case-17)
  - [6.2 Device Reset (Test Case 1.9)](#62-device-reset-test-case-19)
  - [6.3 Sign Out (Test Case 1.6)](#63-sign-out-test-case-16)
- [7. Reusing the AWS IoT MQTT connection](#7-reusing-the-aws-iot-mqtt-connection)
  - [7.1 Working with MQTT](#71-working-with-mqtt)
    - [7.1.1 Subscribing](#711-subscribing)
    - [7.1.2 Publishing](#712-publishing)
    - [7.1.3 Testing](#713-testing)
  - [7.2 Working with Thing-Shadows](#72-working-with-thing-shadows)
    - [7.2.1 Registering delta](#721-registering-delta)
    - [7.2.2 Reporting Update](#722-reporting-update)
    - [7.2.3 Testing](#723-testing)
- [8. Customising for your Board](#8-customising-for-your-board)
  - [8.1 LED](#81-led)
    - [8.1.1 Writing your own LED Driver](#811-writing-your-own-led-driver)
    - [8.1.2 Changing the LED Pattern (Standard Patterns)](#812-changing-the-led-pattern-standard-patterns)
    - [8.1.3 Changing the LED Pattern (Custom Patterns)](#813-changing-the-led-pattern-custom-patterns)
  - [8.2 Button](#82-button)
  - [8.3 Audio Codec](#83-audio-codec)
    - [8.3.1 Using Another Supported Audio Codec](#831-using-another-supported-audio-codec)
    - [8.3.2 Writing your own Audio Codec Driver](#832-writing-your-own-audio-codec-driver)
  - [8.4 DSP Driver](#84-dsp-driver)
    - [8.4.1 Using Another DSP Driver](#841-using-another-dsp-driver)
    - [8.4.2 Writing your own DSP Driver](#842-writing-your-own-dsp-driver)
- [9. Integrating other components](#9-integrating-other-components)
  - [9.1 ESP RainMaker](#91-esp-rainmaker)
    - [9.1.1 Environment Setup](#911-environment-setup)
    - [9.1.2 Device Provisioning](#912-device-provisioning)
    - [9.1.3 Customisation](#913-customisation)
  - [9.2 Smart Home](#92-smart-home)
    - [9.2.1 Usage](#921-usage)
    - [9.2.2 Customisation](#922-customisation)
  - [9.3 Audio Player](#93-audio-player)
    - [9.3.1 Enabling Custom Player](#931-enabling-custom-player)
    - [9.3.2 Customisation](#932-customisation)
- [10. Production Considerations](#10-production-considerations)
  - [10.1 Over-the-air Updates (OTA)](#101-over-the-air-updates-ota)
  - [10.2 Manufacturing](#102-manufacturing)
    - [10.2.1 Mass Manufacturing Utility](#1021-mass-manufacturing-utility)
    - [10.2.2 Pre-Provisioned Modules](#1022-pre-provisioned-modules)
  - [10.3 Security](#103-security)
    - [10.3.1 Secure Boot](#1031-secure-boot)
    - [10.3.2 Flash Encryption](#1032-flash-encryption)
    - [10.3.3 NVS Encryption](#1033-nvs-encryption)
- [A1 Appendix FAQs](#a1-appendix-faqs)
  - [A1.1 Compilation errors](#a11-compilation-errors)
  - [A1.2 Device setup using the Mobile app](#a12-device-setup-using-the-mobile-app)
  - [A1.3 Device crashing](#a13-device-crashing)
  - [A1.4 Device not crashed but not responding](#a14-device-not-crashed-but-not-responding)
  - [A1.5 Migrating to your own AWS account](#a15-migrating-to-your-own-aws-account)

# 0. Important Note
The Wake-word ("Alexa") recognition software that is part of the [GitHub repository](https://github.com/espressif/esp-va-sdk/tree/feature/aia-beta) is for evaluation only. Please contact sales@espressif.com for production-ready Wake-word recognition DSP Firmware that is available from our DSP partners.

# 1. Introduction

Espressif's Alexa SDK allows customers to build Alexa built-in smart devices using the AVS for AWS IoT protocol. This allows customers to leverage Espressif's rich IoT device development capability while simultaneously incorporating voice enablement in these devices.


## 1.1 Solution Architecture

The typical solution architecture of the product is shown as below. 

<center>
    <img src="https://github.com/espressif/esp-va-sdk/wiki/aia_images/esp_aia_solution_architecture.png" alt="Solution Architecture Block Diagram" title="Solution Architecture Block Diagram" width="800" />
</center>

The following are the relevant blocks for the solution:

1.  **ESP32**: This is the primary microcontroller that controls the
    operations of the product.
    1.  **Alexa Client**: It runs the Alexa client that manages the audio communication with the Alexa cloud. The ESP32 is also responsible for any state management, audio encode/decode operations.
    2.  **IoT Device**: It also runs the software that interfaces with your peripherals providing the smart-device functionality that you wish to expose.
2.  **DSP**: The DSP typically performs the Noise Reduction (NR), Acoustic Echo Cancellation (AEC) and run the Wake-Word Engine (WWE). The DSP is interfaced with the Mic Array for the audio input. And it subsequently interacts with the ESP32 for relaying the audio input.
3.  **Codec**: The playback data is received by the Codec which it subsequently sends to the speaker.

## 1.2 The Software

Typically as a device manufacturer, you will want to customise the software running on ESP32. Primarily, this activity will include writing any device drivers for your device being interfaced, and any relevant functionality that you wish to expose.

<center>
    <img src="https://github.com/espressif/esp-va-sdk/wiki/aia_images/esp_aia_software_components.png" alt="Software Components Block Diagram" title="Software Components Block Diagram" width="900" />
</center>

The above block diagram indicates the various components of the Alexa SDK.

## 1.3 The Hardware
This code is made to work exclusively with the M5Stack Core2 ESP32 IoT Development Kit for AWS IoT EduKit available on [Amazon.com](https://www.amazon.com/dp/B08VGRZYJR) or on the [M5Stack store](https://m5stack.com/products/m5stack-core2-esp32-iot-development-kit-for-aws-iot-edukit).

# 2. Development Setup

Visit the AWS Iot EduKit — [Introduction to Alexa for IoT](https://edukit.workshop.aws/en/intro-to-alexa-for-iot.html) tutorial.

# 3. AWS IoT Configuration

The Alexa SDK requires AWS IoT certificates to be configured in your firmware. There are two ways to do this (a) You can either use temporary certificates from Espressif's AWS-IoT account for evaluation, or (b) You can use your own AWS account for generating the certificates. 

## 3.1 Using Espressif's AWS Account

If you wish to do a quick evaluation of AVS for AWS IoT and you do not have an AWS account, you can try it with temporary certificates created in Espressif's AWS account. Please follow this:

1. Certificate Generation:
    1. You can generate temporary AWS IoT certificates from here: [Generate Certificate](https://espressif.github.io/esp-va-sdk/).
    2. You will get an email with the device certificates.
2. Certificate Flashing:
    1. After receiving the device certificates, modify the file mfg_config.csv and add the relevant paths for all the files.
    2. Now run the following command to generate the manufacturing partition (mfg.bin) for your device.
    ```
    python $IDF_PATH/components/nvs_flash/nvs_partition_generator/nvs_partition_gen.py generate /path/to/mfg_config.csv mfg.bin 0x6000
    ```
    3. Use the following command to flash the certificates on the device.
    ```
    $ python $IDF_PATH/components/esptool_py/esptool/esptool.py --chip esp32 --port $ESPPORT write_flash 0x10000 mfg.bin
    ```
    4. Flash the Alexa firmware again as described in the Section [Flashing the Firmware](#24-flashing-the-firmware) and reboot the device.
3. The device is now functional and you can skip to the [Device Provisioning](#4-device-provisioning) Section.

## 3.2 Using Your AWS Account

If you want to use your own AWS account for the AWS IoT certificates and using your account for all the Alexa communication, please follow these instructions for getting it in place.

1.  AWS Account Whitelisting:
    1.  Create an AWS account, if you don't have one: [AWS Account](https://aws.amazon.com/account/).
        1.  Once complete, note the AWS Account ID: [AWS Account Details](https://console.aws.amazon.com/billing/home?#/account)
    2.  Create your AVS product.
        1.  If you wish to do a quick evaluation you may skip the AVS product creation step, and use Espressif's AVS product instead. For a quick evaluation, this step will save you the hassle of regenerating the Android/iOS phone applications that should be linked with your AVS product. If you wish to try this, please contact your Espressif contact person for the procedure, and skip the next bullet item. If not, please proceed with the next steps.
        2.  Steps for creating the AVS product:
            1.  Go to [Amazon Developer Account](http://developer.amazon.com) and create an account. Then go to [AVS Product Page](https://developer.amazon.com/alexa/console/avs/home) and create an AVS product with 'Product category' as 'Smart Home'.
            2.  While doing so, make sure to select 'Yes' in the section 'Is this device associated with one or more AWS IoT Core Accounts?'. Enter your AWS Account ID (that you noted above) there and complete the creation of the product.
            3.  Note: You will also have to build your own Android and iOS app with this product_id and security_profile to sign-in into the device.
    3.  Additionally, your AWS account also needs to be configured with the appropriate CloudFormation templates. You only need to do this step if your AWS Account was created before the year 2020. If your AWS account was created later, feel free to ignore this setup.
        1.  Please get in touch with your Espressif contact for the CloudFormation templates. Please ensure that this configuration is also done.
    4.  You need to create a policy to be attached to the thing you will be creating in the next step.
        1.  Go to AWS Services -> IoT Core (AWS IoT) -> Secure -> Policies
        2.  Click on create.
        3.  Enter the Name: ESP-Alexa-Policy.
        4.  Under Add statements, click on 'Advanced mode'.
        5.  Replace the existing policy with this:
            ```
            {
              "Version": "2012-10-17",
              "Statement": [
                {
                  "Effect": "Allow",
                  "Action": "iot:Connect",
                  "Resource": "*"
                },
                {
                  "Effect": "Allow",
                  "Action": [
                    "iot:Publish",
                    "iot:Subscribe",
                    "iot:Receive",
                    "iot:UpdateThingShadow",
                    "iot:GetThingShadow"
                  ],
                  "Resource": "*"
                }
              ]
            }
            ```
        6.  Click on create.

2.  Generating AWS IoT Certificates:
    1.  Register a thing in AWS IoT as described in this tutorial: [How to register a thing](https://docs.aws.amazon.com/iot/latest/developerguide/register-device.html)
    2.  Please make sure that the thing is 'Activated'
    3.  Download 3 files for the thing:
        1.  Device Certificate
        2.  Device Private Key
        3.  Root CA for AWS IoT
    4.  You will need to attach the policy created above as created above. After clicking on 'Attach a policy', select ESP-Alexa-Policy from the policy list.
3.  Device Firmware: Configure AWS Settings:
    1.  Make a note of your AWS Account ID
        1.  This can be accessed from this location: [AWS Account Details](https://console.aws.amazon.com/billing/home?#/account)
    2.  Make a note of your AWS IoT Endpoint URL
        1.  This can be accessed from this location: AWS Services -> IoT Core (AWS IoT) -> Settings
    3.  Go to the example application *examples/amazon_aia/* and use
        the command:
        ```
        $ idf.py menuconfig
        ```
    4.  Go to 'Voice Assistant Configuration'.
    5.  Update the AWS Account ID to have your account id
    6.  Update the AWS Endpoint to have your IoT endpoint
4.  Device Firmware: Configure AWS IoT Certificates:
    1.  Copy the 3 files downloaded after creating the AWS IoT Thing above, to the directory *examples/amazon_aia/certs/*
    2.  Modify the file *examples/amazon_aia/certs/mfg_config.csv* and add the relevant paths for all the files.
    3.  Generate the manufacturing partition (mfg.bin) with the command:
        ```
        $ cd examples/amazon_alexa/certs

        $ python $IDF_PATH/components/nvs_flash/nvs_partition_generator/nvs_partition_gen.py generate mfg_config.csv mfg.bin 0x6000
        ```
    4.  Flash this manufacturing partition with the command:
        ```
        $ python $IDF_PATH/components/esptool_py/esptool/esptool.py --chip esp32 --port $ESPPORT write_flash 0x10000 mfg.bin
        ```
    5.  Flash the firmware again as described in the Section [Flashing the Firmware](#24-flashing-the-firmware) and reboot the device.

Now the device is functional.

# 4. Device Provisioning

The configuration step consists of (a) configuring the Wi-Fi network and (b) signing into your Alexa account and linking the device. Espressif has released the following phone applications that facilitate the same:

**iOS**: [iOS app](https://apps.apple.com/in/app/esp-alexa/id1464127534) <br>
**Android**: [Android app](https://play.google.com/store/apps/details?id=com.espressif.provbleavs)

Please install the relevant application on your phone before your proceed.

## 4.1 Configuration Steps

Here are the steps to configure the Dev Kit

*   On first boot-up, the Dev Kit is in configuration mode. This is indicated by Orange LED pattern. Please ensure that the LED pattern is seen as described above, before you proceed.
*   Launch the phone app.
*   Select the option *Add New Device*.

<center>
    <img src="https://github.com/espressif/esp-va-sdk/wiki/aia_images/esp_alexa_app_home.png" alt="App Home" title="App Home" width="300" />
</center>

*   A list of devices that are in configuration mode is displayed. Note that the devices are discoverable over BLE (Bluetooth Low Energy). Please ensure that the phone app has the appropriate permissions to access Bluetooth (on Android the *Location* permission is also required for enabling Bluetooth).

<center>
    <img src="https://github.com/espressif/esp-va-sdk/wiki/aia_images/esp_alexa_app_discover_devices.png" alt="App Discover Devices" title="App Discover Devices" width="300" />
</center>

*   Now you can sign-in to your Amazon Alexa account. If you have Amazon Shopping app installed on the same phone, app will automatically sign-in with the account the shopping app is signed in to. Otherwise it will open a login page on the phone's default browser. (It is recommended to install the Amazon Shopping app on your phone to avoid any other browser related errors.)

<center>
    <img src="https://github.com/espressif/esp-va-sdk/wiki/aia_images/esp_alexa_app_sign_in.png" alt="App Sign-in" title="App Sign-in" width="300" />
</center>

*   You can now select the Wi-Fi network that the Dev Kit should connect with, and enter the credentials for this Wi-Fi network.

<center>
    <img src="https://github.com/espressif/esp-va-sdk/wiki/aia_images/esp_alexa_app_wifi_scan_list.png" alt="App Scna List" title="App Scan List" width="300" />
    <img src="https://github.com/espressif/esp-va-sdk/wiki/aia_images/esp_alexa_app_wifi_password.png" alt="App Wi-Fi Password" title="App Wi-Fi Password" width="300" />
</center>

*   On successful Wi-Fi connection, you will see a list of few of the voice queries that you can try with the Dev Kit.

<center>
    <img src="https://github.com/espressif/esp-va-sdk/wiki/aia_images/esp_alexa_app_things_to_try.png" alt="App Things To Try" title="App Things To Try" width="300" />
</center>

*   You are now fully setup. You can now say "Alexa" followed by the query you wish to ask.

## 4.2 Additional Device Settings

Some device settings like Volume Control, Locale Change, etc. can also be controlled through the phone app.

*   Launch the phone app, select the option *Manage devices*.

<center>
    <img src="https://github.com/espressif/esp-va-sdk/wiki/aia_images/esp_alexa_app_home.png" alt="App Home" title="App Home" width="300" />
</center>

*   Make sure you are connected to the same network as the device and also that SSDP packets can be sent on your network.
*   Now select your device from the list of devices for the device settings.

# 5. Device Performance

## 5.1 CPU and Memory usage

The following is the CPU and Memory Usage on running AVS for AWS IoT.

|                                       |Up And Running |Normal Queries |Amazon Music   |
|:-                                     |:-:            |:-:            |:-:            |
|**CPU Usage**                          |6%             |23%            |28%            |
|**Free Internal Memory (328KB DRAM)**  |113KB          |113KB          |113KB          |
|**Free External Memory (4MB PSRAM)**   |2.65MB         |2.62MB         |2.62MB         |

**Flash Usage**: Firmware binary size: 3.1MB

This should give you a good idea about the amount of CPU and free memory that is available for you to run your application's code. For the free memory, a minimum of 50KB of free internal memory is required for healthy Alexa operation.

## 5.2 User Perceived Latency (UPL)

UPL is a measure of how quickly the device responds to a query. This is measured as the time difference between the end of the query from user and the start of the response from Alexa.

This is the summary of the readings. 20 readings were taken and they were arranged in ascending order. Then they were named from T-1 to T-20 (T-1 being the best i.e. least time and T-20 being the worst i.e. most time).

|           |Measured UPL (in seconds)  |
|:-:        |:-:                        |
|**T-1**    |1.788                      |
|**T-10**   |1.866                      |
|**T-18**   |1.916                      |

# 6. Test Case Specific Instructions

These instructions are with reference to the document *Amazon AVS Functional Qualification Tests v3.2*.

## 6.1 Wi-Fi AP Change (Test Case 1.7)

This test requires us to change the Wi-Fi configuration of the device. Please refer to the Section [Button Functions](#131-buttons), for details about how the Wi-Fi configuration can be erased.

Once the Wi-Fi configuration is erased, the deivce will be back in the configuration mode. You can relaunch the phone app, and begin the configuration process as described in the Section [Configuration](#41-configuration-steps). In this case, the phone app will detect that the Alexa configuration is already done and skip those options.

Please note that this Wi-Fi only device configuration mode stays enabled only for 3 minutes. After 3 minutes, the device will reboot and connect to the already configured Wi-Fi Access Point.

## 6.2 Device Reset (Test Case 1.9)

This test requires us to reset the device to its factory state. Please refer to the Section [Button Functions](#131-buttons), for details about how the device can be reset to factory settings.

Once the device is reset, the device will be back in the configuration mode. You can relaunch the phone app, and begin the configuration process as described in the Section [Configuration](#41-configuration-steps).

## 6.3 Sign Out (Test Case 1.6)

This test requires us to sign-out of Alexa. This can be done through the companion phone app.

Ensure that the phone is within the same Wi-Fi network as the device. Launch the phone app and go to *Manage Devices*. A list of devices will be visible to you. Click on the device, and then click on Sign-out.

You can sign back in again, using the companion app using the same workflow. Ensure that the phone is within the same Wi-Fi network as the device. Launch the phone app and go to *Manage Devices*. A list of devices will be visible to you. Click on the device, and then click on Sign-in. Follow the sign-in steps to sign-in to the device.

# 7. Reusing the AWS IoT MQTT connection

The Alexa SDK uses MQTT to communicate with AWS IoT. You can use this same connection to publish or subscribe to your custom AWS topics too. This reusing of the network connection for both voice and your device control allows you to have a more compact memory utilisation.

Example 1: You can subscribe to a topic which tells the device to turn on or turn off the lights, if you are building a lightbulb.

Example 2: You can implement OTA through publish and subscribe.

## 7.1 Working with MQTT

You can refer to *esp-va-sdk/examples/amazon_alexa/main/app_aws_iot.c*. It has the skeletal code for publishing and subscribing to custom topics using the same AWS IoT MQTT connection which is being used by the Alexa SDK.

The AWS IoT agent requires that the publish/subscribe interactions with AWS IoT happens in the same thread's context. This skeletal code allows you to do this with minimum hassle.

### 7.1.1 Subscribing

To begin with, you can subscribe to the topics of your interest in the *app_aws_iot_subscribe_handler()* function. This function is only called once during initial boot-up. Typically, subscription to topics only has to happen once, so this can be easily managed from here.

You can use the AWS IoT function, *aws_iot_mqtt_subscribe()*, to subscribe to your topics of interest. The 'client' parameter that is required for this function can be fetched by making a call to *alexa_mqtt_get_client()*.

### 7.1.2 Publishing

Any updates that you wish to publish to AWS IoT can be handled in the *app_aws_iot_publish_handler()* function. If no updates need to be pushed, then this function does nothing and just returns.

You can use the AWS IoT function, *aws_iot_mqtt_publish()*, to publish to your desired topic. The 'client' parameter that is required for this function can be fetched by making a call to *alexa_mqtt_get_client()*.

### 7.1.3 Testing

You can use mosquitto, which is a MQTT broker, to test your custom publish and subscribe implementations.

You will need to install mosquitto on your Host machine. Then you can communicate with the device using the following commands:

Publish command:
```
$ mosquitto_pub --cafile server.crt --cert device.crt --key device.key -h <aws_endpoint> -p 8883 -q 1 -t <mqtt_topic> -i <device_id> --tls-version tlsv1.2 -m '<your_payload>' -d
```
Subscribe command:
```
$ mosquitto_sub --cafile server.crt --cert device.crt --key device.key -h <aws_endpoint> -p 8883 -q 1 -t <mqtt_topic> -i <device_id> --tls-version tlsv1.2 -d
```
In the commands above:

*   Add the path to the appropriate server.crt, device.crt and device.key files. You will need to create new certificates to be used or it may affect the device's AWS IoT MQTT connection.
*   Replace '<aws_endpoint\>' with your endpoint.
*   Replace '<mqtt_topic\>' with the topic you want to publish/subscribe to.
*   Replace '<device_id\>' with a unique ID for your Host.
*   Replace '<your_payload\>' with the message you want to publish.

## 7.2 Working with Thing-Shadows

You can refer to *esp-va-sdk/examples/amazon_alexa/main/app_aws_iot.c*. It has the skeletal code for registering and updating a thing using the same AWS IoT MQTT shadow connection which is being used by the Alexa SDK.

The AWS IoT agent requires that the registering and updating interactions with AWS IoT happens in the same thread's context. This skeletal code allows you to do this with minimum hassle.

Firstly make sure you are calling *ais_shadow_init()* instead of *ais_mqtt_init()* from *esp-va-sdk/examples/amazon_alexa/main/app_main.c*.

You can also change the 'thing_name' in *esp-va-sdk/examples/amazon_alexa/main/app_aws_iot.h*

### 7.2.1 Registering delta

To begin with, you can register the deltas (states) of your interest in the *app_aws_iot_register_delta()* function. This function is only called once during initial boot-up. Typically, registering of deltas only has to happen once, so this can be easily managed from here.

You can use the AWS IoT function, *aws_iot_shadow_register_delta()*, to register deltas of interest. The 'client' parameter that is required for this function can be fetched by making a call to *alexa_mqtt_get_client()*.

### 7.2.2 Reporting Update

Any updates that you wish to report to AWS IoT can be handled in the *app_aws_iot_update_handler()* function. If no updates need to be reported, then this function does nothing and just returns.

You can use the AWS IoT function:

*   *aws_iot_shadow_init_json_document()*, to initialize the data to be sent.
*   *aws_iot_shadow_add_reported()*, to add the deltas which need to be reported.
*   *aws_iot_finalize_json_document()*, to finish the data to be sent.
*   *aws_iot_shadow_update()*, to report your desired delta.

The 'client' parameter that is required for this function can be fetched
by making a call to *alexa_mqtt_get_client()*.

### 7.2.3 Testing

You can use curl to test your custom shadow implementation.

Get Details:
```
$ curl --tlsv1.2 --cacert server.crt --cert device.crt --key device.key https://<aws_endpoint>:8443/things/<thing_name>/shadow | python -mjson.tool
```
Update:
```
$ curl -d '<your_payload>' --tlsv1.2 --cacert server.crt --cert device.crt --key device.key https://<aws_endpoint>:8443/things/<thing_name>/shadow | python -mjson.tool
```
In the commands above:

*   Add the path to the appropriate server.crt, device.crt and device.key files. You will need to create new certificates to be used or it may affect the device's AWS IoT MQTT connection.
*   Replace '<aws_endpoint\>' with your endpoint.
*   Replace '<thing_name\>' with the thing_name that you want to get the details of update.
*   Replace '<your_payload\>' with the message you want to update. eg.  {"state":{"desired":{"test_thing_state":true}}}

# 8. Customising for your Board

The SDK is so architected that it is quite easy to rebuild the SDK for
your board with a minimal set of changes to the core.

*   The SDK build system takes a parameter *AUDIO_BOARD_PATH*, this parameter can be changed to build the SDK for a different board.
*   The *AUDIO_BOARD_PATH* directory should contain a file *audio_board.cmake*, that includes the correct board specific drivers for your board.
    *   These drivers typically include the LED driver, the button handling code, and the audio codec used for playback.
    *   The *audio_board.cmake* file should also include an IPC_DRV_PATH that includes the path to the DSP Driver.

## 8.1 LED

### 8.1.1 Writing your own LED Driver

You can replace the current LED driver with your own LED driver. The LEDs are driven by calling the function: *void va_led_set_pwm(const uint32_t &ast;va_led_v)* where,

*   the parameter *va_led_v* is an array of 12 uint32 values
*   each integer represents the RGB value for one LED. In this way, a maximum of 12 LEDs are supported per device
*   the RGB values can be extracted from a single uint32 with the following sample code:
    ```
    blue_led_val = 0xFF & (va_led_v[i]);
    green_led_val = 0xFF & (va_led_v[i] >> 8);
    red_led_val = 0xFF & (va_led_v[i] >> 16);
    ```

When you write your LED driver, you should take care to implement the *va_led_set_pwm()* function as described above. Once implemented it gets plugged into the SDK's LED logic.

The LED driver should be initialised within the *va_board_led_init()* function in your audio_board.

### 8.1.2 Changing the LED Pattern (Standard Patterns)

Out of the box, the SDK supports the LED patterns for configurations: single, radial_12 and linear_5. The LED patterns for Alexa are imported from Alexa's standard resource package. You can switch the LED pattern by changing the *LED_PATTERN_PATH* appropriately in your *audio_board.cmake* file.

The selected pattern will be initialised in *va_board_led_init()* by calling the *led_pattern_init()* API for that pattern and then passed to *va_led_init()* which plugs into the SDK's LED handling module.

### 8.1.3 Changing the LED Pattern (Custom Patterns)

Note: Please make sure your custom LED patterns will pass Alexa certification before embarking on this task.

You can completely define your own LED pattern.

*   One LED state is a series of 12 RGB values, along with an initial delay, and a loop-count (va_led_specs_t)
*   One event, say listening state, can have a number of these LED states (va_led_config_t) one after the other to create a pattern
*   You have to define such patterns for all the events supported in the system, listening state, speaking state, alert state, etc.

The best way to get started here is to look at the *components/audio_hal/led_pattern/linear_5/aia/led_pattern.c* that defines the pattern for 5 linear LEDs.

Once this pattern is defined, use this pattern as defined in the subsection above.

## 8.2 Button

The SDK supports buttons connected through ADC using a resistor divider circuit. The button configurations can be done in the *va_board_button_init()* function in your audio_board.

This function typically is defined as shown below:
```
esp_err_t va_board_button_init()
{
    button_cfg_t *ab_button_conf = NULL;
    ab_button_conf = (button_cfg_t *)calloc(1, sizeof(button_cfg_t));
    ab_button_conf->is_adc = true;
    ab_button_conf->va_button_adc_ch_num = ADC1_CHANNEL_3;
    ab_button_conf->va_button_adc_val[VA_BUTTON_TAP_TO_TALK] = 610;
    ab_button_conf->va_button_adc_val[VA_BUTTON_VOLUME_UP] = -1;
    ab_button_conf->va_button_adc_val[VA_BUTTON_VOLUME_DOWN] = -1;
    ab_button_conf->va_button_adc_val[VA_BUTTON_VAL_IDLE] = 3100;
    ab_button_conf->va_button_adc_val[VA_BUTTON_MIC_MUTE] = 1860;
    ab_button_conf->va_button_adc_val[VA_BUTTON_FACTORY_RST] = 520;
    ab_button_conf->va_button_adc_val[VA_BUTTON_CUSTOM_1] = -1;
    ab_button_conf->va_button_adc_val[VA_BUTTON_CUSTOM_2] = -1;
    ab_button_conf->tolerance = 45;
    va_button_init(ab_button_conf, but_cb_reg_handlr);
    return ESP_OK;
}
```

The voltage values that are observed for the various button press events are initialised in the *button_cfg_t* structure as shown above. If a button does not exist, a '-1' entry is used to indicate this to the SDK. A tolerance value is also configured. Finally this configuration is passed to the *va_button_init()* function which plugs into the SDK's button driver.

## 8.3 Audio Codec

### 8.3.1 Using Another Supported Audio Codec

Espressif has production-ready drivers for a known set of Audio Codecs that we support out of the box. Please reach out to your Espressif representative to get a list of these devices and the corresponding driver. Once you have the driver, you can rebuild the SDK by

*   Modifying your *audio_board.cmake* file to point to the appropriate audio codec driver

### 8.3.2 Writing your own Audio Codec Driver

If the audio codec, that you wish to use, is not part of Espressif's supported list. You can write a driver for it yourself. The driver primarily has to implement the function *media_hal_codec_init()*. This function should do the following things at a minimum:

*   Initialise the audio codec with the appropriate configuration. It should also setup the volume to a default volume.
*   Initialise the list of function pointers within the *media_hal* structure to point to the driver's implementation of those functions. These functions typically include functions for controlling the volume setting mute/unmute, I2S and power up/down configurations.

## 8.4 DSP Driver

### 8.4.1 Using Another DSP Driver

Espressif has production-ready drivers for a known set of DSP SoCs that we have partnered with. Please reach out to your Espressif sales representative to get access to these drivers.

Once you have the access to these drivers, you can rebuild the SDK by modifying your *audio_board.cmake* file to point to the appropriate DSP driver.

### 8.4.2 Writing your own DSP Driver

If the DSP that you wish to use is not already supported by Espressif, you can write the DSP driver yourself. A reference hollow DSP driver is available within the SDK at *components/audio_hal/dsp_driver/hollow_dsp/*. This includes all the skeletal code and the empty APIs that the DSP driver is supposed to implement to plug into the SDK.

As you write the driver, please update the *audio_board.cmake* file to point to your DSP driver instead of the original one.

# 9. Integrating other components

## 9.1 ESP RainMaker

### 9.1.1 Environment Setup

Additional setup that needs to be done for integrating [ESP RainMaker](https://rainmaker.espressif.com/):

*   Get the repository:
    ```
    $ git clone https://github.com/espressif/esp-rainmaker.git
    ```
*   Setting cloud_agent:
    ```
    $ export CLOUD_AGENT_PATH=/path/to/esp-rainmaker
    ```
*   Menuconfig changes:
    ```
    $ idf.py menuconfig
    -> Voice Assistant Configuration -> Enable cloud support -> enable this
    -> ESP RainMaker Config -> Use Assisted Claiming -> enable this
    ```

### 9.1.2 Device Provisioning

Instead of using the ESP Alexa app, Use the RainMaker apps with Alexa integration:

**iOS**: Coming soon <br>
**Android**: [Android app]()

*   Launch the app and sign-in.
*   Click on add device and scan the QR code complete the Wi-Fi setup.
*   After that you will be asked to Sign in with Amazon.
*   The phone app will verify and complete the setup after that.

### 9.1.3 Customisation

To customise your own device, you can edit the file components/app_cloud/app_cloud_rainmaker.c. You can check the examples in ESP RainMaker for some more device examples.

## 9.2 Smart Home

One way to add the smart home functionality is to use [ESP RainMaker](#91-esp-rainmaker), and the other way is to use *components/app_smart_home*. This is initialized by default in the appilication.

### 9.2.1 Usage

Once provisioning is done and the device has booted up, the smart home feature of the device can be used via voice commands or through the Alexa app.

Example: By default, the device configured is a 'Light' with 'Power' and 'Brightness' functionalities. Voice commands like 'Turn on the Light' or 'Change Light Brightness to 50' can be used. In the Alexa app, this device will show up as 'Light' and the Power and Brightness can be controlled.

### 9.2.2 Customisation

To customise your own device, you can edit the file components/app_smart_home/app_smart_home.c. You can refer the files *components/voice_assistant/include/smart_home.h* and *components/voice_assistant/include/alexa_smart_home.h* for additional APIs.

A device can have the following types of capabilities/features/parameters:
*   Power: A device can only have a single power param.
*   Toggle: This can be used for params which can be toggled. Example: Turning on/off the swinging of the blades in an air conditioner.
*   Range: This can be used for params which can have a range of values. Example: Changing the brightness of a light.
*   Mode: This can be used for params which need to be selected from a pre-defined set of strings. Example: Selecting the modes of a washing machine.

## 9.3 Audio Player

The audio player (*components/voice_assistant/include/audio_player.h*) can be used to play custom audio files from any source (http url, local spiffs, etc.).

The focus management (what is currently being played) is already implemented internally by the SDK. Speech/Alert/Music from Alexa has higher priority than what is played via the audio_player. So for example, if custom music is being played via the audio_player, and a query is asked, then the music will be paused and the response from Alexa will be played. Once the response is over, the music will be resumed (unless already stopped). Basically, all Alexa audio gets priority over custom audio.

### 9.3.1 Enabling Custom Player

The *components/custom player* is an example using the audio_player. The default example of the custom player can play from http url and/or local spiffs and/or local sdcard but can be easily extended to play from any other source.

Easiest way to try custom player is using http url.

*   Include *custom_player.h* in the application and call *custom_player_init()* after the voice assistant early initialisation has been done.

When the application is now built and flashed on the device, the custom player will play the 3 files showing the usage of the audio_player.

### 9.3.2 Customisation

The default custom player just has a demo code which can be used as a reference to build your own player. The audio player, for now, just supports mp3 and aac audio formats for http urls and only mp3 audio format for local files.

# 10. Production Considerations

## 10.1 Over-the-air Updates (OTA)

ESP-IDF has a component for OTA from any URL. More information and details about implementing can be found here: [esp_https_ota](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/esp_https_ota.html#esp-https-ota).

## 10.2 Manufacturing

### 10.2.1 Mass Manufacturing Utility

AWS IoT operations require that all devices have a unique certificate and key pair programmed on each device. This is used for authentication with the AWS IoT cloud service. These are generally programmed in factory NVS partitions that are unique per device.

ESP-IDF provides a utility to create instances of factory NVS partition images on a per-device basis for mass manufacturing purposes. The NVS partition images are created from CSV files containing user-provided configurations and values.

Details about using the mass manufacturing utility can be found here: [mass_manufacturing](https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/storage/mass_mfg.html).

### 10.2.2 Pre-Provisioned Modules

ESP32 modules can be pre-flashed with private key and device certificates (i.e the factory NVS partition) during manufacturing itself and then be shipped to you. These device certificates are signed by your Certificate Authority (CA). When you register this CA in your cloud, all the devices can connect to the cloud, out of the box. This saves you the overhead of securely generating, encrypting and then programming the certificates into the device at your end. Pre-provisioning is an optional service which Espressif provides.

Please contact your Espressif contact person for more information.

## 10.3 Security

### 10.3.1 Secure Boot

Secure boot ensures that only trusted code runs on the device.

ESP32 supports RSA based secure boot scheme whereby the bootROM verifies the software boot loader for authenticity using the RSA algorithm. The verified software boot loader then checks the partition table and verifies the active application firmware and then boots it.

Details about implementing the secure boot can be found here: [secure_boot](https://docs.espressif.com/projects/esp-idf/en/latest/security/secure-boot.html).

### 10.3.2 Flash Encryption

Flash encryption prevents the plain-text reading of the flash contents.

ESP32 supports AES-256 based flash encryption scheme. The ESP32 flash controller has an ability to access the flash contents encrypted with a key and place them in the cache after decryption. It also has ability to allow to write the data to the flash by encrypting it. Both the read/write encryption operations happen transparently.

Details about implementing the flash encryption can be found here: [flash_encryption](https://docs.espressif.com/projects/esp-idf/en/latest/security/flash-encryption.html).

### 10.3.3 NVS Encryption

For the manufacturing data that needs to be stored on the device in the NVS format, ESP-IDF provides the NVS image creation utility which allows the encryption of NVS partition on the host using a randomly generated (per device unique) or pre-generated (common for a batch) NVS encryption key.

A separate flash partition is used for storing the NVS encryption keys. This flash partition is then encrypted using flash encryption. So, flash encryption becomes a mandatory feature to secure the NVS encryption keys.

Details about implementing the NVS encryption can be found here: [nvs_encryption](https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/storage/nvs_flash.html#nvs-encryption).

# A1 Appendix FAQs

## A1.1 Compilation errors

I cannot build the application:

*   Make sure you are on the correct esp-idf branch. Run 'git submodule update ---init ---recursive' to make sure the submodules are at the correct heads
*   Make sure you have the correct AUDIO_BOARD_PATH selected for your board.
*   Delete the build/ directory and also sdkconfig and sdkconfig.old and then build again.
*   If you are still facing issues, reproduce the issue on the default example and then contact Espressif for help. Please make sure to share these:
    *   The esp-va-sdk and esp-idf branch you are using and the AUDIO_BOARD_PATH that you have set.
    *   The complete build logs.

## A1.2 Device setup using the Mobile app

I cannot *Add a new device* through the phone app:

*   If the device is not being shown while Adding a new device, make sure the Required permissions are given to the app. Also make sure that your Bluetooth is turned on.
*   Android typically requires the *Location* permission also for enabling Bluetooth.
*   If you are still facing issues, update the app to the latest version and try again.
*   Force closing the app and rebooting the device works in most cases if either of them have gone into an unknown state.
*   If you are still facing issues, reproduce the issue on the default example for the device and then contact Espressif for help. Make sure to share these:
    *   Screenshots of the mobile app where it is not working.
    *   Mobile App version.
    *   Mobile Phone model and the Android version or any skin it is running.
    *   Complete device logs taken over UART.
    *   The esp-va-sdk and esp-idf branch you are using and the AUDIO_BOARD_PATH that you have set.

I cannot *Manage device* through the phone app:

*   If the device is not being shown while Managing devices, make sure you are connected to the same network as the device.
*   If you are still facing issues, update the app to the latest version and try again.
*   Force closing the app and rebooting the device works in most cases if either of them have gone into an unknown state.
*   If you are still facing issues, reproduce the issue on the default example for the device and then contact Espressif for help. Make sure to share these:
    *   Screenshots of the mobile app where it is not working.
    *   Mobile App version.
    *   Mobile Phone model and the Android version or any skin it is running.
    *   Complete device logs taken over UART.
    *   The esp-va-sdk and esp-idf branch you are using and the AUDIO_BOARD_PATH that you have set.

## A1.3 Device crashing

My device is crashing:

*   Given the tight footprint requirements of the device, please make sure any issues in your code have been ruled out. If you believe the issue is with the Alexa SDK itself, please recreate the issue on the default example application (without any changes) and go through the following steps:
*   Make sure you are on the correct esp-idf branch. Run 'git submodule update ---init ---recursive' to make sure the submodules are at the correct heads.
*   Make sure you have the correct AUDIO_BOARD_PATH selected for your board.
*   Delete the build/ directory and also sdkconfig and sdkconfig.old and then build and flash again.
*   If you are still facing issues, reproduce the issue on the default example for the device and then contact Espressif for help. Make sure to share these:
    *   The steps you followed to reproduce the issue.
    *   Complete device logs (from device boot-up) taken over UART.
    *   <voice_assistant>.elf file from the build/ directory.
    *   If you have gdb enabled, run the command 'backtrace' and share the output of gdb too.
    *   The esp-va-sdk and esp-idf branch you are using and the AUDIO_BOARD_PATH that you have set.

## A1.4 Device not crashed but not responding

My device is not responding to audio queries:

*   Make sure your device is connected to the Wi-Fi/Internet.
*   If the device is not taking the wake-word, make sure the mic is turned on.
*   Try using the Push-To-Talk button and then ask the query.
*   If you are still facing issues, reproduce the issue on the default example for the device and then contact Espressif for help. Make sure to share these:
    *   The steps you followed to reproduce the issue.
    *   Complete device logs taken over UART.
    *   <voice_assistant>.elf file from the build/ directory.
    *   The esp-va-sdk and esp-idf branch you are using and the AUDIO_BOARD_PATH that you have set.

## A1.5 Migrating to your own AWS account

Where does it ask to link the product ID with the AWS Account?

*   While creating a new product, you will be asked 'Is this device associated with one or more AWS IoT Core Accounts?'. When you select 'Yes', you will be asked to enter the AWS Account ID 'Please provide your AWS Account ID(s) (comma separated)?'.

How to change the phone app for the new product ID?

*   For the documentation on Android and iOS apps, you can refer: [Android](https://github.com/espressif/esp-idf-provisioning-android/tree/versions/avs-ble) and [iOS](https://github.com/espressif/esp-idf-provisioning-ios/tree/versions/avs-ble).

My device is not connecting to AWS.

*   Make sure the device is activated. You can check this here: Go to IoT Core -> Manage -> Select your device -> Security -> Click on the certificate -> Actions -> Activate.
*   Make sure the correct policy is attached with the certificate. You can check this here: Go to IoT Core -> Manage -> Select your device -> Security -> Click on the certificate -> Policies -> Check the policy.

While booting, the device shows '{"code": "INVALID_AWS_ACCOUNT", "description": "The AWS IoT account provided does not support AIS"}' and keeps on booting.

*   Make sure the 'Registration Request Buffer' has the correct parameters.
*   The authentication token has been generated with the new product ID.
*   The device id i.e. the client ID is unique.
*   The AWS Account ID and Endpoint are correct.
*   If all of these are correct, try doing factory reset and try to setup the device again.
