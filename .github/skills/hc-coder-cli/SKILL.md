---
name: hc-coder-cli
description: General-purpose Coder CLI and API reference for agents. Covers binary discovery, template management, workspace operations, external auth, and the REST API. Includes environment-agnostic patterns and key gotchas.
compatibility: Designed for Claude Code, Codex, OpenCode, and Gemini skills format.
metadata:
  owner: holicode
  scope: infra-tooling
---

# Coder CLI / API

General-purpose reference for agents operating in or managing Coder environments. Environment-agnostic — works in Coder workspaces, local machines, and CI.

## Binary Discovery

In Coder workspaces the agent binary lives in a random temp path. Locate it dynamically:

```bash
CODER_BIN=$(which coder 2>/dev/null || find /tmp -maxdepth 2 -name coder -type f 2>/dev/null | head -1)
if [ -z "$CODER_BIN" ]; then
  echo "ERROR: coder binary not found"
  exit 1
fi
$CODER_BIN version
```

> On running workspaces the binary is typically `/tmp/coder.<random>/coder`. `which coder` resolves it when it is on PATH (CI, local installs).

## Template Management

### List templates

```bash
$CODER_BIN templates list
```

### List versions for a template

```bash
$CODER_BIN templates versions list <template-name>
```

Filter to semver-only versions and get the latest:

```bash
$CODER_BIN templates versions list <template-name> \
  | grep -E '^\s*[0-9]+\.[0-9]+' | awk '{print $1}' | sort -V | tail -5
```

### Push a new template version

```bash
$CODER_BIN templates push <template-name> \
  --directory <path-to-directory-containing-main.tf> \
  --name <version> \
  --yes
```

**`--directory` is required.** Without it, Coder runs Terraform from the current working directory — if that directory does not contain `.tf` files, Terraform errors with "No configuration files".

### Holicode dual-template push (ARM + x86)

The ARM and x86 templates share an identical `main.tf` and `variables.tf`; only
`provider.tf` and `terraform.tfvars` differ. All four files must be present in the
directory — Coder uploads the whole directory, and `main.tf` uses `var.*` references.

| Change type | Push ARM (`holicode-agentic`) | Push x86 (`holicode-agentic-x86`) |
|-------------|-------------------------------|-----------------------------------|
| `main.tf` or `variables.tf` | Yes | Yes (use same version name) |
| `coder-template/provider.tf` or `terraform.tfvars` | Yes | No |
| `coder-template-x86/provider.tf` or `terraform.tfvars` | No | Yes |

```bash
# ARM
$CODER_BIN templates push holicode-agentic \
  --directory /home/coder/holicode/scripts/infra/coder-template \
  --name <version> --yes

# x86 (same version name when main.tf changed)
$CODER_BIN templates push holicode-agentic-x86 \
  --directory /home/coder/holicode/scripts/infra/coder-template-x86 \
  --name <version> --yes
```

After pushing, verify both succeeded:
```bash
$CODER_BIN templates versions list holicode-agentic | tail -3
$CODER_BIN templates versions list holicode-agentic-x86 | tail -3
```

### Semver bump rules

Given the latest existing version, pick the next:

| Change type | Bump | Example |
|---|---|---|
| Bug fix, config tweak, no new parameters | Patch | `1.19.4` → `1.19.5` |
| New feature, new Coder parameter, changed default | Minor | `1.19.4` → `1.20.0` |
| Breaking change (removed/renamed resource or parameter) | Major | `1.19.4` → `2.0.0` |

> **Gotcha**: A failed push attempt still registers the version name. Always re-check the version list after a failure — if your intended version already exists (even with status `Failed`), increment to avoid the "already in use" error.

## Workspace Operations

### List workspaces

```bash
$CODER_BIN list                        # workspaces owned by current user
$CODER_BIN list --all-users            # all workspaces (admin)
```

### Start / stop a workspace

```bash
$CODER_BIN start <workspace-name>
$CODER_BIN stop <workspace-name>
```

### SSH into a workspace

```bash
$CODER_BIN ssh <workspace-name>
```

### Open a port-forwarded URL

