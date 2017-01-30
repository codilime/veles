from veles.objects.base import LocalObject


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
