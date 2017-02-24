import binascii

from messages import msgpackwrap


class ObjectID:
    NULL_VAL = b'\00'*24

    def __init__(self, value=None):
        if value is None:
            value = self.NULL_VAL
        if isinstance(value, str):
            value = binascii.a2b_hex(value)
        if not isinstance(value, bytes) or len(value) != 24:
            raise ValueError('value is not valid id')
        self._bytes = value

    @property
    def bytes(self):
        return self._bytes

    def to_bytes(self):
        return self.bytes

    def __str__(self):
        return binascii.b2a_hex(self.bytes).decode('ascii')

    def __repr__(self):
        return f'ObjectID("{str(self)}")'

    def __eq__(self, other):
        if isinstance(other, ObjectID):
            return self.bytes == other.bytes
        return False

    def __hash__(self):
        return hash(self.bytes)

    def __bool__(self):
        return self.bytes != self.NULL_VAL


msgpackwrap.MsgpackWrapper.register_type(ObjectID, 0)
