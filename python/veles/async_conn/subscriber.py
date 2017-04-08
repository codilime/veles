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


from veles.db.subscriber import BaseSubscriber
from veles.proto.exceptions import VelesException


class BaseSubscriberQueryRaw(BaseSubscriber):
    """
    A subscriber of query results.  ``raw_result_changed`` is called whenever
    the result changes.
    """

    def __init__(self, tracker, node, name, params, trace=False):
        self.node = node
        self.name = name
        self.params = params
        self.trace = trace
        super().__init__(tracker)

    def raw_result_changed(self, result, checks):
        raise NotImplementedError

    def error(self, err, checks):
        raise NotImplementedError


class BaseSubscriberQuery(BaseSubscriberQueryRaw):
    """
    A subscriber of query result, with params and result translated
    according to a query signature.  ``result_changed`` is called whenever
    the result changes.
    """

    def __init__(self, tracker, node, sig, params):
        self.sig = sig
        super().__init__(tracker, node, sig.name, sig.params.dump(params))

    def result_changed(self, result):
        raise NotImplementedError

    def raw_result_changed(self, result, checks):
        try:
            self.result_changed(self.sig.result.load(result))
        except VelesException as e:
            self.error(e, checks)
