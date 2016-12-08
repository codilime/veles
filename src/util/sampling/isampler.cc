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
#include "assert.h"

#include "util/sampling/isampler.h"


namespace veles {
namespace util {

/*****************************************************************************/
/* Public methods */
/*****************************************************************************/

ISampler::ISampler(const QByteArray &data) :
    data_(data), start_(0),
    sample_size_(0), resample_trigger_(0), initialised_(false) {
  end_ = (data_.size() > 0) ? (data_.size() - 1) : 0;
}

void ISampler::setRange(size_t start, size_t end) {
  assert(!empty());
  assert(end < static_cast<size_t>(data_.size()));
  start_ = start;
  end_ = end;
  if (start - end < resample_trigger_) {
    resample();
  }
  initialised_ = false;
}

std::pair<size_t, size_t> ISampler::getRange() {
  return std::make_pair(start_, end_);
}

void ISampler::setSampleSize(size_t size) {
  sample_size_ = size;
  resample_trigger_ = std::min(sample_size_, resample_trigger_);
  initialised_ = false;
}

void ISampler::setResampleTrigger(size_t sample_size) {
  resample_trigger_ = sample_size;
}

void ISampler::resample() {
  if (samplingRequired()) resampleImpl();
}

size_t ISampler::getSampleSize() {
  if (empty()) return 0;
  if (!initialised_) {
    init();
  }
  if (!samplingRequired()) {
    return getDataSize();
  }
  return getRealSampleSize();
}

size_t ISampler::getRealSampleSize() {
  return getRequestedSampleSize();
}

size_t ISampler::getRequestedSampleSize() {
  return std::min(getDataSize(), sample_size_);
}

size_t ISampler::getFileOffset(size_t index) {
  assert(!empty());
  if (!initialised_) {
    init();
  }
  size_t sample_s = getRequestedSampleSize();
  assert(index < sample_s);
  if (index == 0) return start_;
  if (index == sample_s - 1) return end_;
  if (!samplingRequired()) {
    return index + start_;
  }
  return start_ + getFileOffsetImpl(index);
}

size_t ISampler::getSampleOffset(size_t index) {
  assert(!empty());
  assert(index >= start_);
  assert(index < end_);
  if (!initialised_) {
    init();
  }
  if (index == start_) return 0;
  if (index == end_ - 1) return getSampleSize() - 1;
  if (!samplingRequired()) {
    return index - start_;
  }
  return getSampleOffsetImpl(index - start_);
}

char ISampler::operator[](size_t index) {
  assert(!empty());
  if (!initialised_) {
    init();
  }
  assert(index < getRequestedSampleSize());
  if (!samplingRequired()) {
    return getDataByte(index);
  }
  return getSampleByte(index);
}

const char* ISampler::data() {
  assert(!empty());
  if (!initialised_) {
    init();
  }
  if (!samplingRequired()) {
    return getRawData();
  }
  return getData();
}

bool ISampler::empty() {
  return data_.isEmpty();
}

/*****************************************************************************/
/* Protected methods */
/*****************************************************************************/

ISampler::ISampler(const ISampler& other) : data_(other.data_),
                   start_(other.start_), end_(other.end_),
                   sample_size_(other.sample_size_),
                   resample_trigger_(other.resample_trigger_),
                   initialised_(false) {}

size_t ISampler::getDataSize() {
  return std::min((size_t)data_.size(), end_ - start_);
}

char ISampler::getDataByte(size_t index) {
  return data_[static_cast<int>(start_ + index)];
}

void ISampler::reinitialisationRequired() {
  initialised_ = false;
}

bool ISampler::isInitialised() {
  return initialised_;
}

const char* ISampler::getRawData() {
  return data_.data() + start_;
}

/*****************************************************************************/
/* Private methods */
/*****************************************************************************/

void ISampler::init() {
  if (samplingRequired()) {
    initialiseSample(getRequestedSampleSize());
  }
  initialised_ = true;
}

size_t ISampler::samplingRequired() {
  return ((!empty()) && getRequestedSampleSize() < getDataSize());
}

}  // namespace util
}  // namespace veles
