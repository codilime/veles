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

from veles.proto import messages, msgpackwrap
from veles.proto.exceptions import VelesException
from veles.schema import nodeid


class Client:
    def __init__(self, sock):
        self.sock = sock
        wrapper = msgpackwrap.MsgpackWrapper()
        self.unpacker = wrapper.unpacker
        self.packer = wrapper.packer

    def getpkt(self):
        while True:
            try:
                return messages.MsgpackMsg.load(self.unpacker.unpack())
            except msgpack.OutOfData:
                pass
            data = self.sock.recv(1024)
            if not data:
                raise Exception("end of file")
            self.unpacker.feed(data)

    def create(self, parent, *, tags=set(), attr={}, data={}, bindata={},
               pos=(None, None)):
        msg = messages.MsgCreate(
            id=nodeid.NodeID(),
            parent=parent,
            pos_start=pos[0],
            pos_end=pos[1],
            tags=tags,
            attr=attr,
            data=data,
            bindata=bindata,
            rid=0,
        )
        self.sock.sendall(self.packer.pack(msg.dump()))
        pkt = self.getpkt()
        if isinstance(pkt, messages.MsgRequestAck) and pkt.rid == 0:
            return msg.id
        elif isinstance(pkt, messages.MsgRequestError) and pkt.rid == 0:
            raise VelesException(pkt.code, pkt.msg)
        else:
            print(pkt)
            raise Exception('weird reply to create')

    def delete(self, objs):
        msg = messages.MsgDelete(
            ids=objs,
            rid=0
        )
        self.sock.sendall(self.packer.pack(msg.dump()))
        pkt = self.getpkt()
        if isinstance(pkt, messages.MsgRequestAck) and pkt.rid == 0:
            return
        elif isinstance(pkt, messages.MsgRequestError) and pkt.rid == 0:
            raise VelesException(pkt.code, pkt.msg)
        else:
            print(pkt)
            raise Exception('weird reply to delete')

    def get(self, obj):
        msg = messages.MsgGet(
            id=obj,
            qid=0,
        )
        self.sock.sendall(self.packer.pack(msg.dump()))
        pkt = self.getpkt()
        if isinstance(pkt, messages.MsgGetReply) and pkt.qid == 0:
            return pkt
        elif isinstance(pkt, messages.MsgQueryError) and pkt.qid == 0:
            raise VelesException(pkt.code, pkt.msg)
        else:
            raise Exception('weird reply to get')

    def get_sub(self, obj):
        msg = messages.MsgGet(
            id=obj,
            qid=0,
            sub=True,
        )
        self.sock.sendall(self.packer.pack(msg.dump()))
        while True:
            pkt = self.getpkt()
            if isinstance(pkt, messages.MsgGetReply) and pkt.qid == 0:
                yield pkt
            elif isinstance(pkt, messages.MsgQueryError) and pkt.qid == 0:
                raise VelesException(pkt.code, pkt.msg)
            else:
                raise Exception('weird reply to get')

    def list_sub(self, obj):
        msg = messages.MsgList(
            qid=0,
            parent=obj,
            sub=True
        )
        self.sock.sendall(self.packer.pack(msg.dump()))
        while True:
            pkt = self.getpkt()
            if isinstance(pkt, messages.MsgListReply) and pkt.qid == 0:
                yield pkt
            elif isinstance(pkt, messages.MsgObjGone) and pkt.qid == 0:
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
