#!/usr/bin/env bash
set -euo pipefail
root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
mkdir -p "$root/artifacts"
c++ -std=c++17 -shared -fPIC "$root/tests/ui-smoke.cpp" \
    -o "$root/artifacts/ui-smoke.so" $(pkg-config --cflags --libs Qt6Widgets)
testroot="$(mktemp -d /tmp/captheme-ui.XXXXXX)"
export XDG_CONFIG_HOME="$testroot/config" XDG_DATA_HOME="$testroot/data" XDG_CACHE_HOME="$testroot/cache"
export XDG_STATE_HOME="$testroot/state"
export XDG_DATA_DIRS="$root/install/share:/usr/local/share:/usr/share"
export QT_QPA_PLATFORM=xcb LIBGL_ALWAYS_SOFTWARE=1 LANGUAGE=en_US LC_ALL=C.UTF-8
export CAPTHEME_UI_TEST=1 CAPTHEME_SCREENSHOT="$root/artifacts/captheme.png"
# Apply LD_PRELOAD only to the editor, not Xvfb, the shell, or dbus-daemon.
for pass in first restart captions; do
    unset CAPTHEME_TEST_CAPTIONS
    if [[ "$pass" == captions ]]; then export CAPTHEME_TEST_CAPTIONS=1; fi
    timeout 60s xvfb-run -a -s '-screen 0 1920x1080x24' \
        dbus-run-session -- env LD_PRELOAD="$root/artifacts/ui-smoke.so" \
        "$root/install/bin/kdenlive" --config capthemerc --no-welcome \
        > "$root/artifacts/ui-smoke-$pass.log" 2>&1
done
printf 'Teste concluído. Captura: %s/artifacts/captheme.png\n' "$root"
