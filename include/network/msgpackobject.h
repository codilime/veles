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
#pragma once

#include <msgpack.hpp>

#include "data/bindata.h"
#include "data/nodeid.h"
#include "fwd_models.h"
#include "proto/exceptions.h"

namespace veles {
namespace messages {

enum class ObjectType {
  NIL,
  BOOLEAN,
  UNSIGNED_INTEGER,
  SIGNED_INTEGER,
  DOUBLE,
  STR,
  BIN,
  ARRAY,
  MAP,
  EXT,
};

class MsgpackObject {
  ObjectType obj_type = ObjectType::NIL;

  union ObjectValue {
    bool boolean;
    int64_t sint;
    uint64_t uint;
    double dbl;
    std::shared_ptr<std::string> str;
    std::shared_ptr<std::vector<uint8_t>> bin;
    std::shared_ptr<std::vector<std::shared_ptr<MsgpackObject>>> array;
    // will we need maps with keys other than string?
    std::shared_ptr<std::map<std::string, std::shared_ptr<MsgpackObject>>> map;
    std::pair<int, std::shared_ptr<std::vector<uint8_t>>> ext;

    ObjectValue() {}
    explicit ObjectValue(bool val) : boolean(val) {}
    explicit ObjectValue(int64_t val) : sint(val) {}
    explicit ObjectValue(uint64_t val) : uint(val) {}
    explicit ObjectValue(double val) : dbl(val) {}
    explicit ObjectValue(const char* val)
        : str(std::make_shared<std::string>(val)) {}
    explicit ObjectValue(const std::string& val)
        : str(std::make_shared<std::string>(val)) {}
    explicit ObjectValue(const std::shared_ptr<std::string>& val) : str(val) {}
    explicit ObjectValue(const std::vector<uint8_t>& val)
        : bin(std::make_shared<std::vector<uint8_t>>(val)) {}
    explicit ObjectValue(const std::shared_ptr<std::vector<uint8_t>>& val)
        : bin(val) {}
    explicit ObjectValue(const std::vector<std::shared_ptr<MsgpackObject>>& val)
        : array(std::make_shared<std::vector<std::shared_ptr<MsgpackObject>>>(
              val)) {}
    explicit ObjectValue(
        const std::shared_ptr<std::vector<std::shared_ptr<MsgpackObject>>>& val)
        : array(val) {}
    explicit ObjectValue(
        const std::map<std::string, std::shared_ptr<MsgpackObject>>& val)
        : map(std::make_shared<
              std::map<std::string, std::shared_ptr<MsgpackObject>>>(val)) {}
    explicit ObjectValue(
        const std::shared_ptr<
            std::map<std::string, std::shared_ptr<MsgpackObject>>>& val)
        : map(val) {}
    ObjectValue(int code, const std::vector<uint8_t>& val)
        : ext(code, std::make_shared<std::vector<uint8_t>>(val)) {}
    ObjectValue(int code, const std::shared_ptr<std::vector<uint8_t>>& val)
        : ext(code, val) {}
    ~ObjectValue() {}
  } value;

  void fromMsgpack(const msgpack::object& obj);
  void destroyValue();

 public:
  MsgpackObject() : obj_type() {}
  explicit MsgpackObject(bool val)
      : obj_type(ObjectType::BOOLEAN), value(val) {}
  explicit MsgpackObject(int64_t val)
      : obj_type(ObjectType::SIGNED_INTEGER), value(val) {}
  explicit MsgpackObject(uint64_t val)
      : obj_type(ObjectType::UNSIGNED_INTEGER), value(val) {}
  explicit MsgpackObject(double val)
      : obj_type(ObjectType::DOUBLE), value(val) {}
  explicit MsgpackObject(const char* val)
      : obj_type(ObjectType::STR), value(val) {}
  explicit MsgpackObject(const std::string& val)
      : obj_type(ObjectType::STR), value(val) {}
  explicit MsgpackObject(const std::shared_ptr<std::string>& val)
      : obj_type(ObjectType::STR), value(val) {}
  explicit MsgpackObject(const std::vector<uint8_t>& val)
      : obj_type(ObjectType::BIN), value(val) {}
  explicit MsgpackObject(const std::shared_ptr<std::vector<uint8_t>>& val)
      : obj_type(ObjectType::BIN), value(val) {}
  explicit MsgpackObject(const std::vector<std::shared_ptr<MsgpackObject>>& val)
      : obj_type(ObjectType::ARRAY), value(val) {}
  explicit MsgpackObject(
      const std::shared_ptr<std::vector<std::shared_ptr<MsgpackObject>>>& val)
      : obj_type(ObjectType::ARRAY), value(val) {}
  explicit MsgpackObject(
      const std::map<std::string, std::shared_ptr<MsgpackObject>>& val)
      : obj_type(ObjectType::MAP), value(val) {}
  explicit MsgpackObject(
      const std::shared_ptr<
          std::map<std::string, std::shared_ptr<MsgpackObject>>>& val)
      : obj_type(ObjectType::MAP), value(val) {}
  explicit MsgpackObject(int code, const std::vector<uint8_t>& val)
      : obj_type(ObjectType::EXT), value(code, val) {}
  explicit MsgpackObject(int code,
                         const std::shared_ptr<std::vector<uint8_t>>& val)
      : obj_type(ObjectType::EXT), value(code, val) {}
  explicit MsgpackObject(const msgpack::object& obj);
  MsgpackObject(const MsgpackObject& other);
  ~MsgpackObject();

