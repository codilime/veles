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

from veles.async_conn.node import AsyncNode
from veles.proto import messages
from veles.proto.exceptions import (
    SchemaError,
)


class BaseSub:
    def __init__(self, sub, proto):
        self.sub = sub
        self.proto = proto
        self.proto.qids[id(self)] = self
        self.alive = True

    def cancel(self):
        self.alive = False
        self.proto.send_msg(messages.MsgCancelSubscription(
            qid=id(self),
        ))

    def handle_reply(self, msg):
        if not self.alive:
            return
        self._handle_reply(msg)

    def _handle_error(self, exc, checks):
        self.sub.error(exc)

    def handle_error(self, exc, checks):
        if not self.alive:
            return
        self._handle_error(exc, checks)


class GetterSub(BaseSub):
    def __init__(self, sub):
        super().__init__(sub, sub.obj.proto)
        self.proto.send_msg(messages.MsgGet(
            qid=id(self),
            id=sub.obj.id,
            sub=True,
        ))

    def _handle_reply(self, msg):
        if not isinstance(msg, messages.MsgGetReply):
            raise SchemaError('weird reply to get')
        self.sub.obj.node = msg.obj
        self.sub.object_changed()


class DataSub(BaseSub):
    def __init__(self, sub):
        super().__init__(sub, sub.obj.proto)
        self.proto.send_msg(messages.MsgGetData(
            qid=id(self),
            id=sub.obj.id,
            key=sub.key,
            sub=True,
        ))

    def _handle_reply(self, msg):
        if not isinstance(msg, messages.MsgGetDataReply):
            raise SchemaError('weird reply to get_data')
        self.sub.data_changed(msg.data)


class BinDataSub(BaseSub):
    def __init__(self, sub):
        super().__init__(sub, sub.obj.proto)
        self.proto.send_msg(messages.MsgGetBinData(
            qid=id(self),
            id=sub.obj.id,
            key=sub.key,
            start=sub.start,
            end=sub.end,
            sub=True,
        ))

    def _handle_reply(self, msg):
        if not isinstance(msg, messages.MsgGetBinDataReply):
            raise SchemaError('weird reply to get_bindata')
        self.sub.bindata_changed(msg.data)


class ListSub(BaseSub):
    def __init__(self, sub):
        self.conn = sub.parent.conn
        super().__init__(sub, sub.parent.proto)
        self.proto.send_msg(messages.MsgGetList(
            qid=id(self),
            parent=sub.parent.id,
            tags=sub.tags,
            pos_filter=sub.pos_filter,
            sub=True,
        ))

    def _handle_reply(self, msg):
        if not isinstance(msg, messages.MsgGetListReply):
            raise SchemaError('weird reply to get_list')
        res = []
        for node in msg.objs:
            obj = self.conn.get_node_norefresh(node.id)
            obj.node = node
            res.append(obj)
        self.sub.list_changed(res, msg.gone)


class QuerySub(BaseSub):
    def __init__(self, sub):
        self.conn = sub.obj.conn
        super().__init__(sub, sub.obj.proto)
        self.proto.send_msg(messages.MsgGetQuery(
            qid=id(self),
            node=sub.obj.id,
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


class AsyncRemoteNode(AsyncNode):
    def __init__(self, conn, id, node):
        super().__init__(conn, id, node)
        self.proto = conn.proto
        self.subs = {}

    # subscriptions

    def _add_sub(self, sub):
        self.subs[sub] = GetterSub(sub)

    def _del_sub(self, sub):
        self.subs[sub].cancel()
        del self.subs[sub]

    def _add_sub_data(self, sub):
        self.subs[sub] = DataSub(sub)

    def _del_sub_data(self, sub):
        self.subs[sub].cancel()
        del self.subs[sub]

    def _add_sub_bindata(self, sub):
        self.subs[sub] = BinDataSub(sub)

    def _del_sub_bindata(self, sub):
        self.subs[sub].cancel()
        del self.subs[sub]

    def _add_sub_list(self, sub):
        self.subs[sub] = ListSub(sub)

    def _del_sub_list(self, sub):
        self.subs[sub].cancel()
        del self.subs[sub]

    def _add_sub_query(self, sub):
        self.subs[sub] = QuerySub(sub)

    def _del_sub_query(self, sub):
        self.subs[sub].cancel()
        del self.subs[sub]
