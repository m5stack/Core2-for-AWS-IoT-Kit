# M5Stack Core2 for AWS IoT Kit Board Support Package (BSP)
This repository contains the drivers specific to the [M5Stack Core2 for AWS IoT Kit](https://m5stack.com/products/m5stack-core2-esp32-iot-development-kit-for-aws-iot-edukit) reference Hardware. This BSP is used in the microcontroller tutorials presented in the [AWS IoT Kit](https://aws.amazon.com/iot/edukit) program.

## Cloning
This repo uses [Git Submodules](https://git-scm.com/book/en/v2/Git-Tools-Submodules) to bring in dependent components.

Note: If you download the ZIP file provided by GitHub UI, you will not get the contents of the submodules. Since the downloaded zip will also not be a git repository, you will not be able to compile the code since that is a toolchain requirement. You must clone the repository using the instructions below.

If using Windows, because this repository and its submodules contain symbolic links, set `core.symlinks` to true with the following command:
```
git config --global core.symlinks true
```
In addition to this, either enable [Developer Mode](https://docs.microsoft.com/en-us/windows/apps/get-started/enable-your-device-for-development) or, whenever using a git command that writes to the system (e.g. `git pull`, `git clone`, and `git submodule update --init --recursive`), use a console elevated as administrator so that git can properly create symbolic links for this repository. Otherwise, symbolic links will be written as normal files with the symbolic links' paths in them as text. [This](https://blogs.windows.com/windowsdeveloper/2016/12/02/symlinks-windows-10/) gives more explanation.

To clone using HTTPS:
```
git clone -b BSP-dev https://github.com/m5stack/Core2-for-AWS-IoT-Kit.git --recurse-submodules
```
Using SSH:
```
git clone -b BSP-dev git@github.com:m5stack/Core2-for-AWS-IoT-Kit.git --recurse-submodules
```

If you have downloaded the repo without using the `--recurse-submodules` argument, you need to run:
```
git submodule update --init --recursive
```

## Usage
It is recommended to use the [project template](https://github.com/m5stack/Project_Template-Core2_for_AWS) instead of the BSP directly. The GitHub project template repository contains all the necessary external dependencies and configuration to work properly with the Core2 for AWS IoT Kit. The BSP is tested for compatibility with [ESP-IDF v4.4.3](https://www.espressif.com/en/products/sdks/esp-idf) or [PlatformIO](https://github.com/platformio/platform-espressif32/tree/v3.5.0) v3.5.0. Please ensure that your installation of PlatformIO is updated to the latest version of PlatformIO Core (5.2.4+) using the command `pio upgrade` from the PlatformIO terminal window. Follow the [AWS IoT Kit — Getting Started](https://aws-iot-kit-docs.m5stack.com/en/getting-started/kit) tutorial for instructions on how to setup your environment.

We also have code examples, drivers, or content available in other frameworks:
- [Arduino](https://github.com/m5stack/aws-iot-kit-examples/tree/main/Basic_Arduino)
- [UIFlow](https://docs.m5stack.com/en/quick_start/core2_for_aws/uiflow)
- [MicroPython](https://github.com/m5stack/Core2forAWS-MicroPython)

## Support
To get support with AWS IoT Kit, post your question in the [content repo's discussions](https://github.com/m5stack/aws-iot-kit-tutorials/discussions).
For issues with the AWS IoT Kit this repo, please [submit an issue](https://github.com/m5stack/Core2-for-AWS-IoT-Kit/issues) to this repository.