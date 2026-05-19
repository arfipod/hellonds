#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
MELONDS_DIR="${MELONDS_DIR:-$HOME/.local/share/hellonds/emulators/melonds/1.1}"
MELONDS_DEFAULT_BIN="$MELONDS_DIR/melonDS"
MELONDS_FLATPAK_ID="${MELONDS_FLATPAK_ID:-net.kuribo64.melonDS}"
MELONDS_UBUNTU_DEPS="libqt6core6t64 libqt6gui6t64 libqt6network6t64 libqt6widgets6t64 libqt6multimedia6 libenet7 libfaad2"

usage() {
  cat <<'USAGE'
Usage:
  scripts/run_melonds.sh gospel_nds
  scripts/run_melonds.sh flappy_test
  scripts/run_melonds.sh path/to/file.nds
  scripts/run_melonds.sh --foreground gospel_nds

Environment:
  MELONDS_BIN         melonDS executable to use, for example /path/to/melonDS.AppImage.
  MELONDS_DIR         Directory containing melonDS. Defaults to ~/.local/share/hellonds/emulators/melonds/1.1.
  MELONDS_FLATPAK_ID Flatpak app id to use when no local binary is found.
USAGE
}

foreground=0

while [ "${1:-}" = "--foreground" ]; do
  foreground=1
  shift
done

if [ "${1:-}" = "-h" ] || [ "${1:-}" = "--help" ]; then
  usage
  exit 0
fi

target="${1:-gospel_nds}"

case "$target" in
  gospel_nds|flappy_test)
    project_dir="$ROOT/$target"
    rom="$project_dir/$target.nds"
    if [ -f /opt/wonderful/bin/wf-env ]; then
      # shellcheck disable=SC1091
      source /opt/wonderful/bin/wf-env
    fi
    make -C "$project_dir"
    ;;
  *.nds)
    rom="$target"
    ;;
  *)
    echo "Unknown target: $target" >&2
    usage >&2
    exit 2
    ;;
esac

rom="$(realpath "$rom")"

if [ -n "${MELONDS_BIN:-}" ]; then
  if [ ! -x "$MELONDS_BIN" ]; then
    cat >&2 <<EOF
MELONDS_BIN is set but is not executable:
  $MELONDS_BIN
EOF
    exit 1
  fi
  melon_cmd=("$MELONDS_BIN")
elif [ -x "$MELONDS_DEFAULT_BIN" ]; then
  melon_cmd=("$MELONDS_DEFAULT_BIN")
elif command -v melonDS >/dev/null 2>&1; then
  melon_cmd=("melonDS")
elif command -v melonds >/dev/null 2>&1; then
  melon_cmd=("melonds")
elif command -v flatpak >/dev/null 2>&1 && flatpak info "$MELONDS_FLATPAK_ID" >/dev/null 2>&1; then
  melon_cmd=("flatpak" "run" "$MELONDS_FLATPAK_ID")
else
  cat >&2 <<EOF
melonDS was not found.

Install it from:
  https://melonds.kuribo64.net/downloads.php

Examples:
  flatpak install flathub $MELONDS_FLATPAK_ID
  MELONDS_BIN=/path/to/melonDS.AppImage scripts/run_melonds.sh $target
EOF
  exit 1
fi

if [ "${melon_cmd[0]}" != "flatpak" ] && command -v ldd >/dev/null 2>&1; then
  melon_exe="${melon_cmd[0]}"
  if [[ "$melon_exe" != /* ]]; then
    melon_exe="$(command -v "$melon_exe")"
  fi

  missing_libs="$(ldd "$melon_exe" 2>/dev/null | awk '/not found/ { print "  " $1 }' || true)"
  if [ -n "$missing_libs" ]; then
    cat >&2 <<EOF
melonDS is installed, but these shared libraries are missing:
$missing_libs

On Ubuntu 24.04/Noble, install them with:
  sudo apt-get install -y $MELONDS_UBUNTU_DEPS

Then rerun:
  scripts/run_melonds.sh $target
EOF
    exit 1
  fi
fi

echo "Launching melonDS:"
echo "  Command: ${melon_cmd[*]}"
echo "  ROM: $rom"

if [ "$foreground" -eq 1 ]; then
  exec "${melon_cmd[@]}" "$rom"
fi

"${melon_cmd[@]}" "$rom" >/dev/null 2>&1 &
echo "Started. Close the melonDS window when finished."
