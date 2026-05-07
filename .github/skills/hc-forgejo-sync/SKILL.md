---
name: hc-forgejo-sync
description: Register or sync local repos in the Forgejo sidecar. Use when a repo is missing from Forgejo, or the web UI shows stale commits/branches.
compatibility: Requires Coder workspace with Forgejo sidecar (holicode-agentic template).
metadata:
  owner: holicode
  scope: infra-workspace
---

# Forgejo Sync

Use this skill when:
- A repo is missing from the Forgejo web UI
- The web UI shows stale commits or old branch tips
- A new repo was cloned into the workspace mid-session

## How it works

Forgejo is configured as a **pull mirror** for each repo in `/home/coder/*/`. It uses
`POST /api/v1/repos/migrate` with `clone_addr=file:///home/coder/<repo>` to create mirrors
that Forgejo manages natively — fetching, branch listing, and DB updates are all handled
internally. This means Forgejo's web UI is always in sync after a mirror-sync call.

The background daemon in `forgejo_init` calls `POST /mirror-sync` every 60s for all repos.

Forgejo URL: `http://localhost:3001` (socat proxy: workspace → sidecar container)
Credentials: `$CODER_WORKSPACE_OWNER_NAME / coder-forgejo-local`

## Procedure

### Instant sync (any case)

```bash
bash ~/holicode/scripts/infra/forgejo-adopt.sh
```

Auto-detects the repo from the current working directory (works from git worktrees).
If the repo is already in Forgejo, triggers `mirror-sync`. If not, creates a mirror.

### Specific repo

```bash
bash ~/holicode/scripts/infra/forgejo-adopt.sh holicode
```

### All repos at once

```bash
bash ~/holicode/scripts/infra/forgejo-adopt.sh --all
```

### Direct API (no script)

```bash
# Trigger immediate sync for a specific repo
curl -sf -X POST http://localhost:3001/api/v1/repos/ciekawy/holicode/mirror-sync \
  -u ciekawy:coder-forgejo-local && echo "Sync triggered"
```

## Requirements

The Forgejo container must have `ALLOW_LOCALNETWORKS=true` set:
```
FORGEJO__migrations__ALLOW_LOCALNETWORKS=true
```
This is set via env var in `main.tf`. If missing (old workspace), the migrate API returns 422
"You can not import from disallowed hosts" — fix by restarting the workspace to pick up the
updated template.

## Key facts

- **Native mirror approach**: Forgejo owns fetch + DB sync via `mirror-sync`. No manual bare
  mirror management, no branch registration scripts, no refspec workarounds.
- **file:// works because**: both workspace and Forgejo containers mount the same home volume
  at `/home/coder`. From Forgejo's perspective, `file:///home/coder/holicode` is a local path.
- **Worktrees**: all VK worktree branches live in the base `.git/refs/heads/`. A single
  `mirror-sync` on the base repo picks up all branches from all worktrees.
- **mirror_interval=10m**: Forgejo also auto-syncs every 10 minutes independently of the daemon.
- **Credentials**: basic auth `<owner>:coder-forgejo-local` from inside the workspace.

## Verification

```bash
# Is the repo registered as a mirror?
curl -sf http://localhost:3001/api/v1/repos/ciekawy/holicode \
  -u ciekawy:coder-forgejo-local | python3 -c "import sys,json; r=json.load(sys.stdin); print('mirror:', r.get('mirror'), 'private:', r.get('private'))"

# How many branches does Forgejo know about?
curl -sf "http://localhost:3001/api/v1/repos/ciekawy/holicode/branches?limit=1" \
  -u ciekawy:coder-forgejo-local -D - 2>/dev/null | grep -i x-total-count

# Web UI reachable?
curl -so /dev/null -w "%{http_code}\n" http://localhost:3001/ciekawy/holicode
```

## Historical note (pre-mirror approach)

Earlier versions used `git clone --bare` + `POST /admin/unadopted` (the "adopt" API). This
approach required manual branch registration after every `git fetch` because fetching bypassed
Forgejo's post-receive hooks. Numerous bugs were found and fixed: bad fetch refspecs from local
clones, Forgejo's 50/page branch API cap, pipe subshell counter bugs. The native mirror approach
eliminates all of this — `mirror-sync` handles everything in one API call.
