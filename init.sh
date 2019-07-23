#!/bin/bash

git lfs install
git lfs pull

(
git submodule add https://github.com/AlloSphere-Research-Group/allolib.git
git submodule update --recursive --init
)