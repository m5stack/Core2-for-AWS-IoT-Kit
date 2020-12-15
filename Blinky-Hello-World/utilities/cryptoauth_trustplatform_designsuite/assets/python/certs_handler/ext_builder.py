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

import enum
from cryptography import x509


class TimeFormat(enum.IntEnum):
    AUTO = 0x00
    UTC_TIME = 0x17
    GENERALIZED_TIME = 0x18


class ExtBuilder(x509.CertificateBuilder):
    def __init__(self, issuer_name=None, subject_name=None,
                 public_key=None, serial_number=None, not_valid_before=None,
                 not_valid_before_format=TimeFormat.AUTO, not_valid_after=None,
                 not_valid_after_format=TimeFormat.AUTO, extensions=[]):
        super().__init__(
            issuer_name,
            subject_name,
            public_key,
            serial_number,
            not_valid_before,
            not_valid_after,
            extensions)
        self._not_valid_before_format = not_valid_before_format
        self._not_valid_after_format = not_valid_after_format

    @staticmethod
    def from_super(builder, not_valid_before_format=None,
                   not_valid_after_format=None):
        return ExtBuilder(
            builder._issuer_name,
            builder._subject_name,
            builder._public_key,
            builder._serial_number,
            builder._not_valid_before,
            not_valid_before_format,
            builder._not_valid_after,
            not_valid_after_format,
            builder._extensions)

    def issuer_name(self, name):
        """
        Sets the CA's distinguished name.
        """
        return ExtBuilder.from_super(
            super().issuer_name(name),
            self._not_valid_before_format,
            self._not_valid_after_format)

    def subject_name(self, name):
        """
        Sets the requestor's distinguished name.
        """
        return ExtBuilder.from_super(
            super().subject_name(name),
            self._not_valid_before_format,
            self._not_valid_after_format)

    def public_key(self, key):
        """
        Sets the requestor's public key (as found in the signing request).
        """
        return ExtBuilder.from_super(
            super().public_key(key),
            self._not_valid_before_format,
            self._not_valid_after_format)

    def serial_number(self, number):
        """
        Sets the certificate serial number.
        """
        return ExtBuilder.from_super(
            super().serial_number(number),
            self._not_valid_before_format,
            self._not_valid_after_format)

    def not_valid_before(self, time, format=TimeFormat.AUTO):
        """
        Sets the certificate activation time.
        """
        if not isinstance(format, TimeFormat):
            raise TypeError('format must be TimeFormat object')
        # Normalize time to UTC before the range check as that could change the year
        if time.tzinfo:
            time_utc = (time - time.utcoffset()).replace(tzinfo=None)
        else:
            time_utc = time
        if format == TimeFormat.UTC_TIME and (time_utc.year < 1950 or time_utc.year > 2049):
            raise ValueError('time must be between 1950 and 2049 for UTC_TIME')
        return ExtBuilder.from_super(
            super().not_valid_before(time),
            format,
            self._not_valid_after_format)

    def not_valid_after(self, time, format=TimeFormat.AUTO):
        """
        Sets the certificate expiration time.
        """
        if not isinstance(format, TimeFormat):
            raise TypeError('format must be TimeFormat object')
        # Normalize time to UTC before the range check as that could change the year
        if time.tzinfo:
            time_utc = (time - time.utcoffset()).replace(tzinfo=None)
        else:
            time_utc = time
        if format == TimeFormat.UTC_TIME and (time_utc.year < 1950 or time_utc.year > 2049):
            raise ValueError('time must be between 1950 and 2049 for UTC_TIME')
        return ExtBuilder.from_super(
            super().not_valid_after(time),
            self._not_valid_before_format,
            format)

    def add_extension(self, extension, critical):
        """
        Adds an X.509 extension to the certificate.
        """
        return ExtBuilder.from_super(
            super().add_extension(extension, critical),
            self._not_valid_before_format,
            self._not_valid_after_format)
