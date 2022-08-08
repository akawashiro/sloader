#! /bin/bash -eu

cd "$(git rev-parse --show-toplevel)"
git ls-files -- '**/*.h' '**/*.cc' '**/*.c'| xargs -P4 clang-format -i
git ls-files -- '**/CMakeLists.txt' | xargs -P4 cmake-format -i
git diff --exit-code -- .
