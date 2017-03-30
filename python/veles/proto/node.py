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

from veles.schema import model, fields
from veles.schema.nodeid import NodeID


class Node(model.Model):
    """
    Stores the lightweight data associated with a node, and an index of heavy
    data (which has to be downloaded separately).  This includes:

    - ``id``: self-descriptive.  The ID of a node cannot be changed.
    - ``parent``: the node parent's ID.
    - ``pos_start`` and ``pos_end``: for nodes where it makes sense (mostly
      chunka), the range of positions occupied by this object.  ``pos_start``
      is inclusive, while ``pos_end`` is exclusive.
    - ``tags``: an unordered set of tags, which are simple strings.
    - ``attr``: a string-keyed dict of attributes with arbitrary data.
    - ``data``: an unordered set of keys with non-null heavyweight data.
      The data has to be downloaded separately.
    - ``bindata``: a dict with an index of available bindata.  The keys in
      this dict correspond to bindata keys, and the values are size of the
      corresponding binary data in bytes.  The actual bindata has to
      be downloaded separately.
    """

    id = fields.NodeID()
    parent = fields.NodeID(default=NodeID.root_id)
    pos_start = fields.Integer(optional=True)
    pos_end = fields.Integer(optional=True)
    tags = fields.Set(fields.String())
    attr = fields.Map(fields.String(), fields.Any())
    data = fields.Set(fields.String())
    bindata = fields.Map(fields.String(), fields.SmallUnsignedInteger())


class PosFilter(model.Model):
    """
    A position range filter for use in list queries to the database.

    Ranges can be independently specified for start and end.  This allows
    for searching for chunks that start/end in given ranges, as well
    as searching for chunks that overlap a given range of positions.

    *_from and *_to are both *inclusive*.

    If *_from is None, it is treated as -inf.  If *_to is None, it is
    treated as +inf.  Also, for purposes of this filter, a pos_start
    of None is treated as -inf, and pos_end of None is treated as +inf.
    """

    start_from = fields.Integer(optional=True)
    start_to = fields.Integer(optional=True)
    end_from = fields.Integer(optional=True)
    end_to = fields.Integer(optional=True)

    @classmethod
    def intersecting_with(cls, pos_from, pos_to):
        """
        Constructs a filter that returns chunks overlapping with a given
        range of positions (left-inclusive, right-exclusive, like pos_start
        and pos_end).
        """
        return cls(
            start_to=pos_to-1,
            end_from=pos_from+1,
        )

    def matches(self, node):
        if self.start_from is not None:
            if node.pos_start is None or node.pos_start < self.start_from:
                return False
        if self.start_to is not None:
            if node.pos_start is not None and node.pos_start > self.start_to:
                return False
        if self.end_from is not None:
            if node.pos_end is not None and node.pos_end < self.end_from:
                return False
        if self.end_to is not None:
            if node.pos_end is None or node.pos_end > self.end_to:
                return False
        return True
