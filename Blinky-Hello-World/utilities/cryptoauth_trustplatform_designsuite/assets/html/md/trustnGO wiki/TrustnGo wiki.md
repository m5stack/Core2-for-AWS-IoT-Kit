[Anaconda]:Anaconda_Navigator_Overview.png
[Jupyter]:Jupyter_FileSystem.png
[ManifestGen]:TNGTLS_Manifest_File_Generation.png
# Getting started with Trust&amp;GO Manifest files

## 1. Retrieve the Trust&amp;GO Manifest file

### 1.1. Purchased directly from Microchip website

When Trust&amp;GO devices ship, the manifest file becomes available for download from the microchip Purchasing & Client Services account.
You just need to log back into the account from which the order has been placed and click "Download Manifest".

### 1.2. Purchased through a Microchip distributor

It is the responsibility of the distributor to deliver the manifest file when they ship the Trust&amp;GO products. Please contact your distributor directly if you can't locate your device's Manifest.

## 2. Rebuild the Manifest from Trust&amp;GO devices

The Manifest contains only public information that is used to register the devices with the individual cloud services.
Therefore, it is possible to build the Manifest of a given collection of Trust&amp;GO devices by reading out some of their public information.
For that purpose, we have developed a Jupyter Notebook to build Manifests. This is only intended for the situations where the Manifest was not delivered or for certain kits which do not necessarily come with a Manifest.


## 1. Jupyter Notebook Installation

The simplest way to install Jupyter Notebooks is as part of the anaconda distribution.
[Download Anaconda](https://www.anaconda.com)
Make sure to download a version that includes Pythin 3.x series. There are packages for Windows, MacOS and Linux

Once installed, start the Anaconda Navigator that just got installed.

## 2. Launching Jupyter Notebook
In the Anaconda Navigator, click on __Launch__ in the Jupyter Notebook application badgge as highlighted below.


![][Anaconda]

Anaconda will start a Jupyter kernel and a local web server in the background. A web page displaying the filesystem similar to the one below will automatically open in your default browser.


![][Jupyter]

### 2. Trust&amp;GO Manifest Notebook Tutorial

In the Jupyter Filesystem page, navidate to the ```Trust Platform Design Suite\TNGTLS_Manifest_Generation\noptebooks``` folder and select ```TNGTLS_Manifest File Generation.ipynb``` notebook. The selected notebook will open in a new browser window.

In the Jupyter top menu, select "Kernel & Run All"
The Manifest file will be generated directly in the folder containing the Notebook.

![][ManifestGen]

