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

import weakref

from veles.async_conn.conn import AsyncConnection
from .node import AsyncRemoteNode


class AsyncRemoteConnection(AsyncConnection):
    def __init__(self, loop, proto):
        self.loop = loop
        self.proto = proto
        self.objs = weakref.WeakValueDictionary()
        self.conns = {}
        self.next_cid = 0
        super().__init__()

    def get_node_norefresh(self, obj_id):
        try:
            return self.objs[obj_id]
        except KeyError:
            res = AsyncRemoteNode(self, obj_id, None)
            self.objs[obj_id] = res
            return res

    def new_conn(self, conn):
        cid = self.next_cid
        self.next_cid += 1
        self.conns[cid] = conn
        print("Conn {} started.".format(cid))
        return cid

    def remove_conn(self, conn):
        print("Conn {} gone.".format(conn.cid))
        self.conns[conn.cid] = None
