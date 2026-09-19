#!/usr/bin/env bash
set -euo pipefail
root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
cmake -S "$root/kdenlive" -B "$root/build" -G Ninja \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_TESTING=OFF \
    -DCMAKE_INSTALL_PREFIX="$root/install"
cmake --build "$root/build" --parallel "${CAPTHEME_JOBS:-6}"
cmake --install "$root/build"
