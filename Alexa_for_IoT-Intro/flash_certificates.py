import os 

Import("env")

platform = env.PioPlatform()
board = env.BoardConfig()

env.AddCustomTarget(
    name="flash_certificates",
    dependencies=None,
    actions=[
        env.VerboseAction(env.AutodetectUploadPort, "Looking for upload port..."),
        '"$PYTHONEXE" "%s" generate esp_alexa_credentials/mfg_config.csv esp_alexa_credentials/mfg.bin 0x6000'
        % (
            os.path.join(platform.get_package_dir("framework-espidf") or "", "components", "nvs_flash", "nvs_partition_generator", "nvs_partition_gen.py"),
        ),
        '"$PYTHONEXE" "%s" --chip %s --port "$UPLOAD_PORT" write_flash %s %s'
        % (
            os.path.join(platform.get_package_dir("tool-esptoolpy") or "", "esptool.py"),
            board.get("build.mcu", "esp32"),
            "0x10000",
            "esp_alexa_credentials/mfg.bin"
        )
    ],
    title="Flash Certificates",
    description="Upload Alexa provision certificates to the target board",
)