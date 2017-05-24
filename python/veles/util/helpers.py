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


def prepare_auth_key(key):
    return bytes(bytearray.fromhex(key) + b'\x00' * (64 - len(key)//2))


def get_client_argparse():
    parser = argparse.ArgumentParser()
    parser.add_argument('server', help='server address')
    parser.add_argument('server_auth_key', help='hex-encoded server auth-key')
    return parser

def get_logging_argparse():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '--log-level', default='INFO',
        help='set log level to one of CRITICAL, ERROR, WARNING, INFO or DEBUG',
        choices = ['CRITICAL', 'ERROR', 'WARNING', 'INFO', 'DEBUG'])
    return parser
