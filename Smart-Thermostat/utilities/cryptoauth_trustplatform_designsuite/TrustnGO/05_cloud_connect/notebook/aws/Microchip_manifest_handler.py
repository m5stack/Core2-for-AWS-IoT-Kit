"""
Load entries from a manifest into AWS IoT
"""

import json
import uuid
import boto3
import botocore
from base64 import b64decode
import jose.jws
from jose.utils import base64url_decode, base64url_encode
from cryptography import x509
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import hashes, serialization

_DEFAULT_POLICY = {
    'Version': '2012-10-17',
    'Statement': [{
            'Effect': 'Allow',
            'Action': [
                'iot:Connect'
            ],
            'Resource': [
                'arn:aws:iot:*:*:client/${iot:ClientId}'
            ]
        }, {
            'Effect': 'Allow',
            'Action': [
                'iot:Publish',
                'iot:Receive'
            ],
            'Resource': [
                'arn:aws:iot:*:*:topic/${iot:ClientId}/*',
                'arn:aws:iot:*:*:topic/$aws/things/${iot:ClientId}/shadow/*'
            ]
        }, {
            'Effect': 'Allow',
            'Action': [
                'iot:Subscribe'
            ],
            'Resource': [
                'arn:aws:iot:*:*:topicfilter/${iot:ClientId}/#',
                'arn:aws:iot:*:*:topicfilter/$aws/things/${iot:ClientId}/shadow/*'
            ]
        }, {
            'Effect': 'Allow',
            'Action': [
                'iot:UpdateThingShadow',
                'iot:GetThingShadow'
            ],
            'Resource': [
                'arn:aws:iot:*:*:topic/$aws/things/${iot:ClientId}/shadow/*'
            ]
        }
    ]
}

verification_algorithms = [
    'RS256', 'RS384', 'RS512', 'ES256', 'ES384', 'ES512'
]


def make_thing(device_id, certificate_arn):
    """Creates an AWS-IOT "thing" and attaches the certificate """
    client = boto3.client('iot')

    try:
        response = client.create_thing(thingName=device_id)
        thing_arn = response['thingArn']
    except:
        return False

    try:
        response = client.attach_thing_principal(thingName=device_id,
                                                 principal=certificate_arn)
    except:
        print("MANIFEST_IMPORT\t\tINFO\tattach_thing_principal failed.")
        return False

    return thing_arn


def fetch_manifest(bucket, manifest, pem):
    """Retrieves a manifest file from an S3 bucket"""
    client = boto3.client('s3')

    local_manifest_path = '/tmp/{}{}'.format(uuid.uuid4(), manifest)
    local_verify_cert_path = '/tmp/{}{}'.format(uuid.uuid4(), pem)

    client.download_file(bucket, manifest, local_manifest_path)
    client.download_file(bucket, pem, local_verify_cert_path)

    return local_verify_cert_path, local_manifest_path


def load_verify_cert_by_file(filename):
    """Load the verification certificate that will be used to verify manifest entries"""
    with open(filename, 'rb') as f:
        verification_cert = f.read()
    return verification_cert


def load_manifest_by_file(filename):
    """Open a manifest file"""
    with open(filename, 'rb') as f:
        manifest = json.load(f)
    return manifest


def import_certificate(certificate_x509_pem, policy_name):
    """Load a certificate from the manifest into AWS-IOT and attach a policy to it"""
    client = boto3.client('iot')
    print("\nTry importing certificate...")
    try:
        response = client.register_certificate_without_ca(certificatePem=certificate_x509_pem)
        print("Response: {}".format(response))
        print("Certificate import complete - returning")

        client.attach_policy(policyName=policy_name, target=response["certificateArn"])

        client.update_certificate(certificateId=response['certificateId'], newStatus='ACTIVE')

        return response["certificateArn"]
    except BaseException as e:
        print("Exception occurred: {}".format(e))

    # Need to check here for attribute error first
    return False


class _ManifestIterator:
    """Iterator for a loaded manifest"""
    def __init__(self, manifest):
        self.manifest = manifest
        self.index = len(manifest)

    def __iter__(self):
        return self

    def __next__(self):
        if self.index == 0:
            raise StopIteration
        self.index = self.index - 1
        return self.manifest[self.index]


