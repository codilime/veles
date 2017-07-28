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
#include "gmock/gmock.h"
#include "util/sampling/isampler.h"

namespace veles {
namespace util {

class MockSampler : public ISampler {
 public:
  explicit MockSampler(const QByteArray& data) : ISampler(data) {}
  MOCK_CONST_METHOD0(cloneImpl, ISampler*());
  MOCK_CONST_METHOD0(getRealSampleSize, size_t());
  MOCK_CONST_METHOD1(getSampleByte, char(size_t index));
  MOCK_CONST_METHOD0(getData, const char*());
  MOCK_CONST_METHOD1(getFileOffsetImpl, size_t(size_t index));
  MOCK_CONST_METHOD1(getSampleOffsetImpl, size_t(size_t index));
  MOCK_METHOD1(prepareResample, ResampleData*(SamplerConfig*));
  MOCK_METHOD1(applyResample, void(ResampleData*));
  MOCK_METHOD1(cleanupResample, void(ResampleData*));

  size_t proxy_getDataSize() { return getDataSize(); }
  char proxy_getDataByte(size_t index) { return getDataByte(index); }
};

class MockCallback {
 public:
  int getCallCount();
  void resetCallCount();
  void operator()();

 private:
  int calls_;
};

// Prepare sample data for sampler
QByteArray prepare_data(size_t size);

}  // namespace util
}  // namespace veles
