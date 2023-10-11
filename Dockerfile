FROM ubuntu:22.04
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y ninja-build cmake gcc g++ git python3 python3-distutils python3-dev python3-pip nasm clang-format libcap-dev tmux zsh neovim
RUN pip3 install torch==1.13.0+cpu -f https://download.pytorch.org/whl/torch_stable.html
COPY . /sloader
WORKDIR /sloader
# TODO
# RUN ./run-format.sh
RUN rm -rf build
RUN mkdir build
RUN cmake -GNinja -S . -B build
RUN cmake --build build
RUN cd build && ctest --output-on-failure -j $(nproc)
RUN ./make-sloader-itself.sh
