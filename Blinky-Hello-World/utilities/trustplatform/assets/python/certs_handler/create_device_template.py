# (c) 2018 Microchip Technology Inc. and its subsidiaries.

# Subject to your compliance with these terms, you may use Microchip software
# and any derivatives exclusively with Microchip products. It is your
# responsibility to comply with third party license terms applicable to your
# use of third party software (including open source software) that may
# accompany Microchip software.

# THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER
# EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
# WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A PARTICULAR
# PURPOSE. IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL,
# PUNITIVE, INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY
# KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP
# HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
# FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
# ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
# THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

from datetime import datetime, timezone
import argparse
import re
from pathlib import Path
import getpass

from cryptography import x509

from .create_certs_common import *
from .ext_builder import ExtBuilder, TimeFormat


def load_or_create_device(serial_num, public_key_hex, cert_filename, ca_key_filename,
    ca_key_password, ca_cert_filename, org_name='Microchip Technology Inc', validity=28):
    if serial_num is None:
        serial_num = '012301020304050601'
    serial_num = serial_num.upper()
    if re.search('^[0-9A-F]{18}$', serial_num) is None:
        raise ValueError('serial_num is not hex string of 18 digits (9 bytes)')

    if public_key_hex is not None:
        if re.search('^[0-9a-fA-F]{128}$', public_key_hex) is None:
            raise ValueError('public_key_hex is not hex string of 128 digits (64 bytes)')

        public_key = ec.EllipticCurvePublicNumbers(
            x=int(public_key_hex[:64], 16),
            y=int(public_key_hex[64:], 16),
            curve=ec.SECP256R1()).public_key(get_backend())
    else:
        public_key = ec.EllipticCurvePublicNumbers(
            x=int('71f1a70da379a3fded6b5010bdad6e1fb9e8eba7df2c4b5c67d35eba84da09e7', 16),
            y=int('7ae8db2ccb9628eeeb85cdaab35c92e53e1c44d55a2ba7a024aa92603b68948a', 16),
            curve=ec.SECP256R1()).public_key(get_backend())

    cert_filename = Path(cert_filename)
    ca_key_filename = Path(ca_key_filename)
    ca_cert_filename = Path(ca_cert_filename)

    with open(str(ca_key_filename), 'rb') as f:
        ca_private_key = serialization.load_pem_private_key(
            data=f.read(),
            password=ca_key_password,
            backend=get_backend())

    with open(str(ca_cert_filename), 'rb') as f:
        ca_certificate = x509.load_pem_x509_certificate(f.read(), get_backend())
    ca_org_name = get_org_name(ca_certificate.subject)

    # Look for certificate
    certificate = None
    if cert_filename.is_file():
        # Found cached certificate file, read it in
        with open(str(cert_filename), 'rb') as f:
            certificate = x509.load_pem_x509_certificate(f.read(), get_backend())

    # Build certificate
    builder = ExtBuilder()
    builder = builder.issuer_name(ca_certificate.subject)
    # Device cert must have minutes and seconds set to 0
    if certificate:
        builder = builder.not_valid_before(certificate.not_valid_before)
    else:
        builder = builder.not_valid_before(datetime.utcnow().replace(tzinfo=timezone.utc, minute=0, second=0))
    #builder = builder.not_valid_after(datetime(9999, 12, 31, 23, 59, 59, tzinfo=timezone.utc))
    builder = builder.not_valid_after(
        builder._not_valid_before.replace(year=builder._not_valid_before.year + validity),
        format=TimeFormat.GENERALIZED_TIME
    )
    builder = builder.subject_name(x509.Name([
        x509.NameAttribute(x509.oid.NameOID.ORGANIZATION_NAME, ca_org_name),
        x509.NameAttribute(x509.oid.NameOID.COMMON_NAME, 'sn'+serial_num)]))
    builder = builder.public_key(public_key)
    # Device certificate is generated from certificate dates and public key
    builder = builder.serial_number(pubkey_cert_sn(16, builder))
    # Add in extensions
    builder = builder.add_extension(
        x509.BasicConstraints(ca=False, path_length=None),
        critical=True)
    builder = builder.add_extension(
        x509.KeyUsage(
            digital_signature=True,
            content_commitment=False,
            key_encipherment=False,
            data_encipherment=False,
            key_agreement=True,
            key_cert_sign=False,
            crl_sign=False,
            encipher_only=False,
            decipher_only=False),
        critical=True)
    builder = builder.add_extension(
        x509.SubjectKeyIdentifier.from_public_key(public_key),
        critical=False)
    issuer_ski = ca_certificate.extensions.get_extension_for_class(x509.SubjectKeyIdentifier)
    builder = builder.add_extension(
        x509.AuthorityKeyIdentifier.from_issuer_subject_key_identifier(issuer_ski.value),
        critical=False)

    # Sign certificate
    certificate_new = builder.sign(
        private_key=ca_private_key,
        algorithm=hashes.SHA256(),
        backend=get_backend())

    certificate = update_x509_certificate(certificate, certificate_new, cert_filename, encoding='PEM')

    return {'certificate': certificate}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Create a device certificate from its public key')
    parser.add_argument(
        '--serial-num',
        default=None,
        metavar='num',
        help='Device 9-byte serial number as an 18 character hex string. If omitted, a default will be used.')
    parser.add_argument(
        '--pub-key',
        default=None,
        metavar='xy',
        help='Device public key as raw X and Y integers as a hex string. If omitted, a default will be used.')
    parser.add_argument(
        '--cert',
        default='device.der',
        metavar='filename',
        help='Filename to write the device certificate to. If omitted, device.der will be used.')
    parser.add_argument(
        '--signer-id',
        default='FFFF',
        metavar='HexID',
        help='Signer ID to use as 4 hex digits. If omitted, FFFF will be used')
    parser.add_argument(
        '--signer-key',
        default='signer_FFFF.key',
        metavar='filename',
        help='Filename for the signer key file. If omitted, signer_FFFF.key will be used, where FFFF is the signer ID.')
    parser.add_argument(
        '--signer-cert',
        default='signer_FFFF.der',
        metavar='filename',
        help='Filename for the signer certificate file. If omitted, signer_FFFF.der will be used, where FFFF is the signer ID.')
    args = parser.parse_args()

    signer_id = args.signer_id.upper() # Signer ID must be uppercase hex
    ca_key_filename = Path(signer_id.join(args.signer_key.rsplit('FFFF', 1)))    # Replace the last instance of FFFF with the signer ID
    ca_cert_filename = Path(signer_id.join(args.signer_cert.rsplit('FFFF', 1)))  # Replace the last instance of FFFF with the signer ID

    ca_key_password = None
    if is_key_file_password_protected(ca_key_filename):
        # Prompt for the CA key file password
        ca_key_password = getpass.getpass(prompt=('{} password:'.format(ca_key_filename.name))).encode('ascii')

    load_or_create_device(
        serial_num=args.serial_num,
        public_key_hex=args.pub_key,
        cert_filename=args.cert,
        ca_key_filename=ca_key_filename,
        ca_key_password=ca_key_password,
        ca_cert_filename=ca_cert_filename)
