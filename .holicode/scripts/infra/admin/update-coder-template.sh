#!/usr/bin/env bash
# Pull-model template updater for the Coder server administered from this
# workspace.
#
# Run this whenever you want to roll out a new image / template version to
# the Coder deployment this admin workspace augments.
#
# Prereqs (one-time):
#   - HoliCode repo cloned to ~/repos/holicode (override with HOLICODE_REPO)
#   - Admin config at ~/.config/holicode/coder.env
#     (see scripts/infra/admin/coder.env.example)
#   - Template tfvars at ~/.config/holicode/template.tfvars
#     (see scripts/infra/coder-template-x86/terraform.tfvars.example)
#   - Coder admin token sourced from 1Password (CODER_TOKEN_OP_REF in coder.env)
#     OR placed at ~/.coder-token
#
# Run scripts/infra/admin/init.sh once to scaffold the config skeleton.

set -euo pipefail

usage() {
  cat >&2 <<EOF
Usage: $(basename "$0")

Reads admin config from \$HOLICODE_ADMIN_DIR (default: ~/.config/holicode):
  coder.env        — CODER_URL, TEMPLATE_NAME, CODER_TOKEN_OP_REF
  template.tfvars  — image_tag, github_app_*, domain, etc.

Environment overrides:
  HOLICODE_REPO          path to holicode checkout (default: ~/repos/holicode)
  HOLICODE_ADMIN_DIR     path to admin config dir (default: ~/.config/holicode)
  HOLICODE_TEMPLATE_DIR  path to Coder template terraform dir
                         (default: \$HOLICODE_REPO/scripts/infra/coder-template-x86)
EOF
  exit 2
}

[ "${1:-}" = "-h" ] || [ "${1:-}" = "--help" ] && usage

HOLICODE_REPO="${HOLICODE_REPO:-$HOME/repos/holicode}"
ADMIN_DIR="${HOLICODE_ADMIN_DIR:-$HOME/.config/holicode}"
TEMPLATE_DIR="${HOLICODE_TEMPLATE_DIR:-$HOLICODE_REPO/scripts/infra/coder-template-x86}"
CODER_ENV="$ADMIN_DIR/coder.env"
TFVARS="$ADMIN_DIR/template.tfvars"

[ -d "$HOLICODE_REPO/.git" ] || { echo "Not a git repo: $HOLICODE_REPO" >&2; exit 1; }
[ -d "$TEMPLATE_DIR" ]       || { echo "Missing template dir: $TEMPLATE_DIR" >&2; exit 1; }
[ -f "$CODER_ENV" ]          || { echo "Missing admin config: $CODER_ENV (run init.sh)" >&2; exit 1; }
[ -f "$TFVARS" ]              || { echo "Missing tfvars: $TFVARS (run init.sh)" >&2; exit 1; }

# Verify clean worktree on main before deploying
REPO_BRANCH="$(git -C "$HOLICODE_REPO" symbolic-ref --short HEAD 2>/dev/null || true)"
[ "$REPO_BRANCH" = "main" ] || { echo "Repo is on '$REPO_BRANCH', expected 'main': $HOLICODE_REPO" >&2; exit 1; }

if [ -n "$(git -C "$HOLICODE_REPO" status --porcelain)" ]; then
  echo "Repo has uncommitted or untracked changes — commit, stash, or remove first:" >&2
  git -C "$HOLICODE_REPO" status --short >&2
  exit 1
fi

# Source admin config (CODER_URL, TEMPLATE_NAME, optional CODER_TOKEN_OP_REF)
# shellcheck disable=SC1090
source "$CODER_ENV"

: "${CODER_URL:?CODER_URL must be set in $CODER_ENV}"
: "${TEMPLATE_NAME:?TEMPLATE_NAME must be set in $CODER_ENV}"

# Sync repo to latest main and verify HEAD matches origin/main
echo "==> Pulling latest holicode/main..."
git -C "$HOLICODE_REPO" fetch origin main
git -C "$HOLICODE_REPO" merge --ff-only origin/main

LOCAL_HEAD="$(git -C "$HOLICODE_REPO" rev-parse HEAD)"
REMOTE_HEAD="$(git -C "$HOLICODE_REPO" rev-parse origin/main)"
if [ "$LOCAL_HEAD" != "$REMOTE_HEAD" ]; then
  echo "HEAD ($LOCAL_HEAD) != origin/main ($REMOTE_HEAD) — local main has diverged" >&2
  exit 1
fi

# Parse image_tag from tfvars (source of truth for what Coder will deploy)
TFVARS_IMAGE_TAG="$(grep -Po '^\s*image_tag\s*=\s*"\K[^"]+' "$TFVARS" || true)"
[ -n "$TFVARS_IMAGE_TAG" ] || { echo "Cannot parse image_tag from $TFVARS" >&2; exit 1; }

# Show what's about to deploy (values from tfvars)
TFVARS_IMAGE_NAME="$(grep -Po '^\s*image_name\s*=\s*"\K[^"]+' "$TFVARS" || true)"
EFFECTIVE_IMAGE_NAME="${TFVARS_IMAGE_NAME:-ghcr.io/holagence/holicode-cde}"
cat <<EOF
==> About to push template to:
    coder_url:   $CODER_URL
    template:    $TEMPLATE_NAME
    image_name:  $EFFECTIVE_IMAGE_NAME
    image_tag:   $TFVARS_IMAGE_TAG
EOF

# Verify the workspace image exists in its registry before pushing — avoids
# leaving the deployed template pointing at a tag that hasn't been built yet.
echo "==> Verifying image availability..."
if ! docker manifest inspect "${EFFECTIVE_IMAGE_NAME}:${TFVARS_IMAGE_TAG}" >/dev/null 2>&1; then
  echo "Image ${EFFECTIVE_IMAGE_NAME}:${TFVARS_IMAGE_TAG} not found in registry — push aborted." >&2
  echo "    For private registries, run: docker login <registry>" >&2
  exit 1
fi

# Resolve admin token
if [ -n "${CODER_TOKEN_OP_REF:-}" ]; then
  echo "==> Reading Coder admin token from 1Password ($CODER_TOKEN_OP_REF)..."
  TOKEN="$(op read "$CODER_TOKEN_OP_REF")"
elif [ -f "$HOME/.coder-token" ] && [ -s "$HOME/.coder-token" ]; then
  TOKEN="$(cat "$HOME/.coder-token")"
else
  echo "Cannot resolve Coder admin token: set CODER_TOKEN_OP_REF in $CODER_ENV or place token at ~/.coder-token" >&2
  exit 1
fi

# Isolate Coder CLI state from any other coder login on this workspace
CODER_CONFIG_DIR="$(mktemp -d)"
export CODER_CONFIG_DIR
cleanup() { rm -rf "$CODER_CONFIG_DIR"; }
trap cleanup EXIT

# Login + push
echo "==> Logging in to $CODER_URL..."
coder login "$CODER_URL" --token "$TOKEN" >/dev/null

echo "==> Pushing template..."
cd "$TEMPLATE_DIR"
coder templates push "$TEMPLATE_NAME" --variable-file "$TFVARS" --yes

cat <<EOF

==> Done.
    New workspaces (or restarts of existing ones) will pull
    ${TFVARS_IMAGE_NAME:-ghcr.io/holagence/holicode-cde}:${TFVARS_IMAGE_TAG} on next provision.
EOF
