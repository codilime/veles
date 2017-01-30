from veles import network_pb2
from veles.objects.base import LocalObject


class Blob(LocalObject):
    def __init__(self, client, proto_obj=None, id_path=None, parent=None):
        super(Blob, self).__init__(client, proto_obj, id_path, parent)
        self._data = None
        self._cursor = 0

    def fetch_data(self):
        if self._data is None:
            req = network_pb2.Request()
            req.type = network_pb2.Request.GET_BLOB_DATA
            req.id.extend(self._id_path)
            self._client._send_req(req)
            self._data = self._client._recv_msg()

    def __iter__(self):
        self.fetch_data()
        return self._data.__iter__()

    def __getitem__(self, i):
        self.fetch_data()
        return self._data[i]

    def get_blob(self):
        return self
