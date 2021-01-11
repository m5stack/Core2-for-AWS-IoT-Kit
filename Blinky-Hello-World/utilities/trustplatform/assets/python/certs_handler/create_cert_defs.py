# (c) 2020 Microchip Technology Inc. and its subsidiaries.

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

import argparse
import re
from pathlib import Path
from ctypes import cast, c_uint8, POINTER, string_at, pointer

from cryptography import x509
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import serialization
import asn1crypto
from asn1crypto import pem
from cryptoauthlib import *

from .x509_find_elements import *


def read_cert(cert_path):
    cert_data = Path(cert_path).read_bytes()
    if pem.detect(cert_data):
        return x509.load_pem_x509_certificate(cert_data, default_backend())
    else:
        return x509.load_der_x509_certificate(cert_data, default_backend())


def create_cert_def(cert, template_id, chain_id, chain_level, signer_id, serial_num=None, ca_cert_def=None):
    if chain_level != 0 and chain_level != 1:
        raise ValueError(f'Unsupported chain_level {chain_level}')

    signer_id = signer_id.upper()
    if re.search('^[0-9A-F]{4}$', signer_id) is None:
        raise ValueError(f'signer_id={signer_id} must be 4 uppercase hex digits')

    if serial_num is not None:
        serial_num = serial_num.upper()
        if re.search('^[0-9A-F]{18}$', serial_num) is None:
            raise ValueError('serial_num is not hex string of 18 digits (9 bytes)')

    template_data = cert.public_bytes(encoding=serialization.Encoding.DER)
    asn1_cert = asn1crypto.x509.Certificate().load(template_data, strict=True)

    elements = []
    if serial_num is not None:
        sn03_offset, sn03_count = name_search_location(asn1_cert, 'subject', serial_num[0:8])
        elements.append(
            {
                'id': 'SN03',
                'device_loc': {
                    'zone': atcacert_device_zone_t.DEVZONE_CONFIG,
                    'slot': 0,
                    'is_genkey': 0,
                    'offset': 0,
                    'count': 4
                },
                'cert_loc': {
                    'offset': sn03_offset,
                    'count': sn03_count
                },
                'transforms': [
                    atcacert_transform_t.TF_BIN2HEX_UC,
                    atcacert_transform_t.TF_NONE
                ]
            }
        )
        sn48_offset, sn48_count = name_search_location(asn1_cert, 'subject', serial_num[8:18])
        elements.append(
            {
                'id': 'SN48',
                'device_loc': {
                    'zone': atcacert_device_zone_t.DEVZONE_CONFIG,
                    'slot': 0,
                    'is_genkey': 0,
                    'offset': 8,
                    'count': 5
                },
                'cert_loc': {
                    'offset': sn48_offset,
                    'count': sn48_count
                },
                'transforms': [
                    atcacert_transform_t.TF_BIN2HEX_UC,
                    atcacert_transform_t.TF_NONE
                ]
            }
        )

    tbs_offset, tbs_count = tbs_location(asn1_cert)
    expire_years = cert.not_valid_after.year - cert.not_valid_before.year
    if expire_years > 31:
        expire_years = 0  # Assume no expiration
    pk_offset, pk_count = public_key_location(asn1_cert)
    sig_offset, sig_count = signature_location(asn1_cert)
    nb_offset, nb_count = validity_location(asn1_cert, 'not_before')
    na_offset, na_count = validity_location(asn1_cert, 'not_after')
    sid_offset, sid_count = name_search_location(
        cert=asn1_cert,
        name='issuer' if chain_level == 0 else 'subject',
        search=signer_id
    )
    sn_offset, sn_count = sn_location(asn1_cert)
    akid_offset, akid_count = auth_key_id_location(asn1_cert)
    skid_offset, skid_count = subj_key_id_location(asn1_cert)

    params = {
        'type': atcacert_cert_type_t.CERTTYPE_X509,
        'template_id': template_id,
        'chain_id': chain_id,
        'private_key_slot': 0,
        'sn_source': atcacert_cert_sn_src_t.SNSRC_PUB_KEY_HASH,
        'cert_sn_dev_loc': {
            'zone': atcacert_device_zone_t.DEVZONE_NONE,
            'slot': 0,
            'is_genkey': 0,
            'offset': 0,
            'count': 0,
        },
        'issue_date_format': atcacert_date_format_t.DATEFMT_RFC5280_UTC,
        'expire_date_format': atcacert_date_format_t.DATEFMT_RFC5280_GEN,
        'tbs_cert_loc': {
            'offset': tbs_offset,
            'count': tbs_count
        },
        'expire_years': expire_years,
        'public_key_dev_loc': {
            'zone': atcacert_device_zone_t.DEVZONE_DATA,
            'slot': 0 if chain_level == 0 else 11,
            'is_genkey': 1 if chain_level == 0 else 0,
            'offset': 0,
            'count': 64 if chain_level == 0 else 72
        },
        'comp_cert_dev_loc': {
            'zone': atcacert_device_zone_t.DEVZONE_DATA,
            'slot': 10 if chain_level == 0 else 12,
            'is_genkey': 0,
            'offset': 0,
            'count': 72
        },
        'std_cert_elements': [
            {'offset': pk_offset, 'count': pk_count},      # STDCERT_PUBLIC_KEY
            {'offset': sig_offset, 'count': sig_count},    # STDCERT_SIGNATURE
            {'offset': nb_offset, 'count': nb_count},      # STDCERT_ISSUE_DATE
            {'offset': na_offset, 'count': na_count},      # STDCERT_EXPIRE_DATE
            {'offset': sid_offset, 'count': sid_count},    # STDCERT_SIGNER_ID
            {'offset': sn_offset, 'count': sn_count},      # STDCERT_CERT_SN
            {'offset': akid_offset, 'count': akid_count},  # STDCERT_AUTH_KEY_ID
            {'offset': skid_offset, 'count': skid_count}   # STDCERT_SUBJ_KEY_ID
        ],
        'ca_cert_def': pointer(ca_cert_def) if ca_cert_def else None
    }

    cert_def = atcacert_def_t(**params)
    cert_def.cert_template_size = len(template_data)
    cert_def.cert_template = cast(create_string_buffer(template_data, cert_def.cert_template_size), POINTER(c_uint8))

    if elements:
        cert_def.cert_elements_count = len(elements)
        elems = [atcacert_cert_element_t(**x) for x in elements]
        elems_array = (atcacert_cert_element_t * cert_def.cert_elements_count)(*elems)
        cert_def.cert_elements = cast(elems_array, POINTER(atcacert_cert_element_t))

    return cert_def


