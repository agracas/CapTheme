#!/usr/bin/env bash
set -euo pipefail
root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
mkdir -p "$root/artifacts"
c++ -std=c++17 -shared -fPIC "$root/tests/inspector-smoke.cpp" \
    -o "$root/artifacts/inspector-smoke.so" $(pkg-config --cflags --libs Qt6Widgets Qt6Test)
testroot="$(mktemp -d /tmp/captheme-inspector.XXXXXX)"
ffmpeg -v error -f lavfi -i testsrc2=size=1920x1080:rate=25 \
    -f lavfi -i sine=frequency=440 -t 3 -c:v libx264 -preset ultrafast -crf 30 \
    -c:a aac "$testroot/inspector-fixture.mp4"
export XDG_CONFIG_HOME="$testroot/config" XDG_DATA_HOME="$testroot/data" XDG_CACHE_HOME="$testroot/cache"
export XDG_STATE_HOME="$testroot/state" XDG_DATA_DIRS="$root/install/share:/usr/local/share:/usr/share"
export QT_QPA_PLATFORM=xcb LIBGL_ALWAYS_SOFTWARE=1 LANGUAGE=en_US LC_ALL=C.UTF-8
export CAPTHEME_INSPECTOR_TEST=1 CAPTHEME_SCREENSHOT="$root/artifacts/inspector.png"
# Each test has a fresh project destination, so Save As never overwrites a previous test.
export CAPTHEME_TEST_PROJECT="$testroot/inspector-test.kdenlive"
timeout 90s xvfb-run -a -s '-screen 0 1920x1200x24' \
    dbus-run-session -- env LD_PRELOAD="$root/artifacts/inspector-smoke.so" \
    "$root/install/bin/kdenlive" --config capthemerc --no-welcome -i "$testroot/inspector-fixture.mp4" \
    > "$root/artifacts/inspector-smoke.log" 2>&1
cp "$CAPTHEME_TEST_PROJECT" "$root/artifacts/inspector-test.kdenlive"
python "$root/tests/check-inspector-project.py" "$CAPTHEME_TEST_PROJECT"
