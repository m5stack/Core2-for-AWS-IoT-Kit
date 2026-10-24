# ESP_CRYPTOAUTH_UTILITY

## Supported Targets

| Supported Target Board | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C5 | ESP32-C6 | ESP32-C61 | ESP32-H2 | ESP32-P4 | ESP32-S3 |
| ---------------------- | ----- | -------- | -------- | -------- | -------- | --------- | -------- | -------- | -------- |

## Description
 The python utility helps to configure and provision [ATECC608A](https://www.microchip.com/en-us/product/atecc608a)/[ATECC608B](https://www.microchip.com/en-us/product/atecc608b) chip connected to a Supported Target Board.
 
 There are currently three types of ATECC608 which are [Trust & Go](https://www.microchip.com/wwwproducts/en/ATECC608A-TNGTLS), [TrustFlex](https://www.microchip.com/wwwproducts/en/ATECC608A-TFLXTLS) and [TrustCustom](https://www.microchip.com/wwwproducts/en/ATECC608A). `Trust & Go` and `TrustFlex` chips are preconfigured by the manufacturer (Microchip) so we only need to generate manifest file for those chips. `TrustCustom` type of chips are not configured, so for `TrustCustom` type of chips need to be first configured and then provisioned with a newly  generated device certificate and key pair. The script automatically detects which type of ATECC608 chip is connected to the ESP module so it will proceed to the next required step on its own.

# Hardware Required

* One Supported Target Board (see Supported Targets table above)
* An [ATECC608A](https://www.microchip.com/en-us/product/atecc608a)/[ATECC608B](https://www.microchip.com/en-us/product/atecc608b) connected with the target board using I2C interface.

## Installation
The `esp_cryptoauth_utility` that helps configure the ATECC608 module can be installed with the following command:

``` sh
pip install esp-cryptoauth-utility
```

## Step 1:- Generate Signer Certificate

Signer cert and key pair:
* In case of `TrustCustom` chips ,these certificate and key pair are used to sign the device cert which is going to be generated.

* In case of `Trust & Go` and `TrustFlex` devices the device certs are already signed by microchip signer cert, and the signer cert and key pair generated in this step are used to sign the manifest file.

By default the utility uses the `sample_signer_cert.pem` located in the `sample_certs` directory. If you want to keep using default certificates, then directly proceed to next step (Step 2).

Create a signer key and signer cert by executing following commands sequentially. The second command will ask some details about certificate such as `ORG, CN` which are needed to be filled by the user.

 `Important`: The signer cert `CN`_(Common Name)_ must end with `FFFF` as it is required by the `cert2certdef.py` (file by microchip) to create its definition properly. For example, valid CN = `Sample Signer FFFF` (This is compulsory only in case of `TrustCustom` type of chips and not for the other two).

```bash
openssl ecparam -out signerkey.pem -name prime256v1 -genkey
openssl req -new -x509 -key signerkey.pem -out signercert.pem -days 365
```

## Step 2:- Provision the module/Generate manifest file

### 1) Workflow
*   The tool will automatically detect the type of ATECC608 chip connected to ESP module and perform its intended task which are as follows.

    * For `TrustCustom` type of ATECC608 chip first configure ATECC608 chip with its default configuration options.The tool will create a device cert by generating a private key on slot 0 of the module, passing the CSR to host, sign the CSR with signer cert generated in step above. To set validity of device cert please refer [device_cert_validity](README.md#set-validity-of-device-cert-for-trustcustom). Save the device cert on the ATECC chip as well as on the host machine as `device_cert.pem`, it also saves the cert definitions in `output_files` directory for future use.

    * For `Trust & Go` and `TrustFlex` type of ATECC608 devices this script will generate the manifest file with the name of chip serial number. The generated manifest file can be registered with the cloud to register the device certificate.

---
### 2) Provide I2C pin configuration
The I2C pins of the ESP32 to which ATECC608 chip is connected can be provided as a parameter to the python script. The command option to be given is as follows:
```
python secure_cert_mfg.py --i2c-sda-pin /* SDA pin no */ --i2c-scl-pin /* SCL pin no */ /* + other options */
```

When no pin configurations are provided to the script, by default SDA=21, SCL=22 will be used for the I2C configuration of ATECC608A.

### 3) Execute the script

The final command to be executed is as follows:

```
python secure_cert_mfg.py --signer-cert signercert.pem --signer-cert-private-key signerkey.pem --port /UART/COM/PORT --i2c-sda-pin /* SDA pin no */ --i2c-scl-pin /* SCL pin no */
```

> Note: The names `signercert.pem` and `signerkey.pem` denote the name of the signer cert and key files respectively, you can replace them with `relative/path/to/you/signer/cert` and `key` respectively. The `UART/COM/PORT` represents the host machine COM port to which your ESP32 is connected. Please refer [check serial port](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/establish-serial-connection.html#check-port-on-windows) for obtaining the serial port connected to ESP.

If you do not provide `signer-cert` and `signer-cert-private-key` in above command, `sample_signer_cert.pem` stored at `sample_certs` will be used.


## Additional options supported by the script

### 1) Find type of ATECC608 chip connected to ESP module.

The command is as follows:
```
python secure_cert_mfg.py --port /serial/port --type + /* Other options */
```
It will print the type of ATECC608 chip connected to the ESP module on console.

### 2) Set validity of device cert for TrustCustom
The validity (in years) of device certificate generated for `TrustCustom` chips from the time of generation of cert can be set with `--valid-for-years` option. Please refer the following command:
### Using ATECC608A with ESP32-WROOM-32
`ATECC608A` uses `I2C` interface for serial communication so you will have to make required `I2C` connections such as `SCK, SDA, GND, VDD` between `ESP` and `ATECC608A`.
* If you are using [ESP32-DevKitC](https://www.espressif.com/en/products/devkits/esp32-devkitc/resources) and connected ATECC608A to pins 21 (SDA), and 22(SCK) then you can directly use a prebuilt binary. The command is
```
python secure_cert_mfg.py --port /serial/port --valid-for-years /Years + /* Other options */
```

>Note: If `--valid-for-years` is not provided then default value for validity of certiticates will be used, which is 40 years.


See below options [building the firmware](./README.md#building-the-firmware-for-esp_cryptoauth_utility) for more details.

### Building the firmware for "esp_cryptoauth_utility"
When the secure_element (ATECC608A) is connected to I2C pins different than the pins used in sample binaries provided, then the users needs to build the firmware by selecting respective I2C pins

The details on how to build the firmware are provided in the [firmware/README](firmware/README.md).
