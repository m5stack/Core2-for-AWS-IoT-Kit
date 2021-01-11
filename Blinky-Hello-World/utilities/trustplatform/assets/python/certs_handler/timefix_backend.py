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
#
# Uses modified code from the cryptography package https://github.com/pyca/cryptography
# Copyright under the individual contributors and licensed under Apache License, Version 2.0

from cryptography.hazmat.backends.openssl.backend import Backend
from cryptography import utils, x509
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import dsa, ec, rsa
from cryptography.hazmat.backends.openssl.encode_asn1 import (
    _CRL_ENTRY_EXTENSION_ENCODE_HANDLERS,
    _CRL_EXTENSION_ENCODE_HANDLERS, _EXTENSION_ENCODE_HANDLERS,
    _encode_asn1_int_gc, _encode_asn1_str_gc, _encode_name_gc, _txt2obj_gc,
)
from cryptography.hazmat.backends.openssl.x509 import _Certificate

from .ext_builder import ExtBuilder, TimeFormat


def _format_asn1_time_str(date, format=TimeFormat.AUTO):
    if date.tzinfo:
        date = (date - date.utcoffset()).replace(tzinfo=None)
    if format == TimeFormat.UTC_TIME and (time_utc.year < 1950 or time_utc.year > 2049):
        raise ValueError('time must be between 1950 and 2049 for UTC_TIME')
    if (format == TimeFormat.AUTO and date.year >= 1950 and date.year <= 2049) or format == TimeFormat.UTC_TIME:
        date_str = date.strftime("%y%m%d%H%M%SZ")  # UTCTime with 2-digit year
    else:
        date_str = date.strftime("%Y%m%d%H%M%SZ")  # GeneralizedTime with 4-digit year
    return date_str.encode('ascii')


class TimeFixBackend(Backend):
    """
    Creates a backend that can set extended dates on X.509 certificates on
    windows. Normally trying to use the RFC5280-recommended maximum date of
    99991231235959Z to indicate no expiration fails on windows due to date
    handling limitations.
    """

    def create_x509_certificate(self, builder, private_key, algorithm):
        if not isinstance(builder, x509.CertificateBuilder):
            raise TypeError('Builder type mismatch.')
        if not isinstance(algorithm, hashes.HashAlgorithm):
            raise TypeError('Algorithm must be a registered hash algorithm.')

        if (
            isinstance(algorithm, hashes.MD5) and not
            isinstance(private_key, rsa.RSAPrivateKey)
        ):
            raise ValueError(
                "MD5 is not a supported hash algorithm for EC/DSA certificates"
            )

        # Resolve the signature algorithm.
        evp_md = self._lib.EVP_get_digestbyname(
            algorithm.name.encode('ascii')
        )
        self.openssl_assert(evp_md != self._ffi.NULL)

        # Create an empty certificate.
        x509_cert = self._lib.X509_new()
        x509_cert = self._ffi.gc(x509_cert, backend._lib.X509_free)

        # Set the x509 version.
        res = self._lib.X509_set_version(x509_cert, builder._version.value)
        self.openssl_assert(res == 1)

        # Set the subject's name.
        res = self._lib.X509_set_subject_name(
            x509_cert, _encode_name_gc(self, builder._subject_name)
        )
        self.openssl_assert(res == 1)

        # Set the subject's public key.
        res = self._lib.X509_set_pubkey(
            x509_cert, builder._public_key._evp_pkey
        )
        self.openssl_assert(res == 1)

        # Set the certificate serial number.
        serial_number = _encode_asn1_int_gc(self, builder._serial_number)
        res = self._lib.X509_set_serialNumber(x509_cert, serial_number)
        self.openssl_assert(res == 1)

        # Set the "not before" time.
        if isinstance(builder, ExtBuilder):
            time_str = _format_asn1_time_str(builder._not_valid_before, builder._not_valid_before_format)
        else:
            time_str = _format_asn1_time_str(builder._not_valid_before)
        res = self._lib.ASN1_TIME_set_string(
            self._lib.X509_get_notBefore(x509_cert),
            time_str
        )
        if res == self._ffi.NULL:
            self._raise_time_set_error()

        # Set the "not after" time.
        if isinstance(builder, ExtBuilder):
            time_str = _format_asn1_time_str(builder._not_valid_after, builder._not_valid_after_format)
        else:
            time_str = _format_asn1_time_str(builder._not_valid_after)
        res = self._lib.ASN1_TIME_set_string(
            self._lib.X509_get_notAfter(x509_cert),
            time_str
        )
        if res == self._ffi.NULL:
            self._raise_time_set_error()

        # Add extensions.
        self._create_x509_extensions(
            extensions=builder._extensions,
            handlers=_EXTENSION_ENCODE_HANDLERS,
            x509_obj=x509_cert,
            add_func=self._lib.X509_add_ext,
            gc=True
        )

        # Set the issuer name.
        res = self._lib.X509_set_issuer_name(
            x509_cert, _encode_name_gc(self, builder._issuer_name)
        )
        self.openssl_assert(res == 1)

        # Sign the certificate with the issuer's private key.
        res = self._lib.X509_sign(
            x509_cert, private_key._evp_pkey, evp_md
        )
        if res == 0:
            errors = self._consume_errors()
            self.openssl_assert(
                errors[0]._lib_reason_match(
                    self._lib.ERR_LIB_RSA,
                    self._lib.RSA_R_DIGEST_TOO_BIG_FOR_RSA_KEY
                )
            )
            raise ValueError("Digest too big for RSA key")

        return _Certificate(self, x509_cert)


backend = TimeFixBackend()