def c_hex_array(data, indent='    ', bytes_per_line=16):
    lines = []
    for i in range(0, len(data), bytes_per_line):
        lines.append(indent + ', '.join([f'0x{v:02x}' for v in data[i:i+bytes_per_line]]) + ',')
    return '{\n' + '\n'.join(lines) + '\n}'


def device_loc_to_c(device_loc, indent=''):
    lines = [
        f'.zone = {atcacert_device_zone_t(device_loc.zone).name}',
        f'.slot = {device_loc.slot}',
        f'.is_genkey = {device_loc.is_genkey}',
        f'.offset = {device_loc.offset}',
        f'.count = {device_loc.count}',
    ]

    return ',\n'.join([f'{indent}{line}' for line in lines])


def cert_loc_to_c(cert_loc, indent=''):
    lines = [
        f'.offset = {cert_loc.offset}',
        f'.count = {cert_loc.count}',
    ]

    return ',\n'.join([f'{indent}{line}' for line in lines])


def create_cert_def_c_files(cert_def, chain_level, ca_cert=None, ca_include_filename=None, ca_cert_def_var_name=None):
    if chain_level == 0:
        cert_def_name = 'device'
    elif chain_level == 1:
        cert_def_name = 'signer'
    else:
        raise ValueError(f'Unsupported chain_level {chain_level}')

    template_var_name = f'g_template_{cert_def.template_id}_{cert_def_name}'
    pk_var_name = f'g_cert_ca_public_key_{cert_def.template_id}_{cert_def_name}'
    elements_var_name = f'g_cert_elements_{cert_def.template_id}_{cert_def_name}'
    cert_def_var_name = f'g_cert_def_{cert_def.template_id}_{cert_def_name}'

    c_file = ''
    c_file += '#include "atcacert/atcacert_def.h"\n'
    if ca_include_filename:
        c_file += f'#include "{ca_include_filename}"\n'
    c_file += '\n'

    template_data = string_at(cert_def.cert_template, cert_def.cert_template_size)
    c_file += f'const uint8_t {template_var_name}[{len(template_data)}] = {c_hex_array(template_data)};\n'
    c_file += '\n'

    if chain_level == 1:
        pk_data = ca_cert.public_key().public_bytes(
            format=serialization.PublicFormat.UncompressedPoint,
            encoding=serialization.Encoding.X962
        )[1:]
        c_file += f'const uint8_t {pk_var_name}[{len(pk_data)}] = {c_hex_array(pk_data)};\n'
        c_file += '\n'

    if cert_def.cert_elements_count:
        element_strs = []
        for i in range(cert_def.cert_elements_count):
            element = cert_def.cert_elements[i]
            element_str = ''
            element_str += f'    {{\n'
            element_str += f'        .id = "{element.id.decode("ascii")}",\n'
            element_str += f'        .device_loc = {{\n'
            element_str += device_loc_to_c(element.device_loc, indent='            ') + '\n'
            element_str += f'        }},\n'
            element_str += f'        .cert_loc = {{\n'
            element_str += cert_loc_to_c(element.cert_loc, indent='            ') + '\n'
            element_str += f'        }},\n'
            element_str += f'        .transforms = {{\n'
            element_str += ',\n'.join([f'            {atcacert_transform_t(v).name}' for v in element.transforms]) + '\n'
            element_str += f'        }}\n'
            element_str += f'    }}'
            element_strs.append(element_str)

        c_file += f'const atcacert_cert_element_t {elements_var_name}[{cert_def.cert_elements_count}] = {{\n'
        c_file += ',\n'.join(element_strs) + '\n'
        c_file += '};\n'
        c_file += '\n'

    c_file += f'const atcacert_def_t {cert_def_var_name} = {{\n'
    c_file += f'    .type = {atcacert_cert_type_t(cert_def.type).name},\n'
    c_file += f'    .template_id = {cert_def.template_id},\n'
    c_file += f'    .chain_id = {cert_def.chain_id},\n'
    c_file += f'    .private_key_slot = {cert_def.private_key_slot},\n'
    c_file += f'    .sn_source = {atcacert_cert_sn_src_t(cert_def.sn_source).name},\n'
    c_file += f'    .cert_sn_dev_loc = {{\n'
    c_file += device_loc_to_c(cert_def.cert_sn_dev_loc, indent='        ') + '\n'
    c_file += f'    }},\n'
    c_file += f'    .issue_date_format = {atcacert_date_format_t(cert_def.issue_date_format).name},\n'
    c_file += f'    .expire_date_format = {atcacert_date_format_t(cert_def.expire_date_format).name},\n'
    c_file += f'    .tbs_cert_loc = {{\n'
    c_file += cert_loc_to_c(cert_def.tbs_cert_loc, indent='        ') + '\n'
    c_file += f'    }},\n'
    c_file += f'    .expire_years = {cert_def.expire_years},\n'
    c_file += f'    .public_key_dev_loc = {{\n'
    c_file += device_loc_to_c(cert_def.public_key_dev_loc, indent='        ') + '\n'
    c_file += f'    }},\n'
    c_file += f'    .comp_cert_dev_loc = {{\n'
    c_file += device_loc_to_c(cert_def.comp_cert_dev_loc, indent='        ') + '\n'
    c_file += f'    }},\n'
    std_element_names = [
        'STDCERT_PUBLIC_KEY',
        'STDCERT_SIGNATURE',
        'STDCERT_ISSUE_DATE',
        'STDCERT_EXPIRE_DATE',
        'STDCERT_SIGNER_ID',
        'STDCERT_CERT_SN',
        'STDCERT_AUTH_KEY_ID',
        'STDCERT_SUBJ_KEY_ID'
    ]
    c_file += f'    .std_cert_elements = {{\n'
    for i, name in enumerate(std_element_names):
        c_file += f'        {{ // {name}\n'
        c_file += cert_loc_to_c(cert_def.std_cert_elements[i], indent='            ') + '\n'
        c_file += f'        }},\n'
    c_file += f'    }},\n'
    if cert_def.cert_elements:
        c_file += f'    .cert_elements = {elements_var_name},\n'
        c_file += f'    .cert_elements_count = sizeof({elements_var_name}) / sizeof({elements_var_name}[0]),\n'
    else:
        c_file += f'    .cert_elements = NULL,\n'
        c_file += f'    .cert_elements_count = 0,\n'
    c_file += f'    .cert_template = {template_var_name},\n'
    c_file += f'    .cert_template_size = sizeof({template_var_name}),\n'
    if ca_cert_def_var_name:
        c_file += f'    .ca_cert_def = &{ca_cert_def_var_name}\n'
    else:
        c_file += f'    .ca_cert_def = NULL\n'
    c_file += f'}};\n'

    filename_base = f'cust_def_{cert_def.template_id}_{cert_def_name}'

    h_file = ''
    h_file += f'#ifndef {filename_base.upper()}_H\n'
    h_file += f'#define {filename_base.upper()}_H\n'
    h_file += f'\n'
    h_file += f'#include "atcacert/atcacert_def.h"\n'
    h_file += f'\n'
    h_file += f'#ifdef __cplusplus\n'
    h_file += f'extern "C" {{\n'
    h_file += f'#endif\n'
    h_file += f'extern const atcacert_def_t {cert_def_var_name};\n'
    if chain_level == 1:
        h_file += f'extern const uint8_t {pk_var_name}[];\n'
    h_file += f'#ifdef __cplusplus\n'
    h_file += f'}}\n'
    h_file += f'#endif\n'
    h_file += '\n'
    h_file += '#endif\n'

    return {
        f'{filename_base}.c': c_file.encode('utf-8'),
        f'{filename_base}.h': h_file.encode('utf-8')
    }


