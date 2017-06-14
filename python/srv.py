#!/usr/bin/env python3

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

import logging
import asyncio
import platform
import signal
import importlib

from veles.server.conn import AsyncLocalConnection
from veles.server.proto import (create_unix_server, create_tcp_server,
                                create_ssl_server)
from veles.util import helpers

parser = helpers.get_logging_argparse()
parser.add_argument(
    'url', help='URL that the server will listen on, in form of: '
                'VELES[+SSL|+UNIX]://auth_key@([host]:port|path) '
                '\nThere are 3 supported schemas: VELES - TCP socket, '
                'VELES+SSL - SSL socket '
                'VELES+UNIX - UNIX socket (not available on Windows)')
parser.add_argument(
    'database', nargs='?',
    help='path to database file, in-memory will be used if empty')
parser.add_argument('--plugin', action='append',
                    help='name plugin module to load')
parser.add_argument(
    '--cert-dir', help='Directory in which server will look for certificate '
                       'and key files (named veles.cert and veles.key '
                       'respectively) and save generated ones if not found',
    default='.')
args = parser.parse_args()

logging.basicConfig(level=logging.getLevelName(args.log_level))

logging.info('Świtezianka server is starting up...')
loop = asyncio.get_event_loop()
logging.info('Opening database...')
conn = AsyncLocalConnection(loop, args.database)
logging.info('Loading plugins...')
if args.plugin is not None:
    for pname in args.plugin:
        logging.info('{}...'.format(pname))
        mod = importlib.import_module('veles.plugins.' + pname)
        conn.register_plugin(mod)

url = helpers.parse_url(args.url)
if url.scheme == helpers.UrlScheme.UNIX_SCHEME:
    logging.info('Starting UNIX server...')
    loop.run_until_complete(create_unix_server(conn, url.auth_key, url.path))
elif url.scheme == helpers.UrlScheme.TCP_SCHEME:
    logging.info('Starting TCP server...')
    loop.run_until_complete(
        create_tcp_server(conn, url.auth_key, url.host, url.port))
elif url.scheme == helpers.UrlScheme.SSL_SCHEME:
    logging.info('Starting SSL server...')
    loop.run_until_complete(
        create_ssl_server(
            conn, url.auth_key, url.host, url.port, args.cert_dir))
else:
    raise ValueError('Wrong scheme provided!')

logging.info('Ready.')
try:
    loop.add_signal_handler(signal.SIGINT, loop.stop)
except NotImplementedError:
    pass
if platform.system() == 'Windows':
    # loop.run_forever() breaks Ctrl+C on Windows.
    # See http://bugs.python.org/issue23057.
    while True:
        loop.run_until_complete(asyncio.sleep(0.5))
else:
    loop.run_forever()
logging.info('Goodbye.')
