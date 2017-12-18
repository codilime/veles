from veles.defs.symbol import Symbol
from veles.defs.st import (
    StForm, StList, StAny, StPosIntRaw, StInt, StSymbol, StWrap,
    StEnum, StSymbolRaw
)
from veles.data.repack import Endian, Repacker
from .parser import (
    ParserNamespace,
    ParserStruct,
    ParserExprAdd,
    ParserExprSub,
    ParserExprEq,
    ParserExprConstInt,
    ParserExprGetField,
    ParserExprLast,
    ParserPredEq,
    ParserField,
    ParserParam,
    ParserTypeBindata,
    ParserTypeInt,
    ParserTypeIntArray,
    ParserTypeStruct,
    ParserTypeStructArray,
    ParserCase,
    ParserOpMatch,
    ParserOpField,
    ParserOpCompute,
    ParserOpChildArray,
    ParserOpChildLoop,
    ParserLoopVar,
    ParserFieldInterpBindata,
    ParserFieldInterpInt,
    ParserFieldInterpIntArray,
    ParserBinDisplayString,
    ParserBinDisplaySigned,
    ParserBinDisplayUnsigned,
)


class StExpr(StAny): pass 
class StPred(StAny): pass 
class StStructOp(StAny): pass 
class StTopOp(StAny): pass 
class StFieldMod(StAny): pass 
class StChildLoopMod(StAny): pass 


class StWidth(StForm):
    tag = 'width'
    fields = [
        ('width', StPosIntRaw),
    ]

    def prep_fields(self, namespace, struct):
        pass


SYM_BIG = Symbol('big')
SYM_LITTLE = Symbol('little')


class StEndianValue(StEnum):
    map = {
        'big': Endian.BIG,
        'little': Endian.LITTLE,
    }


class StEndian(StForm):
    tag = 'endian'

    fields = [
        ('endian', StEndianValue)
    ]

    def prep_fields(self, namespace, struct):
        pass


class StStruct(StForm):
    tag = 'struct'
    fields = [
        ('name', StSymbolRaw),
    ]
    rest_field = 'ops'
    rest_matcher = StStructOp


class StParamInt(StForm):
    tag = 'param-int'
    fields = [
        ('name', StSymbolRaw),
    ]

    def prep_fields(self, namespace, struct):
        if self.name in struct.params:
            raise ValueError('duplicate param for {}'.format(struct.name))
        struct.params[self.name] = ParserParam(self.name, ParserTypeInt())

    def xlat_op(self, struct, endian):
        return None


class StFieldBase:
    def prep_fields(self, namespace, struct):
        typ = self.get_type(namespace, struct)
        if self.field not in struct.fields:
            field = ParserField(self.field, typ)
            struct.fields[self.field] = field
        else:
            field = struct.fields[self.field]
            field.type &= typ
        self.field_ref = field


class StComputeInt(StFieldBase, StForm):
    tag = 'compute-int'
    fields = [
        ('field', StSymbolRaw),
        ('expr', StExpr),
    ]

    def get_type(self, namespace, struct):
        return ParserTypeInt()

    def xlat_op(self, struct, endian):
        return ParserOpCompute(self.field_ref, self.expr.xlat(struct))


class StFieldSimple(StFieldBase):
    def post_parse(self):
        self.type = None
        for mod in self.mods:
            if isinstance(mod, StModType):
                if self.type is not None:
                    raise ValueError('two types for field')
                self.type = mod
            else:
                assert 0

    def get_type(self, namespace, struct):
        if self.is_array:
            num = None
            if isinstance(self.num, StExprConstInt):
                num = self.num.val
        else:
            num = 1
        if self.type is None:
            return ParserTypeBindata(self.width, num)
        elif self.is_array:
            return self.type.get_type_array(namespace, self.width, num)
        else:
            return self.type.get_type(namespace, self.width)

    def xlat_op(self, struct, endian):
        if self.is_array:
            num = self.num.xlat(struct)
        else:
            num = ParserExprConstInt(1)
        repacker = Repacker(endian, struct.width, self.width)
        if self.type is None:
            interp = ParserFieldInterpBindata()
        elif self.is_array:
            interp = self.type.get_interp_array(struct, endian)
        else:
            interp = self.type.get_interp(struct, endian)
        return ParserOpField(self.field_ref, repacker, num, interp)


class StField(StFieldSimple, StForm):
    tag = 'field'

    fields = [
        ('field', StSymbolRaw),
        ('width', StPosIntRaw),
    ]

    rest_field = 'mods'
    rest_matcher = StFieldMod

    is_array = False


class StFieldArray(StFieldSimple, StForm):
    tag = 'field*'

    fields = [
        ('field', StSymbolRaw),
        ('width', StPosIntRaw),
        ('num', StExpr),
    ]

    rest_field = 'mods'
    rest_matcher = StFieldMod

    is_array = True


