import socket
import msgpack
import random

from messages import messages


class Client:
    def __init__(self, sock):
        self.sock = sock
        self.unpacker = msgpack.Unpacker(encoding='utf-8')
        self.packer = msgpack.Packer(use_bin_type=True)

    def getpkt(self):
        while True:
            try:
                return messages.MsgpackMsg.loads(self.unpacker)
            except msgpack.OutOfData:
                pass
            data = self.sock.recv(1024)
            if not data:
                raise Exception("end of file")
            self.unpacker.feed(data)

    def create(self, parent, *, tags=[], attr={}, data={}, bindata={},
               pos=(None, None)):
        msg = {
            'id': random.getrandbits(192).to_bytes(24, 'little'),
            'parent': parent,
            'pos_start': pos[0],
            'pos_end': pos[1],
            'tags': tags,
            'attr': attr,
            'data': data,
            'bindata': bindata,
            'rid': 0,
        }
        msg = messages.MsgCreate(**msg)
        self.sock.sendall(msg.dumps(self.packer))
        pkt = self.getpkt()
        if not isinstance(pkt, messages.MsgAck) or pkt.rid != 0:
            print(pkt)
            raise Exception('weird reply to create')
        return msg.id

    def delete(self, objs):
        msg = {
            'ids': objs,
            'rid': 0,
        }
        msg = messages.MsgDelete(**msg)
        self.sock.sendall(msg.dumps(self.packer))
        pkt = self.getpkt()
        if not isinstance(pkt, messages.MsgAck) or pkt.rid != 0:
            raise Exception('weird reply to delete')

    def get(self, obj):
        msg = {
            'id': obj,
            'qid': 0,
        }
        msg = messages.MsgGet(**msg)
        self.sock.sendall(msg.dumps(self.packer))
        pkt = self.getpkt()
        if isinstance(pkt, messages.MsgGetReply) and pkt.qid == 0:
            return pkt
        elif isinstance(pkt, messages.MsgObjGone) and pkt.qid == 0:
            return None
        else:
            raise Exception('weird reply to get')

    def get_sub(self, obj):
        msg = {
            'id': obj,
            'qid': 0,
            'sub': True,
        }
        msg = messages.MsgGet(**msg)
        self.sock.sendall(msg.dumps(self.packer))
        while True:
            pkt = self.getpkt()
            if isinstance(pkt, messages.MsgGetReply) and pkt.qid == 0:
                yield pkt
            elif isinstance(pkt, messages.MsgObjGone) and pkt.qid == 0:
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
        msg = messages.MsgList(**msg)
        self.sock.sendall(msg.dumps(self.packer))
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
