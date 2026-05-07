# Admin: Coder Template Updater

Pull-model script for rolling out new image / template versions to **the**
Coder server administered from this workspace. One admin Coder workspace
augments one Coder deployment — no multi-site registry, no central config.

New workspaces (or restarts of existing ones) on that Coder server pull
the new image from GHCR on next provision.

## One-time setup

```bash
# 1. Clone the repo if not already present
git clone git@github.com:holagence/holicode.git ~/repos/holicode

# 2. Scaffold ~/.config/holicode/ (idempotent — won't overwrite existing files)
~/repos/holicode/scripts/infra/admin/init.sh

# 3. Fill in the values
$EDITOR ~/.config/holicode/coder.env       # CODER_URL, TEMPLATE_NAME, token ref
$EDITOR ~/.config/holicode/template.tfvars # image_tag, GitHub App, domain, etc.
```

### Coder admin token

Two options:

- **1Password (recommended):** set `CODER_TOKEN_OP_REF` in
  `~/.config/holicode/coder.env` to a vault reference like
  `op://Holagence/Coder Admin/token`. The script reads it via `op read`
  at runtime.
- **Plain file:** place the token at `~/.coder-token` (chmod 600).

## Usage

```bash
~/repos/holicode/scripts/infra/admin/update-coder-template.sh
```

What it does:

1. Verify the holicode repo is on `main` with no uncommitted/untracked changes.
2. `git fetch` + `merge --ff-only` to sync with `origin/main`; abort if diverged.
3. Parse `image_name` and `image_tag` from `~/.config/holicode/template.tfvars`
   and print a summary of what's being deployed.
4. Resolve the Coder admin token (1Password or file).
5. `coder login` to the Coder server (isolated config dir per invocation —
   doesn't disturb any other `coder login` you may have on this workspace).
6. `coder templates push <name> --variable-file template.tfvars --yes`.

After it returns: new workspaces (or restarts of existing ones) on that Coder
server pull `ghcr.io/holagence/holicode-cde:<image_tag>` automatically — no
manual `docker pull` needed on the VPS.

## Why this lives on the deployment's admin workspace

GitHub Actions doesn't reach into VPS networks. The admin workspace is the
operator's local control plane for that deployment: it has the Coder admin
token, the per-deployment tfvars (with GitHub App credentials), and direct
network access to the Coder server. Per-deployment secrets stay on that
workspace — never in the repo, never in CI.

If you administer a second Coder deployment, spin up a second admin
workspace there. There's deliberately no "manage all my deployments from
one place" mode — that pattern collects secrets and creates blast radius
for very little operational gain.

## Forward path (not yet)

Today the script needs a full holicode repo clone (it reads the Coder
template terraform from `scripts/infra/coder-template-x86/`). The
`HOLICODE_TEMPLATE_DIR` env var is wired through so a future bundle install
(scripts + template terraform shipped as a release artifact) can point
elsewhere without a script change.

## Troubleshooting

- **`coder: command not found`** — workspace image (≥ 2.1.0) bundles the Coder
  CLI. If missing, install with `curl -fsSL https://coder.com/install.sh | sh`.
- **`unauthorized: HTTP 401`** — token expired. Regenerate in the Coder UI
  (Account → Tokens) and update the vault item or `~/.coder-token`.
- **Template push reports "no changes"** — `template.tfvars` hasn't changed
  since the last push. Confirm `image_tag` was bumped.
- **Workspace still on old image after push** — Coder only re-pulls on
  workspace start. Stop + start (or rebuild) the affected workspace.
