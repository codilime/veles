import struct


class ChunkDataItemTypes(object):
    """enum corresponding to type attribute of chunk items"""
    NONE = 0
    SUBCHUNK = 1
    SUBBLOB = 2
    FIELD = 3
    BITFIELD = 4
    COMPUTED = 5
    PAD = 6


class RepackEndian(object):
    """enum corresponding to endian attribute of item repack"""
    LITTLE = 0
    BIG = 1


class FieldHighMode(object):
    """enum corresponding to mode attribute of item high_type"""
    NONE = 0
    FIXED = 1
    FLOAT = 2
    STRING = 3
    POINTER = 4
    ENUM = 5


class FieldSignMode(object):
    """enum corresponding to sign_mode attribute of item high_type"""
    UNSIGNED = 0
    SIGNED = 1


class FieldFloatMode(object):
    """enum corresponding to float_mode attribute of item high_type"""
    IEEE754_SINGLE = 0
    IEEE754_DOUBLE = 1


class FieldStringMode(object):
    """enum corresponding to string_mode attribute of item high_type"""
    STRING_RAW = 0
    STRING_ZERO_PADDED = 1
    STRING_ZERO_TERMINATED = 2


class FieldStringEncoding(object):
    """enum corresponding to string_encoding attribute of item high_type"""
    ENC_RAW = 0
    ENC_UTF8 = 1
    ENC_UTF16 = 2


class ChunkDataItem(object):
    def __init__(self, proto_obj):
        self._proto_obj = proto_obj
        self.values = self._parse_values()

    def _parse_values(self):
        # TODO implement more types
        raw = self._proto_obj.raw_value
        if self._proto_obj.high_type.mode == FieldHighMode.FIXED:
            formats = {8: 'b', 16: 'h', 32: 'i', 64: 'l'}
            if raw.width not in formats:
                return None
            format = formats[raw.width]
            if self._proto_obj.high_type.sign_mode == FieldSignMode.UNSIGNED:
                format = format.upper()
            return list(struct.unpack('<'+format*raw.size, raw.data))

        if self._proto_obj.high_type.mode == FieldHighMode.FLOAT:
            formats = {FieldFloatMode.IEEE754_SINGLE: 'f',
                       FieldFloatMode.IEEE754_DOUBLE: 'd'}
            format = formats[self._proto_obj.high_type.float_mode]
            return list(struct.unpack('<'+format*raw.size, raw.data))

    @property
    def type(self):
        return self._proto_obj.type

    @property
    def start(self):
        return self._proto_obj.start

    @property
    def end(self):
        return self._proto_obj.end

    @property
    def name(self):
        return self._proto_obj.name

    @property
    def raw_value(self):
        return self._proto_obj.raw_value

    def __getitem__(self, arg):
        return self.raw_value.data[arg]

    def __repr__(self):
        return 'name: {} values: {}'.format(self.name, self.values)
