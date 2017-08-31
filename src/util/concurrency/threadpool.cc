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
#include "util/concurrency/threadpool.h"
#include <condition_variable>
#include <deque>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

namespace veles {
namespace util {
namespace threadpool {

struct TopicInfo {
  std::mutex mutex;
  std::condition_variable cv;
  std::vector<std::thread> workers;
  std::deque<Task> tasks;
  bool mock;
};

std::map<std::string, TopicInfo*> topics_;
std::mutex map_mutex_;

void threadFunction(TopicInfo* ti) {
  while (true) {
    std::unique_lock<std::mutex> lc(ti->mutex);
    while (ti->tasks.empty()) {
      ti->cv.wait(lc);
    }
    Task t = ti->tasks.front();
    ti->tasks.pop_front();
    lc.unlock();
    t();
  }
}

void createTopic(const std::string& topic, size_t workers) {
  std::unique_lock<std::mutex> lc(map_mutex_);
  if (topics_.find(topic) != topics_.end()) {
    return;
  }
  auto* ti = new TopicInfo();
  ti->mock = false;
  topics_[topic] = ti;
  std::unique_lock<std::mutex> topic_lc(ti->mutex);
  for (size_t i = 0; i < workers; ++i) {
    ti->workers.emplace_back(threadFunction, ti);
  }
}

void mockTopic(const std::string& topic) {
  std::unique_lock<std::mutex> lc(map_mutex_);
  if (topics_.find(topic) != topics_.end()) {
    return;
  }
  auto* ti = new TopicInfo();
  ti->mock = true;
  topics_[topic] = ti;
}

SchedulingResult runTask(const std::string& topic, const Task& t) {
  std::unique_lock<std::mutex> lc(map_mutex_);
  if (topics_.find(topic) == topics_.end()) {
    return SchedulingResult::ERR_UNKNOWN_TOPIC;
  }
  TopicInfo* ti = topics_[topic];
  std::unique_lock<std::mutex> topic_lc(ti->mutex);
  lc.unlock();
  if (ti->mock) {
    t();
    return SchedulingResult::SCHEDULED;
  }
  if (ti->workers.empty()) {
    return SchedulingResult::ERR_NO_WORKERS;
  }
  ti->tasks.push_back(t);
  topic_lc.unlock();
  ti->cv.notify_one();
  return SchedulingResult::SCHEDULED;
}

}  // namespace threadpool
}  // namespace util
}  // namespace veles
