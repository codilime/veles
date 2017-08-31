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

#include "util/sampling/uniform_sampler.h"

#include "mock_sampler.h"

namespace veles {
namespace util {

TEST(UniformSampler, testGetData) {
  auto data = prepare_data(100);
  UniformSampler sampler(data);
  sampler.setSampleSize(20);
  ASSERT_EQ(20u, sampler.getSampleSize());
  auto sample = sampler.data();
  char prev = sample[0], curr = sample[1];
  size_t i = 2;
  do {
    ASSERT_LT(prev, data[99]);
    ASSERT_LE(curr, data[99]);
    ASSERT_LT(prev, curr);
    prev = curr;
    curr = sample[i];
    ASSERT_EQ(curr, sampler[i]);
    i += 1;
  } while (i < 20);
}

TEST(UniformSampler, testOffsets) {
  auto data = prepare_data(100);
  UniformSampler sampler(data);
  sampler.setSampleSize(20);
  ASSERT_EQ(20u, sampler.getSampleSize());
  ASSERT_EQ(0u, sampler.getFileOffset(0));
  size_t prev = 0;
  ASSERT_EQ(0u, sampler.getSampleOffset(0));
  for (size_t i = 1; i < 20; ++i) {
    size_t curr = sampler.getFileOffset(i);
    ASSERT_LT(prev, curr);
    ASSERT_LE(curr, 99u);
    ASSERT_EQ(i, sampler.getSampleOffset(curr));
    // ISampler interface doesn't require this condition to be true for
    // any value. However this should be true for UniformSampler
    // implementation, except for first and last byte in sample (which have
    // values defined by getFileOffset() interface)
    if (curr < 19) {
      ASSERT_EQ(data[static_cast<int>(curr)], sampler[i]);
    }
  }
  prev = 0;
  for (size_t i = 0; i < 99; ++i) {
    size_t curr = sampler.getSampleOffset(i);
    ASSERT_GE(curr, 0u);
    ASSERT_LE(curr, 99u);
    ASSERT_LE(prev, curr);
    prev = curr;
  }
}

}  // namespace util
}  // namespace veles
