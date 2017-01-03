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
#ifndef ISAMPLER_H
#define ISAMPLER_H

#include <atomic>
#include <functional>
#include <mutex>
#include <utility>
#include <QByteArray>

namespace veles {
namespace util {

typedef std::mutex SamplerMutex;

/**
 * Abstract interface for Sampler classes.
 * The idea is that any Sampler wraps a byte stream and performs sampling
 * to return a small, representative sample.
 * Specific sample size can be requested by user, but this is only treated
 * as a suggestion and the implementation may return a sample of different
 * size.
 *
 * The actual sampling happens upon first attempt to access sampled data
 * (call to getSampleSize(), operator[] or data() methods).
 *
 * Example usage:
 * MySampler sampler(some_data);
 * MySampler.setSampleSize(2048);
 * for (int i = 0; i < sampler.getSampleSize(); ++i) {
 *   appendByteToMyDataStructure(sampler[i]);
 * }
 */
class ISampler {
 public:
  explicit ISampler(const QByteArray &data);
  virtual ~ISampler() {}

  /**
   * Set the range of bytes from data to use as a base for sampling.
   * Bytes outside given range will be ignored. By default all data is used
   * (start = 0; end = data.size()).
   * May cause re-sampling, invalidates all pointers previously returned by
   * data() method.
   */
  void setRange(size_t start, size_t end);

  /**
   * Get the range of bytes from data used as a base for sampling.
   */
  std::pair<size_t, size_t> getRange();

  /**
   * Request sampler to return the sample of a given size.
   * This is only a suggestion - implementation is allowed to return a sample
   * of different size if requested size does not work well with sampling
   * method!
   *
   * As re-sampling may be costly implementation is not required to create a
   * new sample. It may just reduce sample size by removing bytes outside
   * selected range.
   *
   * This method must be called before using Sampler.
   * May cause re-sampling, invalidates all pointers previously returned by
   * data() method.
   */
  void setSampleSize(size_t size);

  /**
   * Request sampler to automatically re-sample once the effective sample
   * size gets smaller than sample_size (perhaps due to shrinking sample
   * range with setRange).
   *
   * This does not guarantee that the sample size will never be smaller than
   * sample_size!
   */
  void setResampleTrigger(size_t sample_size);

  /**
   * Take new sample from underlying file.
   * Invalidates all pointers previously returned by data() method.
   */
  void resample();

  /**
   * Get the size of the sample.
   * This is the real sample size, which can be different than size requested
   * with setSampleSize(). User should always call this method to get the size
   * of the sample (no assumptions should be made).
   */
  size_t getSampleSize();

  /**
   * Get the index in file corresponding to nth byte in sample.
   * As sampler may generate output based on a function of multiple bytes
   * the semantic of what is actually returned is not very clearly defined.
   * However it should be the location in file that somehow corresponds to
   * the given byte.
   * There are two specific indexes with guaranteed values:
   * - getFileOffset(0) is always first address in underlying data,
   * - getFileOffset(getSampleSize() - 1) the last address.
   */
  size_t getFileOffset(size_t index);

  /**
   * Get the index in sample corresponding to given address in file.
   * As sampler may generate output based on a function of multiple bytes
   * the semantic of what is actually returned is not very clearly defined.
   * However it should be the location in file that somehow corresponds to
   * the given byte.
   * There are two specific indexes with guaranteed values:
   * - getSampleOffset(getRange().first) is always 0
   * - getSampleOffset(getRange().second - 1) is equal to getSampleSize() - 1
   */
  size_t getSampleOffset(size_t index);

  /**
   * Return n-th byte from the sample.
   */
  char operator[](size_t index);

  /**
   * Return whole sample as a simple array. The size of the array is
   * getSampleSize().
   * Sampler keeps ownership of this array. Deallocating the Sampler or
   * performing any operation that causes re-sampling (setRange, setSampleSize)
   * invalidates the pointer.
   */
  const char* data();

  /**
   * Return true if sample is empty.
   * Generally true if underlying data is empty.
   */
  bool empty();

  /**
   * Get lock protecting sampler data.
   * As long as the lock is held no resampling will happen, meaning all methods
   * retrieving sampled data, sample size, etc will return consistent results.
   * In particular pointer returned by data() will not be invalidated while
   * the lock is kept.
   */
  std::unique_lock<SamplerMutex> lock();

  /**
   * Wait until sampler has finished resampling.
   * This waits until all resample operations are done, including those
   * scheduled after the wait has started!
   */
  void wait();

  /**
   * Aquire sampler lock after resampling is finished.
   * All points mentioned in wait() and lock() docstrings apply.
   */
  std::unique_lock<SamplerMutex> waitAndLock();

