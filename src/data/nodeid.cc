#include "data/nodeid.h"

#include "util/encoders/hex_encoder.h"

using veles::util::encoders::HexEncoder;

namespace veles {
namespace data {

const uint8_t NodeID::NIL_VALUE[NodeID::WIDTH] = {};
const uint8_t NodeID::ROOT_VALUE[NodeID::WIDTH] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
};

std::mt19937 NodeID::random_ = []() {
  std::array<int, 5> seed_data;
  std::random_device r;
  std::generate_n(seed_data.data(), seed_data.size(), std::ref(r));
  std::seed_seq seq(seed_data.begin(), seed_data.end());
  return std::mt19937(seq);
}();

NodeID::NodeID() { std::generate_n(value, WIDTH, std::ref(random_)); }

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