class StChildArray(StFieldBase, StForm):
    tag = 'child*'

    fields = [
        ('field', StSymbolRaw),
        ('struct', StSymbolRaw),
        ('num', StExpr),
    ]

    def get_type(self, namespace, struct):
        num = None
        if isinstance(self.num, StExprConstInt):
            num = self.num.val
        sub_struct = namespace.structs[self.struct]
        return ParserTypeStructArray(sub_struct, num)

    def xlat_op(self, struct, endian):
        loop = ParserOpChildArray(struct, self.field_ref, self.num.xlat(struct))
        # XXX params, vars...
        return loop


class StChildLoop(StFieldBase, StForm):
    tag = 'child-loop'

    fields = [
        ('field', StSymbolRaw),
        ('struct', StSymbolRaw),
    ]

    rest_field = 'mods'
    rest_matcher = StChildLoopMod

    def post_parse(self):
        self.vars = []
        self.end = None
        self.params = []
        for mod in self.mods:
            if isinstance(mod, StModLoopVar):
                self.vars.append(mod)
            elif isinstance(mod, StModLoopEnd):
                if self.end is not None:
                    raise ValueError('double end condition')
                self.end = mod.cond
            elif isinstance(mod, StModParam):
                self.params.append(mod)
            else:
                assert 0

    def get_type(self, namespace, struct):
        sub_struct = namespace.structs[self.struct]
        return ParserTypeStructArray(sub_struct, None)

    def xlat_op(self, struct, endian):
        sub_struct = struct.namespace.structs[self.struct]
        loop = ParserOpChildLoop(struct, self.field_ref)
        for var in self.vars:
            if var.name in loop.vars or var.name in struct.fields or var.name in struct.params:
                raise ValueError('variable {} already exists'.format(var.name))
            cvar = ParserLoopVar(var.name, var.initial.xlat(struct))
            loop.var_dict[var.name] = cvar
            loop.vars.append(cvar)
            var.ref = cvar
        for var in self.vars:
            var.ref.next = var.next.xlat(loop)
        loop.end = self.end.xlat(loop)
        loop.params = [
            (sub_struct.params[param.name], param.value.xlat(loop))
            for param in self.params
        ]
        return loop


class StCase(StList):
    fields = [
        ('pred', StPred),
    ]
    rest_field = 'ops'
    rest_matcher = StStructOp


class StMatch(StForm):
    tag = 'match'
    fields = [
        ('expr', StExpr),
    ]
    rest_field = 'cases'
    rest_matcher = StCase

    def prep_fields(self, namespace, struct):
        for case in self.cases:
            for op in case.ops:
                op.prep_fields(namespace, struct)

    def xlat_op(self, struct, endian):
        cases = []
        for case in self.cases:
            cases.append(ParserCase(
                case.pred.xlat(struct),
                ParserCompiler.xlat_ops(struct, case.ops, endian),
            ))
        return ParserOpMatch(
            self.expr.xlat(struct),
            cases
        )


class StModType:
    def get_type(self, namespace, width):
        return self.get_type_array(namespace, width, 1)

    def get_interp(self, struct, endian):
        return self.get_interp_array(struct, endian)


class StModTypeInt(StModType):
    def get_type(self, namespace, width):
        return ParserTypeInt()

    def get_type_array(self, namespace, width, num):
        return ParserTypeIntArray(num)

    def get_interp(self, struct, endian):
        return ParserFieldInterpInt(self.signed)

    def get_interp_array(self, struct, endian):
        return ParserFieldInterpIntArray(self.signed)


class StModUnsignedInt(StModTypeInt, StForm):
    tag = 'unsigned-int'
    fields = []

    signed = False


class StModSignedInt(StModTypeInt, StForm):
    tag = 'signed-int'
    fields = []

    signed = True


class StModTypeBindata(StModType):
    def get_type_array(self, namespace, width, num):
        return ParserTypeBindata(width, num, self.get_display())

    def get_interp_array(self, struct, endian):
        return ParserFieldInterpBindata(self.get_display())


class StModUnsignedWrapInt(StModTypeBindata, StForm):
    tag = 'unsigned-wrap-int'
    fields = []

    def get_display(self):
        return ParserBinDisplayUnsigned()


class StModSignedWrapInt(StModTypeBindata, StForm):
    tag = 'signed-wrap-int'
    fields = []

    def get_display(self):
        return ParserBinDisplaySigned()


class StModString(StModTypeBindata, StForm):
    tag = 'string'
    fields = []

    def get_display(self):
        return ParserBinDisplayString()


class StModLoopVar(StForm):
    tag = 'var'
    fields = [
        ('name', StSymbolRaw),
        ('initial', StExpr),
        ('next', StExpr),
    ]


