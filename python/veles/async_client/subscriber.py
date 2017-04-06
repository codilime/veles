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

from veles.proto import messages
from veles.proto.exceptions import (
    SchemaError,
)


class BaseSubManager:
    def __init__(self, sub):
        self.sub = sub
        self.proto = sub.tracker.proto
        self.proto.qids[id(self)] = self

    def cancel(self):
        self.proto.send_msg(messages.MsgCancelSubscription(
            qid=id(self),
        ))

    def handle_reply(self, msg):
        if not self.sub.active:
            return
        self._handle_reply(msg)

    def _handle_error(self, exc, checks):
        self.sub.error(exc)

    def handle_error(self, exc, checks):
        if not self.sub.active:
            return
        self._handle_error(exc, checks)


class NodeSubManager(BaseSubManager):
    def __init__(self, sub):
        super().__init__(sub, sub.tracker.proto)
        self.proto.send_msg(messages.MsgGet(
            qid=id(self),
            id=sub.node,
            sub=True,
        ))

    def _handle_reply(self, msg):
        if not isinstance(msg, messages.MsgGetReply):
            raise SchemaError('weird reply to get')
        self.sub.node_changed(msg.obj)


class DataSubManager(BaseSubManager):
    def __init__(self, sub):
        super().__init__(sub)
        self.proto.send_msg(messages.MsgGetData(
            qid=id(self),
            id=sub.node,
            key=sub.key,
            sub=True,
        ))

    def _handle_reply(self, msg):
        if not isinstance(msg, messages.MsgGetDataReply):
            raise SchemaError('weird reply to get_data')
        self.sub.data_changed(msg.data)


class BinDataSubManager(BaseSubManager):
    def __init__(self, sub):
        super().__init__(sub)
        self.proto.send_msg(messages.MsgGetBinData(
            qid=id(self),
            id=sub.node,
            key=sub.key,
            start=sub.start,
            end=sub.end,
            sub=True,
        ))

    def _handle_reply(self, msg):
        if not isinstance(msg, messages.MsgGetBinDataReply):
            raise SchemaError('weird reply to get_bindata')
        self.sub.bindata_changed(msg.data)


class ListSubManager(BaseSubManager):
    def __init__(self, sub):
        super().__init__(sub)
        self.proto.send_msg(messages.MsgGetList(
            qid=id(self),
            parent=sub.parent,
            tags=sub.tags,
            pos_filter=sub.pos_filter,
            sub=True,
        ))

    def _handle_reply(self, msg):
        if not isinstance(msg, messages.MsgGetListReply):
            raise SchemaError('weird reply to get_list')
        self.sub.list_changed(msg.objs, msg.gone)


class QuerySubManager(BaseSubManager):
    def __init__(self, sub):
        super().__init__(sub)
        self.proto.send_msg(messages.MsgGetQuery(
            qid=id(self),
            node=sub.node,
            query=sub.name,
            params=sub.params,
            trace=sub.trace,
            sub=True
        ))

    def _handle_reply(self, msg):
        if not isinstance(msg, messages.MsgGetQueryReply):
            raise SchemaError('weird reply to get_query')
        self.sub.raw_result_changed(msg.result, msg.checks)

    def _handle_error(self, exc, checks):
        self.sub.error(exc, checks)