  MsgpackObject& operator=(const MsgpackObject& other);
  bool operator==(const MsgpackObject& other) const;
  bool operator!=(const MsgpackObject& other) const;

  void msgpack_unpack(const msgpack::object& obj);

  template <typename Packer>
  void msgpack_pack(Packer& pk) const {
    switch (obj_type) {
      case ObjectType::NIL:
        pk.pack_nil();
        break;
      case ObjectType::BOOLEAN:
        if (value.boolean) {
          pk.pack_true();
        } else {
          pk.pack_false();
        }
        break;
      case ObjectType::UNSIGNED_INTEGER:
        pk.pack_uint64(value.uint);
        break;
      case ObjectType::SIGNED_INTEGER:
        pk.pack_int64(value.sint);
        break;
      case ObjectType::DOUBLE:
        pk.pack_double(value.dbl);
        break;
      case ObjectType::STR:
        pk.pack_str(static_cast<uint32_t>(value.str->size()));
        pk.pack_str_body(value.str->data(),
                         static_cast<uint32_t>(value.str->size()));
        break;
      case ObjectType::BIN:
        pk.pack_bin(static_cast<uint32_t>(value.bin->size()));
        pk.pack_bin_body(reinterpret_cast<const char*>(value.bin->data()),
                         static_cast<uint32_t>(value.bin->size()));
        break;
      case ObjectType::ARRAY:
        pk.pack_array(static_cast<uint32_t>(value.array->size()));
        for (const auto& el : *value.array) {
          pk.pack(el);
        }
        break;
      case ObjectType::MAP:
        pk.pack_map(static_cast<uint32_t>(value.map->size()));
        for (const auto& el : *value.map) {
          pk.pack_str(static_cast<uint32_t>(el.first.size()));
          pk.pack_str_body(el.first.data(),
                           static_cast<uint32_t>(el.first.size()));
          pk.pack(el.second);
        }
        break;
      case ObjectType::EXT:
        pk.pack_ext(value.ext.second->size(), value.ext.first);
        pk.pack_ext_body(
            reinterpret_cast<const char*>(value.ext.second->data()),
            static_cast<uint32_t>(value.ext.second->size()));
        break;
    }
  }

  ObjectType type() const;
  void setNil();
  bool getBool() const;
  void setBool(bool val);
  uint64_t getUnsignedInt() const;
  void setUnsignedInt(uint64_t val);
  int64_t getSignedInt() const;
  void setSignedInt(int64_t val);
  double getDouble() const;
  void setDouble(double val);
  std::shared_ptr<std::string> getString();
  const std::shared_ptr<std::string> getString() const;
  void setString(const std::shared_ptr<std::string>& val);
  std::shared_ptr<std::vector<uint8_t>> getBin();
  const std::shared_ptr<std::vector<uint8_t>> getBin() const;
  void setBin(const std::shared_ptr<std::vector<uint8_t>>& val);
  std::shared_ptr<std::vector<std::shared_ptr<MsgpackObject>>> getArray();
  const std::shared_ptr<std::vector<std::shared_ptr<MsgpackObject>>> getArray()
      const;
  void setArray(
      const std::shared_ptr<std::vector<std::shared_ptr<MsgpackObject>>>& val);
  std::shared_ptr<std::map<std::string, std::shared_ptr<MsgpackObject>>>
  getMap();
  const std::shared_ptr<std::map<std::string, std::shared_ptr<MsgpackObject>>>
  getMap() const;
  void setMap(const std::shared_ptr<
              std::map<std::string, std::shared_ptr<MsgpackObject>>>& val);
  std::pair<int, std::shared_ptr<std::vector<uint8_t>>> getExt();
  const std::pair<int, std::shared_ptr<std::vector<uint8_t>>> getExt() const;
  void setExt(const std::pair<int, std::shared_ptr<std::vector<uint8_t>>>& val);

