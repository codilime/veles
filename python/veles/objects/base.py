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
from veles import network_pb2


class LocalObject(object):
    # for now it's just a single class for all types of object
    # when we have more final database schema we need to do it in better way
    def __init__(self, client, proto_obj=None, id_path=None, parent=None):
        self._client = client
        if proto_obj is None:
            self._proto_obj = network_pb2.LocalObject()
        else:
            self._proto_obj = proto_obj
        self.parent = parent
        if id_path is None:
            self._id_path = []
        else:
            self._id_path = id_path[:]
        self.children = []
        self.data = None

    def _from_another(self, other):
        self._proto_obj = other._proto_obj
        self.parent = other.parent
        self._id_path = other._id_path
        self.children = other.children
        self.data = other.data

    def get_blob(self):
        raise NotImplemented()

    # maybe in future we should autogenerate it based on proto file
    @property
    def id(self):
        return self._proto_obj.id

    @property
    def name(self):
        return self._proto_obj.name

    @name.setter
    def name(self, name):
        self._proto_obj.name = name

    @property
    def comment(self):
        return self._proto_obj.comment

    @comment.setter
    def comment(self, comment):
        self._proto_obj.comment = comment

    @property
    def type(self):
        return self._proto_obj.type

    @property
    def file_blob_path(self):
        return self._proto_obj.file_blob_path

    @property
    def chunk_start(self):
        return self._proto_obj.chunk_start

    @chunk_start.setter
    def chunk_start(self, chunk_start):
        self._proto_obj.chunk_start = chunk_start

    @property
    def chunk_end(self):
        return self._proto_obj.chunk_end

    @chunk_end.setter
    def chunk_end(self, chunk_end):
        self._proto_obj.chunk_end = chunk_end

    @property
    def chunk_type(self):
        return self._proto_obj.chunk_type

    @chunk_type.setter
    def chunk_type(self, chunk_type):
        self._proto_obj.chunk_type = chunk_type

    @property
    def items(self):
        return self._proto_obj.items

    # TODO setting items

    def __repr__(self):
        return 'id: {} name: {}'.format(self.id, self.name)

    __str__ = __repr__

    def create_chunk(self, name, start=None, end=None, size=None,
                     comment='', chunk_type='', update_cursor=True):
        if start is None:
            start = self._cursor

        if end is None and size is None:
            raise ValueError(
                'You need to specify either size or end of chunk')
        if end is None:
            end = start + size
        result = self._client.create_chunk(self, name, start,
                                           end, comment, chunk_type)
        if update_cursor:
            self._cursor = end
        return result
