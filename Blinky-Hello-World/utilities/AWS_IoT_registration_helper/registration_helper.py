# AWS IoT EduKit Pre-Provisioned MCU Device Registration Helper
# v1.0.0
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

import sys
import argparse
import subprocess
from pyasn1_modules import pem
from cryptography import x509
from cryptography.x509.oid import NameOID
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.backends import default_backend
from datetime import datetime, timedelta
import os
import re
import sys
import binascii
import json

# Verify ESP-IDF is installed and environmental variables are added to PATH
try:
    import esptool
except ImportError: 
    idf_path = os.getenv("IDF_PATH")
    if not idf_path or not os.path.exists(idf_path):
        print("\n\nESP-IDF not found! Install ESP-IDF and run the export script...\n\n")
        raise
    sys.path.insert(0, os.path.join(idf_path, "components", "esptool_py", "esptool"))
    import esptool

# Import the Espressif CryptoAuthLib Utility libraries
sys.path.append(os.path.abspath(os.path.join(os.path.dirname( __file__ ), "..", "..", "components", "esp-cryptoauthlib", "esp_cryptoauth_utility")))
import helper_scripts as esp_hs

# Import the Microchip Trust Platform Design Suite libraries
trustplatform_path = os.path.join(os.getcwd(), "..", "trustplatform", "assets", "python")
sys.path.append(trustplatform_path)
import certs_handler
import trustplatform
from requirements_helper import requirements_installer
import manifest_helper

# Import the Microchip Trust Platform Design Suite AWS and manifest helper libraries
trustplatform_aws_path = os.path.join(os.getcwd(), "..", "trustplatform", "TrustnGO")
sys.path.append(trustplatform_aws_path)
from helper_aws import *
from Microchip_manifest_handler import *


def check_environment():
    """Checks to ensure environment is set per AWS IoT EduKit instructions

    Verifies Miniconda is installed and the 'edukit' virtual environment
    is activated.
    Verifies Python 3.7.x is installed and is being used to execute this script.
    Verifies that the AWS CLI is installed and configured correctly. Prints
    AWS IoT endpoint address.
    """
    conda_env = os.environ.get('CONDA_DEFAULT_ENV')
    if conda_env == None or conda_env == "base":
        print("The 'edukit' Conda environment is not created or activated:\n  To install miniconda, visit https://docs.conda.io/en/latest/miniconda.html.\n  To create the environment, use the command 'conda create -n edukit python=3.7'\n  To activate the environment, use the command 'conda activate edukit'\n")
    print("Conda 'edukit' environment active...")
    
    if sys.version_info[0] != 3 or sys.version_info[1] != 7:
        print(f"Python version {sys.version}")
        print("Incorrect version of Python detected. Must use Python version 3.7.x. You might want to try the command 'conda install python=3.7'.")
        exit(0)
    print("Python 3.7.x detected...")

    aws_iot_endpoint = subprocess.run(["aws", "iot", "describe-endpoint", "--endpoint-type", "iot:Data-ATS"], universal_newlines=True, capture_output=True)
    if aws_iot_endpoint.returncode != 0:
        print("Error with AWS CLI! Follow the configurtion docs at 'https://docs.aws.amazon.com/cli/latest/userguide/install-cliv2.html'")
        exit(0)
    aws_cli_resp = json.loads(aws_iot_endpoint.stdout)
    print(f"AWS CLI configured for IoT endpoint: {aws_cli_resp['endpointAddress']}")


