# Copyright 2017 CodiLime
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Sweet dreams are made of these
# Who am I to disagree?
# To travel the world and seven seas
# Everybody is looking for something

from ..core import Isa
from ..insn import InsnSwitch, Insn
from ..field import IsaField, IsaSubField, IsaMatch, IsaSplitField
from ..parser import ParseWord, ParseInsn
from ..arg import (ArgPCRel, ArgConstReg, ArgImm, ArgSwitch,
                   ArgMem, ArgMemRRS, ArgMemRI)
from ..reg import Register, RegisterSplit
from ..mem import MemSpace
from ..mod import ModIf, ModSwitch, Mod


class SparcArch:
    regs = [
        Register("g{}".format(x), 64) for x in range(8)
    ] + [
        Register("o{}".format(x), 64) for x in range(8)
    ] + [
        Register("l{}".format(x), 64) for x in range(8)
    ] + [
        Register("i{}".format(x), 64) for x in range(8)
    ]

    fregs = [
        Register("f{}".format(x), 32) for x in range(32)
    ]
    fregsd = [None for x in range(32)]
    fregsq = [None for x in range(32)]
    for x in range(0, 32, 2):
        fregsd[x] = RegisterSplit("f{}d".format(x), 64, [
            (0, fregs[x+1]),
            (32, fregs[x]),
        ])
    for x in range(32, 64, 2):
        fregsd[x-32+1] = Register("f{}d".format(x), 64)
    for x in [*range(0, 32, 4), *range(1, 32, 4)]:
        fregsq[x] = RegisterSplit("f{}q".format(x), 128, [
            (0, fregsd[x+2]),
            (64, fregsd[x]),
        ])

    # XXX fsr fields
    reg_fsr = Register("fsr", 64)
    reg_asi = Register("asi", 8)
    reg_icc = Register("icc", 4)
    reg_xcc = Register("xcc", 4)
    regs_fcc = [
        Register("fcc{}".format(x), 4) for x in range(4)
    ]

    mem = MemSpace("", 8, 64)


class SparcFields:
    # The instruction word.
    iw = IsaField(32)
    # Opcode part A.
    op = IsaSubField(iw, 30, 2)
    # Forms 0, 2, 3 - destination reg.
    rd = IsaSubField(iw, 25, 5)
    cc2 = IsaSubField(iw, 25, 2)
    # Form 0 - opcode part B.
    op2 = IsaSubField(iw, 22, 3)
    # Form 0 - 22-bit imm
    imm22 = IsaSubField(iw, 0, 22)
    # Form 0.1 - 19-bit imm
    imm19 = IsaSubField(iw, 0, 19)
    # Form 0.3 - 16-bit imm
    d16lo = IsaSubField(iw, 0, 14)
    d16hi = IsaSubField(iw, 20, 2)
    d16 = IsaSplitField(d16lo, d16hi)
    # Form 0.1, 0.2, 0.5, 0.6
    a = IsaSubField(iw, 29, 1)
    # Form 0.1, 0.2, 0.5, 0.6
    cond = IsaSubField(iw, 25, 4)
    # Form 0.1
    p = IsaSubField(iw, 19, 1)
    # Form 0.1
    cc = IsaSubField(iw, 20, 2)
    # Form 1 - 30-bit imm
    imm30 = IsaSubField(iw, 0, 30)
    # Forms 2, 3 - opcode part B.
    op3 = IsaSubField(iw, 19, 6)
    # Forms 2, 3 - source reg 1.
    rs1 = IsaSubField(iw, 14, 5)
    # Forms 2, 3 - source reg 2.
    rs2 = IsaSubField(iw, 0, 5)
    # Forms 2, 3 - immediate ASI
    imm_asi = IsaSubField(iw, 5, 8)
    # Forms 2, 3 - fp opcode
    opf = IsaSubField(iw, 5, 9)
    # Forms 2, 3 - immediate/register selection
    i = IsaSubField(iw, 13, 1)
    # Forms 2, 3 - shift ext mode
    x = IsaSubField(iw, 12, 1)
    # Forms 2, 3 - 13-bit imm
    imm13 = IsaSubField(iw, 0, 13)
    # Forms 2, 3 - membar imm
    membar = IsaSubField(iw, 0, 7)
    # Forms 2, 3 - shift imms
    shcnt32 = IsaSubField(iw, 0, 5)
    shcnt64 = IsaSubField(iw, 0, 6)


