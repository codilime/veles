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

from veles.async_conn.plugin import method, broadcast
from veles.proto.hello import method_hello, broadcast_hello


@method(method_hello, {'hello', 'hello_en'})
async def hello_en(conn, obj, params):
    return "Hello, {}!".format(params)


@method(method_hello, {'hello', 'hello_pl'})
async def hello_pl(conn, obj, params):
    return "Dzień dobry, {}!".format(params)


@broadcast(broadcast_hello)
async def b_hello_en(conn, params):
    return ["Hello, {}!".format(params)]


@broadcast(broadcast_hello)
async def b_hello_pl(conn, params):
    return ["Dzień dobry, {}!".format(params), "Cześć, {}!".format(params)]
