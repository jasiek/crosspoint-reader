#!/usr/bin/env bash
# build.sh — one-shot configure/build/smoke for the simulator.
# Run after every code update; non-zero exit means something regressed.
#
# Usage:
#   ./build.sh           # configure (if needed) + build + headless smoke
#   ./build.sh clean     # nuke sim/build/ first
#   ./build.sh run       # build, then run interactively (needs DISPLAY)
#   ./build.sh -- <args> # build, then run with extra args

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SIM_DIR="$REPO_ROOT/sim"
BUILD_DIR="$SIM_DIR/build"
BIN="$BUILD_DIR/crosspoint_sim"

cd "$REPO_ROOT"

# --- Parse args -------------------------------------------------------------

mode="smoke"
extra_args=()
case "${1:-}" in
  clean)
    echo "==> rm -rf $BUILD_DIR"
    rm -rf "$BUILD_DIR"
    shift || true
    ;;
  run)
    mode="run"
    shift
    ;;
  --)
    mode="run-args"
    shift
    extra_args=("$@")
    ;;
  "") ;;
  *)
    echo "usage: $0 [clean|run|-- <args>]" >&2
    exit 64
    ;;
esac

# --- Verify host toolchain --------------------------------------------------

missing=()
for tool in cmake make g++ pkg-config; do
  command -v "$tool" >/dev/null 2>&1 || missing+=("$tool")
done
if ! pkg-config --exists sdl2 2>/dev/null; then
  missing+=("libsdl2-dev (or sdl2 via brew)")
fi
if (( ${#missing[@]} )); then
  echo "==> missing prerequisites: ${missing[*]}" >&2
  echo "    macOS:  brew install cmake sdl2 pkg-config" >&2
  echo "    Linux:  sudo apt install build-essential cmake libsdl2-dev pkg-config" >&2
  exit 1
fi

# --- I18n codegen -----------------------------------------------------------
# I18nKeys.h / I18nStrings.{h,cpp} are gitignored; generated from yaml.
# Regenerate when missing or when any yaml is newer than the generated cpp.
i18n_dir="$REPO_ROOT/lib/I18n"
i18n_out="$i18n_dir/I18nStrings.cpp"
need_i18n=0
if [[ ! -f "$i18n_out" ]]; then
  need_i18n=1
else
  for y in "$i18n_dir/translations"/*.yaml; do
    [[ "$y" -nt "$i18n_out" ]] && { need_i18n=1; break; }
  done
fi
if (( need_i18n )); then
  python_bin="$(command -v python3 || command -v python || true)"
  if [[ -z "$python_bin" ]]; then
    echo "==> need python for I18n codegen — install python3" >&2
    exit 1
  fi
  echo "==> regenerating I18n bindings"
  "$python_bin" "$REPO_ROOT/scripts/gen_i18n.py" \
      "$i18n_dir/translations" "$i18n_dir/"
fi

# --- Submodule sanity -------------------------------------------------------

if [[ -d "$REPO_ROOT/open-x4-sdk" && -z "$(ls -A "$REPO_ROOT/open-x4-sdk" 2>/dev/null)" ]]; then
  echo "==> initialising open-x4-sdk submodule"
  git submodule update --init --depth 1 open-x4-sdk
fi

# --- Configure --------------------------------------------------------------

if [[ ! -f "$BUILD_DIR/CMakeCache.txt" ]]; then
  echo "==> cmake configure"
  cmake -S "$SIM_DIR" -B "$BUILD_DIR" -G "Unix Makefiles"
fi

# --- Build ------------------------------------------------------------------

jobs="$( (command -v nproc >/dev/null && nproc) || sysctl -n hw.ncpu 2>/dev/null || echo 4 )"
echo "==> make -j$jobs"
make -C "$BUILD_DIR" -j"$jobs"

# --- Smoke / run ------------------------------------------------------------

case "$mode" in
  smoke)
    echo "==> headless smoke"
    pushd "$BUILD_DIR" >/dev/null
    "$BIN" --headless
    popd >/dev/null
    echo "==> OK"
    ;;
  run)
    echo "==> launching $BIN"
    exec "$BIN"
    ;;
  run-args)
    echo "==> launching $BIN ${extra_args[*]}"
    exec "$BIN" "${extra_args[@]}"
    ;;
esac