class StModParam(StForm):
    tag = 'param'
    fields = [
        ('name', StSymbolRaw),
        ('value', StExpr),
    ]


class StModLoopEnd(StForm):
    tag = 'end'
    fields = [
        ('cond', StExpr),
    ]


class StExprBin(StForm):
    fields = [
        ('e1', StExpr),
        ('e2', StExpr),
    ]

    def xlat(self, env):
        return self.cls(
            self.e1.xlat(env),
            self.e2.xlat(env),
        )


class StExprMinus(StExprBin):
    tag = '-'
    cls = ParserExprSub


class StExprPlus(StExprBin):
    tag = '+'
    cls = ParserExprAdd


class StExprEq(StExprBin):
    tag = '='
    cls = ParserExprEq


class StExprGetField(StForm):
    tag = 'get-field'
    fields = [
        ('expr', StExpr),
        ('field', StSymbolRaw),
    ]

    def xlat(self, env):
        xexpr = self.expr.xlat(env)
        xtype = xexpr.get_type()
        if not isinstance(xtype, ParserTypeStruct):
            raise ValueError('invalid type for get-field')
        return ParserExprGetField(xexpr, xtype.struct.fields[self.field])


class StExprLast(StForm):
    tag = 'last'
    fields = []

    def xlat(self, env):
        return ParserExprLast(env)


class StExprConstInt(StInt):
    def xlat(self, env):
        return ParserExprConstInt(self.val)


class StExprVar(StSymbol):
    def xlat(self, env):
        return env.lookup_var(self.val)


class StPredEq(StWrap):
    field = 'expr'
    matcher = StExpr

    def xlat(self, env):
        return ParserPredEq(self.expr.xlat(env))


StExpr.matchers = [
    StExprConstInt,
    StExprVar,
    StExprMinus,
    StExprPlus,
    StExprEq,
    StExprGetField,
    StExprLast,
]

StPred.matchers = [
    StPredEq,
]

StStructOp.matchers = [
    StWidth,
    StEndian,
    StParamInt,
    StComputeInt,
    StField,
    StFieldArray,
    StChildArray,
    StChildLoop,
    StMatch,
]

StTopOp.matchers = [
    StWidth,
    StEndian,
    StStruct,
]

StFieldMod.matchers = [
    StModUnsignedInt,
    StModSignedWrapInt,
    StModString,
]

StChildLoopMod.matchers = [
    StModLoopVar,
    StModLoopEnd,
    StModParam,
]


class ParserCompiler:
    def __init__(self):
        self.namespace = ParserNamespace()
        self.nodes = []
        self.compiled = False

    def add_sexprs(self, sexprs):
        assert not self.compiled
        for sexpr in sexprs:
            self.nodes.append(StTopOp.parse(sexpr))

    def compile(self):
        assert not self.compiled
        self.prep_structs()
        self.prep_fields()
        self.prep_ops()
        self.compiled = True

    def prep_structs(self):
        width = None
        for node in self.nodes:
            if isinstance(node, StWidth):
                width = node.width
            if isinstance(node, StStruct):
                if node.name in self.namespace.structs:
                    raise ValueError('duplicate struct {}'.format(node.name))
                my_width = None
                for op in node.ops:
                    if isinstance(op, StWidth):
                        if my_width is not None:
                            raise ValueError(
                                'duplicate width in struct {}'.format(
                                    node.name))
                        my_width = op.width
                if my_width is None:
                    my_width = width
                if my_width is None:
                    raise ValueError(
                        'width not specified for struct {}'.format(
                            node.name))
                struct = ParserStruct(self.namespace, node.name, my_width)
                self.namespace.structs[node.name] = struct
                node.ref = struct

    def prep_fields(self):
        for node in self.nodes:
            if isinstance(node, StStruct):
                for op in node.ops:
                    op.prep_fields(self.namespace, node.ref)
                for name in (set(node.ref.params) & set(node.ref.fields)):
                    raise ValueError(
                        '{} is both a param and field'.format(name))

    def prep_ops(self):
        endian = None
        for node in self.nodes:
            if isinstance(node, StEndian):
                endian = node.endian.val
            if isinstance(node, StStruct):
                node.ref.ops = self.xlat_ops(node.ref, node.ops, endian)

    @staticmethod
    def xlat_ops(struct, ops, endian):
        res = []
        for op in ops:
            if isinstance(op, StEndian):
                endian = op.endian.val
            xop = op.xlat_op(struct, endian)
            if xop is not None:
                res.append(xop)
        return res


def compile_parser(sexprs):
    compiler = ParserCompiler()
    compiler.add_sexprs(sexprs)
    compiler.compile()
    return compiler.namespace
