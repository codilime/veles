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

from veles.proto import messages
from veles.proto.exceptions import (
    SchemaError,
)


class NodeGetter:
    def __init__(self, proto, node):
        self.proto = proto
        self.future = asyncio.Future()
        self.proto.qids[id(self)] = self
        self.proto.send_msg(messages.MsgGet(
            qid=id(self),
            id=node
        ))

    def handle_reply(self, msg):
        if not isinstance(msg, messages.MsgGetReply):
            raise SchemaError('weird reply to get')
        self.future.set_result(msg.obj)
        del self.proto.qids[id(self)]

    def handle_error(self, exc, checks):
        self.future.set_exception(exc)
        del self.proto.qids[id(self)]


class DataGetter:
    def __init__(self, proto, node, key):
        self.proto = proto
        self.future = asyncio.Future()
        self.proto.qids[id(self)] = self
        self.proto.send_msg(messages.MsgGetData(
            qid=id(self),
            id=node,
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
    def __init__(self, proto, node, key, start, end):
        self.proto = proto
        self.future = asyncio.Future()
        self.proto.qids[id(self)] = self
        self.proto.send_msg(messages.MsgGetBinData(
            qid=id(self),
            id=node,
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
    def __init__(self, proto, parent, tags, pos_filter):
        self.proto = proto
        self.future = asyncio.Future()
        self.proto.qids[id(self)] = self
        self.proto.send_msg(messages.MsgGetList(
            qid=id(self),
            parent=parent,
            tags=tags,
            pos_filter=pos_filter,
        ))

    def handle_reply(self, msg):
        if not isinstance(msg, messages.MsgGetListReply):
            raise SchemaError('weird reply to get_list')
        self.future.set_result(msg.objs)
        del self.proto.qids[id(self)]

    def handle_error(self, exc, checks):
        self.future.set_exception(exc)
        del self.proto.qids[id(self)]


class QueryGetter:
    def __init__(self, proto, node, query, params, checks):
        self.proto = proto
        self.checks = checks
        self.future = asyncio.Future()
        self.proto.qids[id(self)] = self
        self.proto.send_msg(messages.MsgGetQuery(
            qid=id(self),
            node=node,
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


class Request:
    def __init__(self, proto):
        self.proto = proto
        self.future = asyncio.Future()
        self.proto.rids[id(self)] = self

    def handle_ack(self):
        self.future.set_result(None)
        del self.proto.rids[id(self)]

    def handle_error(self, exc):
        self.future.set_exception(exc)
        del self.proto.rids[id(self)]


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


class BroadcastRunner:
    def __init__(self, proto):
        self.proto = proto
        self.future = asyncio.Future()
        self.proto.bids[id(self)] = self

    def handle_result(self, result):
        self.future.set_result(result)
        del self.proto.bids[id(self)]
