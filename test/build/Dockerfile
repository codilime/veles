FROM ubuntu:16.04

RUN apt-get update \
	&& apt-get install -y \
	cmake \
	qtbase5-dev \
	g++ \
	python3 \
	python3-virtualenv \
	# python-virtualenv \
	# python \
	zlib1g-dev

COPY . /veles

RUN mkdir /veles/build \
	&& cd /veles/build \
	&& cmake -D CMAKE_BUILD_TYPE=Debug .. \
	&& cmake --build . -- -j5 \
	&& cpack -D CPACK_PACKAGE_FILE_NAME=veles-docker -G ZIP -C Debug

