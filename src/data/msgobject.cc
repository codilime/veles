#include "data/msgobject.h"

namespace veles {
namespace data {

const unsigned char NodeID::NIL_VALUE[NodeID::width] = {};

NodeID::NodeID() {
  memcpy(value, NIL_VALUE, width);
}

NodeID::NodeID(const unsigned char *data) {
  memcpy(value, data, width);
}

NodeID::NodeID(const std::string &data) {
  assert(data.size() == 24);
  memcpy(value, data.data(), width);
}

std::string NodeID::toString() {
  std::stringstream ss;
  ss << std::hex << std::setfill('0');
  for (auto byte : value) {
    ss << std::setw(2) << static_cast<unsigned>(byte);
  }
  return ss.str();
}

}  // namespace data
}  // namespace veles
