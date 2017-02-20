import binascii

from messages import msgpackwrap


class ObjectID:
    def __init__(self, value):
        if not isinstance(value, bytes) or len(value) != 24:
            raise ValueError('value is not valid id')
        self.bytes = value

    def to_bytes(self):
        return self.bytes

    def __str__(self):
        return binascii.b2a_hex(self.bytes).decode('ascii')

    def __eq__(self, other):
        if isinstance(other, ObjectID):
            return self.bytes == other.bytes
        return False

    def __hash__(self):
        return hash(self.bytes)

    __repr__ = __str__

msgpackwrap.MsgpackWrapper.register_type(ObjectID, 0)
