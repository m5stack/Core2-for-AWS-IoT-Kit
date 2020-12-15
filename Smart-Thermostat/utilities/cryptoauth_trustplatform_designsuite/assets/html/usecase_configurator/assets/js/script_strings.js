

// When the user clicks on the button, scroll to the top of the document
function topFunction() {
    document.body.scrollTop = 0; // For Safari
    document.documentElement.scrollTop = 0; // For Chrome, Firefox, IE and Opera
}

function toggle(id)
{
    var element = document.getElementById(id);
    element.style.display = (element.style.display == 'block') ? "none" : "block";
}

var script_license = "\
# \\page License \n\
# © 2019 Microchip Technology Inc. and its subsidiaries.\n\
# Subject to your compliance with these terms, you may use Microchip software and \n\
# any derivatives exclusively with Microchip products. It is your responsibility to \n\
# comply with third party license terms applicable to your use of third party software \n\
# (including open source software) that may accompany Microchip software.\n\
# \n\
# THIS SOFTWARE IS SUPPLIED BY MICROCHIP \"AS IS\". NO WARRANTIES, WHETHER EXPRESS, IMPLIED \n\
# OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT,\n\
# MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE LIABLE \n\
# FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE \n\
# OF ANY KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN \n\
# ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW,\n\
# MICROCHIP\'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED \n\
# THE AMOUNT OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.\n\n";

var device_name_apis = "\
def get_device_name(revision):\n\
    '''\n\
    Returns the device name based on the info byte array values returned by atcab_info\n\
    '''\n\
    devices = {0x10: 'ATECC108A',\n\
                0x50: 'ATECC508A',\n\
                0x60: 'ATECC608',\n\
                0x00: 'ATSHA204A',\n\
                0x02: 'ATSHA204A'}\n\
    return devices.get(revision[2], 'UNKNOWN')\n\
\n\
def get_device_type_id(name):\n\
    '''\n\
    Returns the ATCADeviceType value based on the device name\n\
    '''\n\
    devices = {'ATSHA204A': 0,\n\
                'ATECC108A': 1,\n\
                'ATECC508A': 2,\n\
                'ATECC608A': 3,\n\
                'ATECC608B': 3,\n\
                'ATECC608': 3,\n\
             'UNKNOWN': 0x20 }\n\
    return devices.get(name.upper())\n\n";

var config_script_imports = "\
import argparse\n\
from cryptoauthlib import *\n\
from cryptoauthlib.iface import *\n\
\n\
LOCK_ZONE_CONFIG = 0x00\n\
LOCK_ZONE_DATA = 0x01\n\
ATCA_SUCCESS = 0x00\n\
LOCK_ZONE_NO_CRC = 0x80\n\n";

var config_script_config_start = "\
# Configuration array to be loaded into Crypto device\n\
DEVICE_CONFIGURATION = bytearray([ \n";
var config_script_config_end = "]); \n\n";

var script_connect_device = "\
    # Initialize interface\n\
    assert atcab_init(cfg) == ATCA_SUCCESS\n\
    \n\
    # Get connected device type\n\
    info = bytearray(4)\n\
    assert atcab_info(info) == ATCA_SUCCESS\n\
    dev_type = get_device_type_id(get_device_name(info))\n\
    \n\
    # Checking if the connected device matches with selected device\n\
    if dev_type != cfg.devtype:\n\
        assert atcab_release() == ATCA_SUCCESS\n\
        raise ValueError('Device not supported')\n\n";

var script_load_config = "\
def load_config_zone():\n\
    '''\n\
    - Initializes the interface\n\
    - Checks if the correct device is connected to the kit.\n\
    - Checks the device config zone lock status, writes config zone if the config zone\n\
    is unlocked. The device's configuration zone will be locked after writting the zone.\n\
    '''\n\
    \n\
    # Loading cryptoauthlib(python specific call to load cdll into python)\n\
    load_cryptoauthlib()\n\n";
var script_load_config_calls = "\
    # Get Config Zone lock status\n\
    is_locked = AtcaReference(False)\n\
    assert atcab_is_locked(LOCK_ZONE_CONFIG, is_locked) == ATCA_SUCCESS\n\
    \n\
    # Checking config zone lock status and write config if not locked\n\
    if 0 == bool(is_locked.value):\n\
        # Config zone is unlocked... Write config zone and lock it\n\
        # Loading configuration to crypto device\n\
        assert atcab_write_config_zone(DEVICE_CONFIGURATION) == ATCA_SUCCESS\n\
        print('Crypto Device Configuration Zone has been written!!!')\n\
        \n\
        assert atcab_lock(LOCK_ZONE_NO_CRC | LOCK_ZONE_CONFIG, 0) == ATCA_SUCCESS\n\
        print('Crypto Device Configuration Zone has been locked!!!')\n\
    else:\n\
        print('Crypto Device Configuration Zone is already locked!!!')\n\n";
var script_load_data = "\
def load_data_zone():\n\
    '''\n\
    - Initializes the interface\n\
    - Checks if the correct device is connected to the kit.\n\
    - Checks the device config zone lock status, exits if config zone is unlocked\n\
    - Checks the device data zone lock status, writes data zone if the data zone\n\
    is unlocked. The device's data zone will be locked after writting the zone.\n\
    '''\n\
    \n\
    # Loading cryptoauthlib(python specific call to load cdll into python)\n\
    load_cryptoauthlib()\n\n";

var script_release_call = "\
    # Release interface\n\
    atcab_release()\n\n";
var script_load_zone_main_call = "\
if __name__ == '__main__':\n\
    parser = argparse.ArgumentParser(\n\
        description='This script tries to connect an ATECC608A device via CryptoAuth-XSTK. \\n\\\n\
    1. Loads the CryptoAuthLib Cdll into Python environment. \\n\\\n\
    2. Initializes interface to connect Cryto device on HID interface. \\n\\\n\
    3. Checks if the correct device is connected to the kit. \\n\\\n\
    4. Checks the zone lock status. If the zone is unlocked, writes to the zone and locks it.',\n\
        formatter_class=argparse.RawDescriptionHelpFormatter)\n\
    args = parser.parse_args()\n\n";
var script_call_config_api = "\
    # Loading configuration into the device.\n\
    load_config_zone()\n";
var script_call_data_api = "\
    # Loading data into the device.\n\
    load_data_zone()\n";

