#pragma once

#include <memory>
#include <vector>

#include <QString>

#include "ui/disasm/disasm.h"
#include "util/random.h"

namespace veles {
namespace ui {
namespace disasm {
namespace mocks {

template <typename I>
I random_choice(I begin, I end) {
  std::uniform_int_distribution<> dis(0, std::distance(begin, end) - 1);
  std::advance(begin, dis(veles::util::g_mersenne_twister));
  return begin;
}

enum class Instruction { MOV, PUSH, POP, ADD, SUB, INC, DEC };

enum class Register { EAX, EBX, ECX, EDX, EDI, ESI, ESP, EBP };

std::unique_ptr<Sublist> randomInstruction(ChunkID chunk_id);
std::unique_ptr<String> makeString(QString s);
std::unique_ptr<Blank> makeBlank();
std::unique_ptr<Keyword> makeOpcodeKeyword(Instruction instr, ChunkID chunk_id);
std::unique_ptr<Keyword> makeRegisterKeyword(Register reg, ChunkID chunk_id);

}  // namespace mocks
}  // namespace disasm
}  // namespace ui
}  // namespace veles
