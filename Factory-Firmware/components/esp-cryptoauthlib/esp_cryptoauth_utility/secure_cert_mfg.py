#!/usr/bin/env python
# Copyright 2020 Espressif Systems (Shanghai) Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
import argparse
from pyasn1_modules import pem
from sys import exit
import helper_scripts as hs
import binascii
import os
import sys
import zlib
import sample_bins
import sample_certs

supported_targets = {'esp32', 'esp32s3', 'esp32c2', 'esp32c3' , 'esp32c5', 'esp32c6', 'esp32c61', 'esp32p4', 'esp32h2'}
try:
    import esptool
except ImportError:  # cheat and use IDF's copy of esptool if available
    idf_path = os.getenv('IDF_PATH')
    if not idf_path or not os.path.exists(idf_path):
        raise
    sys.path.insert(0, os.path.join(idf_path, 'components', 'esptool_py', 'esptool'))
    import esptool


SAMPLE_SIGNERCERT_PATH = os.path.join(sample_certs.__path__[0],
                                      'sample_signer_cert.pem')

SAMPLE_SIGNERKEY_PATH = os.path.join(sample_certs.__path__[0],
                                     'sample_signer_key.pem')


def get_binary_stub_path(target_chip):
    stub_path = os.path.join(sample_bins.__path__[0],
                             f'{target_chip}/secure_cert_mfg_{target_chip}.bin')
    return stub_path


def main():
    parser = argparse.ArgumentParser(description='''Provision the ESP32 device with
        device_certificate and signer_certificate required for TLS authentication''')

    parser.add_argument(
        '--signer-cert',
        dest='signer_cert',
        default=SAMPLE_SIGNERCERT_PATH,
        metavar='relative/path/to/signer_cert.pem',
        help='relative path(from secure_cert_mfg.py) to signer certificate.')

    parser.add_argument(
        '--signer-cert-private-key',
        dest='signer_privkey',
        default=SAMPLE_SIGNERKEY_PATH,
        metavar='relative/path/to/signer-priv-key',
        help='relative path(from secure_cert_mfg.py) to signer certificate private key')

    parser.add_argument(
        '--pwd', '--password',
        dest='password',
        metavar='[password]',
        help='the password associated with the private key')

    parser.add_argument(
        '--port', '-p',
        dest='port',
        metavar='[port]',
        required=True,
        help='uart com port to which ESP device is connected')

    parser.add_argument(
        '--target_chip',
        dest='target_chip',
        required=True, choices=supported_targets,
        help='Target ESP32 series chip')

    parser.add_argument(
        "--i2c-sda-pin', '-sda_pin'",
        dest='i2c_sda_pin',
        default=21,type=int,
        help='The pin no of I2C SDA pin of esp32 to which atecc608 is connected, default = 21')

    parser.add_argument(
        "--i2c-scl-pin', '-scl_pin'",
        dest='i2c_scl_pin',
        default=22,type=int,
        help='The pin no of I2C SCL pin of esp32 to which atecc608 is connected, default = 22')

    parser.add_argument(
        '--type', '--print-atecc608-type',
        dest='print_atecc608_type',action='store_true',
        help='print type of atecc608 chip connected to your ESP device')

    parser.add_argument(
        '--valid-for-years',
        dest='nva_years',
        default=40,type=int,
        help='number of years for which device cert is valid (from current year), efault = 40')

    parser.add_argument(
        '--lock_slots',
        dest='lock_slots',
        default=False,action='store_true',
        help='Whether to lock the device and signer certificate slot for ATECC TrustCustom '
             '\nSlots shall be permenantly locked if set to true')
    args = parser.parse_args()
    baudrate = 576000

    if args.target_chip == 'esp32':
        baudrate = 230400

    binary_stub_path = get_binary_stub_path(args.target_chip)
    print(binary_stub_path)
    hs.serial.load_app_stub(args.port, baudrate, binary_stub_path)
    init_mfg = hs.serial.cmd_interpreter(port=args.port)

    retval = init_mfg.wait_for_init()
    if retval is not True:
        print('CMD prompt timed out.')
        exit(0)

    retval = init_mfg.exec_cmd(args.port, 'init {0} {1}'.format(args.i2c_sda_pin, args.i2c_scl_pin))
    hs.serial.esp_cmd_check_ok(retval, 'init {0} {1}'.format(args.i2c_sda_pin, args.i2c_scl_pin))

    device_type = retval[1]['Return']
    if 'TrustCustom' in retval[1]['Return']:
        print('ATECC608 chip is of type TrustCustom')
        provision_trustcustom_device(args, init_mfg, device_type)
    elif 'TrustManager' in retval[1]['Return']:
        print('ATECC608 chip is of type TrustManager')
        provision_trustcustom_device(args, init_mfg, device_type)
    elif 'Trust&Go' in retval[1]['Return']:
        print('ATECC608 chip is of type Trust&Go')
        hs.manifest.generate_manifest_file(args, init_mfg)
    elif 'TrustFlex' in retval[1]['Return']:
        print('ATECC608 chip is of type TrustFlex')
        hs.manifest.generate_manifest_file(args, init_mfg)
    else:
        print('Invalid type')
        exit(0)


