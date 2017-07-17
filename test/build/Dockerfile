FROM ubuntu:16.04

RUN apt-get update \
	&& apt-get install -y \
    cmake \
    zlib1g-dev \
    qtbase5-dev \
    g++ \
    python3 \
    python3-venv \
    python3-dev \
    libffi-dev \
    libssl-dev \
    clang-tidy-3.9 \
    clang-format-3.9 \
    libclang-common-3.9-dev \
    # python tests dependencies
    git \
    python2.7 \
    python2.7-dev \
    python3-pip \
  && pip3 install tox>=2.4.0

COPY . /veles

RUN mkdir /veles/build \
	&& cd /veles/build \
	&& cmake -D CMAKE_BUILD_TYPE=Debug .. \
	&& cmake --build . -- -j5 \
	&& cmake --build . --target lint -- -j5 \
	&& cmake --build . --target format -- -j5 \
	&& cpack -D CPACK_PACKAGE_FILE_NAME=veles-docker -G ZIP -C Debug \
  && ( cd /veles/python && ./run_tests.sh )

