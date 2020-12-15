"""
TrustnGO Manifest generation
"""
# (c) 2015-2019 Microchip Technology Inc. and its subsidiaries.
#
# Subject to your compliance with these terms, you may use Microchip software
# and any derivatives exclusively with Microchip products. It is your
# responsibility to comply with third party license terms applicable to your
# use of third party software (including open source software) that may
# accompany Microchip software.
#
# THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
# EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
# WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
# PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT,
# SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE
# OF ANY KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF
# MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE
# FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL
# LIABILITY ON ALL CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED
# THE AMOUNT OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR
# THIS SOFTWARE.

import base64
import json, os
from cryptoauthlib import *
from cryptography import x509
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.utils import int_to_bytes, int_from_bytes
from certs_handler import *
from manifest_helper.tng_manifest_helper import TNGTLSManifest


def read_tng_certificate_chain_from_device():
    """Read the TNG keys and certificate chains from the device."""

    certs = []

    print('Root Certificate loading from Device...', end='')
    root_cert_der_size = AtcaReference(0)
    assert tng_atcacert_root_cert_size(root_cert_der_size) == Status.ATCA_SUCCESS

    root_cert_der = bytearray(root_cert_der_size.value)
    assert tng_atcacert_root_cert(root_cert_der, root_cert_der_size) == Status.ATCA_SUCCESS

    root_cert = x509.load_der_x509_certificate(root_cert_der, default_backend())
    certs.insert(0, root_cert)
    print('OK')
    print(certs_handler.get_cert_print_bytes(root_cert.public_bytes(encoding=Encoding.PEM)))

    # Note that we could, of course, pull this from the root certificate above.
    # However, this demonstrates the tng_atcacert_root_public_key() function.
    root_public_key_raw = bytearray(64)
    assert tng_atcacert_root_public_key(root_public_key_raw) == Status.ATCA_SUCCESS

    root_public_key = ec.EllipticCurvePublicNumbers(
        curve=ec.SECP256R1(),
        x=int_from_bytes(root_public_key_raw[0:32], byteorder='big'),
        y=int_from_bytes(root_public_key_raw[32:64], byteorder='big'),
    ).public_key(default_backend())

    # Prove that cert public key and the public key from the func are the same
    cert_spk_der = root_cert.public_key().public_bytes(
        format=PublicFormat.SubjectPublicKeyInfo,
        encoding=Encoding.DER
    )
    func_spk_der = root_public_key.public_bytes(
        format=PublicFormat.SubjectPublicKeyInfo,
        encoding=Encoding.DER
    )
    assert cert_spk_der == func_spk_der

    print('Validate Root Certificate...', end='')
    root_public_key.verify(
        signature=root_cert.signature,
        data=root_cert.tbs_certificate_bytes,
        signature_algorithm=ec.ECDSA(root_cert.signature_hash_algorithm)
    )
    print('OK')
    print('------------------------------------------------------')

    print('Signer Certificate loading from Device...',end='')
    signer_cert_der_size = AtcaReference(0)
    assert tng_atcacert_max_signer_cert_size(signer_cert_der_size) == Status.ATCA_SUCCESS

    signer_cert_der = bytearray(signer_cert_der_size.value)
    assert tng_atcacert_read_signer_cert(signer_cert_der, signer_cert_der_size) == Status.ATCA_SUCCESS

    signer_cert = x509.load_der_x509_certificate(signer_cert_der, default_backend())
    certs.insert(0, signer_cert)
    print('OK')
    print(certs_handler.get_cert_print_bytes(signer_cert.public_bytes(encoding=Encoding.PEM)))

    # Note that we could, of course, pull this from the signer certificate above.
    # However, this demonstrates the tng_atcacert_signer_public_key() function.
    signer_public_key_raw = bytearray(64)
    assert tng_atcacert_signer_public_key(signer_public_key_raw) == Status.ATCA_SUCCESS

    signer_public_key = ec.EllipticCurvePublicNumbers(
        curve=ec.SECP256R1(),
        x=int_from_bytes(signer_public_key_raw[0:32], byteorder='big'),
        y=int_from_bytes(signer_public_key_raw[32:64], byteorder='big'),
    ).public_key(default_backend())

    # Prove that cert public key and the public key from the func are the same
    cert_spk_der = signer_cert.public_key().public_bytes(
        format=PublicFormat.SubjectPublicKeyInfo,
        encoding=Encoding.DER
    )
    func_spk_der = signer_public_key.public_bytes(
        format=PublicFormat.SubjectPublicKeyInfo,
        encoding=Encoding.DER
    )
    assert cert_spk_der == func_spk_der


    # Note that this is a simple cryptographic validation and does not check
    # any of the actual certificate data (validity dates, extensions, names,
    # etc...)
    print('Validate Signer Certificate...', end='')
    root_public_key.verify(
        signature=signer_cert.signature,
        data=signer_cert.tbs_certificate_bytes,
        signature_algorithm=ec.ECDSA(signer_cert.signature_hash_algorithm)
    )
    print('OK')
    print('------------------------------------------------------')

    print('Device Certificate loading from Device...', end='')
    device_cert_der_size = AtcaReference(0)
    assert tng_atcacert_max_device_cert_size(device_cert_der_size) == Status.ATCA_SUCCESS

    device_cert_der = bytearray(device_cert_der_size.value)
    assert tng_atcacert_read_device_cert(device_cert_der, device_cert_der_size) == Status.ATCA_SUCCESS

    device_cert = x509.load_der_x509_certificate(device_cert_der, default_backend())
    certs.insert(0, device_cert)
    print('OK')
    print(certs_handler.get_cert_print_bytes(device_cert.public_bytes(encoding=Encoding.PEM)))

    # Note that we could, of course, pull this from the device certificate above.
    # However, this demonstrates the tng_atcacert_device_public_key() function.
    device_public_key_raw = bytearray(64)
    assert tng_atcacert_device_public_key(device_public_key_raw, device_cert_der) == Status.ATCA_SUCCESS

    device_public_key = ec.EllipticCurvePublicNumbers(
        curve=ec.SECP256R1(),
        x=int_from_bytes(device_public_key_raw[0:32], byteorder='big'),
        y=int_from_bytes(device_public_key_raw[32:64], byteorder='big'),
    ).public_key(default_backend())

    # Prove that cert public key and the public key from the func are the same
    cert_spk_der = device_cert.public_key().public_bytes(
        format=PublicFormat.SubjectPublicKeyInfo,
        encoding=Encoding.DER
    )
    func_spk_der = device_public_key.public_bytes(
        format=PublicFormat.SubjectPublicKeyInfo,
        encoding=Encoding.DER
    )
    assert cert_spk_der == func_spk_der


    # Note that this is a simple cryptographic validation and does not check
    # any of the actual certificate data (validity dates, extensions, names,
    # etc...)

    print('Validate Device Certificate...', end='')
    signer_public_key.verify(
        signature=device_cert.signature,
        data=device_cert.tbs_certificate_bytes,
        signature_algorithm=ec.ECDSA(device_cert.signature_hash_algorithm)
    )
    print('OK')
    print('------------------------------------------------------')

    return {
        'root_cert': root_cert,
        'signer_cert': signer_cert,
        'device_cert': device_cert
    }


