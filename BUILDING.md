# Building instructions

## Linux

Dependencies:

- `cmake` >= 3.1.0
- `zlib`
- `qt` >= 5.5 ( >= 5.6 is required if you want to rearrange tabs using
drag&drop)
- `Python3.5+`
- `python3-msgpack`
- `python3-openssl` or `python3-pyopenssl` (depending on your distro)
- `OpenSSL` >= 1.0.0, < 1.1.0

Optional dependencies: (recommended for development)
- `clang-tidy`
- `clang-format`
- `libclang-common-dev`
- `gtest`

If your distribution has -dev or -devel packages, you'll also need ones
corresponding to the dependencies above.

On Ubuntu it can be done like this:
```bash
apt-get install cmake zlib1g-dev qtbase5-dev g++ python3 python3-venv \
    python3-dev python3-msgpack python3-openssl libffi-dev libssl-dev \
    clang-tidy-6.0 clang-format-6.0 libclang-common-6.0-dev
```

To build:
```bash
$ mkdir build
$ cd build
$ cmake -D CMAKE_BUILD_TYPE=RelWithDebInfo ..
$ make
```

To install [which is optional], use:
```bash
$ make install
```

To run Veles from build directory we recommend creating virtualenv for Veles:
```bash
$ python3 -m venv ~/venv/veles
$ . ~/venv/veles/bin/activate
(veles) $ pip install -r ../python/requirements.txt
```

Then to run Veles you will need to activate the venv before running Veles:
```bash
$ . ~/venv/veles/bin/activate
(veles) $ ./veles
```

If you want to install to a non-default directory, you'll also need to pass
it as an option to cmake before building, eg.:
```bash
$ cmake -D CMAKE_INSTALL_PREFIX:PATH=/usr/local ..
```

Python tests dependencies:

- `git`
- `Python2.7`
- Python packages:
    - `pip`
    - `tox>=2.4.0`

On Ubuntu it can be done like this:
```bash
apt-get install git python2.7 python2.7-dev python3-pip
pip3 install tox>=2.4.0
```

## Windows

You will need to install the following:

- `Qt` for `msvc2017_64`
- `Visual Studio 2017` (Community edition is enough; you will need MSVC even
when using only Qt Creator)
- `cmake` + `git` (both can be installed with Visual Studio)
- `perl` (only for building OpenSSL; e.g.
[Strawberry Perl](http://strawberryperl.com/))
- `Python3.5+` (you will also need to install dependencies from
`python/requirements.txt`)

and add a folder containing `cmake` to `PATH`.

### Qt Creator

Click `Open Project`, open `CMakeLists.txt` and configure the project for
`MSVC2017 64bit`. Build the project.

In case of problems with perl, set `PERL_EXECUTABLE` in CMake configuration to
the absolute path of `perl.exe`.

For speeding up further builds see the notes under `Visual Studio 2017` section.

### Visual Studio 2017

Open the source folder using `File` -> `Open` -> `Folder...` and then open CMake
settings: `CMake` -> `Change CMake Settings` -> `CMakeLists.txt`.
Example minimal config:

```json
{
  "configurations": [
    {
      "name": "x64-Debug",
      "generator": "Visual Studio 15 2017 Win64",
      "configurationType": "Debug",
      "inheritEnvironments": [ "msvc_x64" ],
      "buildRoot": "DRIVE:\\some_path\\veles-build-${name}",
      "cmakeCommandArgs": "",
      "buildCommandArgs": "-m -v:minimal",
      "ctestCommandArgs": "",
      "variables": [
        {
          "name": "CMAKE_PREFIX_PATH",
          "value": "C:\\Qt\\5.9.1\\msvc2017_64"
        },
        {
          "name": "PERL_EXECUTABLE",
          "value": "C:\\StrawberryPerl\\perl\\bin\\perl.exe"
        }
      ]
    },
    {
      "name": "x64-RelWithDebInfo",
      "generator": "Visual Studio 15 2017 Win64",
      "configurationType": "RelWithDebInfo",
      "inheritEnvironments": [ "msvc_x64" ],
      "buildRoot": "DRIVE:\\some_path\\veles-build-${name}",
      "cmakeCommandArgs": "",
      "buildCommandArgs": "-m -v:minimal",
      "ctestCommandArgs": "",
      "variables": [
        {
          "name": "CMAKE_PREFIX_PATH",
          "value": "C:\\Qt\\5.9.1\\msvc2017_64"
        },
        {
          "name": "PERL_EXECUTABLE",
          "value": "C:\\StrawberryPerl\\perl\\bin\\perl.exe"
        }
      ]
    }
  ]
}
```

After the first successful build you can greatly speedup CMake configuration
step and the build itself:
- Copy `libeay32.dll` and `ssleay32.dll` from
`veles-build-${name}\openssl-1.0.2l\out32dll` to a separate folder outside the
build directory and set CMake var `OPENSSL_DLL_DIR` to it. Do this step for all
used configurations (e.g. `Debug` and `RelWithDebInfo`).
- Copy `zlibstatic.lib` (or `zlibstaticd.lib` for debug builds) from
`veles-build-${name}\zlib\${name}\` to a separate folder and point
`ZLIB_LIBRARY` to it. Do the same with `zlib.h` and `zconf.h` from
`veles-build-${name}\prefix\include\`, this time using `ZLIB_INCLUDE_DIR`.
Repeat for all used configurations.


## Additional Info

If you want to start Veles without doing `install` step, you will additionally
have to set `Server script` field in `Connection` -> `Connect...` dialog to
point to the `srv.py` file (which you can find in `python` folder in the Veles
source tree).

If you want to run tests you will also need `Google Test`:

1. `git clone https://github.com/google/googletest.git`
2. Set `GOOGLETEST_SRC_PATH` to the cloned folder.

Enabling auto code formatting (build target `format`) and static analysis
(target `lint`):

1. Install [LLVM 6.0.0](https://releases.llvm.org/download.html#6.0.0).
2. Set `CLANG_TOOLS_PATH` to a folder containing clang-format and clang-tidy
(e.g. `C:\Program Files\LLVM\bin`).

Correctly configured build should take about 2 minutes on a CPU with 8
hyper-threads. If you find any problems with the instructions listed above, feel
free to create an issue or ping us on IRC
([#veles](https://webchat.freenode.net/?channels=#veles)).
