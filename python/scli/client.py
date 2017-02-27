import socket
import msgpack
import random


def serialize(data):
    return msgpack.dumps(data, use_bin_type=True)


def unserialize(data):
    return msgpack.loads(data, encoding='utf-8')


class Client:
    def __init__(self, sock):
        self.sock = sock
        self.unpacker = msgpack.Unpacker(encoding='utf-8')

    def getpkt(self):
        while True:
            try:
                return self.unpacker.unpack()
            except msgpack.OutOfData:
                pass
            data = self.sock.recv(1024)
            if not data:
                raise Exception("end of file")
            self.unpacker.feed(data)

    def create(self, parent, *, tags=[], attr={}, data={}, bindata={},
               pos=(None, None)):
        msg = {
            'type': 'create',
            'id': random.getrandbits(192).to_bytes(24, 'little'),
            'parent': parent,
            'pos_start': pos[0],
            'pos_end': pos[1],
            'tags': tags,
            'attr': attr,
            'data': data,
            'bindata': bindata,
            'aid': 0,
        }
        self.sock.sendall(serialize(msg))
        pkt = self.getpkt()
        if pkt['type'] != 'ack' or pkt['aid'] != 0:
            print(pkt)
            raise Exception('weird reply to create')
        return msg['id']

    def delete(self, objs):
        msg = {
            'type': 'delete',
            'ids': objs,
            'aid': 0,
        }
        self.sock.sendall(serialize(msg))
        pkt = self.getpkt()
        if pkt['type'] != 'ack' or pkt['aid'] != 0:
            raise Exception('weird reply to delete')

    def get(self, obj):
        msg = {
            'type': 'get',
            'id': obj,
            'qid': 0,
        }
        self.sock.sendall(serialize(msg))
        pkt = self.getpkt()
        if pkt['type'] == 'get_reply' and pkt['qid'] == 0:
            return pkt
        elif pkt['type'] == 'obj_gone' and pkt['qid'] == 0:
            return None
        else:
            raise Exception('weird reply to get')

    def get_sub(self, obj):
        msg = {
            'type': 'get',
            'id': obj,
            'qid': 0,
            'sub': True,
        }
        self.sock.sendall(serialize(msg))
        while True:
            pkt = self.getpkt()
            if pkt['type'] == 'get_reply' and pkt['qid'] == 0:
                yield pkt
            elif pkt['type'] == 'obj_gone' and pkt['qid'] == 0:
                return
            else:
                raise Exception('weird reply to get')

    def list_sub(self, obj):
        msg = {
            'type': 'list',
            'parent': obj,
            'tags': [{}],
            'qid': 0,
            'sub': True,
        }
        self.sock.sendall(serialize(msg))
        while True:
            pkt = self.getpkt()
            if pkt['type'] == 'list_reply' and pkt['qid'] == 0:
                yield pkt
            elif pkt['type'] == 'obj_gone' and pkt['qid'] == 0:
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
