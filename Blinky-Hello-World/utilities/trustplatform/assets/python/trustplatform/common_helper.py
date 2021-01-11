import os
import base64
import re
import unicodedata
from cryptoauthlib import *
from trustplatform import program_flash
programmer = program_flash()

class common_helper():
    """
    Function return trust platform home directory
    """
    def __init__(self):
        pass

    def get_device_name(revision):
        """
        Returns the device name based on the info byte array values returned by atcab_info
        """
        devices = {0x10: 'ATECC108A',
                   0x50: 'ATECC508A',
                   0x60: 'ATECC608',
                   0x00: 'ATSHA204A',
                   0x02: 'ATSHA204A',
                   0x40: 'ATSHA206A'}
        device_name = devices.get(revision[2], 'UNKNOWN')
        return device_name

    def get_device_type_id(name):
        """
        Returns the ATCADeviceType value based on the device name
        """
        devices = {'ATSHA204A': 0,
                   'ATECC108A': 1,
                   'ATECC508A': 2,
                   'ATECC608A': 3,
                   'ATECC608B': 3,
                   'ATECC608': 3,
                   'ATSAH206A': 4,
                   'UNKNOWN': 0x20}
        return devices.get(name.upper())

    def pretty_print_hex(a, l=16, indent=''):
        """
        Format a list/bytes/bytearray object into a formatted ascii hex string
        """
        lines = []
        a = bytearray(a)
        for x in range(0, len(a), l):
            lines.append(indent + ' '.join(['{:02X}'.format(y) for y in a[x:x+l]]))
        return '\n'.join(lines)

    def convert_to_hex_bytes(a):
        hex_bytes = ''
        for x in range(0, len(a), 16):
            hex_bytes += (''.join(['0x%02X, ' % y for y in a[x:x+16]]) + '\n')
        return hex_bytes

    def convert_symmetric_to_pem(raw_symmetric, length):
        """
        Convert raw symmetric key to PEM format
        """
        sym_key_der = bytearray.fromhex('304F300906072A8648CE4C030103') + bytearray([(length + 2), 0x00, 0x04]) + raw_symmetric
        sym_key_der[1] = len(sym_key_der) - 2

        sym_key_b64 = base64.b64encode(sym_key_der).decode('ascii')
        sym_key_pem = (
            '-----BEGIN SYMMETRIC KEY-----\n'
            + '\n'.join(sym_key_b64[i:i + 64] for i in range(0, len(sym_key_b64), 64)) + '\n'
            + '-----END SYMMETRIC KEY-----'
        )
        return sym_key_pem

    def convert_ec_pub_to_pem(raw_pub_key):
        """
        Convert to the key to PEM format. Expects bytes
        """
        public_key_der = bytearray.fromhex('3059301306072A8648CE3D020106082A8648CE3D03010703420004') + raw_pub_key
        public_key_b64 = base64.b64encode(public_key_der).decode('ascii')
        public_key_pem = (
            '-----BEGIN PUBLIC KEY-----\n'
            + '\n'.join(public_key_b64[i:i + 64] for i in range(0, len(public_key_b64), 64)) + '\n'
            + '-----END PUBLIC KEY-----'
        )
        return public_key_pem

    @staticmethod
    def delete_mplablog_files(path):
        if os.path.exists(path):
            files = os.listdir(path)
            for f in files:
                if "mplabxlog" in f.lower():
                    try:
                        os.remove(f)
                    except:
                        pass

    def connect_to_secure_element(device_type, dev_interface, dev_identity):
        cfg = cfg_ateccx08a_kithid_default()
        cfg.devtype = common_helper.get_device_type_id(device_type)
        cfg.cfg.atcahid.dev_interface = dev_interface
        cfg.cfg.atcahid.dev_identity = dev_identity
        assert_msg = "Check switch position on CryptoAuth Trust Platform board, verify connections"
        if atcab_init(cfg) != Status.ATCA_SUCCESS:
            # Check for CryptoAuth Trust Platform connection and default factory reset image
            status = programmer.check_for_factory_program()
            working_dir = os.getcwd()
            common_helper.delete_mplablog_files(working_dir)
            assert status == 'success', status
            assert atcab_init(cfg) == Status.ATCA_SUCCESS, assert_msg

        # Get connected device type
        info = bytearray(4)
        assert_msg = "Cannot read the TFLXTLS device information, Verify device connections on Trust Platform"
        assert atcab_info(info) == Status.ATCA_SUCCESS, assert_msg

        dev_name = common_helper.get_device_name(info)
        if 'ATECC608' in dev_name:
            dev_name = 'ATECC608B'
            if info[3] <= 0x02:
                dev_name = 'ATECC608A'

        dev_type = common_helper.get_device_type_id(dev_name)
        # Checking if the connected device matches with selected device
        if dev_type != cfg.devtype:
            print('Device is not TFLXTLS, Connect TFLXTLS device')
            assert atcab_release() == Status.ATCA_SUCCESS, 'atcab_release failed... Rerun Notebook'
        else:
            print('Connected secure element is \"{}\"'.format(dev_name))

    def make_valid_filename(s):
        """
        Convert an arbitrary string into one that can be used in an ascii filename.
        """
        if sys.version_info[0] <= 2:
            if not isinstance(s, unicode):
                s = str(s).decode('utf-8')
        else:
            s = str(s)
        # Normalize unicode characters
        s = unicodedata.normalize('NFKD', s).encode('ascii', 'ignore').decode('ascii')
        # Remove non-word and non-whitespace characters
        s = re.sub(r'[^\w\s-]', '', s).strip()
        # Replace repeated whitespace with an underscore
        s = re.sub(r'\s+', '_', s)
        # Replace repeated dashes with a single dash
        s = re.sub(r'-+', '-', s)
        return s