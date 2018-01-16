from .symbol import Symbol


class StMatchFail(Exception):
    pass


class StForm:
    rest_matcher = None

    @classmethod
    def parse(cls, sexpr):
        if not isinstance(sexpr, tuple):
            raise StMatchFail('not a list')
        if not sexpr:
            raise StMatchFail('empty list')
        if not isinstance(sexpr[0], Symbol):
            raise StMatchFail('first item not a symbol')
        if sexpr[0].str != cls.tag:
            raise StMatchFail('mismatched tag')
        res = cls()
        if len(sexpr) - 1 < len(cls.fields):
            raise StMatchFail('too few parameters')
        try:
            for (name, matcher), sub in zip(cls.fields, sexpr[1:]):
                setattr(res, name, matcher.parse(sub))
        except StMatchFail as e:
            raise ValueError('invalid argument for {}: {}'.format(
                cls.__name__, sub)) from e
        if cls.rest_matcher is None:
            if len(sexpr) - 1 != len(cls.fields):
                raise StMatchFail('too many parameters')
        else:
            try:
                rest = []
                for sub in sexpr[len(cls.fields)+1:]:
                    rest.append(cls.rest_matcher.parse(sub))
                setattr(res, cls.rest_field, rest)
            except StMatchFail as e:
                raise ValueError('invalid argument for {}: {}'.format(
                    cls.__name__, sub)) from e
        res.post_parse()
        return res

    def post_parse(self):
        pass


class StList:
    rest_matcher = None

    @classmethod
    def parse(cls, sexpr):
        if not isinstance(sexpr, tuple):
            raise StMatchFail('not a list')
        res = cls()
        if len(sexpr) < len(cls.fields):
            raise StMatchFail('too few parameters')
        try:
            for (name, matcher), sub in zip(cls.fields, sexpr):
                setattr(res, name, matcher.parse(sub))
        except StMatchFail as e:
            raise ValueError('invalid argument for {}: {}'.format(
                cls.__name__, sub)) from e
        if cls.rest_matcher is None:
            if len(sexpr) != len(cls.fields):
                raise StMatchFail('too many parameters')
        else:
            try:
                rest = []
                for sub in sexpr[len(cls.fields):]:
                    rest.append(cls.rest_matcher.parse(sub))
                setattr(res, cls.rest_field, rest)
            except StMatchFail as e:
                raise ValueError('invalid argument for {}: {}'.format(
                    cls.__name__, sub)) from e
        res.post_parse()
        return res

    def post_parse(self):
        pass


class StEnum:
    @classmethod
    def parse(cls, sexpr):
        if not isinstance(sexpr, Symbol):
            raise StMatchFail('not a symbol')
        if sexpr.str not in cls.map:
            raise StMatchFail('unknown symbol')
        res = cls()
        res.val = cls.map[sexpr.str]
        return res


class StSymbolRaw:
    @classmethod
    def parse(cls, sexpr):
        if not isinstance(sexpr, Symbol):
            raise StMatchFail('not a symbol')
        return sexpr.str


class StSymbol:
    @classmethod
    def parse(cls, sexpr):
        if not isinstance(sexpr, Symbol):
            raise StMatchFail('not a symbol')
        res = cls()
        res.val = sexpr.str
        return res


class StInt:
    @classmethod
    def validate(cls, val):
        return

    @classmethod
    def parse(cls, sexpr):
        if not isinstance(sexpr, int):
            raise StMatchFail('not an int')
        cls.validate(sexpr)
        res = cls()
        res.val = sexpr
        return res


class StPosInt(StInt):
    @classmethod
    def validate(cls, val):
        if val <= 0:
            raise StMatchFail('int must be positive')


class StIntRaw:
    @classmethod
    def validate(cls, val):
        return

    @classmethod
    def parse(cls, sexpr):
        if not isinstance(sexpr, int):
            raise StMatchFail('not an int')
        cls.validate(sexpr)
        return sexpr


class StPosIntRaw(StIntRaw):
    @classmethod
    def validate(cls, val):
        if val <= 0:
            raise StMatchFail('int must be positive')


class StNil:
    @classmethod
    def parse(cls, sexpr):
        if sexpr is not None:
            raise StMatchFail('not a nil')
        return cls()


class StAny:
    @classmethod
    def parse(cls, sexpr):
        for matcher in cls.matchers:
            try:
                return matcher.parse(sexpr)
            except StMatchFail:
                pass
        raise StMatchFail('no matches')


class StWrap:
    @classmethod
    def parse(cls, sexpr):
        sub = cls.matcher.parse(sexpr)
        res = cls()
        setattr(res, cls.field, sub)
        return res
