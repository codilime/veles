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
import msgpack
import random

from veles.common import base
from veles.messages import definitions
from veles.messages import msgpackwrap


class Client:
    def __init__(self, sock):
        self.sock = sock
        wrapper = msgpackwrap.MsgpackWrapper()
        self.unpacker = wrapper.unpacker
        self.packer = wrapper.packer

    def getpkt(self):
        while True:
            try:
                return definitions.MsgpackMsg.load(self.unpacker)
            except msgpack.OutOfData:
                pass
            data = self.sock.recv(1024)
            if not data:
                raise Exception("end of file")
            self.unpacker.feed(data)

    def create(self, parent, *, tags=[], attr={}, data={}, bindata={},
               pos=(None, None)):
        msg = {
            'id': base.ObjectID(
                random.getrandbits(192).to_bytes(24, 'little')),
            'parent': parent or base.ObjectID(),
            'pos_start': pos[0],
            'pos_end': pos[1],
            'tags': tags,
            'attr': attr,
            'data': data,
            'bindata': bindata,
            'rid': 0,
        }
        msg = definitions.MsgCreate(**msg)
        self.sock.sendall(msg.dump(self.packer))
        pkt = self.getpkt()
        if not isinstance(pkt, definitions.MsgAck) or pkt.rid != 0:
            print(pkt)
            raise Exception('weird reply to create')
        return msg.id

    def delete(self, objs):
        msg = {
            'ids': objs,
            'rid': 0,
        }
        msg = definitions.MsgDelete(**msg)
        self.sock.sendall(msg.dump(self.packer))
        pkt = self.getpkt()
        if not isinstance(pkt, definitions.MsgAck) or pkt.rid != 0:
            raise Exception('weird reply to delete')

    def get(self, obj):
        msg = {
            'id': obj,
            'qid': 0,
            'sub': False
        }
        msg = definitions.MsgGet(**msg)
        self.sock.sendall(msg.dump(self.packer))
        pkt = self.getpkt()
        if isinstance(pkt, definitions.MsgGetReply) and pkt.qid == 0:
            return pkt
        elif isinstance(pkt, definitions.MsgObjGone) and pkt.qid == 0:
            return None
        else:
            raise Exception('weird reply to get')

    def get_sub(self, obj):
        msg = {
            'id': obj,
            'qid': 0,
            'sub': True,
        }
        msg = definitions.MsgGet(**msg)
        self.sock.sendall(msg.dump(self.packer))
        while True:
            pkt = self.getpkt()
            if isinstance(pkt, definitions.MsgGetReply) and pkt.qid == 0:
                yield pkt
            elif isinstance(pkt, definitions.MsgObjGone) and pkt.qid == 0:
                return
            else:
                raise Exception('weird reply to get')

    def list_sub(self, obj):
        msg = {
            'parent': obj,
            'tags': [{}],
            'qid': 0,
            'sub': True,
        }
        msg = definitions.MsgList(**msg)
        self.sock.sendall(msg.dump(self.packer))
        while True:
            pkt = self.getpkt()
            if isinstance(pkt, definitions.MsgListReply) and pkt.qid == 0:
                yield pkt
            elif isinstance(pkt, definitions.MsgObjGone) and pkt.qid == 0:
                return
            else:
                print(pkt)
                raise Exception('weird reply to list')


class UnixClient(Client):
    def __init__(self, path):
        sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        sock.connect(path)
        super().__init__(sock)


class TcpClient(Client):
    def __init__(self, ip, port):
        sock = socket.create_connection((ip, port))
        super().__init__(sock)


def create_client(addr):
    host, _, port = addr.rpartition(':')
    if host == 'UNIX':
        return UnixClient(port)
    else:
        return TcpClient(host, int(port))
