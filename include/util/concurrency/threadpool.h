/*
 * Copyright 2017 CodiLime
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
#ifndef VELES_UTIL_CONCURRENCY_THREADPOOL_H
#define VELES_UTIL_CONCURRENCY_THREADPOOL_H

#include <string>
#include <functional>

namespace veles {
namespace util {
namespace threadpool {

/**
 * Globally accessible thread pool with workers split into
 * topics (worker groups).
 */

typedef std::function<void()> Task;

enum class SchedulingResult {
  SCHEDULED,
  ERR_UNKNOWN_TOPIC,
  ERR_NO_WORKERS,
  ERR_UNKNOWN
};

/**
 * Create a new topic and set number of workers for it.
 */
void createTopic(std::string topic, size_t workers);

/**
 * Create a topic without any workers and run tasks in thread calling runTask().
 * This is meant for testing, when we may not want to actually spawn threads.
 */
void mockTopic(std::string topic);

/**
 * Schedule a job to be run on one of worker threads assigned to a given topic.
 * The job is run asynchronously, use callbacks or similar to communicate its
 * result.
 */
SchedulingResult runTask(std::string topic, Task t);


}  // namespace threadpool
}  // namespace util
}  // namespace veles

#endif
