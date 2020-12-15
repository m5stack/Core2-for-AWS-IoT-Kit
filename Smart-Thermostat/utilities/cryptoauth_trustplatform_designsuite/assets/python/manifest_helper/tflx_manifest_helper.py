"""
TrustFLEX Manifest generation
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
from cryptoauthlib import *
from jose import jws, utils
from cryptography import x509
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import serialization
from manifest_helper import Manifest


class TFLXTLSManifest(Manifest):
    def __init__(self):
        super().__init__()
        self.model = 'ATECC608B'
        self.partNumber = 'ATECC608B-TFLXTLS'
        self.manufacturer = {
            'organizationName': 'Microchip Technology Inc',
            'organizationalUnitName': 'Secure Products Group'}
        self.provisioner = {
            'organizationName': 'Microchip Technology Inc',
            'organizationalUnitName': 'Secure Products Group'}
        self.distributor = {
            'organizationName': 'Microchip Technology Inc',
            'organizationalUnitName': 'Microchip Direct'}

    def set_keys(self):
        for ec_priv_key in [0, 1, 2, 3, 4]:
            public_key = bytearray()
            assert atcab_get_pubkey(ec_priv_key, public_key) == Status.ATCA_SUCCESS, 'Reading Pub key failed'
            x = utils.base64url_encode(public_key[0:32]).decode('ascii')
            y = utils.base64url_encode(public_key[32:64]).decode('ascii')
            self.set_publicJWK(str(ec_priv_key), 'EC', 'P-256', x, y, x5c=None)

        for ec_pub_key in [13, 14, 15]:
            public_key = bytearray()
            assert atcab_read_pubkey(ec_pub_key, public_key) == Status.ATCA_SUCCESS, 'Reading Pub key failed'
            x = utils.base64url_encode(public_key[0:32]).decode('ascii')
            y = utils.base64url_encode(public_key[32:64]).decode('ascii')
            self.set_publicJWK(str(ec_pub_key), 'EC', 'P-256', x, y, x5c=None)

    def set_certs(self, signer_cert=None, device_cert=None, kid=''):
        for key in self.publicKeySet['keys']:
            if kid == key['kid']:
                key['x5c'] = [
                    base64.b64encode(device_cert.public_bytes(encoding=serialization.Encoding.DER)).decode('ascii'),
                    base64.b64encode(signer_cert.public_bytes(encoding=serialization.Encoding.DER)).decode('ascii')
                ]

    def set_uniqueid(self):
        ser_num = bytearray(9)
        assert atcab_read_serial_number(ser_num) == Status.ATCA_SUCCESS
        super().set_unique_id(ser_num)

    def load_manifest_uniqueid_and_keys(self):
        self.set_uniqueid()
        self.set_keys()


# Standard boilerplate to call the main() function to begin
# the program.
if __name__ == '__main__':
    pass
