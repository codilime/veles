from sys import intern


class Symbol:
    def __init__(self, s):
        self.str = intern(s)

    def __eq__(self, other):
        return isinstance(other, Symbol) and self.str is other.str

    def __hash__(self):
        return id(self.str)

    def __repr__(self):
        return self.str
