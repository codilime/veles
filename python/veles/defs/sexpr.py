from veles.defs.symbol import Symbol

token_extra_chars = '_-+?!*.='


def tokenize(data):
    pos = 0
    while pos < len(data):
        c = data[pos]
        if c in '()\'':
            yield c, None
            pos += 1
        elif c.isspace():
            pos += 1
        elif c == '"':
            pos += 1
            start_pos = pos
            res = ''
            while data[pos] != '"':
                if data[pos] == '\\':
                    res += data[start_pos:pos]
                    c = data[pos + 1]
                    pos += 2
                    subs = {
                        '\\': '\\',
                        '\"': '\"',
                        'a': '\a',
                        't': '\t',
                        'n': '\n',
                        'f': '\f',
                        'v': '\v',
                        'r': '\r',
                        '0': '\0',
                    }
                    codes = {
                        'x': 2,
                        'u': 4,
                        'U': 6,
                    }
                    if c in subs:
                        res += subs[c]
                    elif c in codes:
                        l = codes[c]
                        code = data[pos:pos+l]
                        pos += l
                        if not code.isalnum():
                            raise ValueError('bad code \\{}{}'.format(c, code))
                        res += chr(int(code, 16))
                    else:
                        raise ValueError('reserved \\{}'.format(c))
                    start_pos = pos
                else:
                    pos += 1
            res += data[start_pos:pos]
            pos += 1
            yield 'string', res
        elif c == '#':
            c = data[pos+1]
            pos += 2
            if c == 't':
                yield 'bool', True
            elif c == 'f':
                yield 'bool', False
            elif c == 'n' and data[pos:pos+2] == 'il':
                pos += 2
                yield 'nil', None
            elif c == 'x':
                start_pos = pos
                while (pos < len(data) and
                        (data[pos] in token_extra_chars or
                            data[pos].isalpha() or data[pos].isdigit())):
                    pos += 1
                if start_pos == pos:
                    raise ValueError('empty integer')
                yield 'int', int(data[start_pos:pos], 16)
            else:
                raise ValueError('reserved #{}'.format(c))
        elif c == ';':
            while pos < len(data) and data[pos] != '\n':
                pos += 1
        elif c.isdigit():
            start_pos = pos
            while (pos < len(data) and
                    (data[pos] in token_extra_chars or
                        data[pos].isalpha() or data[pos].isdigit())):
                pos += 1
            yield 'int', int(data[start_pos:pos], 0)
        elif c in token_extra_chars or c.isalpha():
            start_pos = pos
            while (pos < len(data) and
                    (data[pos] in token_extra_chars or
                        data[pos].isalpha() or data[pos].isdigit())):
                pos += 1
            yield 'symbol', data[start_pos:pos]
        else:
            raise ValueError('reserved character {}'.format(c))


def parse_sexpr(data):
    stack = []
    for kind, payload in tokenize(data):
        if kind == '(':
            stack.append(())
            continue
        elif kind == ')':
            if not stack:
                raise ValueError('unmatched )')
            thing = stack.pop()
        elif kind == '\'':
            stack.append('\'')
            continue
        elif kind in ('bool', 'int', 'nil', 'string'):
            thing = payload
        elif kind == 'symbol':
            thing = Symbol(payload)
        while stack and stack[-1] == '\'':
            thing = (Symbol('quote'), thing)
            stack.pop()
        if not stack:
            yield thing
        else:
            stack[-1] += thing,
    if stack:
        raise ValueError('non-empty stack left')
