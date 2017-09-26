#include "data/nodeid.h"

#include <algorithm>

#include "util/encoders/hex_encoder.h"
#include "util/random.h"

using veles::util::encoders::HexEncoder;

namespace veles {
namespace data {

const uint8_t NodeID::NIL_VALUE[NodeID::WIDTH] = {};
const uint8_t NodeID::ROOT_VALUE[NodeID::WIDTH] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
};

NodeID::NodeID() {
  std::generate_n(value, WIDTH, std::ref(veles::util::g_mersenne_twister));
}

NodeID::NodeID(const uint8_t* data) { memcpy(value, data, WIDTH); }

NodeID::NodeID(const std::string& data) {
  assert(data.size() == WIDTH);
  memcpy(value, data.data(), WIDTH);
}

NodeID::NodeID(const NodeID& other) { memcpy(value, other.value, WIDTH); }

QString NodeID::toHexString() const {
  return HexEncoder().encode(value, WIDTH);
}

std::shared_ptr<NodeID> NodeID::fromHexString(const QString& val) {
  if (val.size() != WIDTH * 2) {
    return nullptr;
  }
  QByteArray arr = HexEncoder().decode(val);
  return std::make_shared<NodeID>(arr.data());
}

std::vector<uint8_t> NodeID::asStdVector() const {
  return std::vector<uint8_t>(value, value + WIDTH);
}

std::shared_ptr<NodeID> NodeID::getRootNodeId() {
  static auto root = std::make_shared<NodeID>(ROOT_VALUE);
  return root;
}

std::shared_ptr<NodeID> NodeID::getNilId() {
  static auto nil = std::make_shared<NodeID>(NIL_VALUE);
  return nil;
}

bool NodeID::operator==(const NodeID& other) const {
  return memcmp(this->value, other.value, WIDTH) == 0;
}

bool NodeID::operator!=(const NodeID& other) const { return !(*this == other); }

bool NodeID::operator<(const NodeID& other) const {
  for (unsigned int i = 0; i < WIDTH; ++i) {
    if (value[i] < other.value[i]) {
      return true;
    }
    if (value[i] > other.value[i]) {
      return false;
    }
  }

  return false;
}

NodeID::operator bool() const {
  return memcmp(this->value, NIL_VALUE, WIDTH) != 0;
}

}  // namespace data
}  // namespace veles
