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
  c = vapi.VelesApi('127.0.0.1', 3135)

List open files:
::

  files = c.list_files()

Get chunk tree for file:
::

  chunks = c.get_chunk_tree(files[0])

Create new chunk:
::

  from veles import objects
  new_obj = objects.LocalObject()
  new_obj.name = 'custom chunk'
  new_obj.chunk_start = 0x10
  new_obj.chunk_end = 0x20
  c.create_chunk(files[0], new_obj)

Delete chunk:
::

  c.delete_object(files[0].children[0])