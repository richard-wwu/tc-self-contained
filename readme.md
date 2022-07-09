# Self-Contained Docker Image for Tensor Comprehensions

This repository contains a Dockerfile for a self-contained build of [Tensor Comprehensions](https://github.com/facebookresearch/TensorComprehensions). It provides:

- a local copy of the source code of Tensor Comprehension's last commit and all of its dependencies
- local copies of Tensor Comprehension's conda dependencies

## Dependencies

- [nvidia-docker](https://github.com/NVIDIA/nvidia-docker)

## Build Docker Image

    docker build -t tensor-comprehensions-self-contained .

## Run Docker Image

    docker run --rm --gpus all -it tensor-comprehensions-self-contained bash