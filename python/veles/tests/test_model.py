import unittest

from veles.dis import st
from veles.dis import mem
from veles.dis import reg
from veles.messages import msgpackwrap


class TestModel(unittest.TestCase):
    def setUp(self):
        wrap = msgpackwrap.MsgpackWrapper()
        self.packer = wrap.packer
        self.unpacker = wrap.unpacker

    def _pack_and_unpack(self, obj):
        data = obj.dump(self.packer)
        self.unpacker.feed(data)
        return st.IsaSTArg.load(self.unpacker.unpack())

    def test_IsaSTMem(self):
        obj = st.IsaSTMem(
            space=mem.MemSpace(name='test', width=3, addr_width=4),
            expr=st.IsaSTAdd(e1=st.IsaSTUnkArg(), e2=st.IsaSTUnkArg()),
            seg=st.IsaSTImm(width=5, base=6, val=7))
        obj2 = self._pack_and_unpack(obj)

        self.assertEqual(str(obj), str(obj2))
        self.assertEqual(obj.space.width, obj2.space.width)
        self.assertEqual(obj.space.addr_width, obj2.space.addr_width)

    def test_IsaSTReg(self):
        obj = st.IsaSTReg(
            reg=reg.RegisterPC(name='test', width=9, anchor='start', offset=0))
        obj2 = self._pack_and_unpack(obj)

        self.assertEqual(str(obj), str(obj2))
        self.assertEqual(obj.reg.width, obj2.reg.width)
        self.assertEqual(obj.reg.anchor, obj2.reg.anchor)
        self.assertEqual(obj.reg.offset, obj2.reg.offset)
