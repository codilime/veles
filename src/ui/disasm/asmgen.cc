/*
 * Copyright 2018 CodiLime
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

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

std::unique_ptr<Sublist> randomInstruction(ChunkID chunk_id) {
  Instruction instr = *random_choice(begin(instructions), end(instructions));

  std::vector<std::unique_ptr<TextRepr>> children;
  if (ins_oper_num[instr] == 1) {
    children.emplace_back(makeOpcodeKeyword(instr, chunk_id));
    children.emplace_back(makeBlank());
    children.emplace_back(makeRegisterKeyword(
        *random_choice(begin(registers), end(registers)), chunk_id));
  } else if (ins_oper_num[instr] == 2) {
    children.emplace_back(makeOpcodeKeyword(instr, chunk_id));
    children.emplace_back(makeBlank());
    children.emplace_back(makeRegisterKeyword(
        *random_choice(begin(registers), end(registers)), chunk_id));
    children.emplace_back(makeString(","));
    children.emplace_back(makeBlank());
    children.emplace_back(makeRegisterKeyword(
        *random_choice(begin(registers), end(registers)), chunk_id));
  }
  return std::make_unique<Sublist>(std::move(children));
}

std::unique_ptr<String> makeString(QString s) {
  return std::make_unique<String>(s);
}

std::unique_ptr<Blank> makeBlank() { return std::make_unique<Blank>(); }

std::unique_ptr<Keyword> makeOpcodeKeyword(Instruction instr,
                                           ChunkID chunk_id) {
  return std::make_unique<Keyword>(ins_mnemo[instr], KeywordType::OPCODE,
                                   chunk_id);
}

std::unique_ptr<Keyword> makeRegisterKeyword(Register reg, ChunkID chunk_id) {
  return std::make_unique<Keyword>(reg_mnemo[reg], KeywordType::REGISTER,
                                   chunk_id);
}

}  // namespace mocks
}  // namespace disasm
}  // namespace ui
}  // namespace veles
