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

from veles.proto.node import Node, PosFilter
from veles.schema import model, fields
from veles.schema.nodeid import NodeID


class MsgpackMsg(model.PolymorphicModel):
    pass


class MsgConnect(MsgpackMsg):
    object_type = 'connect'

    proto_version = fields.SmallInteger(minimum=1)
    client_name = fields.String(optional=True)
    client_version = fields.String(optional=True)
    client_description = fields.String(optional=True)
    client_type = fields.String(optional=True)
    key = fields.String(optional=True)


class MsgConnected(MsgpackMsg):
    object_type = 'connected'

    proto_version = fields.SmallInteger(minimum=1)
    server_name = fields.String()
    server_version = fields.String()


class MsgConnError(MsgpackMsg):
    object_type = 'conn_error'

    code = fields.String()
    msg = fields.String()


class MsgProtoError(MsgpackMsg):
    object_type = 'proto_error'

    code = fields.Integer()
    msg = fields.String()


class MsgRegisterMethod(MsgpackMsg):
    object_type = 'register_mthd'


class MsgRegisterTrigger(MsgpackMsg):
    object_type = 'register_trigger'


# queries and subscriptions


class MsgCancelSub(MsgpackMsg):
    object_type = 'cancel_sub'

    qid = fields.SmallUnsignedInteger()


class MsgSubCancelled(MsgpackMsg):
    object_type = 'sub_cancelled'

    qid = fields.SmallUnsignedInteger()


class MsgQueryError(MsgpackMsg):
    object_type = 'query_error'

    qid = fields.SmallUnsignedInteger()
    code = fields.String()
    msg = fields.String()


class MsgGet(MsgpackMsg):
    object_type = 'get'

    id = fields.NodeID()
    qid = fields.SmallUnsignedInteger()
    sub = fields.Boolean(default=False)


class MsgGetReply(MsgpackMsg):
    object_type = 'get_reply'

    qid = fields.SmallUnsignedInteger()
    obj = fields.Object(Node)


class MsgGetData(MsgpackMsg):
    object_type = 'get_data'

    qid = fields.SmallUnsignedInteger()
    id = fields.NodeID()
    sub = fields.Boolean(default=False)
    key = fields.String()


class MsgGetDataReply(MsgpackMsg):
    object_type = 'get_data_reply'

    qid = fields.SmallUnsignedInteger()
    data = fields.Any(optional=True)


class MsgGetBinData(MsgpackMsg):
    object_type = 'get_bindata'

    qid = fields.SmallUnsignedInteger()
    id = fields.NodeID()
    sub = fields.Boolean(default=False)
    key = fields.String()
    start = fields.SmallUnsignedInteger()
    end = fields.SmallUnsignedInteger(optional=True)


class MsgGetBinDataReply(MsgpackMsg):
    object_type = 'get_bindata_reply'

    qid = fields.SmallUnsignedInteger()
    data = fields.Binary()


class MsgGetList(MsgpackMsg):
    object_type = 'get_list'

    parent = fields.NodeID(default=NodeID.root_id)
    tags = fields.Set(fields.String())
    qid = fields.SmallUnsignedInteger()
    sub = fields.Boolean(default=False)
    pos_filter = fields.Object(PosFilter, default=PosFilter())


class MsgGetListReply(MsgpackMsg):
    object_type = 'get_list_reply'

    objs = fields.List(fields.Object(Node))
    gone = fields.List(fields.NodeID())
    qid = fields.SmallUnsignedInteger()


class MsgListConnections(MsgpackMsg):
    object_type = 'list_connections'


class MsgConnectionsReply(MsgpackMsg):
    object_type = 'connections_reply'


class MsgListRegistry(MsgpackMsg):
    object_type = 'list_registry'


class MsgRegistryReply(MsgpackMsg):
    object_type = 'registry_reply'


# mutators

class MsgCreate(MsgpackMsg):
    object_type = 'create'

    id = fields.NodeID()
    rid = fields.SmallUnsignedInteger()
    parent = fields.NodeID(default=NodeID.root_id)
    pos_start = fields.Integer(optional=True)
    pos_end = fields.Integer(optional=True)
    attr = fields.Map(fields.String(), fields.Any())
    tags = fields.Set(fields.String())
    data = fields.Map(fields.String(), fields.Any())
    bindata = fields.Map(fields.String(), fields.Binary())


class MsgDelete(MsgpackMsg):
    object_type = 'delete'

    rid = fields.SmallUnsignedInteger()
    id = fields.NodeID()


class MsgSetParent(MsgpackMsg):
    object_type = 'set_parent'

    rid = fields.SmallUnsignedInteger()
    id = fields.NodeID()
    parent = fields.NodeID(default=NodeID.root_id)


class MsgSetPos(MsgpackMsg):
    object_type = 'set_pos'

    rid = fields.SmallUnsignedInteger()
    id = fields.NodeID()
    pos_start = fields.Integer(optional=True)
    pos_end = fields.Integer(optional=True)


class MsgAddTag(MsgpackMsg):
    object_type = 'add_tag'

    rid = fields.SmallUnsignedInteger()
    id = fields.NodeID()
    tag = fields.String()


class MsgDelTag(MsgpackMsg):
    object_type = 'del_tag'

    rid = fields.SmallUnsignedInteger()
    id = fields.NodeID()
    tag = fields.String()


class MsgSetAttr(MsgpackMsg):
    object_type = 'set_attr'

    rid = fields.SmallUnsignedInteger()
    id = fields.NodeID()
    key = fields.String()
    data = fields.Any(optional=True)


class MsgSetData(MsgpackMsg):
    object_type = 'set_data'

    rid = fields.SmallUnsignedInteger()
    id = fields.NodeID()
    key = fields.String()
    data = fields.Any(optional=True)


class MsgSetBinData(MsgpackMsg):
    object_type = 'set_bindata'

    rid = fields.SmallUnsignedInteger()
    id = fields.NodeID()
    key = fields.String()
    start = fields.SmallUnsignedInteger(default=0)
    data = fields.Binary()
    truncate = fields.Boolean(default=False)


class MsgRequestAck(MsgpackMsg):
    object_type = 'request_ack'

    rid = fields.SmallUnsignedInteger()


class MsgRequestError(MsgpackMsg):
    object_type = 'request_error'

    rid = fields.SmallUnsignedInteger()
    code = fields.String()
    msg = fields.String()


# methods & triggers

class MsgMethodRun(MsgpackMsg):
    object_type = 'mthd_run'


class MsgMethodRes(MsgpackMsg):
    object_type = 'mthd_res'


class MsgMethodError(MsgpackMsg):
    object_type = 'mthd_err'


class MsgPluginMethodRun(MsgpackMsg):
    object_type = 'plugin_mthd_run'


class MsgPluginMethodDone(MsgpackMsg):
    object_type = 'plugin_mthd_done'


class MsgPluginMethodError(MsgpackMsg):
    object_type = 'plugin_mthd_err'


class MsgPluginTriggerRun(MsgpackMsg):
    object_type = 'plugin_trigger_run'


class MsgPluginTriggerDone(MsgpackMsg):
    object_type = 'plugin_trigger_done'


class MsgPluginTriggerError(MsgpackMsg):
    object_type = 'plugin_trigger_err'
