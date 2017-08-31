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

#include <QByteArray>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <map>
#include <mutex>
#include <utility>

namespace veles {
namespace util {

typedef std::recursive_mutex SamplerMutex;
typedef std::condition_variable_any SamplerConditionVariable;
typedef std::function<void()> ResampleCallback;
typedef int ResampleCallbackId;

/**
 * Abstract interface for Sampler classes.
 * The idea is that any Sampler wraps a byte stream and performs sampling
 * to return a small, representative sample.
 * Specific sample size can be requested by user, but this is only treated
 * as a suggestion and the implementation may return a sample of different
 * size.
 *
 * By default the sampler works synchronously - any call that creates
 * the need to re-create sample ("resample") will perform the resampling
 * operation immediately. As this can be rather expensive time-wise
 * the sampler can be changed to asynchronous mode. In that case the resampling
 * is performed in a separate thread and a set of registered callbacks is
 * called once it's done.
 *
 * When working in asynchronous mode it is recommended to perform any
 * operations on the data while keeping the mutex returned by sampler.lock().
 * Otherwise the sample we're looking at can suddenly change leading to
 * inconsistencies.
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
  explicit ISampler(const QByteArray& data);
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
   * This method must be called before first using Sampler!
   * May cause re-sampling, invalidates all pointers previously returned by
   * data() method.
   */
  void setSampleSize(size_t size);

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
   * There are three specific indexes with guaranteed values:
   * - getFileOffset(0) is always first address in underlying data,
   * - getFileOffset(getSampleSize() - 1) the last address.
   * - getFileOffset(getSampleSize()) the last address + 1.
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
  bool empty() const;

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
   * This does not use any synchronisation and may give false positives
   * or false negatives unless you're holding sampler lock when calling this.
   */
  bool isFinished() const;

  /**
   * Register a callback that will be called when resampling is finished.
   * There is no guarantee in which thread the callback will be called.
   * Thread calling callbacks will keep sampler lock() while doing so,
   * so it's prefered if they're reasonably cheap.
   * If multiple callbacks are registered they will be called in reverse
   * registration order (stack).
   * This method needs to acquire sampler lock, so it may block.
   * Returned value is callback id, that can be later used to remove this
   * callback.
   */
  ResampleCallbackId registerResampleCallback(const ResampleCallback& cb);

  /**
   * Remove resample callback with a given id.
   */
  void removeResampleCallback(ResampleCallbackId cb_id);

  /**
   * Remove all registered resample callbacks.
   * This method needs to acquire sampler lock, so it may block.
   */
  void clearResampleCallbacks();

  /**
   * Create a copy of this sampler.
   * This will block until all resampling is done and create a copy afterwards.
   */
  ISampler* clone();

  /**
   * Set if you want to allow asynchronous resampling.
   * If true any resampling operation will be run asynchronously, you need to
   * use locks and register callbacks.
   * Otherwise all calls are synchronous and blocking. No locks are being used
   * in this case and calling any methods from multiple threads will have
   * undefined behaviour. No callbacks will be executed.
   * Calling this method while any other method is being run (synchronously
   * or asynchronously) will result in undefined behaviour.
   * Default value is false.
   */
  void allowAsynchronousResampling(bool allow);

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
  struct SamplerConfig {
    size_t start, end, sample_size;
  };

  /**
   * Return the size of the data to sample.
   * This already takes into account limiting the size of input with setRange().
   * If sc is provided it uses the range represented by sc instead of this
   * stored by sampler.
   * This does not do any locking, which doesn't matter if sc != nullptr and
   * may be pretty bad otherwise.
   */
  size_t getDataSize(SamplerConfig* sc = nullptr) const;

  /**
   * Get n-th byte of input data.
   * The data is 0 indexed and has size given by getDataSize(). If setRange()
   * was used data is re-indexed internally to still be 0 indexed.
   * If sc is provided it uses the range represented by sc instead of this
   * stored by sampler.
   */
  char getDataByte(size_t index, SamplerConfig* sc = nullptr) const;

  /**
   * Return the size of sample requested by user (with setSampleSize).
   */
  size_t getRequestedSampleSize(SamplerConfig* sc = nullptr) const;

  /**
   * Return real size of the sample. This method must be overridden by any
   * implementation, that may return sample of different size than
   * getRequestedSampleSize().
   */
  virtual size_t getRealSampleSize() const;

  /**
   * Return the input data as simple array. Size of array is getDataSize().
   * If sc is provided it uses the range represented by sc instead of this
   * stored by sampler.
   */
  const char* getRawData(SamplerConfig* sc = nullptr) const;

  ISampler(const ISampler& other);

 private:
  /**
   * Get n-th byte of sample.
   */
  virtual char getSampleByte(size_t index) const = 0;

  /**
   * Get whole sample as simple array. The size of the array must be the
   * result of getRealSampleSize(). The implementation is required to manage
   * the array, including deleting it on Sampler deallocation and
   * re-initialization.
   */
  virtual const char* getData() const = 0;

  /**
   * Implementation of getFileOffset public method.
   */
  virtual size_t getFileOffsetImpl(size_t index) const = 0;

  /**
   * Implementation of getSampleOffset public method.
   */
  virtual size_t getSampleOffsetImpl(size_t index) const = 0;

  /**
   * Prepare resampled data.
   * This may be called asynchronously in a different thread and should neither
   * modify nor rely on any mutable state of sampler (multiple prepareResample
   * calls may be processed at the same time). Returned ResampleData* will
   * later be passed to applyResample method.
   * Any call to method accepting SamplerConfig (getDataSize(),
   * getRawData(), etc) should pass the provided SamplerConfig.
   */
  virtual ResampleData* prepareResample(SamplerConfig* sc) = 0;

  /**
   * Apply ResampleData prepared by prepareResample method.
   * This will be executed while holding sampler lock and should modify
   * sampler state by applying whatever was produced by prepareResample.
   * Ideally this should be a cheap operation (as it will be executed
   * synchronously).
   * This method takes ownership of passed ResampleData and is responsible
   * for ultimately freeing up the memory.
   */
  virtual void applyResample(ResampleData* rd) = 0;

  /**
   * Delete ResampleData prepared by prepareResample method.
   * This will be called instead of applyResample if the prepared ResampleData
   * is outdated, etc. The method should delete the ResampleData provided
   * doing any necessary cleanup.
   */
  virtual void cleanupResample(ResampleData* rd) = 0;

  /**
   * Implementation of clone method.
   */
  virtual ISampler* cloneImpl() const = 0;

  bool samplingRequired(SamplerConfig* sc = nullptr);
  void applySamplerConfig(SamplerConfig* sc);
  void runResample(SamplerConfig* sc);
  void resampleAsync(int target_version, SamplerConfig* sc);

  const QByteArray& data_;
  size_t start_, end_, sample_size_;
  bool allow_async_;

  SamplerMutex sampler_mutex_;
  SamplerConditionVariable sampler_condition_;
  SamplerConfig last_config_;
  std::atomic<int> current_version_, requested_version_;
  ResampleCallbackId next_cb_id_;
  std::map<ResampleCallbackId, ResampleCallback> callbacks_;
};

}  // namespace util
}  // namespace veles
