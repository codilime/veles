veles - A new age tool for binary analysis
==========================================

See our home page at https://veles.io.


Building, installing
--------------------

Dependencies:

- ``cmake`` >= 3.1.0
- ``zlib``
- ``qt`` >= 5.5
- ``Python3.5+``
    - ``virtualenv``
- ``OpenSSL`` >= 1.0.0, < 1.1.0

Caveats:
``qt`` >= 5.6 is required if you want to rearrange tabs using drag&drop.

Optional dependencies needed for running tests:

- ``gtest``

Additional Windows dependency (only needed to compile OpenSSL):

- ``perl``

If your distribution has -dev or -devel packages, you'll also need ones
corresponding to the dependencies above.

On Ubuntu it can be done like this::

    apt-get install cmake zlib1g-dev qtbase5-dev g++ python3 python3-venv python3-dev libffi-dev libssl-dev

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

Python tests dependencies:

- ``git``
- ``Python2.7``
- Python packages:
    - ``pip``
    - ``tox>=2.4.0``

On Ubuntu it can be done like this::

    apt-get install git python2.7 python3-pip
    pip3 install tox>=2.4.0

On Windows if you want to avoid compiling OpenSSL everytime you clean build directory
you can store `libeay32.dll` and `ssleay32.dll` in some directory, and pass it to cmake
as OPENSSL_DLL_DIR.