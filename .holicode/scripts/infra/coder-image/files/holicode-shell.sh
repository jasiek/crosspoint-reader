#!/bin/bash
# /usr/local/share/holicode/holicode-shell.sh
# Interactive shell enhancements for Coder workspaces.
# Installed under /usr/local/share/holicode/ — deliberately NOT under
# /etc/profile.d/, because /etc/profile sources profile.d BEFORE ~/.bashrc on
# login shells, which would init the tools before user-level
# HOLICODE_SHELL_<TOOL>=0 overrides run. The single canonical entry point is
# the marker block injected into ~/.bashrc by coder_script.shell_dotfile_seed.
# The case-statement guard ensures non-interactive shells (bash -c, build hooks,
# agent-spawned subshells) are completely unaffected. The HOLICODE_SHELL_INITIALIZED
# guard remains as defense-in-depth against accidental double-sourcing.

case $- in *i*) ;; *) return ;; esac
[ -n "$BASH_VERSION" ] || return

# Idempotency guard — neutralize any double-source path (e.g. login shell that
# pulls /etc/profile.d/*.sh AND has ~/.bashrc re-source the same file).
[ -n "${HOLICODE_SHELL_INITIALIZED:-}" ] && return
HOLICODE_SHELL_INITIALIZED=1

# Tier 1 — on by default; opt out with HOLICODE_SHELL_<TOOL>=0 above the
# marker block in ~/.bashrc (which is sourced before this script).

if [ "${HOLICODE_SHELL_ATUIN:-1}" = "1" ] && command -v atuin >/dev/null 2>&1; then
    eval "$(atuin init bash)"
fi

if [ "${HOLICODE_SHELL_ZOXIDE:-1}" = "1" ] && command -v zoxide >/dev/null 2>&1; then
    eval "$(zoxide init bash)"
fi

# Tier 2 — inline ghost-text, off by default; opt in with HOLICODE_SHELL_BLESH=1 in ~/.bashrc
if [ "${HOLICODE_SHELL_BLESH:-0}" = "1" ] && [ -f /usr/local/share/blesh/ble.sh ]; then
    # shellcheck disable=SC1091
    source /usr/local/share/blesh/ble.sh --attach=none
fi

# Prompt last — Starship must own PS1
if [ "${HOLICODE_SHELL_STARSHIP:-1}" = "1" ] && command -v starship >/dev/null 2>&1; then
    eval "$(starship init bash)"
fi

# ble.sh attach must run after Starship init when both are enabled
if [ "${HOLICODE_SHELL_BLESH:-0}" = "1" ] && [ "${BLE_VERSION+x}" ]; then
    ble-attach
fi
