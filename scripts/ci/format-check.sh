#!/bin/bash

set -eo pipefail
set -x

VCS=fossil

function formatting-check-clang-format {
  # clang-format
  python3 run-clang-format.py --style file -r ibus-slimt-t8n
}

function formatting-check-clang-tidy {
  # clang-tidy
  mkdir -p build
  ARGS=(
    -DCMAKE_EXPORT_COMPILE_COMMANDS=on
    -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
  )

  cmake -B build -S . "${ARGS[@]}"
  set +e
  FILES=$(find app ibus-slimt-t8n -type f)
  run-clang-tidy -export-fixes build/clang-tidy.ibus-slimt-t8n.yml -fix -format -p build -header-filter="$PWD/ibus-slimt-t8n" ${FILES[@]}
  CHECK_STATUS=$?
  fossil diff
  set -e
  return $CHECK_STATUS

}

function formatting-check-python {
  python3 -m black --diff --check scripts/
  python3 -m isort --profile black --diff --check scripts/
}

function formatting-check-sh {
  shfmt -i 2 -ci -bn -sr -d scripts/
}

function formatting-check-cmake {
  set +e
  CMAKE_FILES=$(find -name "CMakeLists.txt" -not -path "./3rd-party/*" -not -path "build")
  cmake-format ${CMAKE_FILES[@]} --check
  CHECK_STATUS=$?
  set -e
  cmake-format ${CMAKE_FILES[@]} --in-place
  fossil diff
  return $CHECK_STATUS
}

function formatting-check-iwyu {
  iwyu-tool -p build slimt/* > build/iwyu.out
}

formatting-check-clang-format
formatting-check-python
formatting-check-sh
formatting-check-cmake
formatting-check-clang-tidy
