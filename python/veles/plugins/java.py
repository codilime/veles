from pathlib import Path

from veles.defs.sexpr import parse_sexpr
from veles.parser.compile import compile_parser


ns = compile_parser(parse_sexpr(
    (Path(__file__).parent / 'java_class.sexpr').read_text()
))
