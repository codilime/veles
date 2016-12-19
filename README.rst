veles - A new age tool for binary analysis
==========================================

See our home page at https://codisec.com/.


Building, installing
--------------------

Dependencies:

- ``cmake`` >= 3.1.0
- ``zlib``
- ``qt`` >= 5.5

Optional dependencies needed for running tests:

- ``gtest``

If your distribution has -dev or -devel packages, you'll also need ones
corresponding to the dependencies above.

On ubuntu it can be done like this::

    apt-get install cmake zlib zlib-dev qtbase5 qtbase5-dev

To build ::

    $ mkdir build
    $ cd build
    $ cmake -D CMAKE_BUILD_TYPE=Release ..
    $ make

To install [which is optional], use ::

    $ make install

If you want to install to a non-default directory, you'll also need to pass
it as an option to cmake before building, eg.::

    $ cmake -D CMAKE_INSTALL_PREFIX:PATH=/usr/local ..