def generate_signer_certificate():
    """Generates a x.509 certificate signed by ECDSA key
    This signer certificate is used to generate the device manifest file and helps 
    ensure the validity/ownership of the manifest contents. This signer certificate's
    Distinguished Name (DN) includes the AWS IoT registration code (FDQN)as the 
    common name and can be helpful for fleet provisioning.

    Signer certificate and key is saved in ./output_files/

    Certificate is set to expire in 1 year.
    """
    print("Generating ECDSA 256-bit prime field key...")
    signer_key = ec.generate_private_key(
        curve = ec.SECP256R1(), 
        backend = default_backend()
    )

    signer_key_pem = signer_key.private_bytes(
        encoding=serialization.Encoding.PEM,
        format=serialization.PrivateFormat.TraditionalOpenSSL,
        encryption_algorithm=serialization.NoEncryption(),
    )
    with open(os.path.join("output_files", "signer_key.pem"), "wb") as signer_key_file:
        signer_key_file.write(signer_key_pem)

    print("Generating self-signed x.509 certificate...")
    aws_iot_reg_code = subprocess.run(["aws", "iot", "get-registration-code"], universal_newlines=True, capture_output=True)
    if aws_iot_reg_code.returncode != 0:
        print("Error with the AWS CLI when running the command 'aws iot get-registration-code'.")
        exit(0)
    
    aws_cli_resp = json.loads(aws_iot_reg_code.stdout)
    signer_public_key = signer_key.public_key()
    time_now = datetime.utcnow()
    days_to_expire = 365
    x509_cert = (
        x509.CertificateBuilder()
        .issuer_name(x509.Name([x509.NameAttribute(NameOID.COMMON_NAME, aws_cli_resp['registrationCode']),]))
        .subject_name(x509.Name([x509.NameAttribute(NameOID.COMMON_NAME, aws_cli_resp['registrationCode']),]))
        .serial_number(x509.random_serial_number())
        .public_key(signer_public_key)
        .not_valid_before(time_now)
        .not_valid_after(time_now + timedelta(days=days_to_expire))
        .add_extension(x509.SubjectKeyIdentifier.from_public_key(signer_public_key), False)
        .add_extension(x509.AuthorityKeyIdentifier.from_issuer_public_key(signer_public_key), False)
        .add_extension(x509.BasicConstraints(ca=True, path_length=None), True)
        .sign(signer_key, hashes.SHA256(), default_backend())
    )

    signer_cert_pem = x509_cert.public_bytes(encoding=serialization.Encoding.PEM)

    with open(os.path.join("output_files","signer_cert.crt"), "wb") as signer_cert_file:
        signer_cert_file.write(signer_cert_pem)
    print(f"Successfully created x.509 certificate with expiration in {days_to_expire} days...")


def upload_manifest():
    """Uses Microchip TrustPlatform to register an AWS IoT thing
    Parses through the generated manifest file, creates an AWS IoT thing
    with a thing name that is the ATECC608 secure element serial number,
    applies the device certificate (public key) that is stored in the manifest
    file, and attaches a default policy.
    """
    check_and_install_policy('Default')

    for file in os.listdir("output_files"):
        if re.match("\w+(\_manifest.json)", file):
            manifest_file = open(os.path.join("output_files", file), "r")
            manifest_data = json.loads(manifest_file.read())

            signer_cert = open(os.path.join("output_files","signer_cert.crt"), "r").read()
            signer_cert_bytes = str.encode(signer_cert)
            invoke_import_manifest('Default', manifest_data, signer_cert_bytes)
            invoke_validate_manifest_import(manifest_data, signer_cert_bytes)


def main():
    """AWS IoT EduKit MCU hardware device registration script
    Checkes environment is set correctly, generates ECDSA certificates,
    ensures all required python libraries are included, retrieves on-board 
    device certificate using the esp-cryptoauth library and utility, creates 
    an AWS IoT thing using the AWS CLI and Microchip Trust Platform Design Suite.
    """
    app_binary = 'sample_bins/secure_cert_mfg_esp32.bin'
    parser = argparse.ArgumentParser(description='''Provision the Core2 for AWS IoT EduKit with 
        device_certificate and signer_certificate required for TLS authentication''')

    parser.add_argument(
        "--port", '-p',
        dest='port',
        metavar='[port]',
        required=True,
        help='Serial comm port of the Core2 for AWS IoT EduKit device')

    args = parser.parse_args()

    args.signer_cert = "output_files/signer_cert.crt"
    args.signer_privkey = "output_files/signer_key.pem"
    args.print_atecc608a_type = False
    check_environment()
    
    reqs = requirements_installer('requirements.txt')

    generate_signer_certificate()

    esp = esptool.ESP32ROM(args.port,baud=115200)
    esp_hs.serial.load_app_stub(app_binary, esp)
    init_mfg = esp_hs.serial.cmd_interpreter()

    retval = init_mfg.wait_for_init(esp._port)
    if retval is not True:
        print("CMD prompt timed out.")
        exit(0)

    retval = init_mfg.exec_cmd(esp._port, "init")
    esp_hs.serial.esp_cmd_check_ok(retval, "init")
    esp_hs.generate_manifest_file(esp, args, init_mfg)
    upload_manifest()


if __name__ == "__main__":
    main() 