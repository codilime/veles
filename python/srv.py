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

import argparse
import logging
import asyncio
import signal
import importlib

from veles.server.conn import AsyncLocalConnection
from veles.server.proto import create_unix_server, create_tcp_server

logging.basicConfig(level=logging.INFO)

parser = argparse.ArgumentParser()
parser.add_argument(
    'database', help='path to database file, in-memory will be used if empty')
parser.add_argument(
    'url', help='either UNIX:<socket_path> or <ip>:<tcp port> to listen on')
parser.add_argument(
    'auth_key', help='hex-encoded up to 64bytes value that '
                     'clients need to provide when connecting')
parser.add_argument('plugin', nargs='*', help='name plugin module to load')
args = parser.parse_args()

logging.info('Świtezianka server is starting up...')
loop = asyncio.get_event_loop()
logging.info('Opening database...')
conn = AsyncLocalConnection(loop, args.database)
logging.info('Loading plugins...')
for pname in args.plugin:
    logging.info('{}...'.format(pname))
    mod = importlib.import_module('veles.plugins.' + pname)
    conn.register_plugin(mod)
host, _, port = args.url.rpartition(':')
if host == 'UNIX':
    logging.info('Starting UNIX server...')
    loop.run_until_complete(create_unix_server(conn, args.auth_key, port))
else:
    logging.info('Starting TCP server...')
    loop.run_until_complete(
        create_tcp_server(conn, args.auth_key, host, int(port)))
logging.info('Ready.')
try:
    loop.add_signal_handler(signal.SIGINT, loop.stop)
except NotImplementedError:
    pass
loop.run_forever()
logging.info('Goodbye.')
