#!/bin/bash
(
  cmake --build build/release -j 7
)

result=$?
if [ ${result} == 0 ]; then
  cd bin
    ./modelviewer
fi
