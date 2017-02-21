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

# Somebody's out to get you
# Hiding in shadows - poison arrows

from ..core import Isa
from ..field import IsaField, IsaSubField, IsaMatch
from ..parser import ParseWord, ParseInsn, ParseSwitch
from ..insn import InsnSwitch, Insn
from ..arg import (ArgConst, ArgImm, ArgReg, ArgMem, ArgMemRRS, ArgMemRI,
                   ArgSwitch, ArgConstReg, ArgPCRel)
from ..reg import (Register, RegisterSP, RegisterPC, RegisterSpecial,
                   RegisterSplit, SubRegister)
from ..mem import MemSpace


class FalconArch:
    # The main register file.  16 32-bit registers.
    regs_r = [
        Register("r{}".format(x), 32)
        for x in range(16)
    ]
    # The low 16-bit halves of the registers.  Used by 16-bit operations.
    # If written, the high 16 bits are unmodified.
    regs_rh = [
        SubRegister("r{}h".format(x), reg, 0, 16)
        for x, reg in enumerate(regs_r)
    ]
    # The low 8-bit parts of the registers.  Used by 8-bit operations.
    # If written, the high 24 bits are unmodified.
    regs_rb = [
        SubRegister("r{}b".format(x), reg, 0, 8)
        for x, reg in enumerate(regs_r)
    ]

    # The 8 single-bit predicate registers (really part of the $flags
    # register).
    regs_pred = [
        Register("p{}".format(x), 1)
        for x in range(8)
    ]
    # The other bits of the $flags register (treated as separate registers
    # here).
    regs_flag = regs_pred + [
        # 8-11: operation result flags (carry, overflow, sign, zero).
        Register("ccc", 1),
        Register("cco", 1),
        Register("ccs", 1),
        Register("ccz", 1),
        # 12-15
        None, None, None, None,
        # 16-19: interrupt enables.
        RegisterSpecial("ie0", 1),
        RegisterSpecial("ie1", 1),
        # if somebody knows what #2 is for, please let me know.
        RegisterSpecial("ie2", 1),
        None,
        # 20-23: saved interrupt enables.
        RegisterSpecial("sie0", 1),
        RegisterSpecial("sie1", 1),
        RegisterSpecial("sie2", 1),
        None,
        # 24-25
        # Trap active flag (if you get a trap while this is set, it's
        # a double trap and you're toast).
        RegisterSpecial("ta", 1),
        None,
        # 26-31: NFI.
        RegisterSpecial("unk26", 1),
        RegisterSpecial("unk27", 1),
        RegisterSpecial("unk28", 1),
        RegisterSpecial("unk29", 1),
        RegisterSpecial("unk30", 1),
        RegisterSpecial("unk31", 1),
    ]
    # The assembled flags register.
    reg_flags = RegisterSplit("flags", 32, [
        (x, 1, reg)
        for x, reg in enumerate(regs_flag)
        if reg is not None
    ])

    # The stack pointer.  Not truly a 32-bit register, but let's pretend so.
    # In reality, the low 2 bits are forced to 0 (it's always word-aligned),
    # and only as many bits are implemented as necessary to cover the data RAM.
    reg_sp = RegisterSP("sp", 32)
    # The program counter.  Again, fewer bits may actually be implemented.
    reg_pc = RegisterPC("pc", 32)
    # The special registers file in all its glory.
    regs_special = [
        # The interrupt vectors.  Only as many bits implemented as in the PC.
        RegisterSpecial("iv0", 32),
        RegisterSpecial("iv1", 32),
        None,
        # The trap vector.  Likewise with implemented bits.
        RegisterSpecial("tv", 32),
        # SP & PC also accessible here.
        reg_sp,
        reg_pc,
        # The code and data transfer base registers (full 32-bit).
        RegisterSpecial("xcbase", 32),
        RegisterSpecial("xdbase", 32),
        # Good old flags.
        reg_flags,
        # The crypto transfer override register (really 8-bit?).
        RegisterSpecial("cx", 32),
        # The crypto auth mode register.  Here there be dragons.
        RegisterSpecial("cauth", 32),
        # The transfer ports register.  Only has 3×3-bit fields.
        RegisterSpecial("xports", 32),
        # Trap status register.  Also split into fields...
        RegisterSpecial("tstat", 32),
        None,
        None,
        None,
    ]

    # The data space.  32-bit addressing, 8-bit bytes.
    # Not exactly true on < v4, where only the low 12 or so bits actually
    # matter (depending on the data RAM size), and addresses might just
    # as well be treated as 16-bit, but let's ignore it for now.
    # On v4+ with the UAS option, all bits of a data address are important
    # (to match the UAS window address).
    mem_d = MemSpace("D", 8, 32)
    # The I/O register space.  32-bit addressing, 8-bit bytes.
    # Again not exactly true.  Only bits 2-17 matter on most Falcons (and
    # high 8 bits probably don't matter on any Falcon).
    mem_io = MemSpace("I", 8, 32)


