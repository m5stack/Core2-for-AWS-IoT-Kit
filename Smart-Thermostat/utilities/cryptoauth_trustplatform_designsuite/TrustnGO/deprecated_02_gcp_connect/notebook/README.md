## Objective
Trust&GO device comes with pre-programmed configuration and MCHP certificates loaded into device.

This folder contains gcp-iot notebook to perform manifest registration and running gcp connect gui.

## Prerequisites
   - Refer to parent folder README.md files for tools, installations, sequences and hardware etc..,
   - Generate Manifest file for secure element using TNGTLS_Manifest_Generation\notebooks\TNGTLS Manifest File Generation.ipynb

## Package
 - gcp-iot with ECC608A-TNGTLS.ipynb
This is a Jupyter notebook to upload secure element manifest file to Google account 

Follow below steps,
 - Connect Crypto Trust Platform to PC/Laptop
 - Open gcp-iot with ECC608A-TNGTLS.ipynb notebook
 - Update Google account registry, region and .json file path details under "Configure GCP account credentials"
 - Run all the cells... Read through the cells to Upload Manifest and Validation Certificates using Upload buttons
 - Run the C-example
 - Click gcp_gui button to see the messages from TrustPlatform hardware
 

