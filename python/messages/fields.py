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
        value = self.validate(value)
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
        return value


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
        return value


class Boolean(Field):
    def validate(self, value):
        super().validate(value)
        if value is None:
            return
        if not isinstance(value, bool):
            raise ValueError('Attribute {} has to be bool type.'.format(
                self.name))
        return value


class Float(Field):
    def validate(self, value):
        super().validate(value)
        if value is None:
            return
        if not isinstance(value, float):
            raise ValueError('Attribute {} has to be float type.'.format(
                self.name))
        return value


class String(Field):
    def validate(self, value):
        super().validate(value)
        if value is None:
            return
        if not isinstance(value, str):
            raise ValueError('Attribute {} has to be str type.'.format(
                self.name))
        return value


class Binary(Field):
    def validate(self, value):
        super().validate(value)
        if value is None:
            return
        if not isinstance(value, bytes):
            raise ValueError('Attribute {} has to be str type.'.format(
                self.name))
        return value


class Array(Field):
    def __init__(self, optional=False, elements_types=None, local_type=list):
        super().__init__(optional)
        self._allowed_local = (list, tuple, set, frozenset)
        if elements_types:
            self.elements_types = elements_types
        else:
            self.elements_types = [Any()]
        if local_type not in self._allowed_local:
            raise ValueError('Illegal local_type value')
        self.local_type = local_type

    def validate(self, value):
        super().validate(value)
        if value is None:
            return []
        if not isinstance(value, self._allowed_local):
            raise ValueError(
                'Attribute {} has to be one of {} type.'.format(
                    self.name, self._allowed_local))
        prep_value = []
        for val in value:
            for element_type in self.elements_types:
                try:
                    prep_value.append(element_type.validate(val))
                    break
                except ValueError:
                    pass
            else:
                raise ValueError(
                    '{} doesn\'t fit in allowed element types'.format(val))
        return self.local_type(prep_value)


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
            return {}
        if not isinstance(value, dict):
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
        prep_value = {}
        for name, val in value.items():
            for value_type in self.values_types:
                try:
                    prep_value[name] = value_type.validate(val)
                    break
                except ValueError:
                    pass
            else:
                raise ValueError(
                    '{} doesn\'t fit in allowed value types'.format(val))
        return prep_value


class Extension(Field):
    def __init__(self, obj_type, optional=False):
        super().__init__(optional)
        self.obj_type = obj_type

    def validate(self, value):
        super().validate(value)
        if value is None:
            return
        if not isinstance(value, self.obj_type):
            raise ValueError('Attribute {} has to be {} type.'.format(
                self.name, self.obj_type))
        return value


class Object(Field):
    def __init__(self, optional=False, attributes_spec=[], local_type=None):
        super().__init__(optional)
        self.attributes = attributes_spec
        self.keys_types = [String()]
        self.local_type = local_type

    def validate(self, value):
        super().validate(value)
        if not isinstance(value, dict):
            raise ValueError(
                'Attribute {} has to be dict type.'.format(self.name))
        for val in value.keys():
            for key_type in self.keys_types:
                try:
                    key_type.validate(val)
                    break
                except ValueError:
                    pass

        prep_val = {}
        for attr_name, attr_type in self.attributes:
            prep_val[attr_name] = attr_type.validate(
                value.get(attr_name, None))
        if self.local_type:
            return self.local_type(**prep_val)
        return prep_val
