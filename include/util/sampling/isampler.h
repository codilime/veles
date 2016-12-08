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

#include <utility>
#include <QByteArray>

namespace veles {
namespace util {

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
   */
  size_t getFileOffset(size_t index);

  /**
   * Get the index in sample corresponding to given address in file.
   * As sampler may generate output based on a function of multiple bytes
   * the semantic of what is actually returned is not very clearly defined.
   * However it should be the location in file that somehow corresponds to
   * the given byte.
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

  virtual ISampler* clone() = 0;

 protected:
  /**
   * Return the size of the data to sample.
   * This already takes into account limiting the size of input with setRange().
   */
  size_t getDataSize();

  /**
   * Get n-th byte of input data.
   * The data is 0 indexed and has size given by getDataSize(). If setRange()
   * was used data is re-indexed internally to still be 0 indexed.
   */
  char getDataByte(size_t index);

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
   */
  const char* getRawData();

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
   * Implementation of resample method.
   */
  virtual void resampleImpl() = 0;

  void init();
  size_t samplingRequired();

  const QByteArray &data_;
  size_t start_, end_, sample_size_, resample_trigger_;
  bool initialised_;
};

}  // namespace util
}  // namespace veles

#endif
