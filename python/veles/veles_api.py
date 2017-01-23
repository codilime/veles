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


class VelesClient(object):

    # This corresponds to enum in types.h - we might think about moving it
    # to .proto file so we don't have to modify 2 independent places
    class ObjectTypes(object):
        ROOT = 0
        FILE_BLOB = 1
        SUB_BLOB = 2
        CHUNK = 3

    def __init__(self, ip_addr='127.0.0.1', port=3135):
        self.ip_addr = ip_addr
        self.port = port
        self.cursors = {}
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
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
        id_path = id_path[:] + [res.id]
        obj = objects.LocalObject(res, id_path, parent)
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

    def _raw_create_chunk(self, obj, new_obj):
        req = network_pb2.Request()
        req.type = network_pb2.Request.ADD_CHILD_CHUNK
        req.id.extend(obj._id_path)
        req.name = new_obj.name
        req.comment = new_obj.comment
        req.chunk_start = new_obj.chunk_start
        req.chunk_end = new_obj.chunk_end
        req.chunk_type = new_obj.chunk_type
        results = self._send_req(req)

        new_obj._from_another(results[0])
        new_obj.parent = obj
        obj.children.append(new_obj)

        return new_obj

    def _get_blob_for_chunk(self, chunk):
        blob = chunk
        while blob is not None:
            if blob.type in [self.ObjectTypes.SUB_BLOB,
                             self.ObjectTypes.FILE_BLOB]:
                break
            blob = blob.parent
        if blob is None:
            raise ValueError(
                'object doesn\'t have blob object '
                'in its parent chain')
        return blob

    def cursor_value(self, chunk):
        blob = self._get_blob_for_chunk(chunk)
        return self.cursors.get(blob.id, 0)

    def set_cursor_value(self, chunk, val):
        blob = self._get_blob_for_chunk(chunk)
        self.cursors[blob.id] = val

    def create_chunk(self, parent, name,
                     start=None, end=None, size=None, comment=None,
                     update_cursor=True):
        """Create chunk in convenient manner.

        This method can utilise internal cursor to make creating chunks in
        sequential manner easier - by default cursor will be moved to
        end of chunk after operation, which allows user to ommit start
        of next chunk

        Args:
            parent: parent blob or chunk (needs to have blob
                in chain of its parents)
            name: name of the chunk
            start: start of chunk, can be ommited - in such case internal
                cursor for file will be used to determine start
            end: end of chunk, can be ommited - in such case size must
                specified
            size: size of chunk, can be ommited - in such case end must be
                specified
            comment: comment for chunk
            update_cursor: determines whether cursor will be updated
                after this operation, defaults to True

        Returns:
            Object representing newly created chunk
        """
        new_obj = objects.LocalObject()
        new_obj.name = name

        if start is None:
            start = self.cursor_value(parent)
        new_obj.chunk_start = start

        if end is None and size is None:
            raise ValueError(
                'You need to specify either size or end of chunk')
        if end is None:
            end = new_obj.chunk_start + size
        new_obj.chunk_end = end

        if update_cursor:
            self.set_cursor_value(parent, new_obj.chunk_end)
        if comment:
            new_obj.comment = comment

        self._raw_create_chunk(parent, new_obj)
        return new_obj

    def delete_object(self, obj):
        if obj.type not in [self.ObjectTypes.SUB_BLOB, self.ObjectTypes.CHUNK]:
            raise exc.VelesException('Unsupported object type to delete')

        req = network_pb2.Request()
        req.type = network_pb2.Request.DELETE_OBJECT
        req.id.extend(obj._id_path)
        self._send_req(req)
        if obj.parent:
            obj.parent.children.remove(obj)

    def get_blob_data(self, obj):
        if obj.type not in [self.ObjectTypes.SUB_BLOB,
                            self.ObjectTypes.FILE_BLOB]:
            raise exc.VelesException('Unsupported object type to get data')

        req = network_pb2.Request()
        req.type = network_pb2.Request.GET_BLOB_DATA
        req.id.extend(obj._id_path)
        self._send_req(req)

        obj.data = self._recv_msg()
