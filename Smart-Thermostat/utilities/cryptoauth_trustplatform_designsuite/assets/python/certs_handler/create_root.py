# (c) 2018 Microchip Technology Inc. and its subsidiaries.

# Subject to your compliance with these terms, you may use Microchip software
# and any derivatives exclusively with Microchip products. It is your
# responsibility to comply with third party license terms applicable to your
# use of third party software (including open source software) that may
# accompany Microchip software.

# THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
# EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
# WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A PARTICULAR
# PURPOSE. IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL,
# PUNITIVE, INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY
# KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP
# HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
# FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
# ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
# THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

from pathlib import Path
from datetime import datetime, timezone, timedelta
import argparse

from cryptography import x509
from cryptography.hazmat.primitives.serialization import Encoding, PublicFormat

from .create_certs_common import *
from .ext_builder import ExtBuilder


def load_or_create_root_ca(key_filename, cert_filename, org_name='Microchip Technology Inc',
    common_name='Crypto Authentication Root CA 002'):
    """
    Load a root CA private key and certificate from files. If it doesn't
    exist, new keys and/or certificate will be created and saved.
    """
    key_filename = Path(key_filename)
    cert_filename = Path(cert_filename)
    rebuild_cert = True

    # Load or create key pair
    private_key = load_or_create_key_pair(filename=key_filename)

    # Look for root certificate
    certificate = None
    if cert_filename.is_file():
        rebuild_cert = False
        # Found cached certificate file, read it in
        with open(str(cert_filename), 'rb') as f:
            certificate = x509.load_pem_x509_certificate(f.read(), get_backend())

    if certificate:
        if get_org_name(certificate.subject) != org_name:
            rebuild_cert = True

        cert_pub_bytes = certificate.public_key().public_bytes(format=PublicFormat.SubjectPublicKeyInfo, encoding=Encoding.DER)
        key_pub_bytes = private_key.public_key().public_bytes(format=PublicFormat.SubjectPublicKeyInfo, encoding=Encoding.DER)
        if cert_pub_bytes != key_pub_bytes:
            rebuild_cert = True

    if rebuild_cert:
        print("Building new root certificate")
        # Build new certificate
        builder = ExtBuilder()
        builder = builder.subject_name(x509.Name([
            x509.NameAttribute(x509.oid.NameOID.ORGANIZATION_NAME, org_name),
            x509.NameAttribute(x509.oid.NameOID.COMMON_NAME, common_name)]))
        builder = builder.issuer_name(builder._subject_name)  # Names are the same for a self-signed certificate
        builder = builder.not_valid_before(datetime.utcnow().replace(tzinfo=timezone.utc))
        builder = builder.not_valid_after(builder._not_valid_before + timedelta(days=365*40))
        #builder = builder.not_valid_after(datetime(9999, 12, 31, 23, 59, 59, tzinfo=timezone.utc))
        builder = builder.public_key(private_key.public_key())
        builder = builder.serial_number(random_cert_sn(16))
        builder = builder.add_extension(
            x509.SubjectKeyIdentifier.from_public_key(builder._public_key),
            critical=False)
        builder = builder.add_extension(
            x509.AuthorityKeyIdentifier.from_issuer_public_key(builder._public_key),
            critical=False)
        builder = builder.add_extension(
            x509.BasicConstraints(ca=True, path_length=None),
            critical=True)

        # Sign certificate with its own key
        certificate_new = builder.sign(
            private_key=private_key,
            algorithm=hashes.SHA256(),
            backend=get_backend())

        certificate = update_x509_certificate(certificate, certificate_new, cert_filename)
    else:
        print("Using cached root certificate")

    return {'private_key': private_key, 'certificate': certificate}


if __name__ == '__main__':
    # Create argument parser to document script use
    parser = argparse.ArgumentParser(description='Create a root key and self-signed certificate')
    parser.add_argument(
        '--key',
        default='ecosystem.key',
        metavar='filename',
        help=('Filename for root key. If no file is found, a new key will be created. Otherwise this key will be used.'
              ' If omitted, ecosystem.key will be used.'))
    parser.add_argument(
        '--cert',
        default='ecosystem.crt',
        metavar='filename',
        help='Filename to write root certificate to. If omitted, ecosystem.crt will be used.')
    args = parser.parse_args()

    load_or_create_root_ca(
        key_filename=args.key,
        cert_filename=args.cert)
