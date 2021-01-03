# Set up project config
1. idf.py menu config
    1. Enable the following settings:
        1. components/mbedtls/enable hardware ECDSA sign/verify (Not necessary)
        1. components/esp-cryptoauthlib/
            1. Choose type of ecc608a chip -> TrustCustom
            1. Enable Hardware ECDSA keys for mbedTLS
            1. Enable ATECC608A sign operations for mbedTLS
            1. Enable ATECC608A verify operations for mbedTLS
        1. components/AWS IoT platform
            1. Enable Use hardware secure element for authenticating TLS connections
            1. Specify AWS IoT Endpoint
        1. Example configuration/
            1. set WiFi SSID
            1. set WiFi password
            1. set AWS IoT Client ID (This should be the same as the ECC608A serial number ! )


# Provision ATECC608A
1. cd components/esp-cryptoauthlib/esp_cryptoauth_utility
1. python secure_cert_mfg.py --signer-cert signercert.pem --signer-cert-private-key signerkey.pem --port
    1. Note for this, you don't need to specify the signer certs, and it will reuse the default ones, but it's probably best to create your own.
    1. If you want to make your own:
        1. `openssl ecparam -out signerkey.pem -name prime256v1 -genkey`
        1. `openssl req -new -x509 -key signerkey.pem -out signercert.pem -days 365`
# JITP with AWS IoT Core
CLI Instructions [here](https://github.com/aws/amazon-freertos/tree/master/vendors/microchip/secure_elements/app/example_trust_chain_tool). Note I am too lazy to rewrite these for this demo, but it should lead to how to do it.
Alternatively go through the GUI on the AWS console. 

##Just make sure the following is done
1. Create thing named the same as the ECC608A serial.
1. Upload **signer** cert as a CA.
1. Upload **device** cert as a regular certificate. This was output by the `secure_cert_mfg.py` script. It will be located in `output_files` as `device_cert.pem`. It also gets printed by the stdout when running the script.
1. Activate the certs.
1. Attach policies to certs (I used blanket access to all IoT stuff).
1. Attach device cert to thing.

# Modify code
I have prepared a patch that completes all the code changes. The only step is to swap the contents of the variable `device_cert` in **subscribe_publish_sample.c** with the contents of the device cert created when you provisioned your device.

Apply them with `git apply patch` in the proper git repository. One for **esp-aws-iot** and the other for **esp-cryptoauthlib**.

The code changes summarized:
* Export atcab_basic include path
* Initializ ecc608a at startup
* point device cert to hard coded certificate string
* Initialize MQTT parameters so they use atca stack
