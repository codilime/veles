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
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iterator>
#include <random>
#include <set>

#include "util/sampling/uniform_sampler.h"


namespace veles {
namespace util {

UniformSampler::UniformSampler(const QByteArray &data) :
    ISampler(data), window_size_(0), use_default_window_size_(true),
    buffer_(nullptr) {}

UniformSampler::~UniformSampler() {
  if (buffer_ != nullptr) {
    delete[] buffer_;
  }
}

UniformSampler::UniformSampler(const UniformSampler& other) :
    ISampler(other), window_size_(other.window_size_), buffer_(nullptr) {}

UniformSampler* UniformSampler::clone() {
  return new UniformSampler(*this);
}

void UniformSampler::setWindowSize(size_t size) {
  window_size_ = size;
  use_default_window_size_ = size == 0;
  if (buffer_ != nullptr) {
    delete[] buffer_;
    buffer_ = nullptr;
  }
  reinitialisationRequired();
}

size_t UniformSampler::getRealSampleSize() {
  if (!isInitialised()) {
    return 0;
  }
  return window_size_ * windows_count_;
}

void UniformSampler::initialiseSample(size_t size) {
  // default size == sqrt(sample size)
  if (buffer_ != nullptr) {
    delete[] buffer_;
    buffer_ = nullptr;
  }

  if (use_default_window_size_ || window_size_ == 0) {
    window_size_ = (size_t)floor(sqrt(size));
  }
  windows_count_ = (size_t)floor(size / window_size_);

  // Algorithm:
  // First let's mark windows_count_ as m, window_size_ as k and
  // getDataSize() as n.
  // 1. Take m numbers from {0, 1 ... n - m*k} with repetitions,
  //    marked as (c_i) sequence.
  // 2. Sort (c_i) sequence.
  // 3. Produce the result indices (d_i) in the following way:
  //    d_i = c_i + i*k
  //
  // And why that works:
  // - The smallest value of d_0 is 0.
  // - The largest value of d_{m-1} is
  //   n - m*k + (m-1)*k = n - k
  //   which is exactly what we want because the piece length is k.
  // - For each i the distance d_{i+1}-d_i >= k.
  size_t max_index = getDataSize() - windows_count_ * window_size_;
  std::default_random_engine generator;
  std::uniform_int_distribution<size_t> distribution(0, max_index);
  windows_.resize(windows_count_);
  for (size_t i = 0; i < windows_count_; ++i) {
    windows_[i] = distribution(generator);
  }
  std::sort(windows_.begin(), windows_.end());
  for (size_t i = 0; i < windows_count_; ++i) {
    windows_[i] += i*window_size_;
  }
}

char UniformSampler::getSampleByte(size_t index) {
  if (buffer_ != nullptr) {
    return buffer_[index];
  }
  size_t base_index = windows_[index / window_size_];
  return getDataByte(base_index + (index % window_size_));
}

const char* UniformSampler::getData() {
  if (buffer_ == nullptr) {
    size_t size = getSampleSize();
    const char *raw_data = getRawData();
    char *tmp_buffer = new char[size];
    for (size_t i=0; i < size; ++i) {
      size_t base_index = windows_[i / window_size_];
      tmp_buffer[i] = raw_data[base_index + (i % window_size_)];
    }
    buffer_ = tmp_buffer;
  }
  return buffer_;
}

size_t UniformSampler::getFileOffsetImpl(size_t index) {
  size_t base_index = windows_[index / window_size_];
  return base_index + (index % window_size_);
}

size_t UniformSampler::getSampleOffsetImpl(size_t address) {
  auto previous_window = std::lower_bound(windows_.begin(), windows_.end(),
                                          address);
  size_t base_index = static_cast<size_t>(
    std::distance(windows_.begin(), previous_window) * window_size_);
  return base_index + std::min(window_size_ - 1, address - (*previous_window));
}

void UniformSampler::resampleImpl() {
}

}  // namespace util
}  // namespace veles
