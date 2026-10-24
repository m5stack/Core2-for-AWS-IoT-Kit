from .sys_helper import sys_helper
from .path_helper import path_helper
from pathlib import Path
import hid
import sys, os
import json
import time
import platform

nEDBG_DEBBUGER_PID        = 8565   #0x2175
CRYPTO_TRUST_PLATFORM_PID = 8978   #0x2312
VENDOR_ID                 = 1003   #0x03EB


json_name = "trustplatform.config"

class program_flash():
    """
    Class with methods which help with wrappers and
    helpers for flash hex file into CryptoAuth Trust Platform board
    """
    def __init__(self):
        self.jar_loc = None
        self.java_loc = None

        trustplatform_directory = ".trustplatform"
        self.filename = os.path.join(Path.home(), trustplatform_directory, json_name)

    def get_jar_loc(self):
        """
        Function which fetch the jar location and store it in self.jar_loc
        """
        os_type = platform.system().lower()
        if 'darwin' in os_type:
            ipefile = os.path.join("mplab_platform", "mplab_ipe", "bin", "ipecmd.sh")
        else:
            ipefile = os.path.join("mplab_platform", "mplab_ipe", "ipecmd.jar")

        with open(self.filename, 'r') as file:
            data = json.load(file)
        mplab_path = data["MPLABX"]["ipe_path"]

        self.jar_loc = Path(os.path.join(mplab_path, ipefile))

    def get_java_loc(self):
        """
        Function which fetch the java location associated with mplabx
        """
        with open(self.filename, 'r') as file:
            data = json.load(file)
        mplab_path = data["MPLABX"]["ipe_path"]

        os_type = platform.system().lower()
        if 'darwin' in os_type:
            java_jre = "java"
        else:
            java_jre = "java.exe"

        for (root, dirs, files) in os.walk(os.path.join(mplab_path, 'sys')):
            if java_jre in files:
                if os.path.exists(os.path.join(root, java_jre)):
                    self.java_loc = os.path.join(root, java_jre)

        if self.java_loc is None:
            raise FileNotFoundError("JRE installation not found")

    def check_mplab_path(self):
        """
        Function which checks the mplab ipe path is present or not

        Outputs:
               Returns True or False
               True            mplab ipe path set
               False           mplab ipe path not set
        """
        with open(self.filename, 'r') as file:
            data = json.load(file)

        def_val = data["MPLABX"]["path_set"]
        if def_val.lower() == "true":
            return True
        else:
            return False

    @staticmethod
    def check_debugger_info():
        """
        Function which check the nEDBG CMSIS-DAP debugger connected or not

        Outputs:
               Returns a list of ["path", "vendor_id", "product_id", "serial_number", "manufacturer_string", product_string", "interafce_number"]

               vendor_id                vendor id
               product_id               product id
               serial_number            product serial number
               manufacturer_string      product manufacturer name
               product_string           product name
               interface_number         interface number
        """
        debugger_info = hid.enumerate(VENDOR_ID, nEDBG_DEBBUGER_PID)
        return debugger_info

    @staticmethod
    def check_firmware_info():
        """
        Function which check the firmware info whether Factory reset program flashed or not

        Outputs:
               Returns a list of ["path", "vendor_id", "product_id", "serial_number", "manufacturer_string", product_string", "interafce_number"]

               vendor_id                vendor id
               product_id               product id
               serial_number            product serial number
               manufacturer_string      product manufacturer name
               product_string           product name
               interface_number         interface number
        """
        firmware_info = hid.enumerate(VENDOR_ID, CRYPTO_TRUST_PLATFORM_PID)
        return firmware_info

    def flash_micro(self, hexfile_path):
        """
        Function which flash the hex file into crypto trust platform board by executing command

        Examples:
            To flash hex file "java -jar "C:\\Program Files (x86)\\Microchip\\MPLABX\\v5.30\\mplab_platform\\mplab_ipe\\ipecmd.jar"
                                          -PATSADM21E18A -TPPKOB -ORISWD -OL -M -F"cryptoauth_trust_platform.hex"
        Inputs:
              hexfile_path             Hex file which will be flashed into CryptoAuth Trust Platform

        Outputs:
               Returns a namedtuple of ['returncode', 'stdout', 'stderr']

               returncode              Returns error code from terminal
               stdout                  All standard outputs are accumulated here.
               srderr                  All error and warning outputs
        """
        self.get_jar_loc()
        self.get_java_loc()
        if sys.platform == 'darwin':
            subprocessout = sys_helper.run_subprocess_cmd(cmd=[str(self.jar_loc), "-PATSAMD21E18A", "-TPPKOB", "-OL", "-M", ("-F"+str(Path(hexfile_path)))])
        else:
            subprocessout = sys_helper.run_subprocess_cmd(cmd=[str(Path(self.java_loc)), "-jar", str(self.jar_loc), "-PATSAMD21E18A", "-TPPKOB", "-OL", "-M", ("-F"+str(Path(hexfile_path)))])

        return subprocessout

    def check_for_factory_program(self, firm_valid=False):
        """
        Function which check whether the proper device connected or not, if connected which flash default factory reset image

        Outputs:
              Returns true or error message
              True               when default factory reset program present or successfully factory image programmed
        """
        status = 'success'
        firm_info = self.check_firmware_info()
        if firm_info == [] or firm_info[0]['product_string'] != "CryptoAuth Trust Platform":
            print('Checking for Factory Program')
            debug_info = self.check_debugger_info()
            if debug_info != [] and "MCHP3311" in debug_info[0]['serial_number']:
                print("\tCryptoAuth Trust Platform Factory Program image is not found!")
                if self.check_mplab_path() == True:
                    print("\tProgramming Factory Program image...")
                    homepath = path_helper.get_home_path()
                    hexfile = str(Path(os.path.join(homepath, "assets", "Factory_Program.X", "CryptoAuth_Trust_Platform.hex")))
                    subprocessout = self.flash_micro(hexfile)
                    time.sleep(2)  #delay to allow USB reenumeration
                    if subprocessout.returncode != 0:
                        status = 'Please rerun or flash CryptoAuth Trust Platform with Factory Program image!'
                else:
                    status = 'MPLAB IPE is not enabled, please flash CryptoAuth Trust Platform with Factory Program image!'
            else:
                status = 'Cannot connect to CryptoAuth Trust Platform, check USB connection!'
        return status
