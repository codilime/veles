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
#include "util/sampling/fake_sampler.h"

namespace veles {
namespace util {

FakeSampler* FakeSampler::cloneImpl() const { return new FakeSampler(*this); }

size_t FakeSampler::getRealSampleSize() const { return getDataSize(); }

char FakeSampler::getSampleByte(size_t index) const {
  return getDataByte(index);
}

const char* FakeSampler::getData() const { return getRawData(); }

size_t FakeSampler::getFileOffsetImpl(size_t index) const { return index; }

size_t FakeSampler::getSampleOffsetImpl(size_t address) const {
  return address;
}

ISampler::ResampleData* FakeSampler::prepareResample(SamplerConfig* /*sc*/) {
  return nullptr;
}

void FakeSampler::applyResample(ISampler::ResampleData* /*rd*/) {}

void FakeSampler::cleanupResample(ResampleData* /*rd*/) {}

}  // namespace util
}  // namespace veles
