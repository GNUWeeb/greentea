#!/bin/sh

sudo apt-get update -y;
sudo apt-get install -y \
	make \
	git \
	zlib1g-dev \
	libssl-dev \
	gperf \
	php-cli \
	cmake \
	clang \
	libc++-dev \
	libc++abi-dev;

git submodule update --init --depth 1 --jobs 4;

export CC=clang
export CXX=clang++
export VERBOSE=1

cd submodules/tdlib/td;
mkdir -pv build;
cd build;
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX:PATH="install" ..;
cmake --build . -j$(nproc);
cmake --build . --target install -j$(nproc);
