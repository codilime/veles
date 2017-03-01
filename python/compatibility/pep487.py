import sys
import types


if sys.version_info < (3, 6):
    class NewType(type):
        def __init__(self, name, bases, ns, **kwargs):
            super().__init__(name, bases, ns)

        def __new__(cls, *args, **kwargs):
            if len(args) != 3:
                return super().__new__(cls, *args)

            name, bases, ns = args

            init = ns.get('__init_subclass__')
            if isinstance(init, types.FunctionType):
                ns['__init_subclass__'] = classmethod(init)

            self = super().__new__(cls, name, bases, ns)

            for k, v in self.__dict__.items():
                func = getattr(v, '__set_name__', None)
                if func is not None:
                    func(self, k)
            if bases:
                super(self, self).__init_subclass__(**kwargs)

            return self

    class NewObject(metaclass=NewType):
        @classmethod
        def __init_subclass__(cls, **kwargs):
            pass

else:
    NewType = type
    NewObject = object
