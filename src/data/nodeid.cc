#include "data/nodeid.h"

namespace veles {
namespace data {

const uint8_t NodeID::NIL_VALUE[NodeID::width] = {};
const uint8_t NodeID::ROOT_VALUE[NodeID::width] = {
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
};

NodeID::NodeID() {
  memcpy(value, NIL_VALUE, width);
}

NodeID::NodeID(const uint8_t* data) {
  memcpy(value, data, width);
}

NodeID::NodeID(const std::string& data) {
  assert(data.size() == 24);
  memcpy(value, data.data(), width);
}

std::string NodeID::toHexString() {
  std::stringstream ss;
  ss << std::hex << std::setfill('0');
  for (auto byte : value) {
    ss << std::setw(2) << static_cast<unsigned>(byte);
  }
  return ss.str();
}

}  // namespace data
}  // namespace veles
