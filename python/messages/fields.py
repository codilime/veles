class Field:
    def __init__(self, optional=False):
        self.name = None
        self.optional = optional

    def __get__(self, instance, owner=None):
        if self.optional:
            return instance.__dict__.get(self.name, None)
        else:
            return instance.__dict__[self.name]

    def __set__(self, instance, value):
        self.validate(value)
        # TODO we might want to do some extra processing here - i.e. allow
        # tags to be in a set instead of list
        instance.__dict__[self.name] = value

    def __set_name__(self, owner, name):
        self.name = name

    def add_to_class(self, cls):
        cls.fields.append(self)

    def validate(self, value):
        if not self.optional and value is None:
            raise ValueError(
                'Attribute {} is not optional and can\'t be None.'.format(
                    self.name))


class Any(Field):
    pass


class Integer(Field):
    def __init__(self, optional=False, minimum=-2**63, maximum=2**64-1):
        super().__init__(optional)
        self.minimum = minimum
        self.maximum = maximum

    def validate(self, value):
        super().validate(value)
        if value is None:
            return
        if not isinstance(value, int):
            raise ValueError('Attribute {} has to be int type.'.format(
                self.name))
        if value < self.minimum:
            raise ValueError('Attribute {} minimum value is {}.'.format(
                self.name, self.minimum))
        if value > self.maximum:
            raise ValueError('Attribute {} maximum value is {}.'.format(
                self.name, self.maximum))


class Boolean(Field):
    def validate(self, value):
        super().validate(value)
        if value is None:
            return
        if not isinstance(value, bool):
            raise ValueError('Attribute {} has to be bool type.'.format(
                self.name))


class Float(Field):
    def validate(self, value):
        super().validate(value)
        if value is None:
            return
        if not isinstance(value, float):
            raise ValueError('Attribute {} has to be float type.'.format(
                self.name))


class String(Field):
    def validate(self, value):
        super().validate(value)
        if value is None:
            return
        if not isinstance(value, str):
            raise ValueError('Attribute {} has to be str type.'.format(
                self.name))


class Binary(Field):
    def validate(self, value):
        super().validate(value)
        if value is None:
            return
        if not isinstance(value, bytes):
            raise ValueError('Attribute {} has to be str type.'.format(
                self.name))


class Array(Field):
    def __init__(self, optional=False, elements_types=None):
        super().__init__(optional)
        if elements_types:
            self.elements_types = elements_types
        else:
            self.elements_types = [Any()]

    def validate(self, value):
        super().validate(value)
        if value is None:
            return
        if not isinstance(value, (list, tuple)):
            raise ValueError(
                'Attribute {} has to be list or tuple type.'.format(self.name))
        for val in value:
            for element_type in self.elements_types:
                try:
                    element_type.validate(val)
                    break
                except ValueError:
                    pass
            else:
                raise ValueError(
                    '{} doesn\'t fit in allowed element types'.format(val))


class Map(Field):
    def __init__(self, optional=False, keys_types=None, values_types=None):
        super().__init__(optional)
        if keys_types:
            self.keys_types = keys_types
        else:
            self.keys_types = [Any()]
        if values_types:
            self.values_types = values_types
        else:
            self.values_types = [Any()]

    def validate(self, value):
        super().validate(value)
        if value is None:
            return
        if not isinstance(value, (dict)):
            raise ValueError(
                'Attribute {} has to be dict type.'.format(self.name))
        for val in value.keys():
            for key_type in self.keys_types:
                try:
                    key_type.validate(val)
                    break
                except ValueError:
                    pass
            else:
                raise ValueError(
                    '{} doesn\'t fit in allowed key types'.format(val))
        for val in value.values():
            for value_type in self.values_types:
                try:
                    value_type.validate(val)
                    break
                except ValueError:
                    pass
            else:
                raise ValueError(
                    '{} doesn\'t fit in allowed value types'.format(val))


class Extension(Field):
    pass
