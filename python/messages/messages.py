from enum import Enum

import msgpack

from messages import fields


class MsgpackMsg:
    message_types = {}

    def __init__(self, attrs={}):
        for field_name, field in attrs.items():
            setattr(self, field_name, field)

    def __init_subclass__(cls, **kwargs):
        super().__init_subclass__(**kwargs)

        cls.message_types[cls.msg_type] = cls
        cls.fields = []
        for attr_name, attr in cls.__dict__.items():
            try:
                attr.add_to_class(cls)
            except AttributeError:
                pass

    @classmethod
    def loads(cls, unpacker):
        unpacked = unpacker.unpack()
        if not isinstance(unpacked, dict) and 'type' not in unpacked:
            raise ValueError('Malformed message')
        if unpacked['type'] not in cls.message_types:
            raise ValueError('Unknown message type')
        obj = cls.message_types[unpacked.pop('type')]()
        for field in obj.fields:
            field.__set__(obj, unpacked.get(field.name, None))
        return obj

    def dumps(self, packer):
        obj = {'type': self.msg_type}
        for field in self.fields:
            obj[field.name] = field.__get__(self)
        return packer.pack(obj)


class MsgConnect(MsgpackMsg):
    msg_type = 'connect'

    proto_version = fields.Integer(minimum=1)
    client_name = fields.String()
    client_version = fields.Integer(optional=True, minimum=1)
    client_description = fields.String(optional=True)
    client_type = fields.String(optional=True)
    key = fields.String()


class MsgConnected(MsgpackMsg):
    msg_type = 'connected'

    proto_version = fields.Integer(minimum=1)
    server_name = fields.String()
    server_version = fields.Integer()


class MsgConnError(MsgpackMsg):
    msg_type = 'conn_error'

    code = fields.Integer()
    msg = fields.String()


class MsgProtoError(MsgpackMsg):
    msg_type = 'proto_error'


class MsgRegisterMethod(MsgpackMsg):
    msg_type = 'register_mthd'


class MsgRegisterTrigger(MsgpackMsg):
    msg_type = 'register_trigger'


class MsgList(MsgpackMsg):
    msg_type = 'list'


class MsgCancelSub(MsgpackMsg):
    msg_type = 'cancel_sub'


class MsgSubCancelled(MsgpackMsg):
    msg_type = 'sub_cancelled'


class MsgObjGone(MsgpackMsg):
    msg_type = 'obj_gone'


class MsgListReply(MsgpackMsg):
    msg_type = 'list_reply'


class MsgGet(MsgpackMsg):
    msg_type = 'get'


class MsgGetReply(MsgpackMsg):
    msg_type = 'get_reply'


class MsgGetData(MsgpackMsg):
    msg_type = 'get_data'


class MsgGetDataReply(MsgpackMsg):
    msg_type = 'get_data_reply'


class MsgGedBindata(MsgpackMsg):
    msg_type = 'get_bindata'


class MsgGetBindataReply(MsgpackMsg):
    msg_type = 'get_bindata_reply'


class MsgListConnections(MsgpackMsg):
    msg_type = 'list_connections'


class MsgConnectionsReply(MsgpackMsg):
    msg_type = 'connections_reply'


class MsgListRegistry(MsgpackMsg):
    msg_type = 'list_registry'


class MsgRegistryReply(MsgpackMsg):
    msg_type = 'registry_reply'


class MsgCreate(MsgpackMsg):
    msg_type = 'create'

    id = fields.Binary(optional=True)
    rid = fields.Integer()
    qid = fields.Integer(optional=True)
    parent = fields.Binary(optional=True)
    pos_start = fields.Integer(optional=True)
    pos_end = fields.Integer(optional=True)
    attr = fields.Map(optional=True, keys_types=[fields.String()])
    tags = fields.Array(optional=True, elements_types=[fields.String()])
    data = fields.Map(optional=True, keys_types=[fields.String()])
    bindata = fields.Map(optional=True, keys_types=[fields.String()])


class MsgModify(MsgpackMsg):
    msg_type = 'modify'


class MsgDelete(MsgpackMsg):
    msg_type = 'delete'


class MsgAck(MsgpackMsg):
    msg_type = 'ack'


class MsgModifyError(MsgpackMsg):
    msg_type = 'modify_err'


class MsgMethodRun(MsgpackMsg):
    msg_type = 'mthd_run'


class MsgMethodRes(MsgpackMsg):
    msg_type = 'mthd_res'


class MsgMethodError(MsgpackMsg):
    msg_type = 'mthd_err'


class MsgPluginMethodRun(MsgpackMsg):
    msg_type = 'plugin_mthd_run'


class MsgPluginMethodDone(MsgpackMsg):
    msg_type = 'plugin_mthd_done'


class MsgPluginMethodError(MsgpackMsg):
    msg_type = 'plugin_mthd_err'


class MsgPluginTriggerRun(MsgpackMsg):
    msg_type = 'plugin_trigger_run'


class MsgPluginTriggerDone(MsgpackMsg):
    msg_type = 'plugin_trigger_done'


class MsgPluginTriggerError(MsgpackMsg):
    msg_type = 'plugin_trigger_err'