```bash
$CODER_BIN port-forward <workspace-name> --tcp <local-port>:<remote-port>
```

## External Auth

Coder can act as an OAuth bridge for GitHub (and other providers).

### Check available tokens

```bash
$CODER_BIN external-auth list
```

### Get a GitHub token (for gh CLI or API calls)

```bash
TOKEN=$($CODER_BIN external-auth access-token github 2>/dev/null)
if [ -n "$TOKEN" ]; then
  echo "$TOKEN" | gh auth login --with-token
fi
```

This is the preferred zero-friction auth path inside Coder workspaces. See the `gh-auth` skill for the full fallback chain.

## REST API

The Coder REST API is available at `$CODER_URL/api/v2/`. All requests require a token in `Authorization: Bearer <token>`.

### Get a token

```bash
# From an active workspace session
TOKEN="$CODER_AGENT_TOKEN"   # already set in workspace env

# From the CLI (outside a workspace)
TOKEN=$($CODER_BIN tokens create --name tmp-api-token)
```

### Common endpoints

```bash
BASE="$CODER_URL/api/v2"

# List templates
curl -sf -H "Authorization: Bearer $TOKEN" "$BASE/templates"

# Get template by name
curl -sf -H "Authorization: Bearer $TOKEN" "$BASE/organizations/default/templates/<name>"

# List template versions
curl -sf -H "Authorization: Bearer $TOKEN" "$BASE/templates/<template-id>/versions"

# List workspaces
curl -sf -H "Authorization: Bearer $TOKEN" "$BASE/workspaces"
```

## Terraform Heredoc Gotchas

When writing bash scripts inside Terraform `<<-EOT` heredocs, bash variables with curly braces are parsed as **Terraform interpolations** and cause errors like:

```
Error: Invalid reference — A reference to a resource type must be followed by at least one attribute access
```

**Fix**: escape curly-brace variables with `$$`:

```hcl
# Wrong — Terraform tries to resolve ${MY_VAR}
echo "Value: ${MY_VAR}"

# Correct — $$ produces a literal $ in the rendered script
echo "Value: $${MY_VAR}"
```

Plain `$MY_VAR` (no braces) does **not** need escaping.

## Fetching Public GitHub Releases (No Auth)

For **public** repos, use `curl` + the GitHub REST API instead of `gh release` commands. `gh release` requires authentication; `curl` does not.

```bash
# Latest release tag
LATEST=$(curl -sf "https://api.github.com/repos/<org>/<repo>/releases?per_page=10" \
  | jq -r '[.[] | select(.tag_name | test("^v[0-9]"))] | .[0].tag_name')

# Find tag matching a pinned version prefix
LATEST=$(curl -sf "https://api.github.com/repos/<org>/<repo>/releases?per_page=50" \
  | jq -r --arg v "$VERSION" '[.[] | select(.tag_name | startswith("v\($v)"))] | .[0].tag_name')

# Download a specific asset by name
URL=$(curl -sf "https://api.github.com/repos/<org>/<repo>/releases/tags/$LATEST" \
  | jq -r '.assets[] | select(.name == "my-asset.tgz") | .browser_download_url')
curl -fL "$URL" -o /tmp/my-asset.tgz
```

### BloopAI/vibe-kanban specifics

| Field | Format | Example |
|---|---|---|
| Release tag | `v<semver>-<timestamp14>` | `v0.1.40-20260401153532` |
| npm tarball asset | `vibe-kanban-<semver>.tgz` | `vibe-kanban-0.1.40.tgz` |

Extract bare semver from tag:

```bash
VK_VERSION=$(echo "$TAG" | sed 's/^v//' | sed 's/-[0-9]\{14\}$//')
# v0.1.40-20260401153532 → 0.1.40
```

## Relationship to Other Skills

- **`gh-auth`**: Full fallback chain for gh CLI authentication (Coder bridge → env var → web UI)
- **`agentic-env-lifecycle`**: Workspace session lifecycle (commit → push → PR → merge → new workspace)
- **`workspace-orchestrate`**: Higher-level skill for spinning up new sessions via Vibe Kanban MCP
