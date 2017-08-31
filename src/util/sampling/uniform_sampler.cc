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

/*****************************************************************************/
/* Public methods */
/*****************************************************************************/

UniformSampler::UniformSampler(const QByteArray& data)
    : ISampler(data),
      window_size_(0),
      use_default_window_size_(true),
      buffer_(nullptr) {}

UniformSampler::~UniformSampler() { delete[] buffer_; }

void UniformSampler::setWindowSize(size_t size) {
  auto lc = waitAndLock();
  window_size_ = size;
  use_default_window_size_ = size == 0;
  resample();
}

/*****************************************************************************/
/* Private methods */
/*****************************************************************************/

UniformSampler::UniformSampler(const UniformSampler& other)
    : ISampler(other), window_size_(other.window_size_), buffer_(nullptr) {}

char UniformSampler::getSampleByte(size_t index) const {
  if (buffer_ != nullptr) {
    return buffer_[index];
  }
  size_t base_index = windows_[index / window_size_];
  return getDataByte(base_index + (index % window_size_));
}

const char* UniformSampler::getData() const { return buffer_; }

size_t UniformSampler::getRealSampleSize() const {
  return window_size_ * windows_count_;
}

size_t UniformSampler::getFileOffsetImpl(size_t index) const {
  size_t base_index = windows_[index / window_size_];
  return base_index + (index % window_size_);
}

size_t UniformSampler::getSampleOffsetImpl(size_t address) const {
  // we want the last window less or equal to address (or first window if
  // no such window exists)
  if (address < windows_[0]) {
    return 0;
  }
  auto previous_window =
      std::upper_bound(windows_.begin(), windows_.end(), address);
  if (previous_window != windows_.begin()) {
    --previous_window;
  }
  size_t base_index = static_cast<size_t>(
      std::distance(windows_.begin(), previous_window) * window_size_);
  return base_index + std::min(window_size_ - 1, address - (*previous_window));
}

ISampler::ResampleData* UniformSampler::prepareResample(SamplerConfig* sc) {
  size_t size = getRequestedSampleSize(sc);
  size_t window_size = window_size_;
  if (use_default_window_size_ || window_size_ == 0) {
    window_size = static_cast<size_t>(floor(sqrt(size)));
  }
  size_t windows_count = size / window_size;
  size = window_size * windows_count;
  std::vector<size_t> windows(windows_count);

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
  size_t max_index = getDataSize(sc) - windows_count * window_size;
  std::default_random_engine generator;
  std::uniform_int_distribution<size_t> distribution(0, max_index);
  for (size_t i = 0; i < windows_count; ++i) {
    windows[i] = distribution(generator);
  }
  std::sort(windows.begin(), windows.end());
  for (size_t i = 0; i < windows_count; ++i) {
    windows[i] += i * window_size;
  }

  // Now let's create data array (it's more efficient to do it here,
  // than later calculate values)
  const char* raw_data = getRawData(sc);
  auto* tmp_buffer = new char[size];
  for (size_t i = 0; i < size; ++i) {
    size_t base_index = windows[i / window_size];
    tmp_buffer[i] = raw_data[base_index + (i % window_size)];
  }

  auto* rd = new UniformSamplerResampleData;
  rd->window_size = window_size;
  rd->windows_count = windows_count;
  rd->windows = std::move(windows);
  rd->data = tmp_buffer;
  return rd;
}

void UniformSampler::applyResample(ResampleData* rd) {
  delete[] buffer_;
  auto* usrd = static_cast<UniformSamplerResampleData*>(rd);
  window_size_ = usrd->window_size;
  windows_count_ = usrd->windows_count;
  windows_ = std::move(usrd->windows);
  buffer_ = usrd->data;
  delete usrd;
}

void UniformSampler::cleanupResample(ResampleData* rd) {
  auto* usrd = static_cast<UniformSamplerResampleData*>(rd);
  delete[] usrd->data;
  delete usrd;
}

UniformSampler* UniformSampler::cloneImpl() const {
  return new UniformSampler(*this);
}

}  // namespace util
}  // namespace veles
