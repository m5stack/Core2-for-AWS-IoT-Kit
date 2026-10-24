# AWS IoT Kit Pre-Provisioned MCU Device Registration Helper
# v1.1.0
#
# Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal in
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
# the Software, and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

import argparse
import json
import os
import re
import subprocess
import sys
from datetime import datetime, timedelta, timezone
from pathlib import Path

# os.environ["CRYPTOAUTHLIB_NOUSB"] = "1"

# Install/refresh the Python dependencies required by this script and the
# Trust Platform helper libraries before importing them.
subprocess.check_call([sys.executable, '-m', 'pip', 'install', '-r', 'requirements.txt'])

import boto3
from botocore.exceptions import ClientError
from cryptography import x509
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.x509.oid import NameOID

# Import the Espressif CryptoAuthLib Utility libraries.
ESP_CRYPTOAUTH_UTILITY_DIR = Path(__file__).resolve().parent.parent.parent / "components" / "esp-cryptoauthlib" / "esp_cryptoauth_utility"
sys.path.append(str(ESP_CRYPTOAUTH_UTILITY_DIR))
import helper_scripts as esp_hs

# Import the Microchip Trust Platform Design Suite libraries.
sys.path.append(os.path.join(os.path.dirname(__file__), "..", "trustplatform", "assets", "python"))

# Import the Microchip Trust Platform Design Suite AWS and manifest helper libraries.
sys.path.append(os.path.join(os.path.dirname(__file__), "..", "trustplatform", "TrustnGO"))
from helper_aws import *
from Microchip_manifest_handler import *

# --- Configuration -----------------------------------------------------------
SCRIPT_DIR = Path(__file__).resolve().parent
OUTPUT_DIR = SCRIPT_DIR / "output_files"
BLINKY_SDKCONFIG = SCRIPT_DIR.parent.parent / "sdkconfig.defaults"
THERMOSTAT_SDKCONFIG = SCRIPT_DIR.parent.parent.parent / "Smart-Thermostat" / "sdkconfig.defaults"

ATECC608_I2C_SDA_PIN = 21
ATECC608_I2C_SCL_PIN = 22
APP_BINARY = ESP_CRYPTOAUTH_UTILITY_DIR / "sample_bins" / "esp32" / "secure_cert_mfg_esp32.bin"
APP_BAUDRATE = 230400  # Baud rate used by esp_cryptoauth_utility for the ESP32
POLICY_NAME = 'Kit_Policy'
SIGNER_VALIDITY_DAYS = 365
MQTT_HOST_PLACEHOLDER = "CONFIG_AWS_IOT_MQTT_HOST=\n"

# Matches the "<serial>_manifest.json" file produced by generate_manifest_file().
MANIFEST_FILE_RE = re.compile(r"^\w+_manifest\.json$")

iot = boto3.client('iot')

def replace_file_text(file_path, search_expr, replacement_expr):
    """Replace ``search_expr`` with ``replacement_expr`` in ``file_path``.

    The file is rewritten in place. Missing files are ignored so that the
    script still works when an optional target (e.g. the Smart-Thermostat
    project) is not present.
    """
    path = Path(file_path)
    if not path.is_file():
        return
    text = path.read_text()
    if search_expr in text:
        path.write_text(text.replace(search_expr, replacement_expr))

def check_environment():
    """Checks to ensure environment is set per AWS IoT Kit instructions.
    Verifies Python 3.6.x+ is installed and is being used to execute this script.
    Verifies that the AWS CLI is installed and configured. Prints
    AWS IoT endpoint address.
    """
    
    if sys.version_info < (3, 6):
        print(f"Python version {sys.version}")
        print("Incorrect version of Python detected. Must use Python version 3.6.x+. Please check your Python installation and that you're using the PlatformIO CLI terminal in VS Code.")
        sys.exit(1)
    print(f"Python {sys.version} detected...")

    try:
        aws_iot_endpoint = iot.describe_endpoint(endpointType='iot:Data-ATS')
    except ClientError as e:
        print(e.response['Error']['Code'])
        print("Error with AWS CLI! Follow the configuration docs at 'https://docs.aws.amazon.com/cli/latest/userguide/install-cliv2.html'")
        sys.exit(1)

    endpoint_address = aws_iot_endpoint['endpointAddress']
    print(f"AWS CLI configured for IoT endpoint: {endpoint_address}")
    replacement = f"CONFIG_AWS_IOT_MQTT_HOST={endpoint_address}\n"
    replace_file_text(BLINKY_SDKCONFIG, MQTT_HOST_PLACEHOLDER, replacement)
    replace_file_text(THERMOSTAT_SDKCONFIG, MQTT_HOST_PLACEHOLDER, replacement)

