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
#include <cassert>

#include "util/concurrency/threadpool.h"
#include "util/sampling/isampler.h"

namespace veles {
namespace util {

/*****************************************************************************/
/* Public methods */
/*****************************************************************************/

ISampler::ISampler(const QByteArray& data)
    : data_(data),
      start_(0),
      sample_size_(0),
      allow_async_(false),
      current_version_(0),
      requested_version_(0),
      next_cb_id_(0) {
  end_ = static_cast<size_t>(data_.size());
  last_config_.start = start_;
  last_config_.end = end_;
  last_config_.sample_size = sample_size_;
}

void ISampler::setRange(size_t start, size_t end) {
  assert(!empty());
  assert(end <= static_cast<size_t>(data_.size()));
  auto lc = lock();
  last_config_.start = start;
  last_config_.end = end;
  runResample(new SamplerConfig(last_config_));
}

std::pair<size_t, size_t> ISampler::getRange() {
  auto lc = lock();
  return std::make_pair(start_, end_);
}

void ISampler::setSampleSize(size_t size) {
  auto lc = lock();
  last_config_.sample_size = size;
  runResample(new SamplerConfig(last_config_));
}

void ISampler::resample() {
  auto lc = lock();
  runResample(new SamplerConfig(last_config_));
}

size_t ISampler::getSampleSize() {
  if (empty()) {
    return 0;
  }
  auto lc = lock();
  if (!samplingRequired()) {
    return getDataSize();
  }
  return getRealSampleSize();
}

size_t ISampler::getFileOffset(size_t index) {
  assert(!empty());
  auto lc = lock();
  size_t sample_s = getRequestedSampleSize();
  assert(index <= sample_s);
  if (index == 0) {
    return start_;
  }
  if (index == sample_s - 1) {
    return end_ - 1;
  }
  if (index == sample_s) {
    return end_;
  }
  if (!samplingRequired()) {
    return index + start_;
  }
  return start_ + getFileOffsetImpl(index);
}

size_t ISampler::getSampleOffset(size_t index) {
  assert(!empty());
  auto lc = lock();
  assert(index >= start_);
  assert(index < end_);
  if (index == start_) {
    return 0;
  }
  if (index == end_ - 1) {
    return getSampleSize() - 1;
  }
  if (!samplingRequired()) {
    return index - start_;
  }
  return getSampleOffsetImpl(index - start_);
}

char ISampler::operator[](size_t index) {
  assert(!empty());
  auto lc = lock();
  assert(index < getRequestedSampleSize());
  if (!samplingRequired()) {
    return getDataByte(index);
  }
  return getSampleByte(index);
}

const char* ISampler::data() {
  assert(!empty());
  auto lc = lock();
  if (!samplingRequired()) {
    return getRawData();
  }
  return getData();
}

bool ISampler::empty() const { return data_.isEmpty(); }

std::unique_lock<SamplerMutex> ISampler::lock() {
  return std::unique_lock<SamplerMutex>(sampler_mutex_);
}

void ISampler::wait() {
  if (!allow_async_) {
    return;
  }
  waitAndLock();
}

std::unique_lock<SamplerMutex> ISampler::waitAndLock() {
  auto lc = lock();
  if (!allow_async_) {
    return lc;
  }
  while (!isFinished()) {
    sampler_condition_.wait(lc);
  }
  return lc;
}

bool ISampler::isFinished() const {
  return (current_version_.load() == requested_version_.load());
}

ResampleCallbackId ISampler::registerResampleCallback(
    const ResampleCallback& cb) {
  auto lc = lock();
  ResampleCallbackId id = next_cb_id_++;
  callbacks_[id] = cb;
  return id;
}

void ISampler::removeResampleCallback(ResampleCallbackId cb_id) {
  auto lc = lock();
  callbacks_.erase(cb_id);
}

void ISampler::clearResampleCallbacks() {
  auto lc = lock();
  callbacks_.clear();
}

ISampler* ISampler::clone() {
  auto lc = waitAndLock();
  ISampler* result = cloneImpl();
  result->resample();
  result->wait();
  return result;
}

void ISampler::allowAsynchronousResampling(bool allow) { allow_async_ = allow; }

/*****************************************************************************/
/* Protected methods */
/*****************************************************************************/

ISampler::ISampler(const ISampler& other)
    : data_(other.data_),
      start_(other.start_),
      end_(other.end_),
      sample_size_(other.sample_size_),
      allow_async_(other.allow_async_),
      last_config_(other.last_config_),
      current_version_(0),
      requested_version_(0),
      callbacks_(other.callbacks_) {}

size_t ISampler::getDataSize(SamplerConfig* sc) const {
  if (sc == nullptr) {
    return std::min<size_t>(data_.size(), end_ - start_);
  }
  return std::min<size_t>(data_.size(), sc->end - sc->start);
}

char ISampler::getDataByte(size_t index, SamplerConfig* sc) const {
  size_t start = (sc == nullptr) ? start_ : sc->start;
  return data_[static_cast<int>(start + index)];
}

size_t ISampler::getRealSampleSize() const { return getRequestedSampleSize(); }

size_t ISampler::getRequestedSampleSize(SamplerConfig* sc) const {
  size_t sample_size = (sc == nullptr) ? sample_size_ : sc->sample_size;
  return std::min(getDataSize(sc), sample_size);
}

const char* ISampler::getRawData(SamplerConfig* sc) const {
  size_t start = (sc == nullptr) ? start_ : sc->start;
  return data_.data() + start;
}

/*****************************************************************************/
/* Private methods */
/*****************************************************************************/

bool ISampler::samplingRequired(SamplerConfig* sc) {
  return !empty() && getRequestedSampleSize(sc) < getDataSize(sc);
}

void ISampler::applySamplerConfig(SamplerConfig* sc) {
  if (sc != nullptr) {
    start_ = sc->start;
    end_ = sc->end;
    sample_size_ = sc->sample_size;
  }
}

void ISampler::runResample(SamplerConfig* sc) {
  if (allow_async_) {
    if (!samplingRequired(sc)) {
      auto lc = lock();
      current_version_ = ++requested_version_;
      applySamplerConfig(sc);
      for (auto i = callbacks_.rbegin(); i != callbacks_.rend(); ++i) {
        (i->second)();
      }
      lc.unlock();
      delete sc;
      return;
    }
    threadpool::runTask(
        "visualization",
        std::bind(&ISampler::resampleAsync, this, ++requested_version_, sc));
  } else {
    if (samplingRequired(sc)) {
      ResampleData* prepared = prepareResample(sc);
      applyResample(prepared);
    }
    applySamplerConfig(sc);
    delete sc;
  }
}

void ISampler::resampleAsync(int target_version, SamplerConfig* sc) {
  if (target_version < requested_version_.load()) {
    return;
  }
  ResampleData* prepared = prepareResample(sc);
  auto lc = lock();
  if (target_version > current_version_) {
    applyResample(prepared);
    applySamplerConfig(sc);
    current_version_ = target_version;
    for (auto i = callbacks_.rbegin(); i != callbacks_.rend(); ++i) {
      (i->second)();
    }
    lc.unlock();
    delete sc;
    sampler_condition_.notify_all();
  } else {
    lc.unlock();
    cleanupResample(prepared);
    delete sc;
  }
}

}  // namespace util
}  // namespace veles
