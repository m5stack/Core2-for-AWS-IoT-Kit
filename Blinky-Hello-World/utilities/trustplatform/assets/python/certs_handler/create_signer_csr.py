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


import argparse
import re
from pathlib import Path

from cryptography import x509
from cryptography.hazmat.primitives.serialization import Encoding, PublicFormat

from .create_certs_common import *


def load_or_create_signer_csr(signer_id, key_filename, csr_filename,
    org_name='Microchip Technology Inc', common_name='Crypto Authentication Signer'):
    # Validate signer ID
    if re.search('^[0-9A-F]{4}$', signer_id) is None:
        raise ValueError('signer_id={} must be 4 uppercase hex digits'.format(signer_id))

    key_filename = Path(key_filename)
    csr_filename = Path(csr_filename)
    rebuild_cert = True

    # Load or create key pair
    private_key = load_or_create_key_pair(filename=key_filename)

    csr = None
    if csr_filename.is_file():
        rebuild_cert = False
        # Found cached CSR file, read it in
        with open(str(csr_filename), 'rb') as f:
            csr = x509.load_pem_x509_csr(f.read(), get_backend())

    if csr:
        if get_org_name(csr.subject) != org_name:
            rebuild_cert = True

        csr_pub_bytes = csr.public_key().public_bytes(format=PublicFormat.SubjectPublicKeyInfo, encoding=Encoding.DER)
        key_pub_bytes = private_key.public_key().public_bytes(format=PublicFormat.SubjectPublicKeyInfo, encoding=Encoding.DER)
        if csr_pub_bytes != key_pub_bytes:
            rebuild_cert = True

    if rebuild_cert:
        # Build new certificate
        print("Building new signer csr certificate")
        builder = x509.CertificateSigningRequestBuilder()
        builder = builder.subject_name(x509.Name([
            x509.NameAttribute(x509.oid.NameOID.ORGANIZATION_NAME, org_name),
            x509.NameAttribute(x509.oid.NameOID.COMMON_NAME, common_name + signer_id)]))

        # Add extensions
        builder = add_signer_extensions(
            builder=builder,
            public_key=private_key.public_key())

        csr_new = builder.sign(
            private_key=private_key,
            algorithm=hashes.SHA256(),
            backend=get_backend())

        csr = update_csr(csr, csr_new, csr_filename)
    else:
        print("Using cached signer csr certificate")

    return {'private_key': private_key, 'csr': csr}


def add_signer_extensions(builder, public_key=None, authority_cert=None):
    if public_key is None:
        public_key = builder._public_key # Public key not specified, assume its in the builder (cert builder)

    builder = builder.add_extension(
        x509.KeyUsage(
            digital_signature=True,
            content_commitment=False,
            key_encipherment=False,
            data_encipherment=False,
            key_agreement=False,
            key_cert_sign=True,
            crl_sign=True,
            encipher_only=False,
            decipher_only=False),
        critical=True)
    builder = builder.add_extension(
        x509.BasicConstraints(ca=True, path_length=0),
        critical=True)
    builder = builder.add_extension(
        x509.SubjectKeyIdentifier.from_public_key(public_key),
        critical=False)
    subj_key_id_ext = builder._extensions[-1] # Save newly created subj key id extension

    if authority_cert:
        # We have an authority certificate, use its subject key id
        builder = builder.add_extension(
            x509.AuthorityKeyIdentifier.from_issuer_subject_key_identifier(
                authority_cert.extensions.get_extension_for_class(x509.SubjectKeyIdentifier).value),
            critical=False)
    else:
        # No authority cert, assume this is a CSR and just use its own subject key id
        builder = builder.add_extension(
            x509.AuthorityKeyIdentifier.from_issuer_subject_key_identifier(subj_key_id_ext.value),
            critical=False)

    return builder


if __name__ == '__main__':
    # Create argument parser to document script use
    parser = argparse.ArgumentParser(description='Create a signer key and CSR')
    parser.add_argument(
        '--signer-id',
        default='FFFF',
        metavar='HexID',
        help='Signer ID to use as 4 hex digits. If omitted, FFFF will be used')
    parser.add_argument(
        '--key',
        default='signer_FFFF.key',
        metavar='filename',
        help=('Filename for signer key. If no file is found, a new key will be created. Otherwise this key will be used.'
              ' If omitted, signer_FFFF.key will be used, where FFFF is the signer ID.'))
    parser.add_argument(
        '--csr',
        default='signer_FFFF.csr',
        metavar='filename',
        help='Filename to write signer CSR to. If omitted, signer_FFFF.csr will be used, where FFFF is the signer ID.')
    args = parser.parse_args()

    signer_id = args.signer_id.upper() # Signer ID must be uppercase hex
    key_filename = signer_id.join(args.key.rsplit('FFFF', 1)) # Replace the last instance of FFFF with the signer ID
    csr_filename = signer_id.join(args.csr.rsplit('FFFF', 1)) # Replace the last instance of FFFF with the signer ID

    load_or_create_signer_csr(
        signer_id=signer_id,
        key_filename=key_filename,
        csr_filename=csr_filename)
