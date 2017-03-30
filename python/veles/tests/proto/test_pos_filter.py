# Copyright 2016-2017 CodiLime
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

import unittest

from veles.schema.nodeid import NodeID
from veles.proto.node import Node, PosFilter

NODES = [
    Node(id=NodeID()),
    Node(id=NodeID(), pos_start=17),
    Node(id=NodeID(), pos_end=17),
    Node(id=NodeID(), pos_start=0x123456789abcdef123456789),
    Node(id=NodeID(), pos_start=10, pos_end=20),
    Node(id=NodeID(), pos_start=20, pos_end=30),
    Node(id=NodeID(), pos_start=30, pos_end=40),
    Node(id=NodeID(), pos_start=10, pos_end=40),
    Node(id=NodeID(), pos_start=20),
    Node(id=NodeID(), pos_start=20, pos_end=20),
]
CASES = [
    (PosFilter(), [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]),
    (PosFilter(start_from=9), [1, 3, 4, 5, 6, 7, 8, 9]),
    (PosFilter(start_from=10), [1, 3, 4, 5, 6, 7, 8, 9]),
    (PosFilter(start_from=11), [1, 3, 5, 6, 8, 9]),
    (PosFilter(start_from=20), [3, 5, 6, 8, 9]),
    (PosFilter(start_to=29), [0, 1, 2, 4, 5, 7, 8, 9]),
    (PosFilter(start_to=30), [0, 1, 2, 4, 5, 6, 7, 8, 9]),
    (PosFilter(start_from=15, start_to=25), [1, 5, 8, 9]),
    (PosFilter(end_from=19), [0, 1, 3, 4, 5, 6, 7, 8, 9]),
    (PosFilter(end_from=20), [0, 1, 3, 4, 5, 6, 7, 8, 9]),
    (PosFilter(end_from=21), [0, 1, 3, 5, 6, 7, 8]),
    (PosFilter(end_from=31), [0, 1, 3, 6, 7, 8]),
    (PosFilter(end_to=29), [2, 4, 9]),
    (PosFilter(end_to=30), [2, 5, 4, 9]),
    (PosFilter(end_from=15, end_to=25), [2, 4, 9]),
    (PosFilter.intersecting_with(20, 30), [0, 1, 5, 7, 8]),
]


class TestPosFilter(unittest.TestCase):
    def test_pos_filter(self):
        for fil, res in CASES:
            for i, node in enumerate(NODES):
                if fil.matches(node):
                    self.assertIn(i, res)
                else:
                    self.assertNotIn(i, res)
