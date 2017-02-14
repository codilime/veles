def ehdr_get_listing(srv, obj):
    raise NotImplementedError
def phdr_get_listing(srv, obj):
    raise NotImplementedError
def shdr_get_listing(srv, obj):
    raise NotImplementedError

def ehdr_prep(srv, obj):
    raise NotImplementedError
def ehdr_parse(srv, obj):
    raise NotImplementedError
def phdr_parse(srv, obj):
    raise NotImplementedError
def shdr_parse(srv, obj):
    raise NotImplementedError

class Plugin:
    name = 'elf'
    mthds = [
        ({'elf.ehdr'}, 'get_listing', ehdr_get_listing),
        ({'elf.phdr'}, 'get_listing', phdr_get_listing),
        ({'elf.shdr'}, 'get_listing', shdr_get_listing),
    ]
    triggers = [
        ({'elf.ehdr'}, 'ehdr_prep', ehdr_prep),
        ({'elf.ehdr', 'elf.ehdr.prepped'}, 'ehdr_parse', ehdr_parse),
        ({'elf.phdr'}, 'phdr_parse', phdr_parse),
        ({'elf.shdr'}, 'shdr_parse', shdr_parse),
    ]
    chunk_types = [
    ]
