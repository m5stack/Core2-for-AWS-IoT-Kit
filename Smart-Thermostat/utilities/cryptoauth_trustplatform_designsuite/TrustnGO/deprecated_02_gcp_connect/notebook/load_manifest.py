import os
import argparse
from pathlib import Path
import json
from datetime import datetime
from base64 import b64decode
from jose import jws
from jose.utils import base64url_encode, base64url_decode
from cryptography.hazmat.backends import default_backend
from cryptography import x509
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.serialization import Encoding, PublicFormat
from cryptography.hazmat.primitives.asymmetric import ec
from google.cloud import iot_v1
import csv

gcp_account_CSV = "GCP_test_account_credentials.csv"

home_path = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.realpath(__file__)))))
GCP_ACCOUNT_CREDENTIALS = os.path.join(home_path, 'docs', gcp_account_CSV)

def convert_manifest(mchp_manifest, public_key_slot, signer_cert_pem=None):
    signer_cert = None
    if signer_cert_pem is not None:
        signer_cert = x509.load_pem_x509_certificate(signer_cert_pem, default_backend())

        # Validate the log signer certificate validity period
        now = datetime.utcnow()
        if now < signer_cert.not_valid_before or now > signer_cert.not_valid_after:
            raise ValueError('Log signer certificate has expired.')

        # Convert certificate public key into PEM format for jws api
        public_key_pem = signer_cert.public_key().public_bytes(
            encoding=Encoding.PEM,
            format=PublicFormat.SubjectPublicKeyInfo
        ).decode('ascii')

        ext = signer_cert.extensions.get_extension_for_class(x509.SubjectKeyIdentifier)
        kid = base64url_encode(ext.value.digest).decode('ascii')
        x5t_s256 = base64url_encode(signer_cert.fingerprint(hashes.SHA256())).decode('ascii')

    manifest = []
    for raw_entry in mchp_manifest:
        if signer_cert:
            # Validate and decode signed entry
            jws_compact = '.'.join([raw_entry['protected'], raw_entry['payload'], raw_entry['signature']])
            entry = json.loads(
                jws.verify(
                    token=jws_compact,
                    key=public_key_pem,
                    algorithms=['ES256']
                )
            )
            protected = json.loads(base64url_decode(raw_entry['protected'].encode('ascii')))
            if protected['kid'] != kid:
                raise ValueError('kid does not match value calculated from certificate')
            if protected['x5t#S256'] != x5t_s256:
                raise ValueError('x5t#S256 does not match value calculated from certificate')
            if raw_entry['header']['uniqueId'] != entry['uniqueId']:
                raise ValueError('uniqueId in unprotected header does not match value in payload')
        else:
            entry = json.loads(base64url_decode(raw_entry['protected']))

        # Search public key list for the auth public key specified
        auth_public_key = None
        public_keys = entry.get('publicKeySet', {}).get('keys', [])
        for public_key in public_keys:
            if public_key.get('kid', None) == str(public_key_slot):
                auth_public_key = public_key
                break
        if auth_public_key is None:
            raise ValueError('Auth public key slot {} not found.'.format(public_key_slot))

        if auth_public_key.get('kty', None) != 'EC' or auth_public_key.get('crv', None) != 'P-256':
            raise ValueError('Unsupported public key type for ZTP-EAP format')

        if auth_public_key.get('x5c', []):
            # Use certificate
            cert = x509.load_der_x509_certificate(b64decode(auth_public_key['x5c'][0]), default_backend())
            entry_public_key = {
                'format': 'ES256_X509_PEM',
                'key': cert.public_bytes(encoding=Encoding.PEM).decode('utf8')
            }
        else:
            public_key = ec.EllipticCurvePublicNumbers(
                curve=ec.SECP256R1(),
                x=int.from_bytes(base64url_decode(auth_public_key['x'].encode('utf8')), byteorder='big', signed=False),
                y=int.from_bytes(base64url_decode(auth_public_key['y'].encode('utf8')), byteorder='big', signed=False)
            ).public_key(default_backend())
            entry_public_key = {
                'format': 'ES256_PEM',
                'key': public_key.public_bytes(
                    encoding=Encoding.PEM,
                    format=PublicFormat.SubjectPublicKeyInfo
                ).decode('utf8')
            }

        manifest += [{
            'id': 'd' + entry['uniqueId'].upper(),
            'credentials': [{
                'public_key': entry_public_key
            }],
        }]

    return manifest


def load_manifest(project_id, registry_id, region_id, manifest):
    client = iot_v1.DeviceManagerClient()
    parent = client.registry_path(project_id, region_id, registry_id)

    for device in manifest:
        client.create_device(parent, device)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='GCP Device Provisioning')
    parser.add_argument('-r', '--registry', default='default_regid', help='Registry Id')
    parser.add_argument('-l', '--location', default='us-central1', help='Location or Region Id as it is known in the console')
    parser.add_argument('-c', '--creds', default='iot_manifest.json',help='Credential Json File')
    parser.add_argument(
        'in',
        help='Microchip manifest file to convert',
        metavar='path',
        type=str,
    )
    parser.add_argument(
        '--signer-cert',
        help='Log signer certificate file. If omitted, manifest entries are not validated.',
        metavar='path',
        type=str,
        default=None
    )
    parser.add_argument(
        '--auth-key-slot',
        help='Slot of the public key to use for the primary authorization key',
        metavar='num',
        type=int,
        default=0
    )
    args = parser.parse_args()

    in_path = Path(getattr(args, 'in'))
    mchp_manifest = json.loads(in_path.read_bytes())

    signer_cert = None
    if args.signer_cert is not None:
        signer_cert = Path(args.signer_cert).read_bytes()

    print('Converting Manifest\n')
    manifest = convert_manifest(mchp_manifest, args.auth_key_slot, signer_cert)

    if args.creds is not None:
        os.environ["GOOGLE_APPLICATION_CREDENTIALS"] = args.creds

    with open(os.environ["GOOGLE_APPLICATION_CREDENTIALS"]) as f:
        credentials = json.load(f)
        project = credentials['project_id']

    print('Loading Manifest\n')
    load_manifest(project, args.registry, args.location, manifest)
    print('Done')

def gcp_upload_manifest(credential_file,signer_cert,mchp_manifest):


    print('Converting Manifest\n')
    manifest = convert_manifest(mchp_manifest, 0, signer_cert)

    os.environ["GOOGLE_APPLICATION_CREDENTIALS"] = credential_file

    with open(os.environ["GOOGLE_APPLICATION_CREDENTIALS"]) as f:
        credentials = json.load(f)
        project = credentials['project_id']

    with open(GCP_ACCOUNT_CREDENTIALS, mode='r') as csv_file:
        csv_reader = csv.DictReader(csv_file)
        for row in csv_reader:
            registry_id = row["Registry ID"]
            region_id = row["Region"]

    if registry_id == 'replace_your_registry_id_here':
        print('Configure the file {} in docs folder with your GCP registry id before proceeding'.format(gcp_account_CSV))
        return False

    print('Loading Manifest\n')
    load_manifest(project, registry_id, region_id, manifest)
    print('Device registered succesfully')
    return True