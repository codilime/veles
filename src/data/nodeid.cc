#include "data/nodeid.h"

#include "util/encoders/hex_encoder.h"

using veles::util::encoders::HexEncoder;

namespace veles {
namespace data {

const uint8_t NodeID::NIL_VALUE[NodeID::WIDTH_] = {};
const uint8_t NodeID::ROOT_VALUE[NodeID::WIDTH_] = {
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
};

NodeID::NodeID() {
  memcpy(value, NIL_VALUE, WIDTH_);
}

NodeID::NodeID(const uint8_t* data) {
  memcpy(value, data, WIDTH_);
}

NodeID::NodeID(const QString& data) {
  assert(data.size() == 24);
  memcpy(value, data.data(), WIDTH_);
}

QString NodeID::toHexString() const {
  return HexEncoder().encode(value, WIDTH_);
}

}  // namespace data
}  // namespace veles