class FalconFields:
    # Instruction word A - the main opcode.
    a = IsaField(8)
    # Size - 0 is byte (8-bit), 1 is halfword (16-bit), 2 is word (32-bit),
    # 3 is misc instructions.
    asz = IsaSubField(a, 6, 2)
    # First part of opcode - 0-2 select a form directly, 3 needs aopb to
    # select a form.
    aopa = IsaSubField(a, 4, 2)
    # Second part of opcode.  Selects form (if aopa is 3), or instruction
    # within a form (if aopa != 3).
    aopb = IsaSubField(a, 0, 4)

    # Instruction word B - present on all instructions on <= v3, v4+ started
    # introducing new forms without this.
    b = IsaField(8)
    # Argument 1 - a 4-bit field selecting the first register argument.
    arg1 = IsaSubField(b, 4, 4)
    # Argument 2 - a 4-bit field selecting the second register argument, or
    # an instruction within a form (for single-register forms).
    arg2 = IsaSubField(b, 0, 4)
    # Immediate op - selects an instruction within the immediate-only forms.
    # The top 2 bits of this byte are ignored in this case.
    iop = IsaSubField(b, 0, 6)
    # Predicate register field for the branch true / branch false instructions
    # (is really a part of iop, we just choose to print it nicely).
    iopp = IsaSubField(iop, 0, 3)

    # Instruction word C - present on two-register and three-register <= v3
    # forms.
    c = IsaField(8)
    # Argument 3 - a 4-bit field selecting the third register argument.
    arg3 = IsaSubField(c, 4, 4)
    # Argument 4 - selects the instruction within a form.
    arg4 = IsaSubField(c, 0, 4)

    # Instruction word immediate 8-bit.
    i8 = IsaField(8)
    # Low 5 bits of the immediate - we use this when the immediate is used
    # to select a $flags bit.
    i8f = IsaSubField(i8, 0, 5)

    # Instruction word immediate 16-bit.
    i16 = IsaField(16)
    # Low 8 bits of the immediate - for 8-bit instructions using 16-bit
    # immediate form for some strange reason.
    i16t8 = IsaSubField(i16, 0, 8)

    # Instruction word immediate 24-bit.
    i24 = IsaField(24)


