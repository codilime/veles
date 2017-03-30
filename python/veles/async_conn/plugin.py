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

from veles.schema.plugin import MethodSignature


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

    def __call__(self, params):
        return self.func(params)


def method(sig, tags):
    def inner(func):
        return LocalMethodHandler(sig, tags, func)
    return inner
