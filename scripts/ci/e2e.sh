#!/bin/bash

set -eo pipefail

cmake -B build -S . -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target all # noroot
sudo cmake --build build --target install