  /**
   * Return true if there is no resampling currently going.
   * Note that while the check is atomic there may be a resampling scheduled
   * just after it is done. To be sure nothing is going on you should hold
   * the sampler lock while calling this.
   */
  bool isFinished();

  /**
   * Register a callback that will be called when resampling is finished.
   * There is no guarantee in which thread the callback will be called.
   * If lock_required is true the callback will be called while keeping
   * the sampler lock.
   * If multiple callbacks are registered they will be called in reverse
   * registration order (stack).
   */
  void registerResampleCallback(std::function<void()> cb, bool requires_lock);

  /**
   * Remove all registered resample callbacks.
   */
  void clearResampleCallbacks();

  /**
   * Create a copy of this sampler.
   * This will block until all resampling is done and create a copy afterwards.
   */
  virtual ISampler* clone() = 0;

 protected:
  /**
   * Derive this struct if you want to pass any data between resample and
   * update operations.
   */
  struct ResampleData {};

  /**
   * Contain information required by ISampler to provide data to sampler.
   * This is required if working on sampler in different state then the current
   * state (ex. when asynchronously resyncing after setRange).
   */
  typedef std::pair<size_t, size_t> SamplerConfig;

  /**
   * Return the size of the data to sample.
   * This already takes into account limiting the size of input with setRange().
   * If sc is provided it uses the range represented by sc instead of this
   * stored by sampler.
   */
  size_t getDataSize(SamplerConfig *sc = nullptr);

  /**
   * Get n-th byte of input data.
   * The data is 0 indexed and has size given by getDataSize(). If setRange()
   * was used data is re-indexed internally to still be 0 indexed.
   * If sc is provided it uses the range represented by sc instead of this
   * stored by sampler.
   */
  char getDataByte(size_t index, SamplerConfig *sc = nullptr);

  /**
   * Called by Sampler implementation to trigger re-initialisation of Sampler.
   * This will cause isInitialised() to return false. Upon next data access
   * the class will be re-initialised (causing call to initialiseSample()), as
   * well as re-initialisation of some variables in ISampler.
   */
  void reinitialisationRequired();

  bool isInitialised();

  /**
   * Return the size of sample requested by user (with setSampleSize).
   */
  size_t getRequestedSampleSize();

  /**
   * Return real size of the sample. This method must be overriden by any
   * implementation, that may return sample of different size than
   * getRequestedSampleSize().
   */
  virtual size_t getRealSampleSize();

  /**
   * Return the input data as simple array. Size of array is getDataSize().
   * If sc is provided it uses the range represented by sc instead of this
   * stored by sampler.
   */
  const char* getRawData(SamplerConfig *sc = nullptr);

  ISampler(const ISampler& other);

 private:
  /**
   * Do any work that is require to initialise internal Sampler state here.
   * May be called multiple times in object lifecycle (meaning that Sampler
   * should be re-initialised).
   */
  virtual void initialiseSample(size_t size) = 0;

  /**
   * Get n-th byte of sample. This will only be called after initialiseSample().
   */
  virtual char getSampleByte(size_t index) = 0;

  /**
   * Get whole sample as simple array. The size of the array must be the
   * result of getRealSampleSize(). The implementation is required to manage
   * the array, including deleting it on Sampler deallocation and
   * re-initialisation.
   */
  virtual const char* getData() = 0;

  /**
   * Implementation of getFileOffset public method.
   */
  virtual size_t getFileOffsetImpl(size_t index) = 0;

  /**
   * Implementation of getSampleOffset public method.
   */
  virtual size_t getSampleOffsetImpl(size_t index) = 0;

  /**
   * Prepare resampled data.
   * This may be called asynchronously in a different thread and should neither
   * modify nor rely on any mutable state of sampler (multiple prepareResample
   * calls may be processed at the same time). Returned ResampleData* will
   * later be passed to applyResample method.
   * Any call to method accepting SamplerConfig (getDataSize(),
   * getRawData(), etc) should pass the provided SamplerConfig.
   */
  virtual ResampleData* prepareResample(SamplerConfig *sc) = 0;

  /**
   * Apply ResampleData prepared by prepareResample method.
   * This will be executed while holding sampler lock and should modify
   * sampler state by applying whatever was produced by prepareResample.
   * Ideally this should be a cheap operation (as it will be executed
   * synchronously).
   */
  virtual void applyResample(ResampleData *rd) = 0;

  /**
   * Implementation of clone method.
   */
  virtual ISampler* cloneImpl() = 0;

  void init();
  size_t samplingRequired();

  const QByteArray &data_;
  size_t start_, end_, sample_size_, resample_trigger_;
  bool initialised_;

  SamplerMutex sampler_mutex_;
  std::atomic<int> current_version_, requested_version_;
};

}  // namespace util
}  // namespace veles

#endif