def generate_tngtls_manifest():
    """ Function encode the trustngo manifest data and generate securesigned element
    by signing manifest data and store it in manifest file """
    # Get the certificate chain
    cert_info = read_tng_certificate_chain_from_device()

    filename = 'TNGTLS_devices_manifest.json'
    manifest_ca_key_path = 'manifest_ca.key'
    manifest_ca_cert_path = 'manifest_ca.crt'
    print('--------------------------------------------')
    print("Generating manifest data...", end='')
    # Initializa the TNGTLS Class
    tng_manifest = TNGTLSManifest()
    # set the manifest unique id and public keys
    tng_manifest.load_manifest_uniqueid_and_keys()
    # set the device certificate provisioning time
    tng_manifest.set_provisioning_time(cert_info['device_cert'].not_valid_before)
    # set the signer certificates and device certificates
    tng_manifest.set_certs(cert_info['signer_cert'], cert_info['device_cert'], kid='0')
    # Encode the manifest data
    if os.path.exists(manifest_ca_key_path) and os.path.exists(manifest_ca_cert_path):
        signed_se = tng_manifest.encode_manifest(manifest_ca_key_path, manifest_ca_cert_path)
    else:
        signed_se = tng_manifest.encode_manifest()
    # Dump signed secure element into .json file
    tng_manifest.write_signed_se_into_file(signed_se['signed_se'], filename)
    print('OK (saved to {})'.format(filename))
    print('--------------------------------------------')
    # print the manifest data
    # print(tng_manifest.get_manifest())