def generate_signer_certificate():
    """Generates a x.509 certificate signed by ECDSA key
    This signer certificate is used to generate the device manifest file and helps 
    ensure the validity/ownership of the manifest contents. This signer certificate's
    Distinguished Name (DN) includes the AWS IoT registration code (FDQN)as the 
    common name and can be helpful for fleet provisioning.

    Signer certificate and key is saved in ./output_files/

    Certificate is set to expire in 1 year.
    """
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

    print("Generating ECDSA 256-bit prime field key...")
    signer_key = ec.generate_private_key(curve=ec.SECP256R1())

    signer_key_pem = signer_key.private_bytes(
        encoding=serialization.Encoding.PEM,
        format=serialization.PrivateFormat.TraditionalOpenSSL,
        encryption_algorithm=serialization.NoEncryption(),
    )
    (OUTPUT_DIR / "signer_key.pem").write_bytes(signer_key_pem)

    print("Generating self-signed x.509 certificate...")
    try:
        aws_iot_reg_code = iot.get_registration_code()
    except ClientError as e:
        print(e.response['Error']['Code'])
        print("Error with the AWS CLI when running the command 'aws iot get-registration-code'.")
        sys.exit(1)

    registration_code = aws_iot_reg_code['registrationCode']
    signer_public_key = signer_key.public_key()
    time_now = datetime.now(timezone.utc)
    x509_cert = (
        x509.CertificateBuilder()
        .issuer_name(x509.Name([x509.NameAttribute(NameOID.COMMON_NAME, registration_code)]))
        .subject_name(x509.Name([x509.NameAttribute(NameOID.COMMON_NAME, registration_code)]))
        .serial_number(x509.random_serial_number())
        .public_key(signer_public_key)
        .not_valid_before(time_now)
        .not_valid_after(time_now + timedelta(days=SIGNER_VALIDITY_DAYS))
        .add_extension(x509.SubjectKeyIdentifier.from_public_key(signer_public_key), False)
        .add_extension(x509.AuthorityKeyIdentifier.from_issuer_public_key(signer_public_key), False)
        .add_extension(x509.BasicConstraints(ca=True, path_length=None), True)
        .sign(signer_key, hashes.SHA256())
    )

    signer_cert_pem = x509_cert.public_bytes(encoding=serialization.Encoding.PEM)

    (OUTPUT_DIR / "signer_cert.crt").write_bytes(signer_cert_pem)
    print(f"Successfully created x.509 certificate with expiration in {SIGNER_VALIDITY_DAYS} days...")


def upload_manifest():
    """Uses Microchip TrustPlatform to register an AWS IoT thing
    Parses through the generated manifest file, creates an AWS IoT thing
    with a thing name that is the ATECC608 secure element serial number,
    applies the device certificate (public key) that is stored in the manifest
    file, and attaches a default policy.
    """
    check_and_install_policy(POLICY_NAME)

    # Read the signer certificate once; it is shared by every manifest.
    signer_cert_bytes = (OUTPUT_DIR / "signer_cert.crt").read_bytes()

    for file in sorted(os.listdir(OUTPUT_DIR)):
        if not MANIFEST_FILE_RE.match(file):
            continue
        with open(OUTPUT_DIR / file, "r") as manifest_file:
            manifest_data = json.load(manifest_file)

        invoke_import_manifest(POLICY_NAME, manifest_data, signer_cert_bytes)
        invoke_validate_manifest_import(manifest_data, signer_cert_bytes)


def main():
    """AWS IoT Kit MCU hardware device registration script
    Checkes environment is set correctly, generates ECDSA certificates,
    ensures all required python libraries are included, retrieves on-board 
    device certificate using the esp-cryptoauth library and utility, creates 
    an AWS IoT thing using the AWS CLI and Microchip Trust Platform Design Suite.
    """
    parser = argparse.ArgumentParser(description='''Provision the Core2 for AWS IoT Kit with 
        device_certificate and signer_certificate required for TLS authentication''')

    parser.add_argument(
        "--port", '-p',
        dest='port',
        metavar='[port]',
        required=True,
        help='Serial comm port of the Core2 for AWS IoT Kit device')

    args = parser.parse_args()

    # The esp-cryptoauthlib manifest generator resolves its output directory and
    # the signer key/cert paths against the current working directory, so this
    # script must be run from its own directory.
    if os.getcwd() != str(SCRIPT_DIR):
        print(f"Warning: run this script from {SCRIPT_DIR} so the manifest "
              f"generator can find the signer key/cert and write output_files/.")

    args.signer_cert = "output_files/signer_cert.crt"
    args.signer_privkey = "output_files/signer_key.pem"
    args.print_atecc608_type = False
    check_environment()

    generate_signer_certificate()

    esp_hs.serial.load_app_stub(args.port, APP_BAUDRATE, str(APP_BINARY))
    init_mfg = esp_hs.serial.cmd_interpreter(port=args.port)

    retval = init_mfg.wait_for_init()
    if retval is not True:
        print("CMD prompt timed out.")
        sys.exit(1)

    init_cmd = f"init {ATECC608_I2C_SDA_PIN} {ATECC608_I2C_SCL_PIN}"
    retval = init_mfg.exec_cmd(args.port, init_cmd)
    esp_hs.serial.esp_cmd_check_ok(retval, init_cmd)
    esp_hs.generate_manifest_file(args, init_mfg)
    upload_manifest()


if __name__ == "__main__":
    main()
