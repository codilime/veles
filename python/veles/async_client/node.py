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

import asyncio

from veles.async_conn.node import AsyncNode
from veles.proto import messages
from veles.proto.node import PosFilter
from veles.proto.exceptions import (
    SchemaError,
)


class Getter:
    def __init__(self, node):
        self.node = node
        self.proto = node.proto
        self.future = asyncio.Future()
        self.proto.qids[id(self)] = self
        self.proto.send_msg(messages.MsgGet(
            qid=id(self),
            id=self.node.id
        ))

    def handle_reply(self, msg):
        if not isinstance(msg, messages.MsgGetReply):
            raise SchemaError('weird reply to get')
        self.node.node = msg.obj
        self.future.set_result(self.node)
        del self.proto.qids[id(self)]

    def handle_error(self, exc, checks):
        self.future.set_exception(exc)
        del self.proto.qids[id(self)]


class DataGetter:
    def __init__(self, node, key):
        self.proto = node.proto
        self.future = asyncio.Future()
        self.proto.qids[id(self)] = self
        self.proto.send_msg(messages.MsgGetData(
            qid=id(self),
            id=node.id,
            key=key,
        ))

    def handle_reply(self, msg):
        if not isinstance(msg, messages.MsgGetDataReply):
            raise SchemaError('weird reply to get_data')
        self.future.set_result(msg.data)
        del self.proto.qids[id(self)]

    def handle_error(self, exc, checks):
        self.future.set_exception(exc)
        del self.proto.qids[id(self)]


class BinDataGetter:
    def __init__(self, node, key, start, end):
        self.proto = node.proto
        self.future = asyncio.Future()
        self.proto.qids[id(self)] = self
        self.proto.send_msg(messages.MsgGetBinData(
            qid=id(self),
            id=node.id,
            key=key,
            start=start,
            end=end,
        ))

    def handle_reply(self, msg):
        if not isinstance(msg, messages.MsgGetBinDataReply):
            raise SchemaError('weird reply to get_bindata')
        self.future.set_result(msg.data)
        del self.proto.qids[id(self)]

    def handle_error(self, exc, checks):
        self.future.set_exception(exc)
        del self.proto.qids[id(self)]


class ListGetter:
    def __init__(self, node, tags, pos_filter):
        self.conn = node.conn
        self.proto = node.proto
        self.future = asyncio.Future()
        self.proto.qids[id(self)] = self
        self.proto.send_msg(messages.MsgGetList(
            qid=id(self),
            parent=node.id,
            tags=tags,
            pos_filter=pos_filter,
        ))

    def handle_reply(self, msg):
        if not isinstance(msg, messages.MsgGetListReply):
            raise SchemaError('weird reply to get_list')
        res = []
        for node in msg.objs:
            obj = self.conn.get_node_norefresh(node.id)
            obj.node = node
            res.append(obj)
        self.future.set_result(res)
        del self.proto.qids[id(self)]

    def handle_error(self, exc, checks):
        self.future.set_exception(exc)
        del self.proto.qids[id(self)]


class QueryGetter:
    def __init__(self, node, query, params, checks):
        self.proto = node.proto
        self.checks = checks
        self.future = asyncio.Future()
        self.proto.qids[id(self)] = self
        self.proto.send_msg(messages.MsgGetQuery(
            qid=id(self),
            node=node.id,
            query=query,
            params=params,
            trace=checks is not None,
        ))

    def handle_reply(self, msg):
        if not isinstance(msg, messages.MsgGetQueryReply):
            raise SchemaError('weird reply to get_query')
        if self.checks is not None:
            self.checks += msg.checks
        self.future.set_result(msg.result)
        del self.proto.qids[id(self)]

    def handle_error(self, exc, checks):
        if self.checks is not None:
            self.checks += checks
        self.future.set_exception(exc)
        del self.proto.qids[id(self)]


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


class MethodRunner:
    def __init__(self, proto):
        self.proto = proto
        self.future = asyncio.Future()
        self.proto.mids[id(self)] = self

    def handle_result(self, result):
        self.future.set_result(result)
        del self.proto.mids[id(self)]

    def handle_error(self, exc):
        self.future.set_exception(exc)
        del self.proto.mids[id(self)]


class AsyncRemoteNode(AsyncNode):
    def __init__(self, conn, id, node):
        super().__init__(conn, id, node)
        self.proto = conn.proto
        self.subs = {}

    # getters

    def refresh(self):
        return Getter(self).future

    def get_data(self, key):
        return DataGetter(self, key).future

    def get_bindata(self, key, start, end):
        return BinDataGetter(self, key, start, end).future

    def get_list(self, tags=frozenset(), pos_filter=PosFilter()):
        return ListGetter(self, set(tags), pos_filter).future

    def get_query_raw(self, name, params, checks=None):
        return QueryGetter(self, name, params, checks).future

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

    # mutators

    def run_method_raw(self, method, params):
        mr = MethodRunner(self.proto)
        self.proto.send_msg(messages.MsgMethodRun(
            mid=id(mr),
            node=self.id,
            method=method,
            params=params,
        ))
        return mr.future
