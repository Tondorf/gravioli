FROM ubuntu:xenial
ENV DEBIAN_FRONTEND noninteractive
ENV TERM linux

# update
RUN apt-get update -yqq && \
apt-get install -yqq --no-install-recommends \
apt-utils ca-certificates

# set the locale
RUN apt-get install -yqq --no-install-recommends locales && locale-gen en_US.UTF-8
ENV LANG='en_US.UTF-8' LANGUAGE='en_US:en' LC_ALL='en_US.UTF-8'

# install cmake, boost, wget git and crypto++
RUN apt-get install -yqq --no-install-recommends \
build-essential cmake libboost-all-dev wget git libcrypto++-dev libcrypto++-utils

# install latest clang
RUN echo "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-5.0 main" \
>> /etc/apt/sources.list.d/llvm.list && \
wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - && \
apt-get update -yqq && \
apt-get install -yqq --no-install-recommends clang-5.0 lldb-5.0 lld-5.0
ENV CC=clang-5.0
ENV CXX=clang++-5.0

# create new user
RUN useradd -ms /bin/bash user
USER user

# upgrade cmake to latest version
RUN cd ~ && git clone https://gitlab.kitware.com/cmake/cmake.git && \
cd cmake && cmake . && make -j4
USER root
RUN cd /home/user/cmake && make install && apt-get -yqq purge cmake
USER user

# install zeromq
RUN cd ~ && git clone https://github.com/zeromq/libzmq && \
cd libzmq && mkdir build && cd build && \
cmake .. && make -j4 
USER root
RUN cd /home/user/libzmq/build && make install && ldconfig
USER user

# install flatbuffers
RUN cd ~ && git clone https://github.com/google/flatbuffers.git && \
cd flatbuffers && \
cmake -G "Unix Makefiles" && make -j4
ENV PATH="/home/user/flatbuffers:${PATH}"

# install gravioli server
RUN cd ~ && git clone https://github.com/Tondorf/gravioli.git && \
cd gravioli && \
cd fbs && make && make install && \
cd ~/ && mkdir gravioli-server && cd gravioli-server && \
cmake -DCMAKE_BUILD_TYPE=Release -DFLATBUFFERS_HEADERS=~/flatbuffers/include/flatbuffers \
../gravioli/server/ && make -j4

# clean up
USER root
RUN apt-get purge -yqq \
build-essential libboost-all-dev wget git libcrypto++-dev libcrypto++-utils \
clang-5.0 lldb-5.0 lld-5.0
RUN rm -rf /home/user/cmake /home/user/flatbuffers /home/user/libzmq /home/user/gravioli
RUN apt-get clean && rm -rf /var/lib/apt/lists/*
USER user

# cd gravioli
WORKDIR /home/user/gravioli-server

# start server
EXPOSE 8888
CMD ["./server", "--port=8888"]
