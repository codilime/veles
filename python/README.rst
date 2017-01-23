Veles Python library
====================

This library enables you to operate on Veles objects tree from python.
It connects to running Veles instance via network.

Installing
----------

To use this library you need to have ``network_pb2.py`` file in ``veles`` directory.
(it is generated and copied there when you compile Veles)

Then you need to run
::

  pip install -r requirements.txt
  python setup.py install (or develop based on your preferences)

Usage
-----

Create client:
::

  import veles.veles_api as vapi
  c = vapi.VelesClient('127.0.0.1', 3135)

List open files:
::

  files = c.list_files()

Get chunk tree for file:
::

  chunks = c.get_chunk_tree(files[0])

Create new chunk:
::

  c.create_chunk(files[0], 'custom_chunk', start=0x10, end=0x20)

Delete chunk:
::

  c.delete_object(files[0].children[0])

See examples in ``examples`` directory.
