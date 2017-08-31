/*
 * Copyright 2016 CodiLime
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

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <initializer_list>

#include <QString>

namespace veles {
namespace data {

/** Represents all kinds of uniform-sized raw binary data.

    Conceptually, a BinData instance is an array of <size> <width>-bit sized
    elements, where width can be any reasonably sized integer (up to several
    thousand should be OK).  The data is stored as a big array of octets -
    each element is stored as ceil(width/8) octets in little-endian format.

    This class has value semantics, but copying large instances may be
    rather expensive - use references in this case, or extract a subrange
    of in interesting data to pass around.  */

class BinData {
 public:
  /** Constructs a BinData instance from given width, size, and pointer to
      raw data.  ceil(width/8) * size octets are read from init_data.
      If raw data is not given, the instance is initialized with zeros.  */
  BinData(uint32_t width, size_t size, const uint8_t* init_data = nullptr)
      : width_(width), size_(size) {
    assert(width != 0);
    if (!isInline()) {
      data_ = new uint8_t[octets()];
    }
    if (init_data != nullptr) {
      memcpy(rawData(), init_data, octets());
    } else {
      memset(rawData(), 0, octets());
    }
  }

  /** Constructs a BinData instance from given width and data given as
      uint64_t elements.  width must be <= 64.  */
  BinData(uint32_t width, const std::initializer_list<uint64_t>& init)
      : BinData(width, init.size()) {
    assert(width <= 64);
    size_t pos = 0;
    for (auto x : init) {
      setElement64(pos++, x);
    }
  }

  /** Constructs a BinData instance from another one.  */
  BinData(const BinData& other)
      : BinData(other.width(), other.size(), other.rawData()) {}

  /** Deletes this instance's data and replaces it with that of another one.  */
  BinData& operator=(const BinData& other) {
    if (!isInline()) {
      delete[] data_;
    }
    width_ = other.width_;
    size_ = other.size_;
    if (!isInline()) {
      data_ = new uint8_t[octets()];
    }
    memcpy(rawData(), other.rawData(), octets());
    return *this;
  }

  /** Constructs a BinData instance from another one, with move semantics.
      The internal data storage is moved from the other instance if necessary,
      avoiding a new allocation and a copy.  */
  BinData(BinData&& other) noexcept : width_(other.width_), size_(other.size_) {
    if (isInline()) {
      memcpy(idata_, other.idata_, sizeof idata_);
    } else {
      data_ = other.data_;
      other.size_ = 0;
      other.width_ = 0;
    }
  }

  /** Assigns a BinData instance from another one, with move semantics.
      The internal data storage is moved from the other instance if necessary,
      avoiding a new allocation and a copy.  The old data is destroyed.  */
  BinData& operator=(BinData&& other) noexcept {
    if (!isInline()) {
      delete[] data_;
    }
    width_ = other.width_;
    size_ = other.size_;
    if (isInline()) {
      memcpy(idata_, other.idata_, sizeof idata_);
    } else {
      data_ = other.data_;
      other.size_ = 0;
      other.width_ = 0;
    }
    return *this;
  }

  /** Checks if all attributes of BinData's are equal. */
  bool operator==(const BinData& other) const {
    if (width_ != other.width_ || size_ != other.size_) {
      return false;
    }
    if (isInline()) {
      return memcmp(idata_, other.idata_, octets()) == 0;
    }
    return memcmp(data_, other.data_, octets()) == 0;
  }

  /** Creates a dummy BinData instance.  */
  BinData() : BinData(8, 0) {}

  /** Constructs a BinData instance from given width and raw data.  The length
      of raw data must be a multiple of ceil(width/8).  */
  static BinData fromRawData(uint32_t width,
                             const std::initializer_list<uint8_t>& init) {
    size_t size = init.size() / BinData(width, 0).octetsPerElement();
    BinData res(width, size);
    assert(res.octets() == init.size());
    uint8_t* dst = res.rawData();
    for (auto x : init) {
      *dst++ = x;
    }
    return res;
  }

  /** Destroys instance's storage, if necessary.  */
  ~BinData() {
    if (!isInline()) {
      delete[] data_;
    }
  }

  /** Returns element width, in bits.  */
  uint32_t width() const { return width_; }

  /** Returns data size, in elements.  */
  size_t size() const { return size_; }

  /** Returns element width, in octets - this is how many octets each element
      takes in the raw data.  */
  unsigned octetsPerElement() const { return (width_ + 7) / 8; }

  /** Returns raw data size, in octets. */
  size_t octets() const { return size_ * octetsPerElement(); }

  /** Returns a pointer to the raw data, starting from a given element
      (or from element 0 if not given).  Elements are contiguous in memory,
      with each element octetsPerElement() octets after the previous one.  */
  uint8_t* rawData(size_t el = 0) {
    uint8_t* d = isInline() ? idata_ : data_;
    return d + el * octetsPerElement();
  }

