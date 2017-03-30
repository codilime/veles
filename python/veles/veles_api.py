# Copyright 2016-2017 CodiLime
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
import socket
import struct
import weakref

from veles import exceptions as exc
from veles import network_pb2
from veles import objects


class VelesClient(object):

    # This corresponds to enum in types.h - we might think about moving it
    # to .proto file so we don't have to modify 2 independent places
    class ObjectTypes(object):
        ROOT = 0
        FILE_BLOB = 1
        SUB_BLOB = 2
        CHUNK = 3

        CONSTRUCTORS = {
            ROOT: objects.LocalObject,
            FILE_BLOB: objects.Blob,
            SUB_BLOB: objects.Blob,
            CHUNK: objects.Chunk,
        }

    def __init__(self, ip_addr='127.0.0.1', port=3135):
        self.ip_addr = ip_addr
        self.port = port
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._objects = weakref.WeakValueDictionary()
        try:
            self.sock.connect((ip_addr, port))
        except socket.error as ex:
            raise exc.ConnectionException(str(ex))

    def _recv_msg(self):
        length = self._recv_data(4)
        length = struct.unpack('<I', length)[0]
        return self._recv_data(length)

    def _recv_data(self, length):
        total_recv = 0
        chunks = []
        while total_recv < length:
            try:
                recv = self.sock.recv(min(4096, length - total_recv))
            except socket.error as ex:
                raise exc.ConnectionException(str(ex))
            if recv == b'':
                raise exc.ConnectionException('socket connection broken')
            chunks.append(recv)
            total_recv += len(recv)
        return b''.join(chunks)

    def _send_req(self, req):
        msg = struct.pack('<I', req.ByteSize()) + req.SerializeToString()
        total_sent = 0
        while total_sent < len(msg):
            try:
                sent = self.sock.send(msg[total_sent:])
            except socket.error as ex:
                raise exc.ConnectionException(str(ex))
            if sent == 0:
                raise exc.ConnectionException('socket connection broken')
            total_sent += sent

        response = self._recv_msg()

        resp = network_pb2.Response()
        resp.ParseFromString(response)
        if not resp.ok:
            raise exc.RequestFailed(resp.error_msg)

        objs = []
        for res in resp.results:
            obj = self._prepare_object(res, req.id)
            objs.append(obj)
        return objs

    def _prepare_object(self, res, id_path, parent=None):
        if parent is None and id_path:
            parent = self._objects.get(id_path[-1])
        id_path = id_path[:] + [res.id]
        obj_type = self.ObjectTypes.CONSTRUCTORS[res.type]
        obj = obj_type(self, res, id_path, parent)
        self._objects[res.id] = obj
        for child in res.children:
            child_obj = self._prepare_object(child, id_path, obj)
            obj.children.append(child_obj)
        return obj

    def list_files(self):
        req = network_pb2.Request()
        req.type = network_pb2.Request.LIST_CHILDREN
        results = self._send_req(req)
        return results

    def get_chunk_tree(self, obj):
        req = network_pb2.Request()
        req.type = network_pb2.Request.LIST_CHILDREN_RECURSIVE
        req.id.extend(obj._id_path)
        results = self._send_req(req)

        obj.children = []
        for res in results:
            res.parent = obj
            obj.children.append(res)
        return results

    def create_chunk(self, parent, name,
                     start, end, comment='', chunk_type=''):
        req = network_pb2.Request()
        req.type = network_pb2.Request.ADD_CHILD_CHUNK
        req.id.extend(parent._id_path)

        req.name = name
        req.comment = comment
        req.chunk_start = start
        req.chunk_end = end
        req.chunk_type = chunk_type
        results = self._send_req(req)
        new_chunk = results[0]
        parent.children.append(new_chunk)
        return new_chunk

    def delete_object(self, obj):
        if obj.type not in [self.ObjectTypes.SUB_BLOB, self.ObjectTypes.CHUNK]:
            raise exc.VelesException('Unsupported object type to delete')

        req = network_pb2.Request()
        req.type = network_pb2.Request.DELETE_OBJECT
        req.id.extend(obj._id_path)
        self._send_req(req)
        if obj.parent:
            obj.parent.children.remove(obj)
