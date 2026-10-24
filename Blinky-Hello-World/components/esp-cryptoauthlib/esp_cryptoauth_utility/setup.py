#!/usr/bin/env python
# Copyright 2022 Espressif Systems (Shanghai) Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
import sys

def get_install_requires():
    with open('requirements.txt') as f:
        required = f.read().splitlines()
        return required

try:
    from setuptools import find_packages, setup
except ImportError:
    print(
        'Package setuptools is missing from your Python installation. '
        'Please see the installation section in the esp-cryptoauth-utillity documentation'
        ' for instructions on how to install it.'
    )
    exit(1)

VERSION = '1.2.0'

long_description = """
======================
esp-cryptoauth-utility
======================
The python utility helps to configure and provision ATECC608 chip connected to an ESP module. Currently the utility is supported for ESP32, ESP32-S3, ESP32-C2, ESP32-C3, ESP32-C5, ESP32-C6, ESP32-C61, ESP32-H2 and ESP32-P4.

The esp-cryptoauth-utility is `hosted on github <https://github.com/espressif/esp-cryptoauthlib/tree/master/esp_cryptoauth_utility>`_.

Documentation
-------------
Visit online `esp-cryptoauth-utility documentation <https://github.com/espressif/esp-cryptoauthlib/tree/master/esp_cryptoauth_utility#readme/>`_ \
or run ``secure_cert_mfg.py -h``.

License
-------
The License for the project can be found `here <https://github.com/espressif/esp-cryptoauthlib/blob/master/esp_cryptoauth_utility/LICENSE>`_
"""

setup(
    name='esp-cryptoauth-utility',
    version=VERSION,
    description='A python utility which helps to configure and provision ATECC608 chip connected to an ESP module',
    long_description=long_description,
    long_description_content_type='text/x-rst',
    url='https://github.com/espressif/esp-cryptoauthlib/tree/master/esp_cryptoauth_utility',
    project_urls={
        'Documentation': 'https://github.com/espressif/esp-cryptoauthlib/tree/master/esp_cryptoauth_utility#readme',
        'Source': 'https://github.com/espressif/esp-cryptoauthlib/tree/master/esp_cryptoauth_utility',
    },
    author='Espressif Systems',
    author_email='',
    classifiers=[
        'Intended Audience :: Developers',
        'Natural Language :: English',
        'Operating System :: POSIX',
        'Operating System :: Microsoft :: Windows',
        'Operating System :: MacOS :: MacOS X',
        'Topic :: Software Development :: Embedded Systems',
        'Programming Language :: Python :: 3.7',
        'Programming Language :: Python :: 3.8',
        'Programming Language :: Python :: 3.9',
        'Programming Language :: Python :: 3.10',
    ],
    python_requires='>=3.7',
    setup_requires=(['wheel'] if 'bdist_wheel' in sys.argv else []),
    install_requires=get_install_requires(),
    include_package_data = True,
    packages=find_packages(),
    scripts=['secure_cert_mfg.py'],
    license_files = [],
)
