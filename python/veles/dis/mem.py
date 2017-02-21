# Copyright 2017 CodiLime
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


class MemSpace:
    """
    Represents a memory space.  Fields:

    - name: a string or None, used to print arguments referencing this space.
    - width: width of bytes in this space, in bits
    - addr_width: width of addresses in this space, in bits
    """

    def __init__(self, name, width, addr_width):
        self.name = name
        self.width = width
        self.addr_width = addr_width
