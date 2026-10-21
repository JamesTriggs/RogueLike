#!/bin/sh
set -eu
CHOIR_ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cmake -S "$CHOIR_ROOT" -B "$CHOIR_ROOT/build" -DCHOIR_BUILD_APP=ON -DCMAKE_BUILD_TYPE=Release
cmake --build "$CHOIR_ROOT/build" --parallel
exec "$CHOIR_ROOT/build/choir" "$@"
