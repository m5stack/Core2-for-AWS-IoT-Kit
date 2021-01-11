# NOTE
The file structure of the [Microchip Cryptoauth Trust Platform Design Suite](https://github.com/MicrochipTech/cryptoauth_trustplatform_designsuite/) has been modified to reduce the file path lengths which cause errors for Windows users. As such, the documentation will not match. Please visit the official repo: https://github.com/MicrochipTech/cryptoauth_trustplatform_designsuite/

# Collection of Tools and Examples for TrustPlatform

## TrustPlatform Product Page
The Trust Platform for CryptoAuthentication™ is a combination of multiple elements to simplify the addition of security to any embedded design.
 - Microchip Weblink - [TrustPlatform](https://www.microchip.com/design-centers/security-ics/trust-platform)
 - GitHub Wiki link - [DesignSuite-Wiki](https://github.com/MicrochipTech/cryptoauth_trustplatform_designsuite/wiki/Welcome)

## Objective
This package provides required Documentation and Tools to accelerate the prototyping of the most common use cases.

## Package
start_here.html is the starting point to navigate through the details of Trust Platform for CryptoAuthentication devices. This provides details about Trust&GO, TrustFLEX and TrustCUSTOM family devices, training videos, methodology and Configurators for generating provisioning XML.

For ease of understanding follow the below sequence
1. Refer to start_here.html to get highlevel information
2. Generate assets using resource generation notebooks
3. Run the usecase examples provided both in Notebook and Embedded projects

## Release Notes
 - 20201103
   - Secureboot persistent latch bug in TrustFLEX configurator is fixed

 - 20201016
   - Updated Design Suite to support ECC608B
   - All usecase embedded projects are updated to use CAL3.2.3
   - Moved MCHP cert storing to User folder to retain across repo upgrades

 - 20200704
   - Single cloud connect project for Multiple platforms
   - Azure cloud connect using TNGTLS
   - All projects updated to MPLAB H3 CAL component
   - Ability to use cached root and signer certificates
   - H3 based WINC1500 drivers for cloud connect projects
   - Other minor enhancements and bug fixes

 - 1.3.2
   - Fixed AWS subprocess bug
   - Updated Azure code to work with latest Azure python module
   - Fixed GUI webbrowser bug
   - Other minor changes and bug fixes

 - 1.3.1
   - Updated Azure code to use Common name as serial number
   - Updated resource generation code, now cleans up programming logs
   - Updated GUI to open links in Chrome if available
   - GUI lockfile bug fixed
   - Other minor changes and bug fixes

 - 1.3.0
   - Added TrustPlatform GUI, the repo can be pulled directly from GUI
   - Added asymmetric authentication usecase
   - Added support for devices with the new common name
   - Updated HTML content for asymmetric authentication usecase
   - Updated HTML, JS content for man id, common name and org name.
   - trustplatform.config file is now used to store settings instead of settings.json
   - Other minor changes and minor bug fixes

 - 1.2.4
   - Fixed MPLAB project loading errors
   - Added build configurations for all projects
   - Fixed python upgrade bug
   - Added feature to program hex through ipecmd(MPLAB IPE)
   - Created internal module to add all common code
   - Enabled provision to enter subscription id for Azure cloud example
   - Other minor bug fixes

 - 1.2.3
   - Fixed subprocess python bug which affected MAC
   - Other minor bug fixes

 - 1.2.2
   - Fixed broken HTML links
   - Other minor bug fixes

 - 1.2.1
   - Updated signer name to include organization name.
   - Other minor updates

 - 1.2.0
   - Reorganized the folders for ease of access
   - Azure and Google Connect usecases for TFLXTLS device
   - AWS and GCP Connect usecases for TNGTLS device
   - Revisited Notebooks for improved content and flow
   - GUI migrated to PyQt
   - Other minor updates

 - 1.1.0
   - Added Google Cloud Connection Usecase
   - Updated Firmware Validation Usecase to use an application instead of example bin
   - Other minor edits to overall content

 - 1.0.1
    - Initial version of TrustPlatform DesignSuite