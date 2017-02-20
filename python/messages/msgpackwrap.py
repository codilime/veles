import inspect

import msgpack


class MsgpackWrapper:
    @classmethod
    def pack_obj(cls, obj):
        if isinstance(obj, set):
            return list(obj)
        if obj.__class__ in cls.type_to_code:
            return msgpack.ExtType(
                cls.type_to_code[obj.__class__], obj.to_bytes())
        raise TypeError('Object of unknown type {}'.format(obj))

    @classmethod
    def load_obj(cls, code, data):
        if code in cls.code_to_type:
            return cls.code_to_type[code](data)
        return msgpack.ExtType(code, data)

    type_to_code = {}
    code_to_type = {}

    @classmethod
    def register_type(cls, type_class, type_code):
        if type_code < 0 or type_code > 127:
            raise ValueError('type_code must be between 0 and 127')
        if type_code in cls.code_to_type:
            raise ValueError('type_code already used')
        if not inspect.isclass(type_class):
            raise ValueError('type_class is not a class')
        if not callable(getattr(type_class, "to_bytes", None)):
            raise ValueError('type_class has to have to_bytes method')

        cls.code_to_type[type_code] = type_class
        cls.type_to_code[type_class] = type_code

packer = msgpack.Packer(use_bin_type=True, default=MsgpackWrapper.pack_obj)
unpacker = msgpack.Unpacker(encoding='utf-8', ext_hook=MsgpackWrapper.load_obj)
