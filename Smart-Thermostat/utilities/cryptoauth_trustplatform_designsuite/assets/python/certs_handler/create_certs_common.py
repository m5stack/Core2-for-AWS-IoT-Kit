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

import os
from pathlib import Path

import cryptography
from cryptography.hazmat.primitives import serialization, hashes
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography import x509
from cryptography.x509.oid import NameOID
from cryptography import x509
import asn1crypto.x509
import asn1crypto.csr

from .timefix_backend import backend


_backend = None
def get_backend():
    global _backend
    if not _backend:
        _backend = backend
    return _backend

def get_org_name(name):
    """
    Get the org name string from a distinguished name (RDNSequence)
    """
    for attr in name:
        if attr.oid == x509.oid.NameOID.ORGANIZATION_NAME:
            return attr.value
    return None

def load_or_create_key_pair(filename):
    """
    Load an EC P256 private key from file or create a new one a save it if the
    key file doesn't exist.
    """
    filename = Path(filename)
    priv_key = None
    if filename.is_file():
        # Load existing key
        with open(str(filename), 'rb') as f:
            priv_key = serialization.load_pem_private_key(
                data=f.read(),
                password=None,
                backend=get_backend())
    if priv_key is None:
        # No existing private key found, generate new private key
        priv_key = ec.generate_private_key(
            curve=ec.SECP256R1(),
            backend=get_backend())

        # Save private key to file
        with open(str(filename), 'wb') as f:
            pem_key = priv_key.private_bytes(
                encoding=serialization.Encoding.PEM,
                format=serialization.PrivateFormat.PKCS8,
                encryption_algorithm=serialization.NoEncryption())
            f.write(pem_key)
    return priv_key


def random_cert_sn(size):
    """Create a positive, non-trimmable serial number for X.509 certificates"""
    raw_sn = bytearray(os.urandom(size))
    raw_sn[0] = raw_sn[0] & 0x7F # Force MSB bit to 0 to ensure positive integer
    raw_sn[0] = raw_sn[0] | 0x40 # Force next bit to 1 to ensure the integer won't be trimmed in ASN.1 DER encoding
    return int.from_bytes(raw_sn, byteorder='big', signed=False)


def pubkey_cert_sn(size, builder):
    """Cert serial number is the SHA256(Subject public key + Encoded dates)"""

    # Get the public key as X and Y integers concatenated
    pub_nums = builder._public_key.public_numbers()
    pubkey =  pub_nums.x.to_bytes(32, byteorder='big', signed=False)
    pubkey += pub_nums.y.to_bytes(32, byteorder='big', signed=False)

    # Get the encoded dates
    expire_years = builder._not_valid_after.year - builder._not_valid_before.year
    if builder._not_valid_after.year == 9999:
        expire_years = 0 # This year is used when indicating no expiration
    elif expire_years > 31:
        expire_years = 1 # We default to 1 when using a static expire beyond 31

    enc_dates = bytearray(b'\x00'*3)
    enc_dates[0] = (enc_dates[0] & 0x07) | ((((builder._not_valid_before.year - 2000) & 0x1F) << 3) & 0xFF)
    enc_dates[0] = (enc_dates[0] & 0xF8) | ((((builder._not_valid_before.month) & 0x0F) >> 1) & 0xFF)
    enc_dates[1] = (enc_dates[1] & 0x7F) | ((((builder._not_valid_before.month) & 0x0F) << 7) & 0xFF)
    enc_dates[1] = (enc_dates[1] & 0x83) | (((builder._not_valid_before.day & 0x1F) << 2) & 0xFF)
    enc_dates[1] = (enc_dates[1] & 0xFC) | (((builder._not_valid_before.hour & 0x1F) >> 3) & 0xFF)
    enc_dates[2] = (enc_dates[2] & 0x1F) | (((builder._not_valid_before.hour & 0x1F) << 5) & 0xFF)
    enc_dates[2] = (enc_dates[2] & 0xE0) | ((expire_years & 0x1F) & 0xFF)
    enc_dates = bytes(enc_dates)

    # SAH256 hash of the public key and encoded dates
    digest = hashes.Hash(hashes.SHA256(), backend=cryptography.hazmat.backends.default_backend())
    digest.update(pubkey)
    digest.update(enc_dates)
    raw_sn = bytearray(digest.finalize()[:size])
    raw_sn[0] = raw_sn[0] & 0x7F # Force MSB bit to 0 to ensure positive integer
    raw_sn[0] = raw_sn[0] | 0x40 # Force next bit to 1 to ensure the integer won't be trimmed in ASN.1 DER encoding
    return int.from_bytes(raw_sn, byteorder='big', signed=False)


def is_key_file_password_protected(key_filename):
    try:
        with open(key_filename, 'rb') as f:
            root_ca_priv_key = serialization.load_pem_private_key(data=f.read(), password=None, backend=get_backend())
        return False
    except TypeError:
        return True


def update_x509_certificate(certificate, certificate_new, cert_filename, encoding='PEM'):
    """
    Compare the TBS portion of two X.509 certificates and save the new
    certificate (PEM format) if the TBS has changed.
    """
    if encoding == 'PEM':
        encoding = serialization.Encoding.PEM
    elif encoding == 'DER':
        encoding = serialization.Encoding.DER
    else:
        raise ValueError('Unknown encoding {}'.format(encoding))

    is_new = False
    if certificate:
        # Check to see if the certificate has changed from what was saved
        certificate_tbs = asn1crypto.x509.Certificate.load(
            certificate.public_bytes(encoding=serialization.Encoding.DER))['tbs_certificate']
        certificate_new_tbs = asn1crypto.x509.Certificate.load(
            certificate_new.public_bytes(encoding=serialization.Encoding.DER))['tbs_certificate']
        is_new = (certificate_tbs.dump() != certificate_new_tbs.dump())
    else:
        is_new = True

    if is_new:
        # Write certificate to file
        with open(str(cert_filename), 'wb') as f:
            f.write(certificate_new.public_bytes(encoding=encoding))
        certificate = certificate_new
    return certificate


def update_csr(csr, csr_new, csr_filename):
    """
    Compare the certificationRequestInfo portion of two CSRs and save the new
    CSR (PEM format) if the certificationRequestInfo has changed.
    """
    is_new = False
    if csr:
        # Check to see if the CSR has changed from what was saved
        csr_cri = asn1crypto.csr.CertificationRequest.load(
            csr.public_bytes(encoding=serialization.Encoding.DER))['certification_request_info']
        csr_new_cri = asn1crypto.csr.CertificationRequest.load(
            csr_new.public_bytes(encoding=serialization.Encoding.DER))['certification_request_info']
        is_new = (csr_cri.dump() != csr_new_cri.dump())
    else:
        is_new = True

    if is_new:
        # Write CSR to file
        with open(str(csr_filename), 'wb') as f:
            f.write(csr_new.public_bytes(encoding=serialization.Encoding.PEM))
        csr = csr_new
    return csr

def get_device_sn(cert_subj):
    """
    Check the device common name and return the device serial number  """
    for attr in cert_subj:
        if attr.oid == x509.oid.NameOID.COMMON_NAME:
            x = attr.value.find("0123")
            if (x != -1):
                return attr.value[x:x+18]
            else:
                return None
    return None