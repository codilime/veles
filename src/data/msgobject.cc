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

void NodeID::msgpack_unpack(const msgpack::v2::object &o) {
  if (o.type == msgpack::type::NIL) {
    memcpy(value, NIL_VALUE, width);
    return;
  }
  if (o.type != msgpack::type::EXT) {
    throw msgpack::type_error();
  }
  if (o.via.ext.type() != 0 || o.via.ext.size != width) {
    throw msgpack::type_error();
  }
  memcpy(value, o.via.ext.data(), width);
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
