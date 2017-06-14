# Copyright 2017 CodiLime
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import argparse
import collections
from enum import Enum
import os

from OpenSSL import crypto


class UrlScheme(Enum):
    UNIX_SCHEME = 'veles+unix'
    TCP_SCHEME = 'veles'
    SSL_SCHEME = 'veles+ssl'


def prepare_auth_key(key):
    return bytes(bytearray.fromhex(key) + b'\x00' * (64 - len(key)//2))


def get_client_argparse():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        'server_url',
        help='server URL in form of: VELES[+SSL|+UNIX]://auth_key[:cert-'
             'fingerprint]@([host]:port|path) '
             '\nThere are 3 supported schemas: VELES - TCP socket, '
             'VELES+SSL - SSL socket '
             'VELES+UNIX - UNIX socket (not available on Windows)')
    return parser


def get_logging_argparse():
    parser = argparse.ArgumentParser(add_help=False)
    parser.add_argument(
        '--log-level', default='INFO',
        help='set log level to one of CRITICAL, ERROR, WARNING, INFO or DEBUG',
        choices=['CRITICAL', 'ERROR', 'WARNING', 'INFO', 'DEBUG'])
    return parser


def parse_url(url):
    scheme, url = url.split('://', 1)
    scheme = UrlScheme(scheme.lower())
    auth, loc = url.split('@', 1)
    auth = auth.split(':', 1)
    auth_key = auth[0]
    if len(auth) == 1:
        fingerprint = None
    else:
        fingerprint = auth[1].replace(':', '')
    if scheme == UrlScheme.UNIX_SCHEME:
        path = loc
        host, port = None, None
    else:
        path = None
        host, _, port = loc.rpartition(':')
        port = int(port)
    Url = collections.namedtuple(
        'URL', ['scheme', 'auth_key', 'fingerprint', 'host', 'port', 'path'])
    return Url(scheme, auth_key, fingerprint, host, port, path)


def generate_ssl_cert(cert_path, key_path):
    key = crypto.PKey()
    key.generate_key(crypto.TYPE_RSA, 2048)

    cert = crypto.X509()
    cert.get_subject().C = "RE"
    cert.get_subject().O = "VELES"
    cert.set_serial_number(1000)
    cert.gmtime_adj_notBefore(0)
    cert.gmtime_adj_notAfter(10 * 365 * 24 * 60 * 60)
    cert.set_issuer(cert.get_subject())
    cert.set_pubkey(key)
    cert.sign(key, 'sha256')

    with open(cert_path, 'w') as f:
        f.write(crypto.dump_certificate(crypto.FILETYPE_PEM, cert).decode())

    with os.fdopen(
            os.open(key_path, os.O_WRONLY | os.O_CREAT, 0o400), 'w') as f:
        f.write(crypto.dump_privatekey(crypto.FILETYPE_PEM, key).decode())


def validate_cert(cert, fingerprint):
    cert = crypto.load_certificate(crypto.FILETYPE_ASN1, cert)
    remote_fingerprint = cert.digest('sha256').decode()
    if fingerprint != remote_fingerprint.replace(':', ''):
        raise ValueError(
            'Certificate fingerprint mismatch! '
            'expected: {}, got: {}'.format(
                fingerprint, remote_fingerprint))
