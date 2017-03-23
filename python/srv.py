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

import asyncio
import sys
import signal
import importlib

from veles.server.conn import AsyncLocalConnection
from veles.server.proto import create_unix_server, create_tcp_server

print('Świtezianka server starting up...')
loop = asyncio.get_event_loop()
print('Opening database.')
conn = AsyncLocalConnection(loop, sys.argv[1])
print('Loading plugins.')
for pname in sys.argv[3:]:
    print('{}...'.format(pname))
    mod = importlib.import_module('plugins.' + pname)
    conn.add_local_plugin(mod.Plugin)
host, _, port = sys.argv[2].rpartition(':')
if host == 'UNIX':
    print('Starting UNIX server.')
    loop.run_until_complete(create_unix_server(conn, port))
else:
    print('Starting TCP server.')
    loop.run_until_complete(create_tcp_server(conn, host, int(port)))
print('Ready.')
try:
    loop.add_signal_handler(signal.SIGINT, loop.stop)
except NotImplementedError:
    pass
loop.run_forever()
print('Goodbye.')