class SparcArgs:
    fields = SparcFields
    arch = SparcArch

    disp30 = ArgPCRel(64, fields.imm30, shift=2)
    disp22 = ArgPCRel(64, fields.imm22, shift=2)
    disp19 = ArgPCRel(64, fields.imm19, shift=2)
    disp16 = ArgPCRel(64, fields.d16, shift=2)
    imm22 = ArgImm(64, fields.imm22)
    simm13 = ArgImm(64, fields.imm13, signed=True)
    imm_asi = ArgImm(8, fields.imm_asi)
    membar = ArgImm(7, fields.membar)
    prefetch_fcn = ArgImm(5, fields.rd)
    shcnt32 = ArgImm(5, fields.shcnt32)
    shcnt64 = ArgImm(5, fields.shcnt64)

    regs_arg = [ArgConstReg(reg) for reg in arch.regs]
    fregs_arg = [ArgConstReg(reg) for reg in arch.fregs]
    fregsd_arg = [ArgConstReg(reg) for reg in arch.fregsd]
    fregsq_arg = [ArgConstReg(reg) if reg else None for reg in arch.fregsq]

    rs1 = ArgSwitch(fields.rs1, [
        IsaMatch(x, y)
        for x, y in enumerate(regs_arg)
    ])
    rs2 = ArgSwitch(fields.rs2, [
        IsaMatch(x, y)
        for x, y in enumerate(regs_arg)
    ])
    rd = ArgSwitch(fields.rd, [
        IsaMatch(x, y)
        for x, y in enumerate(regs_arg)
    ])
    fs1 = ArgSwitch(fields.rs1, [
        IsaMatch(x, y)
        for x, y in enumerate(fregs_arg)
    ])
    fs2 = ArgSwitch(fields.rs2, [
        IsaMatch(x, y)
        for x, y in enumerate(fregs_arg)
    ])
    fd = ArgSwitch(fields.rd, [
        IsaMatch(x, y)
        for x, y in enumerate(fregs_arg)
    ])
    fs1d = ArgSwitch(fields.rs1, [
        IsaMatch(x, y)
        for x, y in enumerate(fregsd_arg)
    ])
    fs2d = ArgSwitch(fields.rs2, [
        IsaMatch(x, y)
        for x, y in enumerate(fregsd_arg)
    ])
    fdd = ArgSwitch(fields.rd, [
        IsaMatch(x, y)
        for x, y in enumerate(fregsd_arg)
    ])
    fs1q = ArgSwitch(fields.rs1, [
        IsaMatch(x, y)
        for x, y in enumerate(fregsq_arg)
        if y is not None
    ])
    fs2q = ArgSwitch(fields.rs2, [
        IsaMatch(x, y)
        for x, y in enumerate(fregsq_arg)
        if y is not None
    ])
    fdq = ArgSwitch(fields.rd, [
        IsaMatch(x, y)
        for x, y in enumerate(fregsq_arg)
        if y is not None
    ])
    asi = ArgConstReg(arch.reg_asi)
    fsr = ArgConstReg(arch.reg_fsr)
    cc = ArgSwitch(fields.cc, [
        IsaMatch(0, ArgConstReg(arch.reg_icc)),
        IsaMatch(2, ArgConstReg(arch.reg_xcc)),
    ])
    fcc = ArgSwitch(fields.cc, [
        IsaMatch(x, ArgConstReg(y))
        for x, y in enumerate(arch.regs_fcc)
    ])
    fcc2 = ArgSwitch(fields.cc2, [
        IsaMatch(x, ArgConstReg(y))
        for x, y in enumerate(arch.regs_fcc)
    ])

    memrr = ArgMemRRS(arch.mem, 0, rs1, rs2, 1)
    memri = ArgMemRI(arch.mem, 0, rs1, simm13)
    memrrai = ArgMemRRS(arch.mem, 0, rs1, rs2, 1, seg=imm_asi)
    memriar = ArgMemRI(arch.mem, 0, rs1, simm13, seg=asi)
    memrai = ArgMem(arch.mem, 0, rs1, seg=imm_asi)
    memrar = ArgMem(arch.mem, 0, rs1, seg=asi)
    mem = ArgSwitch(fields.i, [
        IsaMatch(0, memrr),
        IsaMatch(1, memri),
    ])
    mema = ArgSwitch(fields.i, [
        IsaMatch(0, memrrai),
        IsaMatch(1, memriar),
    ])
    memra = ArgSwitch(fields.i, [
        IsaMatch(0, memrai),
        IsaMatch(1, memrar),
    ])

    ris2 = ArgSwitch(fields.i, [
        IsaMatch(0, rs2),
        IsaMatch(1, simm13),
    ])
    rish32 = ArgSwitch(fields.i, [
        IsaMatch(0, rs2),
        IsaMatch(1, shcnt32),
    ])
    rish64 = ArgSwitch(fields.i, [
        IsaMatch(0, rs2),
        IsaMatch(1, shcnt64),
    ])

    a = ModIf(fields.a, "a")
    p = ModSwitch(fields.p, [
        IsaMatch(0, Mod("pn")),
        IsaMatch(1, Mod("pt")),
    ])