class FalconArgs:
    arch = FalconArch
    fields = FalconFields

    # Immediate arguments.

    # The simple 8-bit immediate (used for shift amounts, bit positions,
    # and 8-bit instructions).
    i8 = ArgImm(8, fields.i8)
    # The 8-bit immediate extended to 16 bits (used for 16-bit instructions
    # and sethi).
    i8zx16 = ArgImm(16, fields.i8)
    i8sx16 = ArgImm(16, fields.i8, signed=True)
    # The 8-bit immediate extended to 32 bits (used for most 32-bit
    # instructions, and memory addressing).
    i8zx32 = ArgImm(32, fields.i8)
    i8sx32 = ArgImm(32, fields.i8, signed=True)
    # The 8-bit immediate extended to 32 bits and shifted left by 1 or 2
    # (used for memory addressing).
    i8zx32s1 = ArgImm(32, fields.i8, shift=1)
    i8zx32s2 = ArgImm(32, fields.i8, shift=2)
    # These select the proper 8-bit immediate based on instruction size
    # field.
    i8zxs = ArgSwitch(fields.asz, [
        IsaMatch(0, i8),
        IsaMatch(1, i8zx16),
        IsaMatch(2, i8zx32),
    ])
    i8sxs = ArgSwitch(fields.asz, [
        IsaMatch(0, i8),
        IsaMatch(1, i8sx16),
        IsaMatch(2, i8sx32),
    ])

    # The 16-bit immediate truncated to 8 bits (used by 8-bit instructions if
    # the 16-bit immediate form is selected for some weirdo reason).
    i16t8 = ArgImm(8, fields.i16t8)
    # The simple 16-bit immediate (used for 16-bit instructions and sethi).
    i16 = ArgImm(16, fields.i16)
    # The 16-bit immediate extended to 32 bits (used for most 32-bit
    # instructions).
    i16zx32 = ArgImm(32, fields.i16)
    i16sx32 = ArgImm(32, fields.i16, signed=True)
    # These select the proper 16-bit immediate based on instruction size
    # field.
    i16zxs = ArgSwitch(fields.asz, [
        IsaMatch(0, i16t8),
        IsaMatch(1, i16),
        IsaMatch(2, i16zx32),
    ])
    i16sxs = ArgSwitch(fields.asz, [
        IsaMatch(0, i16t8),
        IsaMatch(1, i16),
        IsaMatch(2, i16sx32),
    ])

    # The 24-bit immediate extended to 32 bits (used for v4 jumps/calls).
    i24zx32 = ArgImm(32, fields.i24)

    # The PC-relative fields (for branches).
    pc8 = ArgPCRel(32, fields.i8)
    pc16 = ArgPCRel(32, fields.i16)

    # Register arguments.

    # 32-bit registers as selected by the fields.arg* fields.
    r1 = ArgReg(fields.arg1, arch.regs_r)
    r2 = ArgReg(fields.arg2, arch.regs_r)
    r3 = ArgReg(fields.arg3, arch.regs_r)
    # 16-bit register low halves, likewise.
    r1h = ArgReg(fields.arg1, arch.regs_rh)
    r2h = ArgReg(fields.arg2, arch.regs_rh)
    r3h = ArgReg(fields.arg3, arch.regs_rh)
    # 8-bit register low parts, likewise.
    r1b = ArgReg(fields.arg1, arch.regs_rb)
    r2b = ArgReg(fields.arg2, arch.regs_rb)
    r3b = ArgReg(fields.arg3, arch.regs_rb)
    # Select 8/16/32-bit registers according to instruction size field.
    r1s = ArgSwitch(fields.asz, [
        IsaMatch(0, r1b),
        IsaMatch(1, r1h),
        IsaMatch(2, r1),
    ])
    r2s = ArgSwitch(fields.asz, [
        IsaMatch(0, r2b),
        IsaMatch(1, r2h),
        IsaMatch(2, r2),
    ])
    r3s = ArgSwitch(fields.asz, [
        IsaMatch(0, r3b),
        IsaMatch(1, r3h),
        IsaMatch(2, r3),
    ])

    # The $sp register (for use in memory addressing).
    sp = ArgConstReg(arch.reg_sp)
    # The predicate registers, for use in bt/bf branch instructions.
    # This selection is really treated as part of the opcode field by
    # the encoding schame, but we pretty-print it as a register name.
    pred = ArgReg(fields.iopp, arch.regs_pred)
    # The flag "registers", for use in single-flag manipulation instructions.
    # This is really treated as an 8-bit immediate by the hardware, but we
    # pretty-print it as a register name.
    flag = ArgReg(fields.i8f, arch.regs_flag)

    # The special registers as selected by the fields.arg* fields.
    sr1 = ArgReg(fields.arg1, arch.regs_special)
    sr2 = ArgReg(fields.arg2, arch.regs_special)

    # Memory arguments.

    # D[$rX]
    memr8 = ArgMem(arch.mem_d, 8, r1)
    memr16 = ArgMem(arch.mem_d, 16, r1)
    memr32 = ArgMem(arch.mem_d, 32, r1)
    memr = ArgSwitch(fields.asz, [
        IsaMatch(0, memr8),
        IsaMatch(1, memr16),
        IsaMatch(2, memr32),
    ])
    # D[$rX + imm]
    memri8 = ArgMemRI(arch.mem_d, 8, r1, i8zx32)
    memri16 = ArgMemRI(arch.mem_d, 16, r1, i8zx32s1)
    memri32 = ArgMemRI(arch.mem_d, 32, r1, i8zx32s2)
    memri = ArgSwitch(fields.asz, [
        IsaMatch(0, memri8),
        IsaMatch(1, memri16),
        IsaMatch(2, memri32),
    ])
    # D[$sp + imm]
    memspi8 = ArgMemRI(arch.mem_d, 8, sp, i8zx32)
    memspi16 = ArgMemRI(arch.mem_d, 16, sp, i8zx32s1)
    memspi32 = ArgMemRI(arch.mem_d, 32, sp, i8zx32s2)
    memspi = ArgSwitch(fields.asz, [
        IsaMatch(0, memspi8),
        IsaMatch(1, memspi16),
        IsaMatch(2, memspi32),
    ])
    # D[$sp + $rX * scale]
    memspr8 = ArgMemRRS(arch.mem_d, 8, sp, r2, 1)
    memspr16 = ArgMemRRS(arch.mem_d, 16, sp, r2, 2)
    memspr32 = ArgMemRRS(arch.mem_d, 32, sp, r2, 4)
    memspr = ArgSwitch(fields.asz, [
        IsaMatch(0, memspr8),
        IsaMatch(1, memspr16),
        IsaMatch(2, memspr32),
    ])
    # D[$rX + $rY * scale]
    memrr8 = ArgMemRRS(arch.mem_d, 8, r1, r2, 1)
    memrr16 = ArgMemRRS(arch.mem_d, 16, r1, r2, 2)
    memrr32 = ArgMemRRS(arch.mem_d, 32, r1, r2, 4)
    memrr = ArgSwitch(fields.asz, [
        IsaMatch(0, memrr8),
        IsaMatch(1, memrr16),
        IsaMatch(2, memrr32),
    ])

    # I[$rX]
    ior = ArgMem(arch.mem_io, 32, r1)
    # I[$rX + $rY * 4]
    iorr = ArgMemRRS(arch.mem_io, 32, r1, r2, 4)
    # I[$rX + imm]
    iori = ArgMemRI(arch.mem_io, 32, r1, i8zx32s2)


