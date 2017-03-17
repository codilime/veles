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

from veles.proto import node
from veles.schema import nodeid, model
from . import fields


class MsgpackMsg(model.PolymorphicModel):
    pass


class MsgConnect(MsgpackMsg):
    object_type = 'connect'

    proto_version = fields.Integer(minimum=1)
    client_name = fields.String(optional=True)
    client_version = fields.String(optional=True)
    client_description = fields.String(optional=True)
    client_type = fields.String(optional=True)
    key = fields.String(optional=True)


class MsgConnected(MsgpackMsg):
    object_type = 'connected'

    proto_version = fields.Integer(minimum=1)
    server_name = fields.String()
    server_version = fields.String()


class MsgConnError(MsgpackMsg):
    object_type = 'conn_error'

    code = fields.String()
    msg = fields.String()


class MsgProtoError(MsgpackMsg):
    object_type = 'proto_error'

    code = fields.Integer()
    msg = fields.String(optional=True)


class MsgRegisterMethod(MsgpackMsg):
    object_type = 'register_mthd'


class MsgRegisterTrigger(MsgpackMsg):
    object_type = 'register_trigger'


class MsgList(MsgpackMsg):
    object_type = 'list'

    parent = fields.Extension(obj_type=nodeid.NodeID, optional=True)
    tags = fields.Array(elements_types=[fields.Map(
        keys_types=[fields.String()], values_types=[fields.Boolean()])])
    qid = fields.Integer()
    sub = fields.Boolean()
    pos_start = fields.Integer(optional=True)
    pos_end = fields.Integer(optional=True)


class MsgCancelSub(MsgpackMsg):
    object_type = 'cancel_sub'

    qid = fields.Integer()


class MsgSubCancelled(MsgpackMsg):
    object_type = 'sub_cancelled'

    qid = fields.Integer()


class MsgObjGone(MsgpackMsg):
    object_type = 'obj_gone'

    qid = fields.Integer()


class MsgListReply(MsgpackMsg):
    object_type = 'list_reply'

    objs = fields.Array(elements_types=[fields.Object(
        local_type=node.Node)])
    gone = fields.Array(elements_types=[fields.Extension(
        obj_type=nodeid.NodeID)])
    qid = fields.Integer()


class MsgGet(MsgpackMsg):
    object_type = 'get'

    id = fields.Extension(obj_type=nodeid.NodeID)
    qid = fields.Integer()
    sub = fields.Boolean()


class MsgGetReply(MsgpackMsg):
    object_type = 'get_reply'

    qid = fields.Integer()
    obj = fields.Object(local_type=node.Node)


class MsgGetData(MsgpackMsg):
    object_type = 'get_data'

    qid = fields.Integer()
    id = fields.Extension(obj_type=nodeid.NodeID)
    sub = fields.Boolean()
    key = fields.String()


class MsgGetDataReply(MsgpackMsg):
    object_type = 'get_data_reply'

    qid = fields.Integer()
    data = fields.Any()


class MsgGetBindata(MsgpackMsg):
    object_type = 'get_bindata'


class MsgGetBindataReply(MsgpackMsg):
    object_type = 'get_bindata_reply'


class MsgListConnections(MsgpackMsg):
    object_type = 'list_connections'


class MsgConnectionsReply(MsgpackMsg):
    object_type = 'connections_reply'


class MsgListRegistry(MsgpackMsg):
    object_type = 'list_registry'


class MsgRegistryReply(MsgpackMsg):
    object_type = 'registry_reply'


class MsgCreate(MsgpackMsg):
    object_type = 'create'

    id = fields.Extension(obj_type=nodeid.NodeID)
    rid = fields.Integer()
    qid = fields.Integer(optional=True)
    parent = fields.Extension(obj_type=nodeid.NodeID, optional=True)
    pos_start = fields.Integer(optional=True)
    pos_end = fields.Integer(optional=True)
    attr = fields.Map(optional=True, keys_types=[fields.String()])
    tags = fields.Array(optional=True, elements_types=[fields.String()],
                        local_type=set)
    data = fields.Map(optional=True, keys_types=[fields.String()])
    bindata = fields.Map(optional=True, keys_types=[fields.String()])


class MsgModify(MsgpackMsg):
    object_type = 'modify'


class MsgDelete(MsgpackMsg):
    object_type = 'delete'

    rid = fields.Integer()
    ids = fields.Array(
        elements_types=[fields.Extension(obj_type=nodeid.NodeID)])


class MsgAck(MsgpackMsg):
    object_type = 'ack'

    rid = fields.Integer()


class MsgModifyError(MsgpackMsg):
    object_type = 'modify_err'


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
