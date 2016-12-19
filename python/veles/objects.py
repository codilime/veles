# for now it's just a single class for all types of object
# when we have more final database schema we need to do it in better way
import network_pb2


class LocalObject(object):
    def __init__(self, proto_obj=None, id_path=None, parent=None):
        if proto_obj is None:
            self._proto_obj = network_pb2.LocalObject()
        else:
            self._proto_obj = proto_obj
        self.parent = parent
        if id_path is None:
            self.id_path = []
        else:
            self.id_path = id_path[:]
        self.children = []

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
        return 'id: %s name: %s' % (self.id, self.name)

    __str__ = __repr__
