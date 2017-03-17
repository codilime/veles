from veles.compatibility.pep487 import NewObject


class VelesException(Exception, NewObject):
    types = {}

    def __init__(self, *args):
        if type(self) is VelesException:
            code, msg = args
            if code in VelesException.types:
                self.__class__ = VelesException.types[code]
                args = msg,
            else:
                self.code = code
        else:
            code = self.code
            if args:
                msg, = args
            else:
                msg = self.msg
        self.msg = msg
        super(VelesException, self).__init__(*args)

    def __init_subclass__(cls, **kwargs):
        super(VelesException, cls).__init_subclass__(**kwargs)
        VelesException.types[cls.code] = cls


class ObjectGoneError(VelesException):
    code = 'object_gone'
    msg = "Object has been deleted, or never existed"


class ObjectExistsError(VelesException):
    code = 'object_exists'
    msg = "Object with the given id already exists"


class WritePastEndError(VelesException):
    code = 'write_past_end'
    msg = "Data written past the end of object"
