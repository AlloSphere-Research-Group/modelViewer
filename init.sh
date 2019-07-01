#!/bin/bash

git init

git clone --depth 1 https://github.com/AlloSphere-Research-Group/allolib.git allolib
cd allolib
git submodule update --init --recursive
cd ..


