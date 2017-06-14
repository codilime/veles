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

from __future__ import unicode_literals

import socket
import ssl

import msgpack

from veles.proto import messages, msgpackwrap
from veles.proto.messages import PROTO_VERSION
from veles.schema import nodeid
from veles.util import helpers


class Client(object):
    def __init__(self, sock, key, name='scli', version='1.0',
                 description='', type='scli', quit_on_close=False):
        self.sock = sock
        wrapper = msgpackwrap.MsgpackWrapper()
        self.unpacker = wrapper.unpacker
        self.packer = wrapper.packer
        self.client_name = name
        self.client_version = version
        self.client_description = description
        self.client_type = type
        self.quit_on_close = quit_on_close
        self._authorize(helpers.prepare_auth_key(key))

    def _authorize(self, key):
        self.sock.sendall(key)
        self.send_msg(messages.MsgConnect(
            proto_version=PROTO_VERSION,
            client_name=self.client_name,
            client_version=self.client_version,
            client_description=self.client_description,
            client_type=self.client_type,
            quit_on_close=self.quit_on_close,
        ))
        pkt = self.getpkt()
        if isinstance(pkt, messages.MsgConnected):
            print('Connected to server: {}'.format(pkt.server_name))
        elif isinstance(pkt, messages.MsgConnectionError):
            raise pkt.err
        else:
            print(pkt)
            raise Exception('weird reply when attempting to connect')

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

    def send_msg(self, msg):
        self.sock.sendall(self.packer.pack(msg.dump()))

    def request(self, msg):
        self.send_msg(msg)
        pkt = self.getpkt()
        if isinstance(pkt, messages.MsgRequestAck) and pkt.rid == 0:
            return msg.id
        elif isinstance(pkt, messages.MsgRequestError) and pkt.rid == 0:
            raise pkt.err
        else:
            print(pkt)
            raise Exception('weird reply to request')

    def create(self, parent, tags=set(), attr={}, data={}, bindata={},
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
        self.request(msg)
        return msg.id

    def delete(self, obj):
        msg = messages.MsgDelete(
            id=obj,
            rid=0
        )
        self.request(msg)

    def set_parent(self, obj, parent):
        msg = messages.MsgSetParent(
            id=obj,
            parent=parent,
            rid=0
        )
        self.request(msg)

    def set_pos(self, obj, start, end):
        msg = messages.MsgSetPos(
            id=obj,
            pos_start=start,
            pos_end=end,
            rid=0
        )
        self.request(msg)

    def add_tag(self, obj, tag):
        msg = messages.MsgAddTag(
            id=obj,
            tag=tag,
            rid=0
        )
        self.request(msg)

    def del_tag(self, obj, tag):
        msg = messages.MsgDelTag(
            id=obj,
            tag=tag,
            rid=0
        )
        self.request(msg)

    def set_attr(self, obj, key, data):
        msg = messages.MsgSetAttr(
            id=obj,
            key=key,
            data=data,
            rid=0
        )
        self.request(msg)

    def set_data(self, obj, key, data):
        msg = messages.MsgSetData(
            id=obj,
            rid=0,
            key=key,
            data=data,
        )
        self.request(msg)

    def set_bindata(self, obj, key, start, data, truncate=False):
        msg = messages.MsgSetBinData(
            id=obj,
            rid=0,
            key=key,
            start=start,
            data=data,
            truncate=truncate,
        )
        self.request(msg)

    def get(self, obj):
        msg = messages.MsgGet(
            id=obj,
            qid=0,
        )
        self.send_msg(msg)
        pkt = self.getpkt()
        if isinstance(pkt, messages.MsgGetReply) and pkt.qid == 0:
            return pkt.obj
        elif isinstance(pkt, messages.MsgQueryError) and pkt.qid == 0:
            raise pkt.err
        else:
            raise Exception('weird reply to get')

    def get_sub(self, obj):
        msg = messages.MsgGet(
            id=obj,
            qid=0,
            sub=True,
        )
        self.send_msg(msg)
        while True:
            pkt = self.getpkt()
            if isinstance(pkt, messages.MsgGetReply) and pkt.qid == 0:
                yield pkt.obj
            elif isinstance(pkt, messages.MsgQueryError) and pkt.qid == 0:
                raise pkt.err
            else:
                raise Exception('weird reply to get')

    def get_data(self, obj, key):
        msg = messages.MsgGetData(
            id=obj,
            qid=0,
            key=key,
        )
        self.send_msg(msg)
        pkt = self.getpkt()
        if isinstance(pkt, messages.MsgGetDataReply) and pkt.qid == 0:
            return pkt.data
        elif isinstance(pkt, messages.MsgQueryError) and pkt.qid == 0:
            raise pkt.err
        else:
            raise Exception('weird reply to get_data')

    def get_data_sub(self, obj, key):
        msg = messages.MsgGetData(
            id=obj,
            qid=0,
            key=key,
            sub=True
        )
        self.send_msg(msg)
        while True:
            pkt = self.getpkt()
            if isinstance(pkt, messages.MsgGetDataReply) and pkt.qid == 0:
                yield pkt.data
            elif isinstance(pkt, messages.MsgQueryError) and pkt.qid == 0:
                raise pkt.err
            else:
                raise Exception('weird reply to get_data')

    def get_bindata(self, obj, key, start=0, end=None):
        msg = messages.MsgGetBinData(
            id=obj,
            qid=0,
            key=key,
            start=start,
            end=end,
        )
        self.send_msg(msg)
        pkt = self.getpkt()
        if isinstance(pkt, messages.MsgGetBinDataReply) and pkt.qid == 0:
            return pkt.data
        elif isinstance(pkt, messages.MsgQueryError) and pkt.qid == 0:
            raise pkt.err
        else:
            raise Exception('weird reply to get_bindata')

    def get_bindata_sub(self, obj, key, start=0, end=None):
        msg = messages.MsgGetBinData(
            id=obj,
            qid=0,
            key=key,
            start=start,
            end=end,
            sub=True,
        )
        self.send_msg(msg)
        while True:
            pkt = self.getpkt()
            if isinstance(pkt, messages.MsgGetBinDataReply) and pkt.qid == 0:
                yield pkt.data
            elif isinstance(pkt, messages.MsgQueryError) and pkt.qid == 0:
                raise pkt.err
            else:
                raise Exception('weird reply to get_bindata')

    def list(self, obj):
        msg = messages.MsgGetList(
            qid=0,
            parent=obj,
        )
        self.send_msg(msg)
        pkt = self.getpkt()
        if isinstance(pkt, messages.MsgGetListReply) and pkt.qid == 0:
            return pkt.objs
        elif isinstance(pkt, messages.MsgQueryError) and pkt.qid == 0:
            raise pkt.err
        else:
            print(pkt)
            raise Exception('weird reply to list')

    def list_sub(self, obj):
        msg = messages.MsgGetList(
            qid=0,
            parent=obj,
            sub=True
        )
        self.send_msg(msg)
        while True:
            pkt = self.getpkt()
            if isinstance(pkt, messages.MsgGetListReply) and pkt.qid == 0:
                yield pkt
            elif isinstance(pkt, messages.MsgQueryError) and pkt.qid == 0:
                raise pkt.err
            else:
                print(pkt)
                raise Exception('weird reply to list')

    def query(self, obj, sig, params, checks=None):
        params = sig.params.dump(params)
        msg = messages.MsgGetQuery(
            qid=0,
            node=obj,
            query=sig.name,
            params=params,
            trace=checks is not None
        )
        self.send_msg(msg)
        pkt = self.getpkt()
        if isinstance(pkt, messages.MsgGetQueryReply) and pkt.qid == 0:
            if checks is not None:
                checks += pkt.checks
            return sig.result.load(pkt.result)
        elif isinstance(pkt, messages.MsgQueryError) and pkt.qid == 0:
            if checks is not None:
                checks += pkt.checks
            raise pkt.err
        else:
            print(pkt)
            raise Exception('weird reply to get_query')

    def query_sub(self, obj, sig, params, checks=None):
        params = sig.params.dump(params)
        msg = messages.MsgGetQuery(
            qid=0,
            node=obj,
            query=sig.name,
            params=params,
            trace=checks is not None,
            sub=True
        )
        self.send_msg(msg)
        while True:
            pkt = self.getpkt()
            if isinstance(pkt, messages.MsgGetQueryReply) and pkt.qid == 0:
                if checks is not None:
                    checks += pkt.checks
                yield sig.result.load(pkt.result)
            elif isinstance(pkt, messages.MsgQueryError) and pkt.qid == 0:
                if checks is not None:
                    checks += pkt.checks
                raise pkt.err
            else:
                print(pkt)
                raise Exception('weird reply to get_query')

    def run_method(self, obj, sig, params):
        params = sig.params.dump(params)
        msg = messages.MsgMethodRun(
            mid=0,
            node=obj,
            method=sig.name,
            params=params
        )
        self.send_msg(msg)
        pkt = self.getpkt()
        if isinstance(pkt, messages.MsgMethodResult) and pkt.mid == 0:
            return sig.result.load(pkt.result)
        elif isinstance(pkt, messages.MsgMethodError) and pkt.mid == 0:
            raise pkt.err
        else:
            print(pkt)
            raise Exception('weird reply to run_method')

    def run_broadcast(self, sig, params):
        params = sig.params.dump(params)
        msg = messages.MsgBroadcastRun(
            bid=0,
            broadcast=sig.name,
            params=params
        )
        self.send_msg(msg)
        pkt = self.getpkt()
        if isinstance(pkt, messages.MsgBroadcastResult) and pkt.bid == 0:
            return [sig.result.load(result) for result in pkt.results]
        else:
            print(pkt)
            raise Exception('weird reply to run_broadcast')

    def list_connections(self):
        msg = messages.MsgListConnections(
            qid=0,
        )
        self.send_msg(msg)
        pkt = self.getpkt()
        if isinstance(pkt, messages.MsgConnectionsReply) and pkt.qid == 0:
            return pkt.connections
        elif isinstance(pkt, messages.MsgQueryError) and pkt.qid == 0:
            raise pkt.err
        else:
            print(pkt)
            raise Exception('weird reply to list_connections')

    def list_connections_sub(self):
        msg = messages.MsgListConnections(
            qid=0,
            sub=True
        )
        self.send_msg(msg)
        while True:
            pkt = self.getpkt()
            if isinstance(pkt, messages.MsgConnectionsReply) and pkt.qid == 0:
                yield pkt
            elif isinstance(pkt, messages.MsgQueryError) and pkt.qid == 0:
                raise pkt.err
            else:
                print(pkt)
                raise Exception('weird reply to list_connections')


class UnixClient(Client):
    def __init__(self, path, key, **kwargs):
        sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        sock.connect(path)
        super(UnixClient, self).__init__(sock, key, **kwargs)


class TcpClient(Client):
    def __init__(self, ip, port, key, **kwargs):
        sock = socket.create_connection((ip, port))
        super(TcpClient, self).__init__(sock, key, **kwargs)


class SslClient(Client):
    def __init__(self, ip, port, key, fingerprint, **kwargs):
        sock = socket.create_connection((ip, port))
        sc = ssl.SSLContext()
        sock = sc.wrap_socket(sock)
        cert = sock.getpeercert(True)
        helpers.validate_cert(cert, fingerprint)
        super(SslClient, self).__init__(sock, key, **kwargs)


def create_client(url):
    url = helpers.parse_url(url)
    if url.scheme == helpers.UrlScheme.UNIX_SCHEME:
        return UnixClient(url.path, url.auth_key)
    elif url.scheme == helpers.UrlScheme.TCP_SCHEME:
        return TcpClient(url.host, url.port, url.auth_key)
    elif url.scheme == helpers.UrlScheme.SSL_SCHEME:
        return SslClient(url.host, url.port, url.auth_key, url.fingerprint)
    else:
        raise ValueError('Wrong scheme provided!')
