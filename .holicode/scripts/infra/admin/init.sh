#!/usr/bin/env bash
# Scaffold ~/.config/holicode/ with admin config skeletons.
#
# Idempotent: existing files are never overwritten. Re-run safely after
# updating the example templates upstream — only missing files are created.

set -euo pipefail

HOLICODE_REPO="${HOLICODE_REPO:-$HOME/repos/holicode}"
ADMIN_DIR="${HOLICODE_ADMIN_DIR:-$HOME/.config/holicode}"

CODER_ENV_EXAMPLE="$HOLICODE_REPO/scripts/infra/admin/coder.env.example"
TFVARS_EXAMPLE="$HOLICODE_REPO/scripts/infra/coder-template-x86/terraform.tfvars.example"

[ -f "$CODER_ENV_EXAMPLE" ] || { echo "Missing example: $CODER_ENV_EXAMPLE" >&2; exit 1; }
[ -f "$TFVARS_EXAMPLE" ]    || { echo "Missing example: $TFVARS_EXAMPLE" >&2; exit 1; }

mkdir -p "$ADMIN_DIR"

CODER_ENV="$ADMIN_DIR/coder.env"
TFVARS="$ADMIN_DIR/template.tfvars"

if [ ! -f "$CODER_ENV" ]; then
  cp "$CODER_ENV_EXAMPLE" "$CODER_ENV"
  chmod 600 "$CODER_ENV"
  echo "Created $CODER_ENV"
else
  echo "Exists,  $CODER_ENV (left untouched)"
fi

if [ ! -f "$TFVARS" ]; then
  cp "$TFVARS_EXAMPLE" "$TFVARS"
  chmod 600 "$TFVARS"
  echo "Created $TFVARS"
else
  echo "Exists,  $TFVARS (left untouched)"
fi

cat <<EOF

Next:
  1. Edit $CODER_ENV     (CODER_URL, TEMPLATE_NAME, CODER_TOKEN_OP_REF)
  2. Edit $TFVARS        (image_tag, github_app_*, domain, etc.)
  3. Run: $HOLICODE_REPO/scripts/infra/admin/update-coder-template.sh
EOF
