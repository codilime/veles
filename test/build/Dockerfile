FROM ubuntu:18.04

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
    clang-tidy-6.0 \
    clang-format-6.0 \
    libclang-common-6.0-dev \
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
	&& cmake --build . -- -j8 \
	&& cmake --build . --target lint -- -j8 \
	&& cmake --build . --target format -- -j8 \
	&& cpack -D CPACK_PACKAGE_FILE_NAME=veles-docker -G ZIP -C Debug \
  && ( cd /veles/python && ./run_tests.sh )

