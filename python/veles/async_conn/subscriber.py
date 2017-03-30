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


from veles.proto.node import PosFilter


class BaseSubscriber:
    """
    A subscriber of node modifications.  To watch for node modifications,
    create a subclass of this, override object_changed and error callbacks,
    and create an instance.  When the subscription is no longer used, call
    cancel().
    """

    def __init__(self, obj):
        self.obj = obj
        self.alive = True
        self.obj._add_sub(self)

    def cancel(self):
        if self.alive:
            self.alive = False
            self.obj._del_sub(self)

    def object_changed(self):
        raise NotImplementedError

    def error(self, err):
        raise NotImplementedError


class BaseSubscriberData:
    """
    A subscriber of node data modifications.  ``data_changed`` will be called
    whenever the data value is changed.
    """

    def __init__(self, obj, key):
        self.obj = obj
        self.key = key
        self.alive = True
        self.obj._add_sub_data(self)

    def cancel(self):
        if self.alive:
            self.alive = False
            self.obj._del_sub_data(self)

    def data_changed(self, data):
        raise NotImplementedError

    def error(self, err):
        raise NotImplementedError


class BaseSubscriberBinData:
    """
    A subscriber of node binary data modifications.  ``bindata_changed`` will
    be called whenever the given bindara range is changed.
    """

    def __init__(self, obj, key, start, end):
        self.obj = obj
        self.key = key
        self.start = start
        self.end = end
        self.alive = True
        self.obj._add_sub_bindata(self)

    def cancel(self):
        if self.alive:
            self.alive = False
            self.obj._del_sub_bindata(self)

    def bindata_changed(self, data):
        raise NotImplementedError

    def error(self, err):
        raise NotImplementedError


class BaseSubscriberQuery:
    """
    A subscriber of query results.  ``result_changed`` is called whenever
    the result changes.
    """

    def __init__(self, obj, name, params):
        self.obj = obj
        self.name = name
        self.params = params
        self.alive = True
        self.obj._add_sub_query(self)

    def cancel(self):
        if self.alive:
            self.alive = False
            self.obj._del_sub_query(self)

    def result_changed(self, data):
        raise NotImplementedError

    def error(self, err):
        raise NotImplementedError


class BaseSubscriberList:
    def __init__(self, parent, tags=frozenset(), pos_filter=PosFilter()):
        self.parent = parent
        self.tags = tags
        self.pos_filter = pos_filter
        self.alive = True
        self.parent._add_sub_list(self)

    def cancel(self):
        if self.alive:
            self.alive = False
            self.parent._del_sub_list(self)

    def matches(self, node):
        if node is None:
            return False
        if node.parent != self.parent.id:
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
