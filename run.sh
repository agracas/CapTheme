#!/usr/bin/env bash
set -euo pipefail
root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
if [[ ! -x "$root/install/bin/kdenlive" ]]; then
    echo 'Compile primeiro: ./build.sh' >&2
    exit 1
fi
# Keep the fork's preferences, layout, cache and user resources inside this project.
export XDG_CONFIG_HOME="$root/.runtime/config"
export XDG_DATA_HOME="$root/.runtime/data"
export XDG_CACHE_HOME="$root/.runtime/cache"
export XDG_STATE_HOME="$root/.runtime/state"
export XDG_DATA_DIRS="$root/install/share:${XDG_DATA_DIRS:-/usr/local/share:/usr/share}"
export XDG_CONFIG_DIRS="$root/install/etc/xdg:${XDG_CONFIG_DIRS:-/etc/xdg}"
export QT_PLUGIN_PATH="$root/install/lib/plugins${QT_PLUGIN_PATH:+:$QT_PLUGIN_PATH}"
export QML_IMPORT_PATH="$root/install/lib/qml${QML_IMPORT_PATH:+:$QML_IMPORT_PATH}"
export PATH="$root/install/bin:$PATH"
export LD_LIBRARY_PATH="$root/install/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
mkdir -p "$XDG_CONFIG_HOME" "$XDG_DATA_HOME" "$XDG_CACHE_HOME" "$XDG_STATE_HOME"
exec "$root/install/bin/kdenlive" --config capthemerc --no-welcome "$@"
