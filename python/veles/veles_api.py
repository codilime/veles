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
import socket
import struct

from veles import exceptions as exc
from veles import network_pb2
from veles import objects


class VelesApi(object):

    def __init__(self, ip_addr='127.0.0.1', port=3135):
        self.ip_addr = ip_addr
        self.port = port
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            self.sock.connect((ip_addr, port))
        except socket.error as e:
            raise exc.ConnectionException(str(e))

    def _recv_data(self, length):
        total_recv = 0
        chunks = []
        while total_recv < length:
            try:
                recv = self.sock.recv(min(4096, length - total_recv))
            except socket.error as e:
                raise exc.ConnectionException(str(e))
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
            except socket.error as e:
                raise exc.ConnectionException(str(e))
            if sent == 0:
                raise exc.ConnectionException('socket connection broken')
            total_sent += sent

        length = self._recv_data(4)
        length = struct.unpack('<I', length)[0]

        response = self._recv_data(length)
        resp = network_pb2.Response()
        resp.ParseFromString(response)
        if not resp.ok:
            raise exc.RequestFailed(response.error_msg)

        objects = []
        for res in resp.results:
            obj = self._prepare_object(res, req.id)
            objects.append(obj)
        return objects

    def _prepare_object(self, res, id_path, parent=None):
        id_path = id_path[:] + [res.id]
        obj = objects.LocalObject(res, id_path)
        for child in res.children:
            child_obj = self._prepare_object(child, id_path, obj)
            obj.children.append(child_obj)
        return obj

    def list_files(self):
        req = network_pb2.Request()
        req.type = 1
        results = self._send_req(req)
        return results

    def get_chunk_tree(self, obj):
        req = network_pb2.Request()
        req.type = 2
        req.id.extend(obj.id_path)
        results = self._send_req(req)

        obj.children = []
        for res in results:
            res.parent = obj
            obj.children.append(res)
        return results

    def create_chunk(self, obj, new_obj):
        req = network_pb2.Request()
        req.type = 3
        req.id.extend(obj.id_path)
        req.name = new_obj.name
        req.chunk_start = new_obj.chunk_start
        req.chunk_end = new_obj.chunk_end
        req.chunk_type = new_obj.chunk_type
        results = self._send_req(req)

        new_obj._from_another(results[0])
        new_obj.parent = obj
        obj.children.append(new_obj)

        return new_obj

    def delete_object(self, obj):
        if obj.type not in [2, 3]:
            raise exc.VelesException('Unsupported object type to delete')

        req = network_pb2.Request()
        req.type = 4
        req.id.extend(obj.id_path)
        self._send_req(req)
        if obj.parent:
            obj.parent.children.remove(obj)
