/*
 * Copyright 2017 CodiLime
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
#include "network/msgpackobject.h"
namespace veles {
namespace messages {

void MsgpackObject::fromAnother(const MsgpackObject &other)
{
  obj_type = other.type();
  switch(obj_type) {
  case ObjectType::BOOLEAN:
    value.boolean = other.value.boolean;
    break;
  case ObjectType::UNSIGNED_INTEGER:
    value.uint = other.value.uint;
    break;
  case ObjectType::SIGNED_INTEGER:
    value.sint = other.value.sint;
    break;
  case ObjectType::DOUBLE:
    value.dbl = other.value.dbl;
    break;
  case ObjectType::STR:
    new (&value.str) std::shared_ptr<std::string>(other.value.str);
    break;
  case ObjectType::BIN:
    new (&value.bin) std::shared_ptr<std::vector<uint8_t>>(other.value.bin);
    break;
  case ObjectType::ARRAY:
    new (&value.array) std::shared_ptr<std::vector<std::shared_ptr<MsgpackObject>>>(other.value.array);
    break;
  case ObjectType::MAP:
    new (&value.map) std::shared_ptr<std::map<std::string, std::shared_ptr<MsgpackObject>>>(other.value.map);
    break;
  case ObjectType::EXT:
    new (&value.ext) std::pair<int, std::shared_ptr<std::vector<uint8_t>>>(other.value.ext);
    break;
  default:
    break;
  }
}

MsgpackObject::MsgpackObject(const msgpack::v2::object &obj) {
  fromMsgpack(obj);
}

MsgpackObject::MsgpackObject(const MsgpackObject &other) {
  fromAnother(other);
}

MsgpackObject::~MsgpackObject() {
  destroyValue();
}

MsgpackObject &MsgpackObject::operator=(const MsgpackObject& other) {
  fromAnother(other);
  return *this;
}

void MsgpackObject::destroyValue() {
  switch(obj_type) {
  case ObjectType::STR:
    (&value.str)->std::shared_ptr<std::string>::~shared_ptr();
    break;
  case ObjectType::BIN:
    (&value.bin)->std::shared_ptr<std::vector<uint8_t>>::~shared_ptr();
    break;
  case ObjectType::ARRAY:
    (&value.array)->std::shared_ptr<std::vector<std::shared_ptr<MsgpackObject>>>::~shared_ptr();
    break;
  case ObjectType::MAP:
    (&value.map)->std::shared_ptr<std::map<std::string, std::shared_ptr<MsgpackObject>>>::~shared_ptr();
    break;
  case ObjectType::EXT:
    (&value.ext)->std::pair<int, std::shared_ptr<std::vector<uint8_t>>>::~pair();
    break;
  default:
    // Other types are POD
    break;
  }
}

void MsgpackObject::fromMsgpack(const msgpack::v2::object &obj) {
  switch(obj.type) {
  case msgpack::type::NIL:
    obj_type = ObjectType::NIL;
    break;
  case msgpack::type::BOOLEAN:
    obj_type = ObjectType::BOOLEAN;
    obj.convert(value.boolean);
    break;
  case msgpack::type::POSITIVE_INTEGER:
    obj_type = ObjectType::UNSIGNED_INTEGER;
    obj.convert(value.uint);
    break;
  case msgpack::type::NEGATIVE_INTEGER:
    obj_type = ObjectType::SIGNED_INTEGER;
    obj.convert(value.sint);
    break;
  case msgpack::type::FLOAT32:
  case msgpack::type::FLOAT64:
    // TODO test if convert won't fail if we get FLOAT32
    obj_type = ObjectType::DOUBLE;
    obj.convert(value.dbl);
    break;
  case msgpack::type::STR:
    obj_type = ObjectType::STR;
    new (&value.str) std::shared_ptr<std::string>;
    obj.convert(value.str);
    break;
  case msgpack::type::BIN:
    obj_type = ObjectType::BIN;
    new (&value.bin) std::shared_ptr<std::vector<uint8_t>>;
    obj.convert(value.bin);
    break;
  case msgpack::type::ARRAY:
    obj_type = ObjectType::ARRAY;
    new (&value.array) std::shared_ptr<std::vector<MsgpackObject>>;
    obj.convert(value.array);
    break;
  case msgpack::type::MAP:
    obj_type = ObjectType::MAP;
    new (&value.map) std::shared_ptr<std::map<std::string, MsgpackObject>>;
    obj.convert(value.map);
    break;
  case msgpack::type::EXT:
    obj_type = ObjectType::EXT;
    new (&value.ext) std::pair<int, std::shared_ptr<std::vector<uint8_t>>>;
    auto obj_ext = obj.via.ext;
    value.ext.first = obj_ext.type();
    value.ext.second = std::make_shared<std::vector<uint8_t>>(obj_ext.data(), obj_ext.data()+obj_ext.size);
    break;
  }
}

void MsgpackObject::msgpack_unpack(const msgpack::v2::object& obj) {
  destroyValue();
  fromMsgpack(obj);
}

ObjectType MsgpackObject::type() const {
  return obj_type;
}

bool& MsgpackObject::getBool() {
  return value.boolean;
}

bool MsgpackObject::getBool() const{
  return value.boolean;
}

uint64_t& MsgpackObject::getUnsignedInt() {
  return value.uint;
}

uint64_t MsgpackObject::getUnsignedInt() const{
  return value.uint;
}

int64_t& MsgpackObject::getSignedInt() {
  return value.sint;
}

int64_t MsgpackObject::getSignedInt() const{
  return value.sint;
}

double& MsgpackObject::getDouble() {
  return value.dbl;
}

double MsgpackObject::getDouble() const{
  return value.dbl;
}

std::shared_ptr<std::string> MsgpackObject::getString() {
  return value.str;
}

const std::shared_ptr<std::string> MsgpackObject::getString() const{
  return value.str;
}

std::shared_ptr<std::vector<uint8_t> > MsgpackObject::getBin() {
  return value.bin;
}

const std::shared_ptr<std::vector<uint8_t> > MsgpackObject::getBin() const{
  return value.bin;
}

std::shared_ptr<std::vector<std::shared_ptr<MsgpackObject> > > MsgpackObject::getArray() {
  return value.array;
}

const std::shared_ptr<std::vector<std::shared_ptr<MsgpackObject> > > MsgpackObject::getArray() const{
  return value.array;
}

std::shared_ptr<std::map<std::string, std::shared_ptr<MsgpackObject> > > MsgpackObject::getMap() {
  return value.map;
}

const std::shared_ptr<std::map<std::string, std::shared_ptr<MsgpackObject> > > MsgpackObject::getMap() const{
  return value.map;
}

std::pair<int, std::shared_ptr<std::vector<uint8_t> > > MsgpackObject::getExt() {
  return value.ext;
}

const std::pair<int, std::shared_ptr<std::vector<uint8_t> > > MsgpackObject::getExt() const{
  return value.ext;
}

template<>
std::shared_ptr<MsgpackObject> toMsgpackObject(const std::vector<uint8_t>& val) {
  return std::make_shared<MsgpackObject>(val);
}

template<>
std::shared_ptr<MsgpackObject> toMsgpackObject(const std::shared_ptr<std::vector<uint8_t>> val) {
  if (val == nullptr) throw std::bad_cast();
  return std::make_shared<MsgpackObject>(val);
}

std::shared_ptr<MsgpackObject> toMsgpackObject(const MsgpackObject& obj) {
  return std::make_shared<MsgpackObject>(obj);
}

std::shared_ptr<MsgpackObject> toMsgpackObject(const std::shared_ptr<MsgpackObject> obj) {
  if (obj == nullptr) throw std::bad_cast();
  return obj;
}

std::shared_ptr<MsgpackObject> toMsgpackObject(bool val) {
  return std::make_shared<MsgpackObject>(val);
}

std::shared_ptr<MsgpackObject> toMsgpackObject(uint64_t val) {
  return std::make_shared<MsgpackObject>(val);
}

std::shared_ptr<MsgpackObject> toMsgpackObject(int64_t val) {
  return std::make_shared<MsgpackObject>(val);
}

std::shared_ptr<MsgpackObject> toMsgpackObject(const std::string& val) {
  return std::make_shared<MsgpackObject>(val);
}

std::shared_ptr<MsgpackObject> toMsgpackObject(const std::shared_ptr<std::string> val) {
  if (val == nullptr) throw std::bad_cast();
  return std::make_shared<MsgpackObject>(val);
}

std::shared_ptr<MsgpackObject> toMsgpackObject(const data::NodeID& val) {
  return std::make_shared<MsgpackObject>(0, val.asVector());
}

std::shared_ptr<MsgpackObject> toMsgpackObject(const std::shared_ptr<data::NodeID> val) {
  if (val == nullptr) throw std::bad_cast();
  return std::make_shared<MsgpackObject>(0, val->asVector());
}

std::shared_ptr<MsgpackObject> toMsgpackObject(const double val) {
  return std::make_shared<MsgpackObject>(val);
}

void fromMsgpackObject(const std::shared_ptr<MsgpackObject> obj, std::shared_ptr<data::NodeID>& out) {
  if (obj == nullptr) {
    out = nullptr;
  } else if (obj->type() == ObjectType::NIL) {
    out = std::make_shared<data::NodeID>(data::NodeID::NIL_VALUE);
  } else {
    out = std::make_shared<data::NodeID>(obj->getExt().second->data());
  }
}

void fromMsgpackObject(const std::shared_ptr<MsgpackObject> obj, std::shared_ptr<MsgpackObject>& out) {
  out = obj;
}

void fromMsgpackObject(const std::shared_ptr<MsgpackObject> obj, std::shared_ptr<std::string>& out) {
  if (obj == nullptr) {
    out = nullptr;
  } else {
    out = obj->getString();
  }
}

void fromMsgpackObject(const std::shared_ptr<MsgpackObject> obj, std::shared_ptr<std::vector<uint8_t> >& out) {
  if (obj == nullptr) {
    out = nullptr;
  } else {
    out = obj->getBin();
  }
}

// TODO what to do if obj is nullptr ?
void fromMsgpackObject(const std::shared_ptr<MsgpackObject> obj, uint64_t& out) {
  if (obj == nullptr) {
    out = 0;
  } else {
    out = obj->getUnsignedInt();
  }
}

void fromMsgpackObject(const std::shared_ptr<MsgpackObject> obj, int64_t& out) {
  if (obj == nullptr) {
    out = 0;
  } else {
    out = obj->getSignedInt();
  }
}

void fromMsgpackObject(const std::shared_ptr<MsgpackObject> obj, bool& out) {
  if (obj == nullptr) {
    out = false;
  } else {
    out = obj->getBool();
  }
}

void fromMsgpackObject(const std::shared_ptr<MsgpackObject> obj, double& out) {
  if (obj == nullptr) {
    out = 0.0;
  } else {
    out = obj->getDouble();
  }
}

}  // namespace messages
}  // namespace veles