class ManifestItem:

    def __init__(self, signed_se, verification_cert):
        self.signed_se = signed_se
        self.ski_ext = verification_cert.extensions.get_extension_for_class(
            extclass=x509.SubjectKeyIdentifier
        )

        self.verification_cert_kid_b64 = base64url_encode(
            self.ski_ext.value.digest
        ).decode('ascii')

        self.verification_public_key_pem = verification_cert.public_key().public_bytes(
            encoding=serialization.Encoding.PEM,
            format=serialization.PublicFormat.SubjectPublicKeyInfo
        ).decode('ascii')

        self.verification_cert_x5t_s256_b64 = base64url_encode(
            verification_cert.fingerprint(hashes.SHA256())
        ).decode('ascii')
        self.certificate_chain = ''
        self.run()

    def get_identifier(self):
        return self.identifier

    def get_certificate_chain(self):
        return self.certificate_chain

    def run(self):
        self.identifier = self.signed_se['header']['uniqueId']
        print('\tUnique ID: {}'.format(self.identifier.upper()))

        # Decode the protected header
        protected = json.loads(
            base64url_decode(
                self.signed_se['protected'].encode('ascii')
            )
        )
        if protected['kid'] != self.verification_cert_kid_b64:
            raise ValueError('kid does not match certificate value')
        if protected['x5t#S256'] != self.verification_cert_x5t_s256_b64:
            raise ValueError('x5t#S256 does not match certificate value')
        # Convert JWS to compact form as required by python-jose
        jws_compact = '.'.join([
            self.signed_se['protected'],
            self.signed_se['payload'],
            self.signed_se['signature']
        ])
        # Verify and decode the payload. If verification fails an exception will
        # be raised.
        se = json.loads(
            jose.jws.verify(
                token=jws_compact,
                key=self.verification_public_key_pem,
                algorithms=verification_algorithms
            ))
        try:
            public_keys = se['publicKeySet']['keys']
        except KeyError:
            public_keys = []
        for jwk in public_keys:
            cert_chain = ''
            for cert_b64 in jwk.get('x5c', []):
                cert = x509.load_der_x509_certificate(
                    data=b64decode(cert_b64),
                    backend=default_backend()
                )
                self.certificate_chain = self.certificate_chain + cert.public_bytes(
                    encoding=serialization.Encoding.PEM
                ).decode('ascii')


def invoke_import_manifest(policy_name, manifest, cert_pem):
    """Processes a manifest and loads entries into AWS-IOT"""

    verification_cert = x509.load_pem_x509_certificate(data=cert_pem, backend=default_backend())

    iterator = _ManifestIterator(manifest)
    print("Number of certificates: {}\n".format(iterator.index))

    while iterator.index != 0:
        print("Loading the manifest_item...")
        manifest_item = ManifestItem(next(iterator), verification_cert)

        certificate_arn = import_certificate(manifest_item.get_certificate_chain(), policy_name)

        thing_arn = make_thing(manifest_item.identifier, certificate_arn)

        if thing_arn is False:
            print("MANIFEST_IMPORT\t\tFAIL\t{}".format(certificate_arn))

        print("MANIFEST_IMPORT\t\tSUCCESS\t{}\t{}".format(certificate_arn, thing_arn))


def invokeImportLocal(skuname, manifest_filename, pem_filename):
    """Load a manifest into AWS-IOT from a local file"""
    cert = load_verify_cert_by_file(pem_filename)
    manifest = load_manifest_by_file(manifest_filename)
    policy_name = "{}-Policy".format(skuname)
    invoke_import_manifest(policy_name, manifest, cert)


def invokeImport(skuname, bucket, manifest, pem):
    """Load a manifest into AWS-IOT from a file retreived from an S3 bucket"""
    (manifest_verify_cert, manifest_file) = fetch_manifest(bucket, manifest, pem)
    invokeImportLocal(skuname, manifest_file, manifest_verify_cert)


def invoke_validate_manifest_import(manifest, cert_pem):
    """Checks to ensure the manifest was loaded"""
    verification_cert = x509.load_pem_x509_certificate(data=cert_pem, backend=default_backend())

    iterator = _ManifestIterator(manifest)
    print("\nNumber of ThingIDs to check: {}".format(iterator.index))

    client = boto3.client('iot')
    while iterator.index != 0:
        print("Checking the manifest item({})".format(iterator.index))
        manifest_item = ManifestItem(next(iterator), verification_cert)

        try:
            response = client.list_thing_principals(thingName=manifest_item.get_identifier())
            response = client.describe_certificate(certificateId=response['principals'][0].split('/')[-1])

            if response['certificateDescription']['certificatePem'] != manifest_item.get_certificate_chain():
                raise BaseException('Certificate Mismatch for {}'.format(manifest_item.get_identifier()))

        except BaseException as e:
            print("Exception occurred: {}".format(e))

    print("Manifest was loaded successfully")


def check_and_install_policy(policy_name='Default', policy_document=_DEFAULT_POLICY):
    client = boto3.client('iot')

    try:
        response = client.get_policy(policyName=policy_name)
    except botocore.exceptions.ClientError as e:
        if e.response['Error']['Code'] == 'ResourceNotFoundException':
            response = client.create_policy(
                policyName=policy_name,
                policyDocument=json.dumps(policy_document))
            print('Created policy {}'.format(policy_name))
        else:
            raise