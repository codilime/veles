class ParserNamespace:
    def __init__(self):
        self.structs = {}


class ParserStruct:
    def __init__(self, namespace, name, width):
        self.namespace = namespace
        self.name = name
        # Needs to be updated later.
        self.ops = None
        self.fields = {}
        self.params = {}
        self.width = width

    def lookup_var(self, var):
        if var in self.fields:
            return ParserExprField(self.fields[var])
        elif var in self.params:
            return ParserExprParam(self.params[var])
        raise ValueError('unknown var {}'.format(var))


class ParserParam:
    def __init__(self, name, type):
        self.name = name
        self.type = type

    def __str__(self):
        return str(self.type)


class ParserField:
    def __init__(self, name, type):
        self.name = name
        self.type = type

    def __str__(self):
        return str(self.type)


class ParserOp:
    pass


class ParserOpField(ParserOp):
    def __init__(self, field, repacker, num, interp):
        self.field = field
        self.repacker = repacker
        self.num = num
        self.interp = interp


class ParserOpChildArray(ParserOp):
    def __init__(self, struct, field, num):
        self.struct = struct
        self.field = field
        self.num = num
        # To be updated later.
        self.vars = []
        self.var_dict = {}
        self.params = []

    def lookup_var(self, var):
        if var in self.var_dict:
            return ParserExprLoopVar(self.var_dict[var])
        return self.struct.lookup_var(var)

    def get_item_type(self):
        return ParserTypeStruct(self.field.type.struct)


class ParserOpChildLoop(ParserOp):
    def __init__(self, struct, field):
        self.struct = struct
        self.field = field
        # To be updated later.
        self.vars = []
        self.var_dict = {}
        self.end = None
        self.params = []

    def lookup_var(self, var):
        if var in self.var_dict:
            return ParserExprLoopVar(self.var_dict[var])
        return self.struct.lookup_var(var)

    def get_item_type(self):
        return ParserTypeStruct(self.field.type.struct)


class ParserOpMatch(ParserOp):
    def __init__(self, expr, cases):
        self.expr = expr
        self.cases = cases


class ParserOpCompute(ParserOp):
    def __init__(self, field, expr):
        self.field = field
        self.expr = expr


class ParserLoopVar:
    def __init__(self, name, initial):
        self.name = name
        self.initial = initial
        # To be updated later.
        self.next = None


class ParserCase:
    def __init__(self, pred, ops):
        self.pred = pred
        self.ops = ops


class ParserExpr:
    pass


class ParserExprParam(ParserExpr):
    def __init__(self, param):
        self.param = param

    def get_type(self):
        return self.param.type


# XXX lots of get_types need to be implemented

class ParserExprField(ParserExpr):
    def __init__(self, field):
        self.field = field


class ParserExprLoopVar(ParserExpr):
    def __init__(self, var):
        self.var = var


class ParserExprGetField(ParserExpr):
    def __init__(self, expr, field):
        self.expr = expr
        self.field = field


class ParserExprLast(ParserExpr):
    def __init__(self, env):
        self.env = env

    def get_type(self):
        return self.env.get_item_type()


class ParserExprConstInt(ParserExpr):
    def __init__(self, value):
        self.value = value


class ParserExprAdd(ParserExpr):
    def __init__(self, e1, e2):
        self.e1 = e1
        self.e2 = e2


class ParserExprSub(ParserExpr):
    def __init__(self, e1, e2):
        self.e1 = e1
        self.e2 = e2


class ParserExprEq(ParserExpr):
    def __init__(self, e1, e2):
        self.e1 = e1
        self.e2 = e2


class ParserPred:
    pass


class ParserPredEq(ParserPred):
    def __init__(self, val):
        self.val = val


class ParserBinDisplay:
    pass


class ParserBinDisplayRaw(ParserBinDisplay):
    pass


class ParserBinDisplayUnsigned(ParserBinDisplay):
    pass


class ParserBinDisplaySigned(ParserBinDisplay):
    pass


class ParserBinDisplayString(ParserBinDisplay):
    pass


class ParserType:
    pass


class ParserTypeBindata(ParserType):
    def __init__(self, width, num, display=ParserBinDisplayRaw()):
        self.width = width
        self.num = num
        self.display = display

    def __str__(self):
        if isinstance(self.display, ParserBinDisplayRaw):
            res = 'b{}'.format(self.width)
        elif isinstance(self.display, ParserBinDisplayUnsigned):
            res = 'u{}'.format(self.width)
        elif isinstance(self.display, ParserBinDisplaySigned):
            res = 's{}'.format(self.width)
        elif isinstance(self.display, ParserBinDisplayString):
            res = 'c{}'.format(self.width)
        else:
            assert 0
        if self.num is None:
            res = '{}[]'.format(res)
        elif self.num != 1:
            res = '{}[{}]'.format(res, self.num)
        return res

    def __and__(self, other):
        # XXX
        raise NotImplementedError


class ParserTypeInt(ParserType):
    def __and__(self, other):
        if not isinstance(other, ParserTypeInt):
            raise TypeError('type conflict: {} vs {}'.format(self, other))
        return ParserTypeInt()

    def __str__(self):
        return 'int'


class ParserTypeIntArray(ParserType):
    def __init__(self, num):
        self.num = num

    def __and__(self, other):
        if not isinstance(other, ParserTypeIntArray):
            raise TypeError('type conflict: {} vs {}'.format(self, other))
        num = self.num
        if other.num != num:
            num = None
        return ParserTypeIntArray(num)

    def __str__(self):
        if self.num is None:
            return 'int[]'
        elif self.num != 1:
            return 'int[{}]'.format(self.num)


class ParserTypeStructArray(ParserType):
    def __init__(self, struct, num):
        self.struct = struct
        self.num = num

    def __and__(self, other):
        if (not isinstance(other, ParserTypeStructArray)
                or self.struct is not other.struct):
            raise TypeError('type conflict: {} vs {}'.format(self, other))
        num = self.num
        if other.num != num:
            num = None
        return ParserTypeStructArray(self.struct, num)

    def __str__(self):
        res = 'struct {}'.format(self.struct.name)
        if self.num is None:
            return '{}[]'.format(res)
        elif self.num != 1:
            return '{}[{}]'.format(res, self.num)


class ParserTypeStruct(ParserType):
    def __init__(self, struct):
        self.struct = struct

    def __and__(self, other):
        if (not isinstance(other, ParserTypeStruct)
                or self.struct is not other.struct):
            raise TypeError('type conflict: {} vs {}'.format(self, other))
        return self

    def __str__(self):
        return 'struct {}'.format(self.struct.name)


class ParserFieldInterpBindata:
    def __init__(self, display=ParserBinDisplayRaw()):
        self.display = display


class ParserFieldInterpInt:
    def __init__(self, signed):
        self.signed = signed


class ParserFieldInterpIntArray:
    def __init__(self, signed):
        self.signed = signed
