FROM nvidia/cuda:9.0-cudnn7-devel-ubuntu16.04

# copy source files
COPY setup /setup
COPY TensorComprehensions /TensorComprehensions

# install build tools
RUN apt-get update
RUN apt-get install -y libgmp3-dev cmake automake libtool
RUN apt-get install -y unzip git

# create conda environment
ENV CONDA_PKGS_DIRS "/setup"
RUN /setup/Anaconda3-5.1.0-Linux-x86_64.sh -b -p /anaconda && \
    echo "export PATH=/anaconda/bin:\$PATH" >> ~/.bashrc
ENV PATH /anaconda/bin:$PATH
RUN . /anaconda/bin/activate
RUN conda update -y -n base conda
RUN conda create -y --name tc_build python=3.6

# make RUN commands and interactive sessions use the new environment
SHELL ["conda", "run", "-n", "tc_build", "/bin/bash", "-c"]
RUN conda init bash
RUN echo "conda activate tc_build" >> /root/.bashrc

# install conda dependencies
RUN conda install -y pyyaml mkl-include pytest
RUN conda install -y -c nicolasvasilache llvm-trunk halide
RUN conda install -y -c conda-forge eigen
RUN conda install -y -c nicolasvasilache caffe2
RUN conda install -y -c pytorch pytorch=0.4.0 torchvision cuda90
RUN conda remove -y cudatoolkit --force

# build TC
WORKDIR /TensorComprehensions
RUN CLANG_PREFIX=$(${CONDA_PREFIX}/bin/llvm-config --prefix) CUDA_TOOLKIT_ROOT_DIR=/usr/local/cuda/ ./build.sh

# install TC python package
RUN python setup.py install
RUN export PYTHONPATH=${PYTHONPATH}:$(find /anaconda/envs/tc_build/lib/ -name site-packages)