class FalconIsa(Isa):
    arch = FalconArch
    fields = FalconFields
    args = FalconArgs

    # Instruction forms.

    # The forms are named according to the kind of their arguments, in order:
    #
    # - s: sized form - means the high 2 bits of opcode select operation size
    #   (8-bit, 16-bit, or 32-bit).
    # - r: a read-only register argument
    # - w: a write-only register argument
    # - m: a read-modify-write register argument
    # - i8: an 8-bit immediate argument
    # - i16: a 16-bit immediate argument
    # - i24: a 24-bit immediate argument
    # - n: no arguments at all
    #
    # Note that Falcon encodes write-only (destination) registers in the last
    # opcode field, while we display the destination register first in
    # disassembly.  This is why the register arguments all seem out of order
    # here.  If we displayed destination last, almost all instructions would
    # have their register arguments in order...
    #
    # Also note that the forms are not always strictly followed - if a form
    # has an "m" register, some instructions in the form may actually treat
    # it as read-only, or as write-only.

    # Sized R, R, I8 - store reg to D[reg+imm].
    form_srri8 = InsnSwitch(fields.aopb, [
        IsaMatch(0, Insn("st", args.memri, args.r2s)),
    ])
    # Sized R, W, I8 - three-address binary ops with immediate.
    form_srwi8 = InsnSwitch(fields.aopb, [
        IsaMatch(0, Insn("add", args.r2s, args.r1s, args.i8zxs)),
        IsaMatch(1, Insn("adc", args.r2s, args.r1s, args.i8zxs)),
        IsaMatch(2, Insn("sub", args.r2s, args.r1s, args.i8zxs)),
        IsaMatch(3, Insn("sbb", args.r2s, args.r1s, args.i8zxs)),
        IsaMatch(4, Insn("shl", args.r2s, args.r1s, args.i8)),
        IsaMatch(5, Insn("shr", args.r2s, args.r1s, args.i8)),
        IsaMatch(7, Insn("sar", args.r2s, args.r1s, args.i8)),
        IsaMatch(8, Insn("ld", args.r2s, args.memri)),
        IsaMatch(0xc, Insn("shlc", args.r2s, args.r1s, args.i8)),
        IsaMatch(0xd, Insn("shrc", args.r2s, args.r1s, args.i8)),
    ])
    # Sized R, W, I16 - three-address binary ops with immediate.
    form_srwi16 = InsnSwitch(fields.aopb, [
        IsaMatch(0, Insn("add", args.r2s, args.r1s, args.i16zxs)),
        IsaMatch(1, Insn("adc", args.r2s, args.r1s, args.i16zxs)),
        IsaMatch(2, Insn("sub", args.r2s, args.r1s, args.i16zxs)),
        IsaMatch(3, Insn("sbb", args.r2s, args.r1s, args.i16zxs)),
    ])
    # Sized R, I8 - store reg to D[$sp+imm] and compares with immediate.
    form_sri8 = InsnSwitch(fields.arg2, [
        IsaMatch(1, Insn("st", args.memspi, args.r1s)),
        IsaMatch(4, Insn("cmpu", args.r1s, args.i8zxs)),
        IsaMatch(5, Insn("cmps", args.r1s, args.i8sxs)),
        IsaMatch(6, Insn("cmp", args.r1s, args.i8sxs)),
    ])
    # Sized R, I16 - compares with immediate.
    form_sri16 = InsnSwitch(fields.arg2, [
        IsaMatch(4, Insn("cmpu", args.r1s, args.i16zxs)),
        IsaMatch(5, Insn("cmps", args.r1s, args.i16sxs)),
        IsaMatch(6, Insn("cmp", args.r1s, args.i16sxs)),
    ])
    # Sized W, I8 - load from D[$sp+imm].
    form_swi8 = InsnSwitch(fields.arg2, [
        IsaMatch(0, Insn("ld", args.r1s, args.memspi)),
    ])
    # Sized RW, I8 - two-address binary ops wih immediate.
    form_smi8 = InsnSwitch(fields.arg2, [
        IsaMatch(0, Insn("add", args.r1s, args.i8zxs)),
        IsaMatch(1, Insn("adc", args.r1s, args.i8zxs)),
        IsaMatch(2, Insn("sub", args.r1s, args.i8zxs)),
        IsaMatch(3, Insn("sbb", args.r1s, args.i8zxs)),
        IsaMatch(4, Insn("shl", args.r1s, args.i8)),
        IsaMatch(5, Insn("shr", args.r1s, args.i8)),
        IsaMatch(7, Insn("sar", args.r1s, args.i8)),
        IsaMatch(0xc, Insn("shlc", args.r1s, args.i8)),
        IsaMatch(0xd, Insn("shrc", args.r1s, args.i8)),
    ])
    # Sized RW, I16 - two-address binary ops wih immediate.
    form_smi16 = InsnSwitch(fields.arg2, [
        IsaMatch(0, Insn("add", args.r1s, args.i16zxs)),
        IsaMatch(1, Insn("adc", args.r1s, args.i16zxs)),
        IsaMatch(2, Insn("sub", args.r1s, args.i16zxs)),
        IsaMatch(3, Insn("sbb", args.r1s, args.i16zxs)),
    ])
    # Sized R, R - stores and compares.
    form_srr = InsnSwitch(fields.arg4, [
        IsaMatch(0, Insn("st", args.memr, args.r2s)),
        IsaMatch(1, Insn("st", args.memspr, args.r1s)),
        IsaMatch(4, Insn("cmpu", args.r1s, args.r2s)),
        IsaMatch(5, Insn("cmps", args.r1s, args.r2s)),
        IsaMatch(6, Insn("cmp", args.r1s, args.r2s)),
    ])
    # Sized R, W - three-address unary ops.
    # Ain't compiler terminology confusing?
    form_srw = InsnSwitch(fields.arg4, [
        IsaMatch(0, Insn("not", args.r2s, args.r1s)),
        IsaMatch(1, Insn("neg", args.r2s, args.r1s)),
        IsaMatch(2, Insn("mov", args.r2s, args.r1s)),
        IsaMatch(3, Insn("hswap", args.r2s, args.r1s)),
    ])
    # Sized W, R [!] - a funny load form.
    form_swr = InsnSwitch(fields.arg4, [
        IsaMatch(0, Insn("ld", args.r1s, args.memspr)),
    ])
    # Sized RW, R - Two-argument binary ops.
    form_smr = InsnSwitch(fields.arg4, [
        IsaMatch(0, Insn("add", args.r1s, args.r2s)),
        IsaMatch(1, Insn("adc", args.r1s, args.r2s)),
        IsaMatch(2, Insn("sub", args.r1s, args.r2s)),
        IsaMatch(3, Insn("sbb", args.r1s, args.r2s)),
        IsaMatch(4, Insn("shl", args.r1s, args.r2b)),
        IsaMatch(5, Insn("shr", args.r1s, args.r2b)),
        IsaMatch(7, Insn("sar", args.r1s, args.r2b)),
        IsaMatch(0xc, Insn("shlc", args.r1s, args.r2b)),
        IsaMatch(0xd, Insn("shrc", args.r1s, args.r2b)),
    ])
    # Sized RW, R - Three-argument binary ops.
    form_srrw = InsnSwitch(fields.arg4, [
        IsaMatch(0, Insn("add", args.r3s, args.r1s, args.r2s)),
        IsaMatch(1, Insn("adc", args.r3s, args.r1s, args.r2s)),
        IsaMatch(2, Insn("sub", args.r3s, args.r1s, args.r2s)),
        IsaMatch(3, Insn("sbb", args.r3s, args.r1s, args.r2s)),
        IsaMatch(4, Insn("shl", args.r3s, args.r1s, args.r2b)),
        IsaMatch(5, Insn("shr", args.r3s, args.r1s, args.r2b)),
        IsaMatch(7, Insn("sar", args.r3s, args.r1s, args.r2b)),
        IsaMatch(8, Insn("ld", args.r3s, args.memrr)),
        IsaMatch(0xc, Insn("shlc", args.r3s, args.r1s, args.r2b)),
        IsaMatch(0xd, Insn("shrc", args.r3s, args.r1s, args.r2b)),
    ])
    # Sized RW - two-address unary ops + misc.
    form_sm = InsnSwitch(fields.arg2, [
        IsaMatch(0, Insn("not", args.r1s)),
        IsaMatch(1, Insn("neg", args.r1s)),
        IsaMatch(2, Insn("mov", args.r1s)),
        IsaMatch(3, Insn("hswap", args.r1s)),
        IsaMatch(4, Insn("clr", args.r1s)),
        IsaMatch(5, Insn("tst", args.r1s)),
    ])

    # I24 - v4 long jumps and calls.  Not a sized form, even though the opcode
    # is in the low range.
    form_i24 = InsnSwitch(fields.asz, [
        IsaMatch(0, Insn("jmp", args.i24zx32)),
        IsaMatch(1, Insn("call", args.i24zx32)),
    ])

    # R, W, I8 - three-address binary ops with immediate.
    form_rwi8 = InsnSwitch(fields.aopb, [
        IsaMatch(0, Insn("mulu", args.r2, args.r1, args.i8zx32)),
        IsaMatch(1, Insn("muls", args.r2, args.r1, args.i8sx32)),
        IsaMatch(2, Insn("sext", args.r2, args.r1, args.i8)),
        IsaMatch(3, Insn("extrs", args.r2, args.r1, args.i8zx32)),
        IsaMatch(4, Insn("and", args.r2, args.r1, args.i8zx32)),
        IsaMatch(5, Insn("or", args.r2, args.r1, args.i8zx32)),
        IsaMatch(6, Insn("xor", args.r2, args.r1, args.i8zx32)),
        IsaMatch(7, Insn("extr", args.r2, args.r1, args.i8zx32)),
        IsaMatch(8, Insn("xbit", args.r2, args.r1, args.i8)),
        IsaMatch(0xb, Insn("ins", args.r2, args.r1, args.i8zx32)),
        IsaMatch(0xc, Insn("div", args.r2, args.r1, args.i8zx32)),
        IsaMatch(0xd, Insn("mod", args.r2, args.r1, args.i8zx32)),
        IsaMatch(0xe, Insn("iords", args.r2, args.iori)),
        IsaMatch(0xf, Insn("iord", args.r2, args.iori)),
    ])
    # R, R, I8 - I/O writes to I[reg+imm]
    form_rri8 = InsnSwitch(fields.aopb, [
        IsaMatch(0, Insn("iowr", args.iori, args.r2)),
        IsaMatch(1, Insn("iowrs", args.iori, args.r2)),
    ])
    # R, W, I16 - three-address binary ops with immediate.
    form_rwi16 = InsnSwitch(fields.aopb, [
        IsaMatch(0, Insn("mulu", args.r2, args.r1, args.i16zx32)),
        IsaMatch(1, Insn("muls", args.r2, args.r1, args.i16sx32)),
        IsaMatch(3, Insn("extrs", args.r2, args.r1, args.i16zx32)),
        IsaMatch(4, Insn("and", args.r2, args.r1, args.i16zx32)),
        IsaMatch(5, Insn("or", args.r2, args.r1, args.i16zx32)),
        IsaMatch(6, Insn("xor", args.r2, args.r1, args.i16zx32)),
        IsaMatch(7, Insn("extr", args.r2, args.r1, args.i16zx32)),
        IsaMatch(0xb, Insn("ins", args.r2, args.r1, args.i16zx32)),
        IsaMatch(0xc, Insn("div", args.r2, args.r1, args.i16zx32)),
        IsaMatch(0xd, Insn("mod", args.r2, args.r1, args.i16zx32)),
    ])
    # RW, I8 - two-address binary ops with immediate + mov/sethi.
    form_mi8 = InsnSwitch(fields.arg2, [
        IsaMatch(0, Insn("mulu", args.r1, args.i8zx32)),
        IsaMatch(1, Insn("muls", args.r1, args.i8sx32)),
        IsaMatch(2, Insn("sext", args.r1, args.i8)),
        IsaMatch(3, Insn("sethi", args.r1, args.i8zx16)),
        IsaMatch(4, Insn("and", args.r1, args.i8zx32)),
        IsaMatch(5, Insn("or", args.r1, args.i8zx32)),
        IsaMatch(6, Insn("xor", args.r1, args.i8zx32)),
        IsaMatch(7, Insn("mov", args.r1, args.i8sx32)),
        IsaMatch(9, Insn("setb", args.r1, args.i8)),
        IsaMatch(0xa, Insn("clrb", args.r1, args.i8)),
        IsaMatch(0xb, Insn("tglb", args.r1, args.i8)),
        IsaMatch(0xc, Insn("getf", args.r1, args.flag)),
    ])
    # RW, I16 - two-address binary ops with immediate + mov/sethi.
    form_mi16 = InsnSwitch(fields.arg2, [
        IsaMatch(0, Insn("mulu", args.r1, args.i16zx32)),
        IsaMatch(1, Insn("muls", args.r1, args.i16sx32)),
        IsaMatch(3, Insn("sethi", args.r1, args.i16)),
        IsaMatch(4, Insn("and", args.r1, args.i16zx32)),
        IsaMatch(5, Insn("or", args.r1, args.i16zx32)),
        IsaMatch(6, Insn("xor", args.r1, args.i16zx32)),
        IsaMatch(7, Insn("mov", args.r1, args.i16sx32)),
    ])
    # R, I8 - some misc weirdos.
    form_ri8 = InsnSwitch(fields.arg2, [
        IsaMatch(0x8, Insn("putf", args.flag, args.r1)),
        IsaMatch(0xc, Insn("cc", args.i8, args.r1)),
    ])
    # I8 - branches and other immediate-only insns.
    form_i8 = InsnSwitch(fields.iop, [
        IsaMatch((0x00, 0x38), Insn("bt", args.pred, args.pc8)),
        IsaMatch(0x08, Insn("bc", args.pc8)),
        IsaMatch(0x09, Insn("bo", args.pc8)),
        IsaMatch(0x0a, Insn("bs", args.pc8)),
        IsaMatch(0x0b, Insn("bz", args.pc8)),
        IsaMatch(0x0c, Insn("ba", args.pc8)),
        IsaMatch(0x0d, Insn("bna", args.pc8)),
        IsaMatch(0x0e, Insn("bra", args.pc8)),
        IsaMatch((0x10, 0x38), Insn("bf", args.pred, args.pc8)),
        IsaMatch(0x18, Insn("bnc", args.pc8)),
        IsaMatch(0x19, Insn("bno", args.pc8)),
        IsaMatch(0x1a, Insn("bns", args.pc8)),
        IsaMatch(0x1b, Insn("bnz", args.pc8)),
        IsaMatch(0x1c, Insn("bg", args.pc8)),
        IsaMatch(0x1d, Insn("ble", args.pc8)),
        IsaMatch(0x1e, Insn("bl", args.pc8)),
        IsaMatch(0x1f, Insn("bge", args.pc8)),
        IsaMatch(0x20, Insn("jmp", args.i8zx32)),
        IsaMatch(0x21, Insn("call", args.i8zx32)),
        IsaMatch(0x28, Insn("sleep", args.flag)),
        IsaMatch(0x30, Insn("addsp", args.i8sx32)),
        IsaMatch(0x31, Insn("setf", args.flag)),
        IsaMatch(0x32, Insn("clrf", args.flag)),
        IsaMatch(0x33, Insn("tglf", args.flag)),
        IsaMatch(0x3c, Insn("cc", args.i8zx16)),
    ])
    # I16 - branches and other immediate-only insns.
    form_i16 = InsnSwitch(fields.iop, [
        IsaMatch((0x00, 0x38), Insn("bt", args.pred, args.pc16)),
        IsaMatch(0x08, Insn("bc", args.pc16)),
        IsaMatch(0x09, Insn("bo", args.pc16)),
        IsaMatch(0x0a, Insn("bs", args.pc16)),
        IsaMatch(0x0b, Insn("bz", args.pc16)),
        IsaMatch(0x0c, Insn("ba", args.pc16)),
        IsaMatch(0x0d, Insn("bna", args.pc16)),
        IsaMatch(0x0e, Insn("bra", args.pc16)),
        IsaMatch((0x10, 0x38), Insn("bf", args.pred, args.pc16)),
        IsaMatch(0x18, Insn("bnc", args.pc16)),
        IsaMatch(0x19, Insn("bno", args.pc16)),
        IsaMatch(0x1a, Insn("bns", args.pc16)),
        IsaMatch(0x1b, Insn("bnz", args.pc16)),
        IsaMatch(0x1c, Insn("bg", args.pc16)),
        IsaMatch(0x1d, Insn("ble", args.pc16)),
        IsaMatch(0x1e, Insn("bl", args.pc16)),
        IsaMatch(0x1f, Insn("bge", args.pc16)),
        IsaMatch(0x20, Insn("jmp", args.i16zx32)),
        IsaMatch(0x21, Insn("call", args.i16zx32)),
        IsaMatch(0x30, Insn("addsp", args.i16sx32)),
        IsaMatch(0x3c, Insn("cc", args.i16)),
    ])
    # No arguments - funny control instructions.
    form_n = InsnSwitch(fields.arg2, [
        IsaMatch(0, Insn("ret")),
        IsaMatch(1, Insn("iret")),
        IsaMatch(2, Insn("halt")),
        IsaMatch(3, Insn("xdwait")),
        IsaMatch(6, Insn("xdbar")),
        IsaMatch(7, Insn("xcwait")),
        IsaMatch(8, Insn("trap", ArgConst(2, 0))),
        IsaMatch(9, Insn("trap", ArgConst(2, 1))),
        IsaMatch(0xa, Insn("trap", ArgConst(2, 2))),
        IsaMatch(0xb, Insn("trap", ArgConst(2, 3))),
    ])
    # R - more funny control instructions.
    form_r = InsnSwitch(fields.arg2, [
        IsaMatch(0, Insn("push", args.r1)),
        IsaMatch(1, Insn("addsp", args.r1)),
        IsaMatch(4, Insn("jmp", args.r1)),
        IsaMatch(5, Insn("call", args.r1)),
        IsaMatch(8, Insn("itlb", args.r1)),
        IsaMatch(9, Insn("setf", args.r1)),
        IsaMatch(0xa, Insn("clrf", args.r1)),
        IsaMatch(0xb, Insn("tglf", args.r1)),
    ])
    # RR - yet more funny control instructions.
    form_rr = InsnSwitch(fields.arg4, [
        IsaMatch(0x0, Insn("iowr", args.ior, args.r2)),
        IsaMatch(0x1, Insn("iowrs", args.ior, args.r2)),
        IsaMatch(0x4, Insn("xcld", args.r1, args.r2)),
        IsaMatch(0x5, Insn("xdld", args.r1, args.r2)),
        IsaMatch(0x6, Insn("xdst", args.r1, args.r2)),
        IsaMatch(0x8, Insn("putf", args.r2, args.r1)),
    ])
    # W - even more funny control instructions.
    form_w = InsnSwitch(fields.arg2, [
        IsaMatch(0, Insn("pop", args.r1)),
    ])
    # RW, R - two-address binary ops.
    form_mr = InsnSwitch(fields.arg4, [
        IsaMatch(0, Insn("mulu", args.r1, args.r2)),
        IsaMatch(1, Insn("muls", args.r1, args.r2)),
        IsaMatch(2, Insn("sext", args.r1, args.r2)),
        IsaMatch(4, Insn("and", args.r1, args.r2)),
        IsaMatch(5, Insn("or", args.r1, args.r2)),
        IsaMatch(6, Insn("xor", args.r1, args.r2)),
        IsaMatch(9, Insn("setb", args.r1, args.r2)),
        IsaMatch(0xa, Insn("clrb", args.r1, args.r2)),
        IsaMatch(0xb, Insn("tglb", args.r1, args.r2)),
    ])
    # R, W - SR movs, TLB queries, and other weird things.
    form_rw = InsnSwitch(fields.arg4, [
        IsaMatch(0x0, Insn("mov", args.sr2, args.r1)),
        IsaMatch(0x1, Insn("mov", args.r2, args.sr1)),
        IsaMatch(0x2, Insn("ptlb", args.r2, args.r1)),
        IsaMatch(0x3, Insn("vtlb", args.r2, args.r1)),
        IsaMatch(0xc, Insn("getf", args.r2, args.r1)),
    ])
    # R, R, W - three-address binary ops.
    form_rrw = InsnSwitch(fields.arg4, [
        IsaMatch(0, Insn("mulu", args.r3, args.r1, args.r2)),
        IsaMatch(1, Insn("muls", args.r3, args.r1, args.r2)),
        IsaMatch(2, Insn("sext", args.r3, args.r1, args.r2)),
        IsaMatch(3, Insn("extrs", args.r3, args.r1, args.r2)),
        IsaMatch(4, Insn("and", args.r3, args.r1, args.r2)),
        IsaMatch(5, Insn("or", args.r3, args.r1, args.r2)),
        IsaMatch(6, Insn("xor", args.r3, args.r1, args.r2)),
        IsaMatch(7, Insn("extr", args.r3, args.r1, args.r2)),
        IsaMatch(8, Insn("xbit", args.r3, args.r1, args.r2)),
        IsaMatch(0xc, Insn("div", args.r3, args.r1, args.r2)),
        IsaMatch(0xd, Insn("mod", args.r3, args.r1, args.r2)),
        IsaMatch(0xe, Insn("iords", args.r3, args.iorr)),
        IsaMatch(0xf, Insn("iord", args.r3, args.iorr)),
    ])

    # The parser.

    # The default arguments in the following helper functions are a hack needed
    # to use the instruction words inside the function - as class-level
    # variables/fields, they are otherwise inaccessible until the class is
    # constructed, and we need these functions to construct the parser.

    def parse_ab(form):
        """
        Generates the remainder of a parser for two-byte forms (a, b).
        """
        fields = FalconFields
        return [
            ParseWord(fields.b),
            ParseInsn(form),
        ]

    def parse_abc(form):
        """
        Generates the remainder of a parser for three-byte forms (a, b, c).
        """
        fields = FalconFields
        return [
            ParseWord(fields.b),
            ParseWord(fields.c),
            ParseInsn(form),
        ]

    def parse_abi8(form):
        """
        Generates the remainder of a parser for i8 forms (a, b, i8).
        """
        fields = FalconFields
        return [
            ParseWord(fields.b),
            ParseWord(fields.i8),
            ParseInsn(form),
        ]

    def parse_abi16(form):
        """
        Generates the remainder of a parser for i16 forms (a, b, i16).
        """
        fields = FalconFields
        return [
            ParseWord(fields.b),
            ParseWord(fields.i16, 'little'),
            ParseInsn(form),
        ]

    def parse_ai24(form):
        """
        Generates the remainder of a parser for the i24 form (a, i24).
        """
        fields = FalconFields
        return [
            ParseWord(fields.i24, 'little'),
            ParseInsn(form),
        ]

    # The parse tree for ops 0x00-0xbf.
    parser_s = ParseSwitch(fields.aopa, [
        IsaMatch(0, parse_abi8(form_srri8)),
        IsaMatch(1, parse_abi8(form_srwi8)),
        IsaMatch(2, parse_abi16(form_srwi16)),
        IsaMatch(3, ParseSwitch(fields.aopb, [
            IsaMatch(0, parse_abi8(form_sri8)),
            IsaMatch(1, parse_abi16(form_sri16)),
            IsaMatch(4, parse_abi8(form_swi8)),
            IsaMatch(6, parse_abi8(form_smi8)),
            IsaMatch(7, parse_abi16(form_smi16)),
            IsaMatch(8, parse_abc(form_srr)),
            IsaMatch(9, parse_abc(form_srw)),
            IsaMatch(0xa, parse_abc(form_swr)),
            IsaMatch(0xb, parse_abc(form_smr)),
            IsaMatch(0xc, parse_abc(form_srrw)),
            IsaMatch(0xd, parse_ab(form_sm)),
            IsaMatch(0xe, parse_ai24(form_i24)),
        ])),
    ])

    # The parse tree for ops 0xc0-0xff.
    parser_u = ParseSwitch(fields.aopa, [
        IsaMatch(0, parse_abi8(form_rwi8)),
        IsaMatch(1, parse_abi8(form_rri8)),
        IsaMatch(2, parse_abi16(form_rwi16)),
        IsaMatch(3, ParseSwitch(fields.aopb, [
            IsaMatch(0, parse_abi8(form_mi8)),
            IsaMatch(1, parse_abi16(form_mi16)),
            IsaMatch(2, parse_abi8(form_ri8)),
            IsaMatch(4, parse_abi8(form_i8)),
            IsaMatch(5, parse_abi16(form_i16)),
            IsaMatch(8, parse_ab(form_n)),
            IsaMatch(9, parse_ab(form_r)),
            IsaMatch(0xa, parse_abc(form_rr)),
            IsaMatch(0xc, parse_ab(form_w)),
            IsaMatch(0xd, parse_abc(form_mr)),
            IsaMatch(0xe, parse_abc(form_rw)),
            IsaMatch(0xf, parse_abc(form_rrw)),
        ])),
    ])

    # The parse tree root.
    parser = [
        ParseWord(fields.a),
        ParseSwitch(fields.asz, [
            IsaMatch(0, parser_s),
            IsaMatch(1, parser_s),
            IsaMatch(2, parser_s),
            IsaMatch(3, parser_u),
        ])
    ]

# Somebody's out to break you
# Hiding in narrows - poison arrows
