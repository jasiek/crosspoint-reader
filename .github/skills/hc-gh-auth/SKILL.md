---
name: hc-gh-auth
description: Authenticate the gh CLI in Coder workspaces. Run before any gh pr create/merge/view command when gh auth status fails.
compatibility: Designed for Claude Code, Codex, OpenCode, and Gemini skills format.
metadata:
  owner: holicode
  scope: toolchain-auth
---

# gh Auth (GitHub CLI Authentication)

Ensure the `gh` CLI is authenticated before running any GitHub operations (`gh pr create`, `gh pr view`, `gh pr merge`, etc.).

## When to Run

Run this skill when:
- `gh auth status` returns a non-zero exit code or "not logged in"
- A `gh` command fails with "authentication required" or "401"
- Starting a new workspace session before the first PR operation

Do NOT run if `gh auth status` already succeeds — skip silently.

## Quick Check

```bash
gh auth status
```

- Exit code 0, output contains "Logged in to github.com" → already authenticated, stop here.
- Any other result → proceed with the steps below.

## Authentication Steps (agent / headless)

### Step 1: Try Coder external auth bridge

Coder workspaces may have a GitHub external auth provider configured. Zero-friction when available:

```bash
TOKEN=$(coder external-auth access-token github 2>/dev/null)
if [[ -n "$TOKEN" ]]; then
  echo "$TOKEN" | gh auth login --with-token
fi
```

Then verify:

```bash
gh auth status
```

If this succeeds → done. If it fails (Coder auth not configured, exit code non-zero, or token lacks scopes) → continue to Step 2.

### Step 2: Try GITHUB_TOKEN / GH_TOKEN env var

```bash
TOKEN="${GITHUB_TOKEN:-${GH_TOKEN:-}}"
if [[ -n "$TOKEN" ]]; then
  echo "$TOKEN" | gh auth login --with-token
fi
```

Verify with `gh auth status`. If successful → done. Otherwise → continue.

### Step 3: Escalate to human — direct them to the web UI

Agents cannot do interactive browser auth. Inform the user and direct them to the
gh-auth web app (device flow) bundled in this repo:

```bash
# Get the Coder port-forward URL for the auth server
PROXY="${VSCODE_PROXY_URI/\{\{port\}\}/3456}"
echo "Open the gh-auth web UI to authenticate:"
echo "  ${PROXY:-http://localhost:3456}"
echo ""
echo "Start the server with:"
echo "  node scripts/infra/gh-auth-app/server.mjs"
```

The web UI guides the user through the GitHub device flow (enter a short code at
github.com/login/device) and injects the resulting token directly into `gh auth login`.

As a secondary fallback, also provide the manual GitHub URL:

```bash
BRANCH=$(git rev-parse --abbrev-ref HEAD)
REMOTE=$(git remote get-url origin | sed 's|git@github.com:|https://github.com/|; s|\.git$||')
echo "Manual PR: ${REMOTE}/pull/new/${BRANCH}"
```

## Scope Check (if auth succeeds but commands fail)

If `gh auth status` shows logged in but commands still fail with permission errors, the token may lack required scopes:

```bash
gh auth status
```

Required scopes for PR operations: `repo` (or `public_repo` for public repos), `workflow` for Actions changes.

If scopes are insufficient — direct the user to re-authenticate via the web UI and select the needed scopes from the scope picker.

## Relationship to Other Skills

- **Called by**: `agentic-env-lifecycle` (Step 3 — PR Create) when gh is not authenticated
- **Complements**: `agentic-env-lifecycle` (lifecycle orchestration), `github-pr-create` workflow (PR creation)
- **Web UI**: `scripts/infra/gh-auth-app/server.mjs` — device flow + Coder bridge + scope picker
- **Does not handle**: PR creation itself — that is `github-pr-create` workflow's responsibility

## Constraints

- Never attempt interactive/browser-based auth — agents run headless
- Never store or log tokens
- Always provide human escalation path (web UI URL) when automated methods fail
- This skill is read/exec only — it does not modify any project files
