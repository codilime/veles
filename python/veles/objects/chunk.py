from veles import network_pb2
from veles.objects.base import LocalObject
from veles.objects.chunk_data_item import (
    ChunkDataItemTypes, RepackEndian, FieldHighMode, FieldSignMode,
    FieldFloatMode, FieldStringMode, FieldStringEncoding, ChunkDataItem)


class Chunk(LocalObject):
    class AddressMode(object):
        ABSOLUTE = 0
        RELATIVE = 1

    _default_address_mode = AddressMode.RELATIVE

    def __init__(self, client, proto_obj=None, id_path=None, parent=None):
        super(Chunk, self).__init__(client, proto_obj, id_path, parent)
        self._data = None
        self._start = proto_obj.chunk_start
        self._end = proto_obj.chunk_end
        self._blob = parent.get_blob() if parent is not None else None
        self._cursor = self._start
        self.items = [ChunkDataItem(item) for item in self._proto_obj.items]

    def _from_another(self, other):
        super(Chunk, self)._from_another(other)
        self.items = other.items

    def _items_from_another(self, other):
        self._proto_obj = other._proto_obj
        self.items = other.items

    @classmethod
    def setDefaultAddressMode(cls, mode):
        cls._default_address_mode = mode

    def get_blob(self):
        return self._blob

    def __iter__(self):
        return self._blob[self._start:self._end].__iter__()

    def __getitem__(self, arg):
        if self._default_address_mode == self.AddressMode.ABSOLUTE:
            return self._blob[arg]
        if isinstance(arg, int):
            return self._blob(arg + self._start)
        if isinstance(arg, slice):
            start = arg.start if arg.start is not None else 0
            start += self._start
            stop = (arg.stop if arg.stop is not None
                    else self._end - self._start)
            stop += self._start
            return self._blob[slice(start, stop, arg.step)]

    def create_chunk_item(self, name, num_elements=1, start=None,
                          update_cursor=True,
                          endianness=RepackEndian.LITTLE,
                          bit_width=8, high_pad=0, low_pad=0,
                          mode=FieldHighMode.FIXED,
                          shift=0,
                          sign_mode=FieldSignMode.UNSIGNED,
                          float_mode=FieldFloatMode.IEEE754_SINGLE,
                          float_complex=False,
                          string_mode=FieldStringMode.STRING_RAW,
                          string_encoding=FieldStringEncoding.ENC_RAW,
                          type_name=""):
        """Create ChunkDataItem for this chunk

        ChunkDataItem can represent many different entities, currently Python
        interface only supports FIELD type

        FIELD - contains 1 or more elements of format given by other parameters
        There are multiple parameters, some of them only make sense in context
        of specific mode

        Args:
            name: name of ChunkDataItem
            num_elements: number of elements that this item contains,
                defaults to 1
            start: start of ChunkDataItem, defaults to cursor value
            update_cursor: determines whether cursor will be updated
                after this operation, defaults to True
            endianness: endianness of elements, defaults to RepackEndian.LITTLE
            bit_width: width in bits of each elements, defaults to 8
            high_pad: number of bits between MSB and actual element,
                defaults to 0
            low_pad: number of bits between LSB and actual element,
                defaults to 0
            mode: type of element, defaults to FieldHighMode.FIXED
            shift: for FieldHighMode.FIXED and FieldHighMode.POINTER,
                positive means the raw value is scaled up by a PoT,
                negative is for FieldHighMode.FIXED only
                and means a fractional value, defaults to 0
            sign_mode: decides if value is signed or unsigned,
                defaults to FieldSignMode.UNSIGNED
            float_mode: decides if floating point is single or
                double precision, defaults to FieldFloatMode.IEEE754_SINGLE
            float_complex: if true this field should be made of an even number
                of elements - even ones are real parts, odd ones are imaginary,
                defaults to False
            string_mode: mode of string, defaults to FieldStringMode.STRING_RAW
            string_encoding: string encoding,
                defaults to FieldStringEncoding.ENC_RAW
            type_name: for FieldHighMode.ENUM names of enum type,
                for POINTER: names the pointed-to type,
                defaults to empty string
        """
        req = network_pb2.Request()
        req.type = network_pb2.Request.ADD_CHUNK_ITEM
        req.id.extend(self._id_path)
        # TODO other types than just field
        req.chunk_item.type = ChunkDataItemTypes.FIELD
        if start is None:
            start = self._cursor
        req.chunk_item.start = start
        req.chunk_item.name = name
        req.chunk_item.repack.endian = endianness
        req.chunk_item.repack.width = bit_width
        req.chunk_item.repack.high_pad = high_pad
        req.chunk_item.repack.low_pad = low_pad
        req.chunk_item.num_elements = num_elements
        req.chunk_item.high_type.mode = mode
        req.chunk_item.high_type.shift = shift
        req.chunk_item.high_type.sign_mode = sign_mode
        req.chunk_item.high_type.float_mode = float_mode
        req.chunk_item.high_type.float_complex = float_complex
        req.chunk_item.high_type.string_mode = string_mode
        req.chunk_item.high_type.string_encoding = string_encoding
        req.chunk_item.high_type.type_name = type_name
        results = self._client._send_req(req)

        self._items_from_another(results[0])
        if update_cursor:
            self._cursor = self.items[-1].end
        return self.items[-1]
