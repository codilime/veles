import socket
import struct
import unittest

import mock

from veles import exceptions
from veles import veles_api


class TestVelesApi(unittest.TestCase):
    def setUp(self):
        socket_patcher = mock.patch('veles.veles_api.socket.socket')
        self.socket_mock = socket_patcher.start()
        self.addCleanup(socket_patcher.stop)

    def _create_client(self):
        return veles_api.VelesApi('127.0.0.1', 3135)

    def test_create(self):
        self._create_client()
        self.socket_mock.assert_called_once_with(
            socket.AF_INET, socket.SOCK_STREAM)
        self.socket_mock().connect.assert_called_once_with(('127.0.0.1', 3135))

    def test_recv_data(self):
        client = self._create_client()
        self.socket_mock().recv.side_effect = [b'1', b'2', b'3', b'4']

        ret = client._recv_data(3)
        self.assertEqual(ret, b'123')
        self.assertEqual(self.socket_mock().recv.call_count, 3)

    def test_recv_data_error(self):
        client = self._create_client()
        self.socket_mock().recv.side_effect = [b'1', b'']

        with self.assertRaises(exceptions.ConnectionException):
            client._recv_data(3)

    @mock.patch('veles.network_pb2.Response')
    def test_send_req(self, RespClass):
        client = self._create_client()
        req = mock.MagicMock()
        req.ByteSize.return_value = 4
        req.SerializeToString.return_value = b'foobar'
        msg = struct.pack('<I', 4) + b'foobar'
        self.socket_mock().recv.side_effect = [struct.pack('<I', 6), b'123456']
        self.socket_mock().send.side_effect = [4, 6]
        client._send_req(req)

        self.socket_mock().send.assert_has_calls(
            [mock.call(msg), mock.call(msg[4:])])
        self.socket_mock().recv.assert_has_calls([mock.call(4), mock.call(6)])
        RespClass.assert_called_once_with()
        RespClass().ParseFromString.assert_called_once_with(b'123456')