def InsnBcc(name):
    return Insn(name, SparcArgs.disp22, mods=[SparcArgs.a])


def InsnBPcc(name):
    return Insn(name, SparcArgs.cc, SparcArgs.disp19,
                mods=[SparcArgs.a, SparcArgs.p])


def InsnBPfcc(name):
    return Insn(name, SparcArgs.fcc, SparcArgs.disp19,
                mods=[SparcArgs.a, SparcArgs.p])


def InsnBPr(name):
    return Insn(name, SparcArgs.rs1, SparcArgs.disp16,
                mods=[SparcArgs.a, SparcArgs.p])


def InsnRRR(name):
    return Insn(name, SparcArgs.rs1, SparcArgs.rs2, SparcArgs.rd)


def InsnRRIR(name):
    return Insn(name, SparcArgs.rs1, SparcArgs.ris2, SparcArgs.rd)


class SparcIsa(Isa):
    arch = SparcArch
    fields = SparcFields
    args = SparcArgs

    iconds = [
        "n",
        "e",
        "le",
        "l",
        "leu",
        "cs",
        "neg",
        "vs",
        "a",
        "ne",
        "g",
        "ge",
        "gu",
        "cc",
        "pos",
        "vc",
    ]
    fconds = [
        "n",
        "ne",
        "lg",
        "ul",
        "l",
        "ug",
        "g",
        "u",
        "a",
        "e",
        "ue",
        "ge",
        "uge",
        "le",
        "ule",
        "o",
    ]

    insn_0_1 = InsnSwitch(fields.cond, [
        IsaMatch(x, InsnBPcc("b" + y))
        for x, y in enumerate(iconds)
    ])
    insn_0_2 = InsnSwitch(fields.cond, [
        IsaMatch(x, InsnBcc("b" + y))
        for x, y in enumerate(iconds)
    ])
    insn_0_3 = InsnSwitch(fields.cond, [
        IsaMatch(0x1, InsnBPr("brz")),
        IsaMatch(0x2, InsnBPr("brlez")),
        IsaMatch(0x3, InsnBPr("brlz")),
        IsaMatch(0x5, InsnBPr("brnz")),
        IsaMatch(0x6, InsnBPr("brgz")),
        IsaMatch(0x7, InsnBPr("brgez")),
    ])
    insn_0_5 = InsnSwitch(fields.cond, [
        IsaMatch(x, InsnBPfcc("fb" + y))
        for x, y in enumerate(fconds)
    ])
    insn_0_6 = InsnSwitch(fields.cond, [
        IsaMatch(x, InsnBcc("fb" + y))
        for x, y in enumerate(fconds)
    ])
    insn_0 = InsnSwitch(fields.op2, [
        IsaMatch(0, Insn("illtrap", args.imm22)),
        IsaMatch(1, insn_0_1),
        IsaMatch(2, insn_0_2),
        IsaMatch(3, insn_0_3),
        IsaMatch(4, Insn("sethi", args.imm22, args.rd)),
        IsaMatch(5, insn_0_5),
        IsaMatch(6, insn_0_6),
        # XXX ccc branch [v8]
    ])

    insn_2_31 = InsnSwitch(fields.rd, [
        IsaMatch(0, Insn("saved")),
        IsaMatch(1, Insn("restored")),
        IsaMatch(2, Insn("allclean")),
        # XXX otherw, invalw, normalw
    ])

    insn_2_34 = InsnSwitch(fields.opf, [
        IsaMatch(0x001, Insn("fmovs", args.fs2, args.fd)),
        IsaMatch(0x002, Insn("fmovd", args.fs2d, args.fdd)),
        IsaMatch(0x003, Insn("fmovq", args.fs2q, args.fdq)),
        IsaMatch(0x005, Insn("fnegs", args.fs2, args.fd)),
        IsaMatch(0x006, Insn("fnegd", args.fs2d, args.fdd)),
        IsaMatch(0x007, Insn("fnegq", args.fs2q, args.fdq)),
        IsaMatch(0x009, Insn("fabss", args.fs2, args.fd)),
        IsaMatch(0x00a, Insn("fabsd", args.fs2d, args.fdd)),
        IsaMatch(0x00b, Insn("fabsq", args.fs2q, args.fdq)),

        IsaMatch(0x029, Insn("fsqrts", args.fs2, args.fd)),
        IsaMatch(0x02a, Insn("fsqrtd", args.fs2d, args.fdd)),
        IsaMatch(0x02b, Insn("fsqrtq", args.fs2q, args.fdq)),

        IsaMatch(0x041, Insn("fadds", args.fs1, args.fs2, args.fd)),
        IsaMatch(0x042, Insn("faddd", args.fs1d, args.fs2d, args.fdd)),
        IsaMatch(0x043, Insn("faddq", args.fs1q, args.fs2q, args.fdq)),
        IsaMatch(0x045, Insn("fsubs", args.fs1, args.fs2, args.fd)),
        IsaMatch(0x046, Insn("fsubd", args.fs1d, args.fs2d, args.fdd)),
        IsaMatch(0x047, Insn("fsubq", args.fs1q, args.fs2q, args.fdq)),
        IsaMatch(0x049, Insn("fmuls", args.fs1, args.fs2, args.fd)),
        IsaMatch(0x04a, Insn("fmuld", args.fs1d, args.fs2d, args.fdd)),
        IsaMatch(0x04b, Insn("fmulq", args.fs1q, args.fs2q, args.fdq)),
        IsaMatch(0x04d, Insn("fdivs", args.fs1, args.fs2, args.fd)),
        IsaMatch(0x04e, Insn("fdivd", args.fs1d, args.fs2d, args.fdd)),
        IsaMatch(0x04f, Insn("fdivq", args.fs1q, args.fs2q, args.fdq)),

        IsaMatch(0x069, Insn("fsmuld", args.fs1, args.fs2, args.fdd)),
        IsaMatch(0x06e, Insn("fdmulq", args.fs1d, args.fs2d, args.fdq)),

        IsaMatch(0x081, Insn("fstox", args.fs2, args.fdd)),
        IsaMatch(0x082, Insn("fdtox", args.fs2d, args.fdd)),
        IsaMatch(0x083, Insn("fqtox", args.fs2q, args.fdd)),
        IsaMatch(0x084, Insn("fxtos", args.fs2d, args.fd)),
        IsaMatch(0x088, Insn("fxtod", args.fs2d, args.fdd)),
        IsaMatch(0x08c, Insn("fxtoq", args.fs2d, args.fdq)),

        IsaMatch(0x0c4, Insn("fitos", args.fs2, args.fd)),
        IsaMatch(0x0c6, Insn("fdtos", args.fs2d, args.fd)),
        IsaMatch(0x0c7, Insn("fqtos", args.fs2q, args.fd)),
        IsaMatch(0x0c8, Insn("fitod", args.fs2, args.fdd)),
        IsaMatch(0x0c9, Insn("fstod", args.fs2, args.fdd)),
        IsaMatch(0x0cb, Insn("fqtod", args.fs2q, args.fdd)),
        IsaMatch(0x0cc, Insn("fitoq", args.fs2, args.fdq)),
        IsaMatch(0x0cd, Insn("fstoq", args.fs2, args.fdq)),
        IsaMatch(0x0ce, Insn("fdtoq", args.fs2d, args.fdq)),
        IsaMatch(0x0d1, Insn("fstoi", args.fs2, args.fd)),
        IsaMatch(0x0d2, Insn("fdtoi", args.fs2d, args.fd)),
        IsaMatch(0x0d3, Insn("fqtoi", args.fs2q, args.fd)),
    ])

    insn_2_35 = InsnSwitch(fields.opf, [
        IsaMatch(0x051, Insn("fcmps", args.fs1, args.fs2, args.fcc2)),
        IsaMatch(0x052, Insn("fcmpd", args.fs1d, args.fs2d, args.fcc2)),
        IsaMatch(0x053, Insn("fcmpq", args.fs1q, args.fs2q, args.fcc2)),
        IsaMatch(0x055, Insn("fcmpes", args.fs1, args.fs2, args.fcc2)),
        IsaMatch(0x056, Insn("fcmped", args.fs1d, args.fs2d, args.fcc2)),
        IsaMatch(0x057, Insn("fcmpeq", args.fs1q, args.fs2q, args.fcc2)),
        # XXX A.33, A.34
    ])

    # undefined by SPARC V9, VIS on UA2005
    insn_2_36 = InsnSwitch(fields.opf, [
        IsaMatch(0x000, InsnRRR("edge8cc")),
        IsaMatch(0x002, InsnRRR("edge8lcc")),
        IsaMatch(0x004, InsnRRR("edge16cc")),
        IsaMatch(0x006, InsnRRR("edge16lcc")),
        IsaMatch(0x008, InsnRRR("edge32cc")),
        IsaMatch(0x00a, InsnRRR("edge32lcc")),
        IsaMatch(0x010, InsnRRR("array8")),
        IsaMatch(0x012, InsnRRR("array16")),
        IsaMatch(0x014, InsnRRR("array32")),
        IsaMatch(0x018, InsnRRR("alignaddr")),
        IsaMatch(0x019, InsnRRR("bmask")),
        IsaMatch(0x01a, InsnRRR("alignaddrl")),

        IsaMatch(0x020, Insn("fcmple16", args.fs1d, args.fs2d, args.rd)),
        IsaMatch(0x022, Insn("fcmpne16", args.fs1d, args.fs2d, args.rd)),
        IsaMatch(0x024, Insn("fcmple32", args.fs1d, args.fs2d, args.rd)),
        IsaMatch(0x026, Insn("fcmpne32", args.fs1d, args.fs2d, args.rd)),
        IsaMatch(0x028, Insn("fcmpgt16", args.fs1d, args.fs2d, args.rd)),
        IsaMatch(0x02a, Insn("fcmpeq16", args.fs1d, args.fs2d, args.rd)),
        IsaMatch(0x02c, Insn("fcmpgt32", args.fs1d, args.fs2d, args.rd)),
        IsaMatch(0x02e, Insn("fcmpeq32", args.fs1d, args.fs2d, args.rd)),

        IsaMatch(0x048, Insn("faligndata", args.fs1d, args.fs2d, args.fdd)),
        IsaMatch(0x04c, Insn("bshuffle", args.fs1d, args.fs2d, args.fdd)),
        IsaMatch(0x04d, Insn("fexpand", args.fs2, args.fdd)),
    ])

    insn_2_3e = InsnSwitch(fields.rd, [
        IsaMatch(0, Insn("done")),
        IsaMatch(1, Insn("retry")),
    ])

    insn_2 = InsnSwitch(fields.op3, [
        IsaMatch(0x00, InsnRRIR("add")),
        IsaMatch(0x01, InsnRRIR("and")),
        IsaMatch(0x02, InsnRRIR("or")),
        IsaMatch(0x03, InsnRRIR("xor")),
        IsaMatch(0x04, InsnRRIR("sub")),
        IsaMatch(0x05, InsnRRIR("andn")),
        IsaMatch(0x06, InsnRRIR("orn")),
        IsaMatch(0x07, InsnRRIR("xnor")),
        IsaMatch(0x08, InsnRRIR("addc")),
        IsaMatch(0x09, InsnRRIR("mulx")),
        IsaMatch(0x0a, InsnRRIR("umul")),
        IsaMatch(0x0b, InsnRRIR("smul")),
        IsaMatch(0x0c, InsnRRIR("subc")),
        IsaMatch(0x0d, InsnRRIR("udivx")),
        IsaMatch(0x0e, InsnRRIR("udiv")),
        IsaMatch(0x0f, InsnRRIR("sdiv")),
        IsaMatch(0x10, InsnRRIR("addcc")),
        IsaMatch(0x11, InsnRRIR("andcc")),
        IsaMatch(0x12, InsnRRIR("orcc")),
        IsaMatch(0x13, InsnRRIR("xorcc")),
        IsaMatch(0x14, InsnRRIR("subcc")),
        IsaMatch(0x15, InsnRRIR("andncc")),
        IsaMatch(0x16, InsnRRIR("orncc")),
        IsaMatch(0x17, InsnRRIR("xnorcc")),
        IsaMatch(0x18, InsnRRIR("addccc")),
        # 19 unused
        IsaMatch(0x1a, InsnRRIR("umulcc")),
        IsaMatch(0x1b, InsnRRIR("smulcc")),
        IsaMatch(0x1c, InsnRRIR("subccc")),
        # 1d unused
        IsaMatch(0x1e, InsnRRIR("udivcc")),
        IsaMatch(0x1f, InsnRRIR("sdivcc")),
        IsaMatch(0x20, InsnRRIR("taddcc")),
        IsaMatch(0x21, InsnRRIR("tsubcc")),
        IsaMatch(0x22, InsnRRIR("taddcctv")),
        IsaMatch(0x23, InsnRRIR("tsubcctv")),
        IsaMatch(0x24, InsnRRIR("mulscc")),
        IsaMatch(0x25, InsnSwitch(fields.x, [
            IsaMatch(0, Insn("sll", args.rs1, args.rish32, args.rd)),
            IsaMatch(1, Insn("sllx", args.rs1, args.rish64, args.rd)),
        ])),
        IsaMatch(0x26, InsnSwitch(fields.x, [
            IsaMatch(0, Insn("srl", args.rs1, args.rish32, args.rd)),
            IsaMatch(1, Insn("srlx", args.rs1, args.rish64, args.rd)),
        ])),
        IsaMatch(0x27, InsnSwitch(fields.x, [
            IsaMatch(0, Insn("sra", args.rs1, args.rish32, args.rd)),
            IsaMatch(1, Insn("srax", args.rs1, args.rish64, args.rd)),
        ])),
        IsaMatch(0x28, InsnSwitch(fields.rs1, [
            # XXX A.44 rdasr
            IsaMatch(0x0f, InsnSwitch(fields.i, [
                IsaMatch(0, Insn("stbar")),
                IsaMatch(1, Insn("membar", args.membar)),
            ])),
        ])),
        # XXX 29 rdspr, rdhpr
        # XXX 2a A.43 rdpr, rdwim
        IsaMatch(0x2b, InsnSwitch(fields.i, [
            # XXX rdtbr
            IsaMatch(0, Insn("flushw")),
        ])),
        # XXX 2c A.35 mov<cc>
        IsaMatch(0x2d, InsnRRIR("sdivx")),
        IsaMatch(0x2e, InsnSwitch(fields.rs1, [
            IsaMatch(0x00, Insn("popc", args.ris2, args.rd)),
        ])),
        # XXX 2f A.36 movr<cc>
        IsaMatch(0x30, InsnSwitch(fields.rd, [
            # XXX A.63 wrasr
            IsaMatch(0x0f, InsnSwitch(fields.i, [
                IsaMatch(1, InsnSwitch(fields.rs1, [
                    IsaMatch(0x00, Insn("sir", args.simm13)),
                ])),
            ])),
        ])),
        IsaMatch(0x31, insn_2_31),
        # XXX 32 A.62 wrpr
        # 33 unused
        IsaMatch(0x34, insn_2_34),
        IsaMatch(0x35, insn_2_35),
        IsaMatch(0x36, insn_2_36),
        # 37 impdep2
        IsaMatch(0x38, Insn("jmpl", args.mem, args.rd)),
        IsaMatch(0x39, Insn("return", args.mem)),
        # XXX 3a A.61 t<cc>
        IsaMatch(0x3b, Insn("flush", args.mem)),
        IsaMatch(0x3c, InsnRRIR("save")),
        IsaMatch(0x3d, InsnRRIR("restore")),
        IsaMatch(0x3e, insn_2_3e),
        # 3f unused
    ])

    insn_3 = InsnSwitch(fields.op3, [
        IsaMatch(0x00, Insn("lduw", args.mem, args.rd)),
        IsaMatch(0x01, Insn("ldub", args.mem, args.rd)),
        IsaMatch(0x02, Insn("lduh", args.mem, args.rd)),
        IsaMatch(0x03, Insn("ldd", args.mem, args.rd)),
        IsaMatch(0x04, Insn("stw", args.rd, args.mem)),
        IsaMatch(0x05, Insn("stb", args.rd, args.mem)),
        IsaMatch(0x06, Insn("sth", args.rd, args.mem)),
        IsaMatch(0x07, Insn("std", args.rd, args.mem)),
        IsaMatch(0x08, Insn("ldsw", args.mem, args.rd)),
        IsaMatch(0x09, Insn("ldsb", args.mem, args.rd)),
        IsaMatch(0x0a, Insn("ldsh", args.mem, args.rd)),
        IsaMatch(0x0b, Insn("ldx", args.mem, args.rd)),
        # 0c unused
        IsaMatch(0x0d, Insn("ldstub", args.mem, args.rd)),
        IsaMatch(0x0e, Insn("stx", args.rd, args.mem)),
        IsaMatch(0x0f, Insn("swap", args.mem, args.rd)),
        IsaMatch(0x10, Insn("lduwa", args.mema, args.rd)),
        IsaMatch(0x11, Insn("lduba", args.mema, args.rd)),
        IsaMatch(0x12, Insn("lduha", args.mema, args.rd)),
        IsaMatch(0x13, Insn("ldda", args.mema, args.rd)),
        IsaMatch(0x14, Insn("stwa", args.rd, args.mema)),
        IsaMatch(0x15, Insn("stba", args.rd, args.mema)),
        IsaMatch(0x16, Insn("stha", args.rd, args.mema)),
        IsaMatch(0x17, Insn("stda", args.rd, args.mema)),
        IsaMatch(0x18, Insn("ldswa", args.mema, args.rd)),
        IsaMatch(0x19, Insn("ldsba", args.mema, args.rd)),
        IsaMatch(0x1a, Insn("ldsha", args.mema, args.rd)),
        IsaMatch(0x1b, Insn("ldxa", args.mema, args.rd)),
        # 1c unused
        IsaMatch(0x1d, Insn("ldstuba", args.mema, args.rd)),
        IsaMatch(0x1e, Insn("stxa", args.rd, args.mema)),
        IsaMatch(0x1f, Insn("swapa", args.mema, args.rd)),
        IsaMatch(0x20, Insn("ld", args.mem, args.fd)),
        IsaMatch(0x21, InsnSwitch(fields.rd, [
            IsaMatch(0, Insn("ld", args.mem, args.fsr)),
            IsaMatch(1, Insn("ldx", args.mem, args.fsr)),
        ])),
        IsaMatch(0x22, Insn("ldq", args.mem, args.fdq)),
        IsaMatch(0x23, Insn("ldd", args.mem, args.fdd)),
        IsaMatch(0x24, Insn("st", args.fd, args.mem)),
        IsaMatch(0x25, InsnSwitch(fields.rd, [
            IsaMatch(0, Insn("st", args.fsr, args.mem)),
            IsaMatch(1, Insn("stx", args.fsr, args.mem)),
        ])),
        IsaMatch(0x26, Insn("stq", args.fdq, args.mem)),
        IsaMatch(0x27, Insn("std", args.fdd, args.mem)),
        # 28-2c unused
        IsaMatch(0x2d, Insn("prefetch", args.mem, args.prefetch_fcn)),
        # 2e-2f unused
        IsaMatch(0x30, Insn("lda", args.mema, args.fd)),
        # 31 unused
        IsaMatch(0x32, Insn("ldqa", args.mema, args.fdq)),
        IsaMatch(0x33, Insn("ldda", args.mema, args.fdd)),
        IsaMatch(0x34, Insn("sta", args.fd, args.mema)),
        # 35 unused
        IsaMatch(0x36, Insn("stqa", args.fdq, args.mema)),
        IsaMatch(0x37, Insn("stda", args.fdd, args.mema)),
        # 38-3b unused
        IsaMatch(0x3c, Insn("casa", args.memra, args.rs2, args.rd)),
        IsaMatch(0x3d, Insn("prefetcha", args.mema, args.prefetch_fcn)),
        IsaMatch(0x3e, Insn("casxa", args.memra, args.rs2, args.rd)),
        # 3f unused
    ])

    insn_top = InsnSwitch(fields.op, [
        IsaMatch(0, insn_0),
        IsaMatch(1, Insn("call", args.disp30)),
        IsaMatch(2, insn_2),
        IsaMatch(3, insn_3),
    ])

    parser = [
        ParseWord(fields.iw, 'big'),
        ParseInsn(insn_top),
    ]
