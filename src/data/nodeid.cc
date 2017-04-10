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
  for (size_t i = 0; i < WIDTH_; i+=sizeof(unsigned int)) {
    *reinterpret_cast<unsigned int*>(&value[i]) = random_();
  }
}

NodeID::NodeID(const uint8_t* data) {
  memcpy(value, data, WIDTH_);
}

NodeID::NodeID(const std::string& data) {
  assert(data.size() == WIDTH_);
  memcpy(value, data.data(), WIDTH_);
}

NodeID::NodeID(const NodeID &other) {
  memcpy(value, other.value, WIDTH_);
}

QString NodeID::toHexString() const {
  return HexEncoder().encode(value, WIDTH_);
}

std::shared_ptr<NodeID> NodeID::fromHexString(QString &val) {
  if (val.size() != 48) {
    return nullptr;
  }
  QByteArray arr = HexEncoder().decode(val);
  return std::make_shared<NodeID>(arr.data());
}

std::vector<uint8_t> NodeID::asStdVector() const {
  return std::vector<uint8_t>(value, value + WIDTH_);
}

std::shared_ptr<NodeID> NodeID::getRootNodeId() {
  static std::shared_ptr<NodeID> root(new NodeID(ROOT_VALUE));
  return root;
}

std::shared_ptr<NodeID> NodeID::getNilId() {
  static std::shared_ptr<NodeID> nil(new NodeID(NIL_VALUE));
  return nil;
}

bool NodeID::operator==(const NodeID &other) const {
  return memcmp(this->value, other.value, WIDTH_) == 0;
}

bool NodeID::operator!=(const NodeID &other) const {
  return !(*this == other);
}

NodeID::operator bool() const {
  return memcmp(this->value, NIL_VALUE, WIDTH_) == 0;
}

}  // namespace data
}  // namespace veles