  /** Returns a pointer to the raw data, starting from a given element
      (or from element 0 if not given).  Elements are contiguous in memory,
      with each element octetsPerElement() octets after the previous one.  */
  const uint8_t* rawData(size_t el = 0) const {
    const uint8_t* d = isInline() ? idata_ : data_;
    return d + el * octetsPerElement();
  }

  /** Returns a subrange of data, starting from `offset`.  `offset` is counted
   * in elements from start of the array.  `size` is counted in elements of the
   * array.  The result has the same width as this instance.  */
  BinData data(size_t offset, size_t size) const {
    assert(offset + size <= size_);
    return BinData(width_, size, rawData(offset));
  }

  /** Returns a single element of data, as a single-element BinData
      instance of the same width.  */
  BinData operator[](size_t pos) const { return data(pos, 1); }

  /** Returns a subrange of bits of a single element of data.  Bits are
      counted from LSB, 0-based.  Result is a single-element BinData
      with a width equal to num_bits.  */
  BinData bits(size_t el, unsigned start_bit, unsigned num_bits) const {
    assert(start_bit + num_bits <= width_);
    assert(el < size_);
    BinData res(num_bits, 1);
    copyBits(res.rawData(), 0, rawData(el), start_bit, num_bits);
    return res;
  }

  /** Create new bindata by contacting two other BinDatas. Both BinData objects
   * must have the same width. */
  BinData operator+(const BinData& other) const {
    assert(width_ == other.width_);
    BinData res = BinData(width_, size_ + other.size_);
    res.setData(0, size_, *this);
    res.setData(size_, other.size_, other);
    return res;
  }

  /** Returns a subrange of bits of a single element of data.  Bits are
      counted from LSB, 0-based.  num_bits must be at most 64.  */
  uint64_t bits64(size_t el, unsigned start_bit, unsigned num_bits) const {
    assert(start_bit + num_bits <= width_);
    assert(num_bits <= 64);
    assert(el < size_);
    uint8_t octets[8] = {0};
    uint64_t res = 0;
    copyBits(octets, 0, rawData(el), start_bit, num_bits);
    for (int i = 0; i < 8; i++) {
      res |= static_cast<uint64_t>(octets[i]) << (8 * i);
    }
    return res;
  }

  /** Returns an element as an uint64_t.  Width must be at most 64.  */
  uint64_t element64(size_t el = 0) const { return bits64(el, 0, width_); }

  /** Replaces a range of elements with the contents of another
      BinData instance.  Both BinData objects must have the same width,
      and size of the replaced range must be equal to the size
      of the other BinData.  Addressing is the same as in data()
      method.  */
  void setData(size_t offset, size_t size, const BinData& other) {
    assert(offset + size <= size_);
    assert(size == other.size_);
    assert(width_ == other.width_);
    memcpy(rawData(offset), other.rawData(0), other.octets());
  }

  /** Replaces a range of bits of a single element with the contents
      of a single-element BinData.  The other BinData's width must
      be equal to num_bits.  */
  void setBits(size_t el, unsigned start_bit, unsigned num_bits,
               const BinData& other) {
    assert(other.size_ == 1);
    assert(other.width_ == num_bits);
    assert(start_bit + num_bits <= width_);
    assert(el < size_);
    copyBits(rawData(el), start_bit, other.rawData(), 0, num_bits);
  }

  /** Replaces a range of bits of a single element with the LSBs of
      a given number.  num_bits can be at most 64.  */
  void setBits64(size_t el, unsigned start_bit, unsigned num_bits,
                 uint64_t bits) {
    assert(start_bit + num_bits <= width_);
    assert(el < size_);
    uint8_t octets[8];
    for (int i = 0; i < 8; i++) {
      octets[i] = bits >> (8 * i);
    }
    copyBits(rawData(el), start_bit, octets, 0, num_bits);
  }

  /** Sets an element as an uint64_t.  Width must be at most 64.  */
  void setElement64(size_t el, uint64_t val) { setBits64(el, 0, width_, val); }

  /** Sets the 0th element as an uint64_t.  Width must be at most 64.  */
  void setElement64(uint64_t val) { setElement64(0, val); }

  /** Create string of coma separated elements represented as hex values */
  QString toString(size_t maxElements = 0);

  /** A helper function copying a range of bits from one arbitrarily-sized
      little-endian element to another.  `dst` and `src` are pointers to the
      start of the corresponding element's raw data, `dst_bit` and `src_bit`
      are starting bit indices.  */
  static void copyBits(uint8_t* dst, unsigned dst_bit, const uint8_t* src,
                       unsigned src_bit, unsigned num_bits);

 private:
  uint32_t width_;
  size_t size_;
  union {
    /** Pointer to raw data iff isInline() is false.  */
    uint8_t* data_;
    /** The array containing raw data iff isInline() is true.  */
    uint8_t idata_[8];
  };
  /** Returns true iff this instance has inline data, ie. stores the raw data
      directly in the instance (as opposed to new[]-allocated memory).  This
      is currently done for single-element arrays of up to 64-bit width.  */
  bool isInline() const { return size_ <= 1 && width_ <= (sizeof idata_ * 8); }
};

}  // namespace data
}  // namespace veles
