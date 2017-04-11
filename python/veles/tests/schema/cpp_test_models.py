from veles.schema import fields
from veles.schema import model
from veles.schema import enumeration


class Any(model.Model):
    a = fields.Any()


class AnyOptional(model.Model):
    a = fields.Any(optional=True)

# TODO what about Empty?


class Integer(model.Model):
    a = fields.Integer(default=-42)


class IntegerOptional(model.Model):
    a = fields.Integer(optional=True)


class UnsignedInteger(model.Model):
    a = fields.UnsignedInteger(default=42)


class UnsignedIntegerOptional(model.Model):
    a = fields.UnsignedInteger(optional=True)


class SmallInteger(model.Model):
    a = fields.SmallInteger(default=-42)


class SmallIntegerOptional(model.Model):
    a = fields.SmallInteger(optional=True)


class SmallUnsignedInteger(model.Model):
    a = fields.SmallUnsignedInteger(default=42)


class SmallUnsignedIntegerOptional(model.Model):
    a = fields.SmallUnsignedInteger(optional=True)


class Boolean(model.Model):
    a = fields.Boolean(default=True)


class BooleanOptional(model.Model):
    a = fields.Boolean(optional=True)


class Float(model.Model):
    a = fields.Float(default=5.0)


class FloatOptional(model.Model):
    a = fields.Float(optional=True)


class String(model.Model):
    a = fields.String()


class StringOptional(model.Model):
    a = fields.String(optional=True)


class Binary(model.Model):
    a = fields.Binary()


class BinaryOptional(model.Model):
    a = fields.Binary(optional=True)


class NodeIDModel(model.Model):
    a = fields.NodeID()


class NodeIDModelOptional(model.Model):
    a = fields.NodeID(optional=True)

# TODO BinData


class List(model.Model):
    a = fields.List(fields.SmallInteger())


class ListOptional(model.Model):
    a = fields.List(fields.SmallInteger(), optional=True, default=None)


class Set(model.Model):
    a = fields.Set(fields.SmallInteger())


class SetOptional(model.Model):
    a = fields.Set(fields.SmallInteger(), optional=True, default=None)


class Map(model.Model):
    a = fields.Map(fields.String(), fields.SmallInteger())


class MapOptional(model.Model):
    a = fields.Map(
        fields.String(), fields.SmallInteger(), optional=True, default=None)


class Object(model.Model):
    a = fields.Object(String)


class ObjectOptional(model.Model):
    a = fields.Object(String, optional=True)


class TestEnum(enumeration.EnumModel):
    OPT1 = 'OPT1_VAL'
    OPT2 = 'OPT2_VAL'


class Enum(model.Model):
    a = fields.Enum(TestEnum)


class EnumOptional(model.Model):
    a = fields.Enum(TestEnum, optional=True)

# TODO improve it (we can't simply depend on sequence of declaration since
# module __dict__ isn't ordered)


enums = [TestEnum]
models = [
    Any, AnyOptional, Integer, IntegerOptional,
    UnsignedInteger, UnsignedIntegerOptional,
    SmallInteger, SmallIntegerOptional,
    SmallUnsignedInteger, SmallUnsignedIntegerOptional,
    Boolean, BooleanOptional, Float, FloatOptional, String, StringOptional,
    Binary, BinaryOptional, NodeIDModel, NodeIDModelOptional,
    List, ListOptional, Set, SetOptional, Map, MapOptional,
    Object, ObjectOptional, Enum, EnumOptional]

# TODO polymodel tests
