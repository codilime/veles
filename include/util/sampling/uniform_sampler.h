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

#include <vector>
#include "util/sampling/isampler.h"

namespace veles {
namespace util {

class UniformSampler : public ISampler {
 public:
  explicit UniformSampler(const QByteArray& data);
  ~UniformSampler() override;

  void setWindowSize(size_t size);

 private:
  struct UniformSamplerResampleData : public ResampleData {
    size_t window_size, windows_count;
    std::vector<size_t> windows;
    char* data;
  };

  UniformSampler(const UniformSampler& other);
  char getSampleByte(size_t index) const override;
  const char* getData() const override;
  size_t getRealSampleSize() const override;
  size_t getFileOffsetImpl(size_t index) const override;
  size_t getSampleOffsetImpl(size_t address) const override;
  ResampleData* prepareResample(SamplerConfig* sc) override;
  void applyResample(ResampleData* rd) override;
  void cleanupResample(ResampleData* rd) override;
  UniformSampler* cloneImpl() const override;

  size_t window_size_, windows_count_;
  bool use_default_window_size_;
  std::vector<size_t> windows_;
  char* buffer_;
};

}  // namespace util
}  // namespace veles
