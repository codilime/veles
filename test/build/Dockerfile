FROM ubuntu:16.04

RUN apt-get update
RUN apt-get install -y wget software-properties-common
RUN wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -
RUN apt-add-repository "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-5.0 main"
RUN apt-get update
RUN apt-get install -y \
    cmake \
    zlib1g-dev \
    qtbase5-dev \
    g++ \
    python3 \
    python3-venv \
    python3-dev \
    libffi-dev \
    libssl-dev \
    clang-tidy-5.0 \
    clang-format-5.0 \
    # python tests dependencies
    git \
    python2.7 \
    python2.7-dev \
    python3-pip \
    libgtest-dev \
    google-mock
RUN update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-5.0 10000
RUN update-alternatives --install /usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-5.0 10000

RUN pip3 install tox>=2.4.0

COPY . /veles

RUN rm -rf /veles/build
RUN mkdir /veles/build
RUN cd /veles/build \
	&& cmake -D CMAKE_BUILD_TYPE=Debug .. \
	&& cmake --build . -- -j2 \
	&& cmake --build . --target lint -- -j2 VERBOSE=1 \
	&& cmake --build . --target format -- -j2 VERBOSE=1 \
	&& cpack -D CPACK_PACKAGE_FILE_NAME=veles-docker -G ZIP -C Debug
RUN cd /veles/python && ./run_tests.sh