 private:
  void fromAnother(const MsgpackObject& other);
};

std::shared_ptr<MsgpackObject> toMsgpackObject(bool val);

std::shared_ptr<MsgpackObject> toMsgpackObject(const std::string& val);
std::shared_ptr<MsgpackObject> toMsgpackObject(
    const std::shared_ptr<std::string>& val);

std::shared_ptr<MsgpackObject> toMsgpackObject(uint64_t val);
std::shared_ptr<MsgpackObject> toMsgpackObject(int64_t val);
std::shared_ptr<MsgpackObject> toMsgpackObject(double val);

std::shared_ptr<MsgpackObject> toMsgpackObject(const data::NodeID& val);
std::shared_ptr<MsgpackObject> toMsgpackObject(
    const std::shared_ptr<data::NodeID>& val_ptr);
std::shared_ptr<MsgpackObject> toMsgpackObject(
    const std::shared_ptr<data::BinData>& val_ptr);
std::shared_ptr<MsgpackObject> toMsgpackObject(
    const std::shared_ptr<proto::VelesException>& val_ptr);

namespace details_ {

template <class T>
std::shared_ptr<MsgpackObject> convertCollectionHelper(const T& val);

template <class T>
std::shared_ptr<MsgpackObject> convertMapHelper(const T& val);

std::shared_ptr<MsgpackObject> convertNodeIDHelper(const data::NodeID& val);

}  // namespace details_

template <class T>
std::shared_ptr<MsgpackObject> toMsgpackObject(
    const std::unordered_set<T>& val) {
  return details_::convertCollectionHelper(val);
}

template <class T>
std::shared_ptr<MsgpackObject> toMsgpackObject(
    const std::shared_ptr<std::unordered_set<T>>& val_ptr) {
  if (!val_ptr) throw proto::SchemaError("Unexpected nullptr");
  return details_::convertCollectionHelper(*val_ptr);
}

template <class T>
std::shared_ptr<MsgpackObject> toMsgpackObject(
    const std::unordered_set<std::shared_ptr<T>>& val) {
  return details_::convertCollectionHelper(val);
}

template <class T>
std::shared_ptr<MsgpackObject> toMsgpackObject(
    const std::shared_ptr<std::unordered_set<std::shared_ptr<T>>>& val_ptr) {
  if (!val_ptr) throw proto::SchemaError("Unexpected nullptr");
  return details_::convertCollectionHelper(*val_ptr);
}

template <class T>
std::shared_ptr<MsgpackObject> toMsgpackObject(const std::vector<T>& val) {
  return details_::convertCollectionHelper(val);
}

template <class T>
std::shared_ptr<MsgpackObject> toMsgpackObject(
    const std::shared_ptr<std::vector<T>>& val_ptr) {
  if (!val_ptr) throw proto::SchemaError("Unexpected nullptr");
  return details_::convertCollectionHelper(*val_ptr);
}

template <class T>
std::shared_ptr<MsgpackObject> toMsgpackObject(
    const std::vector<std::shared_ptr<T>>& val) {
  return details_::convertCollectionHelper(val);
}

template <class T>
std::shared_ptr<MsgpackObject> toMsgpackObject(
    const std::shared_ptr<std::vector<std::shared_ptr<T>>>& val_ptr) {
  if (!val_ptr) throw proto::SchemaError("Unexpected nullptr");
  return details_::convertCollectionHelper(*val_ptr);
}

template <>
std::shared_ptr<MsgpackObject> toMsgpackObject(const std::vector<uint8_t>& val);

template <>
std::shared_ptr<MsgpackObject> toMsgpackObject(
    const std::shared_ptr<std::vector<uint8_t>>& val_ptr);

template <class T>
std::shared_ptr<MsgpackObject> toMsgpackObject(
    const std::unordered_map<std::string, T>& val) {
  return details_::convertMapHelper(val);
}

template <class T>
std::shared_ptr<MsgpackObject> toMsgpackObject(
    const std::shared_ptr<std::unordered_map<std::string, T>>& val_ptr) {
  if (!val_ptr) throw proto::SchemaError("Unexpected nullptr");
  return details_::convertMapHelper(*val_ptr);
}

template <class T>
std::shared_ptr<MsgpackObject> toMsgpackObject(
    const std::unordered_map<std::string, std::shared_ptr<T>>& val) {
  return details_::convertMapHelper(val);
}

template <class T>
std::shared_ptr<MsgpackObject> toMsgpackObject(
    const std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<T>>>&
        val_ptr) {
  if (!val_ptr) throw proto::SchemaError("Unexpected nullptr");
  return details_::convertMapHelper(*val_ptr);
}

std::shared_ptr<MsgpackObject> toMsgpackObject(const MsgpackObject& obj);
std::shared_ptr<MsgpackObject> toMsgpackObject(
    const std::shared_ptr<MsgpackObject>& obj);

void fromMsgpackObject(const std::shared_ptr<MsgpackObject>& obj, bool* out);
void fromMsgpackObject(const std::shared_ptr<MsgpackObject>& obj, int64_t* out);
void fromMsgpackObject(const std::shared_ptr<MsgpackObject>& obj,
                       uint64_t* out);
void fromMsgpackObject(const std::shared_ptr<MsgpackObject>& obj, double* out);
void fromMsgpackObject(const std::shared_ptr<MsgpackObject>& obj,
                       std::shared_ptr<data::NodeID>* out);
void fromMsgpackObject(const std::shared_ptr<MsgpackObject>& obj,
                       std::shared_ptr<data::BinData>* out);
void fromMsgpackObject(const std::shared_ptr<MsgpackObject>& obj,
                       std::shared_ptr<proto::VelesException>* out);
void fromMsgpackObject(const std::shared_ptr<MsgpackObject>& obj,
                       std::shared_ptr<MsgpackObject>* out);
void fromMsgpackObject(const std::shared_ptr<MsgpackObject>& obj,
                       std::shared_ptr<std::string>* out);
void fromMsgpackObject(const std::shared_ptr<MsgpackObject>& obj,
                       std::shared_ptr<std::vector<uint8_t>>* out);

template <class T>
void fromMsgpackObject(const std::shared_ptr<MsgpackObject>& obj,
                       std::shared_ptr<std::vector<T>>* out) {
  if (obj == nullptr) {
    *out = nullptr;
  } else {
    auto tmp = obj->getArray();
    *out = std::make_shared<std::vector<T>>();
    for (const auto& el : *tmp) {
      T conv;
      fromMsgpackObject(el, &conv);
      (*out)->push_back(conv);
    }
  }
}

template <class T>
void fromMsgpackObject(const std::shared_ptr<MsgpackObject>& obj,
                       std::shared_ptr<std::unordered_set<T>>* out) {
  if (obj == nullptr) {
    *out = nullptr;
  } else {
    *out = std::make_shared<std::unordered_set<T>>();
    auto tmp = obj->getArray();
    for (const auto& el : *tmp) {
      T conv;
      fromMsgpackObject(el, &conv);
      (*out)->insert(conv);
    }
  }
}

template <class T>
void fromMsgpackObject(
    const std::shared_ptr<MsgpackObject>& obj_ptr,
    std::shared_ptr<std::unordered_map<std::string, T>>* out) {
  if (!obj_ptr) {
    *out = nullptr;
  } else {
    *out = std::make_shared<std::unordered_map<std::string, T>>();
    auto tmp = obj_ptr->getMap();
    for (const auto& el : *tmp) {
      T conv;
      fromMsgpackObject(el.second, &conv);
      (**out)[el.first] = conv;
    }
  }
}

namespace details_ {

template <class T>
std::shared_ptr<MsgpackObject> convertCollectionHelper(const T& val) {
  std::vector<std::shared_ptr<MsgpackObject>> values;
  for (const auto& elem : val) {
    values.push_back(toMsgpackObject(elem));
  }
  return std::make_shared<MsgpackObject>(values);
}

template <class T>
std::shared_ptr<MsgpackObject> convertMapHelper(const T& val) {
  std::map<std::string, std::shared_ptr<MsgpackObject>> value;
  for (const auto& elem : val) {
    value[elem.first] = toMsgpackObject(elem.second);
  }
  return std::make_shared<MsgpackObject>(value);
}

}  // namespace details_

}  // namespace messages
}  // namespace veles
