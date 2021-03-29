# ESP Alexa Certificates and Configuration

Place the certificates you received from Espressif to use their sandbox account to try Alexa features on the M5Stack Core2 for AWS IoT EduKit. Files required:
- account.info
- device.cert
- device.info
- device.key
- endpoint.info
- server.cert

Do not create additional directories, just save each of the attached files into this folder. There is also no need to modify the mfg_config.csv file as that's used to create the flash partition and preset to use the filenames and the path within this folder. After running the included script, a mfg.bin file will be generated with the contents of the files and then that binary is flashed onto the device non-volitile storage (nvs).