def create_signer_cert_def(cert, template_id, chain_id, signer_id):
    return create_cert_def(
        cert=cert,
        template_id=template_id,
        chain_id=chain_id,
        chain_level=1,
        signer_id=signer_id
    )


def create_device_cert_def(cert, template_id, chain_id, signer_id, serial_num=None, signer_cert_def=None):
    if not serial_num:
        serial_num = None
    return create_cert_def(
        cert=cert,
        template_id=template_id,
        chain_id=chain_id,
        chain_level=0,
        signer_id=signer_id,
        serial_num=serial_num,
        ca_cert_def=signer_cert_def
    )


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Create signer and device atcacert_def_t c files.')
    parser.add_argument(
        '--ecosystem-cert',
        default='ecosystem.crt',
        metavar='filename',
        help='Ecosystem certificate filename. If omitted, ecosystem.crt is used.')
    parser.add_argument(
        '--signer-cert',
        default='signer_FFFF.der',
        metavar='filename',
        help='Filename for the signer certificate file. If omitted, signer_FFFF.der is used.')
    parser.add_argument(
        '--device-cert',
        default='device.der',
        metavar='filename',
        help='Filename for the device template certificate file. If omitted, device.der is used.')
    parser.add_argument(
        '--signer-id',
        default='FFFF',
        metavar='HexID',
        help='Signer ID to use as 4 hex digits. If omitted, FFFF will be used')
    args = parser.parse_args()

    signer_id = args.signer_id.upper()  # Signer ID must be uppercase hex
    signer_cert_path = signer_id.join(args.signer_cert.rsplit('FFFF', 1))  # Replace the last instance of FFFF with the signer ID

    signer_cert_def = create_signer_cert_def(
        cert=read_cert(signer_cert_path),
        template_id=1,
        chain_id=0,
        signer_id=signer_id
    )
    signer_files = create_cert_def_c_files(
        cert_def=signer_cert_def,
        chain_level=1,
        ca_cert=read_cert(args.ecosystem_cert)
    )
    for file_name, file_data in signer_files.items():
        Path(file_name).write_bytes(file_data)
        if file_name[-2:] == '.h':
            signer_include_filename = file_name
            match = re.search(r'atcacert_def_t\s+([a-zA-Z0-9_]+)', file_data.decode('utf-8'))
            signer_cert_def_var_name = match.group(1)

    device_cert_def = create_device_cert_def(
        cert=read_cert(args.device_cert),
        template_id=2,
        chain_id=0,
        signer_id=signer_id,
        serial_num='012301020304050601',
        signer_cert_def=signer_cert_def
    )

    device_files = create_cert_def_c_files(
        cert_def=device_cert_def,
        chain_level=0,
        ca_include_filename=signer_include_filename,
        ca_cert_def_var_name=signer_cert_def_var_name
    )
    for file_name, file_data in device_files.items():
        Path(file_name).write_bytes(file_data)
