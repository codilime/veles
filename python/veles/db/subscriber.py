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


from veles.proto.node import PosFilter


class BaseSubscriber:
    def __init__(self, tracker):
        self.active = True
        self.tracker = tracker
        self.tracker.register_subscriber(self)

    def cancel(self):
        if self.active:
            self.active = False
            self.tracker.unregister_subscriber(self)


class BaseSubscriberNode(BaseSubscriber):
    """
    A subscriber of node modifications.  To watch for node modifications,
    create a subclass of this, override node_changed and error callbacks,
    and create an instance.  When the subscription is no longer used, call
    cancel().
    """

    def __init__(self, tracker, node):
        self.node = node
        super().__init__(tracker)

    def node_changed(self, node):
        raise NotImplementedError

    def error(self, err):
        raise NotImplementedError


class BaseSubscriberData(BaseSubscriber):
    """
    A subscriber of node data modifications.  ``data_changed`` will be called
    whenever the data value is changed.
    """

    def __init__(self, tracker, node, key):
        self.node = node
        self.key = key
        super().__init__(tracker)

    def data_changed(self, data):
        raise NotImplementedError

    def error(self, err):
        raise NotImplementedError


class BaseSubscriberBinData(BaseSubscriber):
    """
    A subscriber of node binary data modifications.  ``bindata_changed`` will
    be called whenever the given bindara range is changed.
    """

    def __init__(self, tracker, node, key, start, end):
        self.node = node
        self.key = key
        self.start = start
        self.end = end
        super().__init__(tracker)

    def bindata_changed(self, data):
        raise NotImplementedError

    def error(self, err):
        raise NotImplementedError


class BaseSubscriberList(BaseSubscriber):
    def __init__(self, tracker, parent, tags=frozenset(),
                 pos_filter=PosFilter()):
        self.parent = parent
        self.tags = tags
        self.pos_filter = pos_filter
        super().__init__(tracker)

    def matches(self, node):
        if node is None:
            return False
        if node.parent != self.parent:
            return False
        if not self.tags <= node.tags:
            return False
        if not self.pos_filter.matches(node):
            return False
        return True

    def list_changed(self, changed, gone):
        raise NotImplementedError

    def error(self, err):
        raise NotImplementedError
