FROM ubuntu:20.04 as base
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update -y
RUN apt-get install -y \
    build-essential \
    wget \
    curl \
    python3-pip \
    python3-dev \
    cmake \
    make \
    build-essential --fix-missing \
    && cd /usr/local/bin \
    && ln -s /usr/bin/python3 python \
    && rm -rf /var/lib/apt/lists/*
RUN curl https://sh.rustup.rs -sSf | sh -s -- --default-toolchain nightly -y
ENV PATH=/root/.cargo/bin:$PATH
RUN cargo install cbindgen bindgen-cli
RUN rustup install nightly-2023-06-26
RUN rustup default nightly-2023-06-26
RUN rustup target add thumbv7em-none-eabihf --toolchain nightly-2023-06-26
RUN pip3 install PyYaml

FROM base as pillar
RUN wget -q https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2019q4/gcc-arm-none-eabi-9-2019-q4-major-x86_64-linux.tar.bz2 -O  gcc-arm-none-eabi.tar.bz2
RUN mkdir gcc-arm-none-eabi && tar xjf gcc-arm-none-eabi.tar.bz2 -C gcc-arm-none-eabi --strip-components 1
RUN rm gcc-arm-none-eabi.tar.bz2
ENV PATH="/gcc-arm-none-eabi/bin:${PATH}"
WORKDIR /keystone3-firmware
RUN rm -rf build