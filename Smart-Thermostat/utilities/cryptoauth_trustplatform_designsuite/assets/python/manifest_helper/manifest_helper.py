import os
import json
import jose
import jose.jws
from jose.utils import base64url_decode, base64url_encode
from cryptography import x509
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import ec, utils as crypto_utils
from cryptography.utils import int_to_bytes
from base64 import b64decode, urlsafe_b64encode
from datetime import datetime, timezone, timedelta
from pathlib import Path


class Manifest():
    def __init__(self):
        self.version = 1
        self.model = ''
        self.partNumber = ''
        self.manufacturer = {
            'organizationName': '',
            'organizationalUnitName': ''}
        self.provisioner = {
            'organizationName': '',
            'organizationalUnitName': ''}
        self.distributor = {
            'organizationName': '',
            'organizationalUnitName': ''}
        self.provisioningTimestamp = ''
        self.uniqueId = ''
        self.publicKeySet = dict()

    def set_provisioning_time(self, time):
        """
        Initialize the device provisioning time stamp

        Inputs:
              time             device certificate provisioned time
        """
        self.provisioningTimestamp = time.strftime('%Y-%m-%dT%H:%M:%S.%f')[:-3] + 'Z'

    def set_unique_id(self, uniqueid):
        """
        Initialize the device unique id

        Inputs:
              uniqueid          device uniqueid which added into manifest
        """
        if isinstance(uniqueid, str):
            self.uniqueId = uniqueid
        elif isinstance(uniqueid, bytearray):
            self.uniqueId = uniqueid.hex()

    def set_publicJWK(self, kid, kty='', crv='', x=None, y=None, x5c=None):
        """
        Set JSON Web Keys (public keys and x509 certificates)

        Inputs:
              kid          key id or slot id of public/private key
              kty          key type
              crv          Algorithm curve type
              x            x component of public key
              y            y component of public key
              x5c          x509 certificates if any
        """
        publicJWK = dict()
        publicJWK['kid'] = kid
        if kty in ['EC', 'RSA', 'oct']:
            publicJWK['kty'] = kty
        if crv in ['P-256', 'P-384', 'P-521']:
            publicJWK['crv'] = crv
        if x is not None:
            publicJWK['x'] = x
        if y is not None:
            publicJWK['y'] = y
        if isinstance(x5c, list):
            publicJWK['x5c'] = x5c

        if 'keys' not in self.publicKeySet:
            self.publicKeySet['keys'] = []

        self.publicKeySet['keys'].append(publicJWK)

    def encode_manifest(self, ca_key_path=None, ca_cert_path=None):
        """
        Function generate secure signed element object which contains payload,
        protected, header and signature.

        Inputs:
              ca_key_path        log signer key path which sign the jws data
              ca_cert_path       log signer cert path which contains cert
                                 extension and cert fingerprint

        Outputs:
              signed_se          return encoded manifest which contains
                                 securesignedelement object
              ca_key_path        log signer key path which sign the jws data
              ca_cert_path       log signer cert path which contains cert
                                 extension and cert fingerprint
        """
        # Read ca key
        if ca_key_path is None:
            ca_cert_path = None

        ca_key_path, ca_key = self.__load_ca_key(ca_key_path)
        ca_cert_path, ca_cert = self.__load_ca_cert(ca_key, ca_cert_path)

        # Precompute the JWT header
        sse_protected_header = {
            'typ': 'JWT',
            'alg': 'ES256',
            'kid': self.__jws_b64encode(
                ca_cert.extensions.get_extension_for_class(
                    x509.SubjectKeyIdentifier).value.digest),
            'x5t#S256': self.__jws_b64encode(ca_cert.fingerprint(hashes.SHA256()))
        }

        se_object = self.get_manifest()
        signed_se = {
            'payload': self.__jws_b64encode(json.dumps(
                se_object).encode('ascii')),
            'protected': self.__jws_b64encode(
                json.dumps(sse_protected_header).encode('ascii')),
            'header': {
                'uniqueId': self.uniqueId
            }
        }

        # Sign sse object
        tbs = signed_se['protected'] + '.' + signed_se['payload']
        signature = ca_key.sign(tbs.encode('ascii'), ec.ECDSA(hashes.SHA256()))
        r_int, s_int = crypto_utils.decode_dss_signature(signature)
        signed_se['signature'] = self.__jws_b64encode(
            int_to_bytes(r_int, 32) + int_to_bytes(s_int, 32))

        return {
            'signed_se': signed_se,
            'ca_key_path': ca_key_path,
            'ca_cert_path': ca_cert_path
        }

    def decode_manifest(self, signed_se, ca_cert_path=None):
        """
        Function decode the SecureSignedElement Object by verifying the
        signature in it

        Inputs:
            signed_se       contains SecureSignedElement Object
            ca_cert_path    Path to verification cert to verify the signature
                            in it
        Outputs:
            se              contains secure element
        """
        # List out allowed verification algorithms for the JWS.
        # Only allows public-key based ones.
        verification_algorithms = [
            'RS256', 'RS384', 'RS512', 'ES256', 'ES384', 'ES512']

        # Load verification certificate in PEM format
        cert_data = Path(ca_cert_path).read_bytes()
        verification_cert = x509.load_pem_x509_certificate(
            cert_data, default_backend())

        # Convert verification certificate public key to PEM format
        verification_public_key_pem = verification_cert.public_key().public_bytes(
            encoding=serialization.Encoding.PEM,
            format=serialization.PublicFormat.SubjectPublicKeyInfo).decode('ascii')

        # Get the base64url encoded subject key identifier for the v
        # verification cert
        ski_ext = verification_cert.extensions.get_extension_for_class(
            extclass=x509.SubjectKeyIdentifier)
        verification_cert_kid_b64 = base64url_encode(
            ski_ext.value.digest).decode('ascii')

        # Get the base64url encoded sha-256 thumbprint for the verification
        # cert
        verification_cert_x5t_s256_b64 = base64url_encode(
            verification_cert.fingerprint(hashes.SHA256())).decode('ascii')

        # Decode the protected header
        protected = json.loads(base64url_decode(
            signed_se['protected'].encode('ascii')))

        if protected['kid'] != verification_cert_kid_b64:
            print('kid does not match certificate value')
            return {}

        if protected['x5t#S256'] != verification_cert_x5t_s256_b64:
            print('x5t#S256 does not match certificate value')
            return {}

        # Convert JWS to compact form as required by python-jose
        jws_compact = '.'.join([
            signed_se['protected'],
            signed_se['payload'],
            signed_se['signature']])

        # Verify and decode the payload. If verification fails an exception
        # will be raised.
        try:
            se_objects = jose.jws.verify(
                token=jws_compact,
                key=verification_public_key_pem,
                algorithms=verification_algorithms)
        except:
            print('Invalid Signature in Signed SE')
            return {}

        se = json.loads(se_objects)
        if se['uniqueId'] != signed_se['header']['uniqueId']:
            print('uniqueId in header "{}" does not match version in payload "{}"').format(
                signed_se['header']['uniqueId'], se['uniqueId'])
            return {}

        return se

    def extract_public_data_pem(self, se_object):
        '''
        Function extract the publickey JWK object from secure element and return it in pem format
        Inputs:
              se_object           contains secure element object
        Outputs:
              slots               contains public keys in pem format
        '''
        try:
            public_keys = se_object['publicKeySet']['keys']
        except KeyError:
            public_keys = []

        slots = []
        for jwk in public_keys:
            if jwk['kty'] not in ['EC', 'RSA', 'oct']:
                print('Unsupported {}'.format(json.dumps({'kty': jwk['kty']})))
                continue
            if jwk['crv'] not in ['P-256', 'P-384', 'P-521']:
                print('Unsupported {}'.format(json.dumps({'crv': jwk['crv']})))
                continue

            slot = {}
            slot.update({'id': jwk['kid']})
            public_key = ec.EllipticCurvePublicNumbers(
                curve=ec.SECP256R1(),
                x=int.from_bytes(base64url_decode(
                    jwk['x'].encode('utf8')), byteorder='big', signed=False),
                y=int.from_bytes(base64url_decode(
                    jwk['y'].encode('utf8')), byteorder='big', signed=False)
            ).public_key(default_backend())
            slot.update({'pub_key': public_key.public_bytes(
                encoding=serialization.Encoding.PEM,
                format=serialization.PublicFormat.SubjectPublicKeyInfo).decode('ascii')})

            # Decode any available certificates
            slot_certs = []
            for cert_b64 in jwk.get('x5c', []):
                cert = x509.load_der_x509_certificate(
                    data=b64decode(cert_b64), backend=default_backend())
                slot_certs.append(
                    cert.public_bytes(
                        encoding=serialization.Encoding.PEM).decode('ascii'))
            slot.update({'certs': slot_certs})
            slots.append(slot)

        return slots

    def get_manifest(self):
        return self.__dict__

    def __load_ca_cert(self, ca_key, ca_cert_path):
        '''
        Function load manifest certificate extension and fingerprint from given cert path or will
        create a new manifest certificate and load it from the same
        Inputs:
              ca_cert_path       manifest signer cert path which contains cert
                                 extension and cert fingerprint
        Outputs:
              ca_cert_path       manifest signer cert path which contains cert
                                 extension and cert fingerprint
              ca_cert            contains manifest signer cert
        '''
        if ca_cert_path is None:
            # Create root CA certificate
            builder = x509.CertificateBuilder()
            builder = builder.serial_number(self.__random_cert_sn(16))

            name = x509.Name([
                x509.NameAttribute(
                    x509.oid.NameOID.ORGANIZATION_NAME,
                    self.provisioner['organizationName']),
                x509.NameAttribute(
                    x509.oid.NameOID.COMMON_NAME,
                    self.provisioner['organizationalUnitName'])])
            valid_date = datetime.utcnow().replace(tzinfo=timezone.utc)

            builder = builder.issuer_name(name)
            builder = builder.not_valid_before(valid_date)
            builder = builder.not_valid_after(
                valid_date + timedelta(days=365 * 25))
            builder = builder.subject_name(name)
            builder = builder.public_key(ca_key.public_key())
            builder = builder.add_extension(
                x509.SubjectKeyIdentifier.from_public_key(ca_key.public_key()),
                critical=False)
            builder = builder.add_extension(
                x509.BasicConstraints(ca=True, path_length=None),
                critical=True)

            # Self-sign certificate
            logger_cert = builder.sign(
                private_key=ca_key,
                algorithm=hashes.SHA256(),
                backend=default_backend())

            # Write CA certificate to file
            ca_cert_path = Path('manifest_ca.crt')
            with open(ca_cert_path, 'wb') as f:
                f.write(logger_cert.public_bytes(
                    encoding=serialization.Encoding.PEM))

        cert_data = Path(ca_cert_path).read_bytes()
        ca_cert = x509.load_pem_x509_certificate(cert_data, default_backend())

        return ca_cert_path, ca_cert

    def __load_ca_key(self, ca_key_path):
        '''
        Function create new ca key if given path is not found or load a key from existing path
        Inputs:
              ca_key_path        manifest signer key path which sign the secure element
        Outputs:
              ca_key             manifest signer key
              ca_key_path        manifest signer key path which sign the secure element
        '''
        if ca_key_path is None:
            ca_key_path = Path('manifest_ca.key')
            ca_key = ec.generate_private_key(
                curve=ec.SECP256R1(),
                backend=default_backend()).private_bytes(
                    encoding=serialization.Encoding.PEM,
                    format=serialization.PrivateFormat.PKCS8,
                    encryption_algorithm=serialization.NoEncryption())
            with open(str(ca_key_path), 'wb') as f:
                f.write(ca_key)

        ca_key_path = Path(ca_key_path)
        assert ca_key_path.is_file(), 'File Not found'
        with open(str(ca_key_path), 'rb') as f:
            ca_key = serialization.load_pem_private_key(
                data=f.read(),
                password=None,
                backend=default_backend())

        return ca_key_path, ca_key

    def __random_cert_sn(self, size):
        '''Create a positive, non-trimmable serial number for X.509
        certificates'''
        raw_sn = bytearray(os.urandom(size))
        # Force MSB bit to 0 to ensure positive
        # integer
        raw_sn[0] = raw_sn[0] & 0x7F
        # Force next bit to 1 to ensure the
        # integer won't be trimmed in ASN.1 DER encoding
        raw_sn[0] = raw_sn[0] | 0x40

        return int.from_bytes(raw_sn, byteorder='big', signed=False)

    def __jws_b64encode(self, source):
        """Simple helper function to remove base64 padding"""
        return urlsafe_b64encode(source).decode('ascii').rstrip('=')

    def write_signed_se_into_file(self, signed_se, filename):
        """
        Write signed manifest data (SecureSignedElement object) into given file

        Inputs:
                signed_se           contains SignedSecureElement Object
                filename            path to JSON file to dump the signedsecureelement object
        """
        if os.path.exists(filename):
            with open(filename, 'rb') as f:
                manifest_file = json.load(f)

            for secure_elem in manifest_file:
                if secure_elem['header']['uniqueId'] == signed_se['header']['uniqueId']:
                    manifest_file.remove(secure_elem)
                    break

            manifest_file.append(signed_se)
            file_manifest = json.dumps(manifest_file, indent=2).encode('ascii')
        else:
            file_manifest = json.dumps([signed_se], indent=2).encode('ascii')

        with open(filename, 'wb') as f:
            f.write(file_manifest)


# Standard boilerplate to call the main() function to begin
# the program.
if __name__ == '__main__':
    pass
