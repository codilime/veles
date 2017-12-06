#pragma once

#include <vector>

#include <QString>

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

QString randomInstruction();
QString randomRegister();

}  // namespace mocks
}  // namespace disasm
}  // namespace ui
}  // namespace veles
