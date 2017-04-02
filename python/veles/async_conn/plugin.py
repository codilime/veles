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

from veles.schema.plugin import (
    MethodSignature,
    QuerySignature,
    BroadcastSignature,
)
from veles.proto.exceptions import VelesException
from .tracer import AsyncTracer


class MethodHandler:
    def __init__(self, method, tags):
        if not isinstance(method, str):
            raise TypeError("method must be str")
        if not isinstance(tags, set):
            raise TypeError("tags must be a set")
        if any(not isinstance(tag, str) for tag in tags):
            raise TypeError("tag must be str")
        self.method = method
        self.tags = tags

    def run_method(self, conn, node, params):
        """
        Returns an awaitable of result.
        """
        raise NotImplementedError


class LocalMethodHandler(MethodHandler):
    def __init__(self, sig, tags, func):
        if not isinstance(sig, MethodSignature):
            raise TypeError("sig must be MethodSignature")
        super().__init__(sig.name, tags)
        self.sig = sig
        self.func = func

    async def _run_method(self, conn, node, params):
        params = self.sig.params.load(params)
        result = await self.func(conn, node, params)
        result = self.sig.result.dump(result)
        return result

    def run_method(self, conn, node, params):
        loop = asyncio.get_event_loop()
        return loop.create_task(self._run_method(conn, node, params))


class QueryHandler:
    def __init__(self, query, tags):
        if not isinstance(query, str):
            raise TypeError("query must be str")
        if not isinstance(tags, set):
            raise TypeError("tags must be a set")
        if any(not isinstance(tag, str) for tag in tags):
            raise TypeError("tag must be str")
        self.query = query
        self.tags = tags

    def get_query(self, conn, node, params, checks):
        """
        Returns an awaitable of result.
        """
        raise NotImplementedError


class LocalQueryHandler(QueryHandler):
    def __init__(self, sig, tags, func):
        if not isinstance(sig, QuerySignature):
            raise TypeError("sig must be QuerySignature")
        super().__init__(sig.name, tags)
        self.sig = sig
        self.func = func

    async def _get_query(self, conn, node, params, checks):
        params = self.sig.params.load(params)
        tracer = AsyncTracer(conn)
        tracer._inject_node(node)
        try:
            result = await self.func(conn, node.id, params, tracer)
        finally:
            if checks is not None:
                checks += tracer.checks
        result = self.sig.result.dump(result)
        return result

    def get_query(self, conn, node, params, checks):
        loop = asyncio.get_event_loop()
        return loop.create_task(self._get_query(conn, node, params, checks))


class BroadcastHandler:
    def __init__(self, broadcast):
        if not isinstance(broadcast, str):
            raise TypeError("broadcast must be str")
        self.broadcast = broadcast

    def run_broadcast(self, conn, params):
        """
        Returns an awaitable of results.
        """
        raise NotImplementedError


class LocalBroadcastHandler(BroadcastHandler):
    def __init__(self, sig, func):
        if not isinstance(sig, BroadcastSignature):
            raise TypeError("sig must be BroadcastSignature")
        super().__init__(sig.name)
        self.sig = sig
        self.func = func

    async def _run_broadcast(self, conn, params):
        try:
            params = self.sig.params.load(params)
            results = await self.func(conn, params)
            results = [self.sig.result.dump(result) for result in results]
            return results
        except VelesException:
            return []

    def run_broadcast(self, conn, params):
        loop = asyncio.get_event_loop()
        return loop.create_task(self._run_broadcast(conn, params))


class TriggerHandler:
    def __init__(self, trigger, tags):
        if not isinstance(trigger, str):
            raise TypeError("trigger must be str")
        if not isinstance(tags, set):
            raise TypeError("tags must be a set")
        if any(not isinstance(tag, str) for tag in tags):
            raise TypeError("tag must be str")
        self.trigger = trigger
        self.tags = tags

    def run_trigger(self, conn, node, checks):
        """
        Returns an awaitable of None.
        """
        raise NotImplementedError


class LocalTriggerHandler(TriggerHandler):
    def __init__(self, trigger, tags, func):
        super().__init__(trigger, tags)
        self.func = func

    async def _run_trigger(self, conn, node, checks):
        tracer = AsyncTracer(conn)
        tracer._inject_node(node)
        try:
            await self.func(conn, node.id, tracer)
        finally:
            checks += tracer.checks

    def run_trigger(self, conn, node, checks):
        loop = asyncio.get_event_loop()
        return loop.create_task(self._run_trigger(conn, node, checks))


def method(sig, tags):
    def inner(func):
        return LocalMethodHandler(sig, tags, func)
    return inner


def query(sig, tags):
    def inner(func):
        return LocalQueryHandler(sig, tags, func)
    return inner


def broadcast(sig):
    def inner(func):
        return LocalBroadcastHandler(sig, func)
    return inner


def trigger(name, tags):
    def inner(func):
        return LocalTriggerHandler(name, tags, func)
    return inner
