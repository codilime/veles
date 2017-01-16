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
#include <iostream>
#include <fstream>

#include "test/universe.h"
#include "test/fileblob.h"

using veles::Universe;
using veles::FileBlob;
using veles::PBlob;

int main(int argc, char **argv) {
  std::ifstream file(argv[1], std::ios::binary | std::ios::ate);
  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);
  std::vector<uint8_t> data(size);
  if (!file.read(static_cast<char *>(data.data()), size)) {
    std::cerr << "FAIL\n";
    return 1;
  }
  Universe *un = new Universe;
  PBlob b = un->Create<FileBlob>(argv[1], data);
  b->dump();
  delete un;
  return 0;
}
