#include "ui/disasm/asmgen.h"

#include <map>

namespace veles {
namespace ui {
namespace disasm {
namespace mocks {

std::vector<Instruction> instructions = {
    Instruction::MOV, Instruction::PUSH, Instruction::POP, Instruction::ADD,
    Instruction::SUB, Instruction::INC,  Instruction::DEC,
};

std::map<Instruction, QString> ins_mnemo = {
    {Instruction::MOV, "mov"}, {Instruction::PUSH, "push"},
    {Instruction::POP, "pop"}, {Instruction::ADD, "add"},
    {Instruction::SUB, "sub"}, {Instruction::INC, "inc"},
    {Instruction::DEC, "dec"},
};

std::map<Instruction, int> ins_oper_num = {
    {Instruction::MOV, 2}, {Instruction::PUSH, 1}, {Instruction::POP, 1},
    {Instruction::ADD, 2}, {Instruction::SUB, 2},  {Instruction::INC, 1},
    {Instruction::DEC, 1},
};

std::vector<Register> registers = {Register::EAX, Register::EBX, Register::ECX,
                                   Register::EDX, Register::EDI, Register::ESI,
                                   Register::ESP, Register::EBP};

std::map<Register, QString> reg_mnemo = {
    {Register::EAX, "eax"}, {Register::EBX, "ebx"}, {Register::ECX, "ecx"},
    {Register::EDX, "edx"}, {Register::EDI, "edi"}, {Register::ESI, "esi"},
    {Register::ESP, "esp"}, {Register::EBP, "ebp"}};

QString randomInstruction() {
  auto instr = *random_choice(begin(instructions), end(instructions));
  if (ins_oper_num[instr] == 1) {
    return QString("%1 %2").arg(ins_mnemo[instr]).arg(randomRegister());
  } else if (ins_oper_num[instr] == 2) {
    return QString("%1 %2, %3")
        .arg(ins_mnemo[instr])
        .arg(randomRegister())
        .arg(randomRegister());
  }
  return "[Instruction]";
}

QString randomRegister() {
  auto reg = *random_choice(begin(registers), end(registers));
  return reg_mnemo[reg];
}

}  // namespace mocks
}  // namespace disasm
}  // namespace ui
}  // namespace veles
