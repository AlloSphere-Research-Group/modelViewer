#!/bin/bash
(
  cmake --build build/release -j 7
)

result=$?
if [ ${result} == 0 ]; then
    ./bin/modelviewer
fi