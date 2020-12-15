[Anaconda]:Anaconda_Navigator_Overview.png
[Jupyter]:Jupyter_FileSystem.png
[ResourceGen]:Resource_Generator.png
# Getting started with Jupyter Notebooks

## 1. Installation

The simplest way to install Jupyter Notebooks is as part of the anaconda distribution.
[Download Anaconda](https://www.anaconda.com)
Make sure to download a version that includes Pythin 3.x series. There are packages for Windows, MacOS and Linux

Once installed, start the Anaconda Navigator that just got installed.

## 2. Launching Jupyter Notebook
In the Anaconda Navigator, click on __Launch__ in the Jupyter Notebook application badgge as highlighted below.


![][Anaconda]

Anaconda will start a Jupyter kernel and a local web server in the background. A web page displaying the filesystem similar to the one below will automatically open in your default browser.


![][Jupyter]

### 2.1 Resource Generator Notebook

In the Jupyter Filesystem page, navidate to the ```Trust Platform Design Suite\TFLXTLS_resource_generation``` folder and select ```TFLXTLS Resource Generator.ipynb``` notebook. The selected notebook will open in a new browser window.
It is good practice to restart the jupyter kernel and clear any output from previous sessions. Select __Kernel__ in the top menu followed by __Restart and Clear Output__


![][ResourceGen]

The resource generator will build a collection of development keys and a custom Public Key infrastructure (PKI) chain for prototyping purposes. The keys are saved in PEM and C array format directly on the PC filesystem in the same folder as the Resource Generator Notebook.
Keys and certificates are generated for each updatable slot of the TrustFLEX device.
These are:

#### Keys

* __slot_5_secret_key.pem__
* __slot_5_secret_key.h__
* __slot_6_secret_key.pem__
* __slot_6_secret_key.h__
* __slot_9_secret_key.pem__
* __slot_9_secret_key.h__
* __slot_13_ecc_key_pair.pem__
* __slot_14_ecc_key_pair.pem__
* __slot_15_ecc_key_pair.pem__

<font color="red">WARNING!!</font> These keys should never be used for anything but prototyping purposes. They should be replaced by adequately protected keys before going to production

### Custom PKI development certificate templates

* __cust_def_1_signer.c__
* __cust_def_1_signer.h__
* __cust_def_2_device.c__
* __cust_def_2_device.h__

### Custom PKI Certificates and Keys

* __root-ca.crt__  This is a root CA self signed certificate for development purposes only.
* __root-ca.key__ This is the root CA private key. <font color="red">Never use any certificate that signed by this key for production </font>
* __signer-ca.crt__ This is the Issuing CA certificate (signed by the root CA key)
* __signer-ca.key__ THis is the Issuing CA signing key. <font color="red">Never use any certificate signed by this key for production </font>
* __device_cert.crt__ THis is the device certificate (signed by the signer ca). The device public key used in this certificate is the one corresponding to the private key in TrustFLEX slot 0.

The Custom PKI files are C templates need to be added to your C project along with CryptoAuthLib to correctly reconstruct the Custom PKI X.509 certificates.




These secrets are automatically saved into the TrustFLEX device selected by the switch combination on the USB Prototyping Dongle. They will be automatically used by all subsequent notebook tutorial.


### 2.2 Use Case Notebook Tutorials

Each TrustFLEX Use Case can be tested with a separate Notebook tutorial. Notebook tutorials allow you to run complete use cases using your PC as a host in the system without the need for developing __any__ embeeded C code.

The tool collection ships with the following supported Use Cases:

* Firmware validation
* Secure Public Key Rotation
* IP Protection
* Disposable / Accessory Authentication
* Custom PKI

Once you have generated a colection of development keys & certificates, within Jupyter filesystem explorer, navigate to ```usecasetool\TFLXTLS_examples\notebooks```. each use case Notebook Tutorial is stored along with its helper functions in a subfolder. Open the Notebook Tutorial of your choice and run it step by step to verify that the TrustFLEX device is correctly provisioned with the development keys.

Once the Notebook tutorials execute flawlessly, you can start migrating to the embedded C environment to replicate the use cases on the SAMD21 MCU present on the USB Prototyping Dongle.

### 2.3 Use Case C examples

Each TrustFLEX Use Case comes with a C reference implementation that corresponds to the Host / ECC608A transactions exercised in the Notebook Tutorials. A careful study of the Notebook tutorial steps and the C code reference implementation will highlight that both execute the same transactions.

If your design requires to combine multiple use cases, you will need to manually combine the C reference code into a single project.

The C reference code implementations are located in separate subfolders under ```usecasetool\TFLXTLS_examples\c```.
Each implementation file must be incorporated to a project of your favorite IDE and modified to fit the specific application needs along with cryptoauthlib library source code.


## 3. Secret Exchange files generator for prodution

Once the application is successfully tested with an MCU as a host (e.g. SAMD21 of the USB Prototyping Dongle) you can easily generate the production files required to provision devices in any volume by Microchip.
For this step, follow the instructions of the following page: <a href="../../../TrustFlex_Configurator.html" target="_blank">Secret Exchange files Generator</a>