def provision_trustcustom_device(args, init_mfg, device_type):

    retval = init_mfg.exec_cmd(args.port, 'print-chip-info')
    hs.serial.esp_cmd_check_ok(retval, 'print-chip-info')

    index = retval[1]['Return'].find('Serial Number:\r\n')
    index += len('Serial Number:\r\n')
    serial_number = bytearray(9)
    s = retval[1]['Return'][index:]
    s = s.strip()
    serial_number = bytearray.fromhex(s)
    serial_number_hex = (binascii.hexlify(serial_number)).decode()
    print('Serial Number:')
    print(serial_number_hex.upper())

    if args.print_atecc608_type is True:
        # print chip info and exit
        exit(0)
    print('Provisioning the Device')
    retval = init_mfg.exec_cmd(args.port, 'generate-keys 0')
    hs.serial.esp_cmd_check_ok(retval, 'generate-keys')

    retval = init_mfg.exec_cmd(args.port, 'generate-csr')
    hs.serial.esp_cmd_check_ok(retval, 'generate-csr')

    print('CSR obtained from device is:')
    print(retval[1]['Return'])

    try:
        # load private keys of signers to sign the CSR
        private_key = hs.cert_sign.load_privatekey(args.signer_privkey, args.password)
        signer_cert = hs.cert_sign.load_certificate(args.signer_cert)
        # Sign the CSR using the generated keys
        device_cert = hs.cert_sign.sign_csr(retval[1]['Return'].encode(), signer_cert, private_key, serial_number_hex, args.nva_years)
        print('Device cert generated: \n')
        dec_device_cert = device_cert.decode()
        print(dec_device_cert)
        print('Saving device cert to output_files/device_cert.pem')
        if not os.path.exists('output_files'):
            os.makedirs('output_files')

        if esp_handle_file('./output_files/device_cert.pem', 'write', dec_device_cert) is not True:
            print('Error in writing device certificate')
            exit(0)
        cert_der = esp_handle_file('./output_files/device_cert.pem', 'pem_read')
    except ValueError:
        print('Unsupported Key,Cert or CSR format specified.')
        exit(0)

    # get the cert definition and template data in string format
    print('program device cert')
    slot_lock = 0

    if (args.lock_slots):
        slot_lock = 1

    cert_def_str = hs.cert2certdef.esp_create_cert_def_str(cert_der, 'DEVICE_CERT')

    retval = init_mfg.exec_cmd(args.port, 'provide-cert-def 0', cert_def_str)
    hs.serial.esp_cmd_check_ok(retval, 'program-device-cert-def')

    crc32_value = zlib.crc32(device_cert) & 0xFFFFFFFF  # Ensure 32-bit unsigned value
    retval = init_mfg.exec_cmd(args.port, f'program-dev-cert {slot_lock} {crc32_value}', device_cert)
    hs.serial.esp_cmd_check_ok(retval, f'program-dev-cert {slot_lock} {crc32_value}')
    print(retval[1]['Return'])

    signer_cert_data = esp_handle_file(args.signer_cert, 'read')
    cert_der = esp_handle_file(args.signer_cert, 'pem_read')

    if 'TrustManager' in device_type:
        return True

    print('Signer cert is:')
    print(signer_cert_data)

    print('program signer cert')
    cert_def_str = hs.cert2certdef.esp_create_cert_def_str(cert_der, 'SIGNER_CERT')

    retval = init_mfg.exec_cmd(args.port, 'provide-cert-def 1', cert_def_str)
    hs.serial.esp_cmd_check_ok(retval, 'program-signer-cert-def')

    crc32_value = zlib.crc32(signer_cert_data.encode()) & 0xFFFFFFFF  # Ensure 32-bit unsigned value
    retval = init_mfg.exec_cmd(args.port, f'program-signer-cert {slot_lock} {crc32_value}', signer_cert_data)
    hs.serial.esp_cmd_check_ok(retval, f'program-signer-cert {slot_lock} {crc32_value}')


def esp_handle_file(file_name, operation, data=None):
    if operation == 'read':
        with open(file_name, 'r') as cert_file:
            data = cert_file.read()
        return data
    elif operation == 'pem_read':
        with open(file_name, 'r') as cert_file:
            data = pem.readPemFromFile(cert_file)
        return data
    elif operation == 'write':
        with open(file_name, 'w+') as cert_file:
            cert_file.write(data)
        return True


if __name__ == '__main__':
    main()
