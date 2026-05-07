---
name: git-commit-manager
description: Create safe, conventional commits via the repo's commit script and report results.
---

# Git Commit Manager (Slim Caller)

## Agent Identity
Role: Semantic commit operations manager  
Responsibilities:
- Delegate all commit logic to machine-layer script
- Ensure conventional commits and atomic changes
- Keep guidance human-readable and minimal

Success Criteria:
- All commits follow conventional format (warning-only if not)
- Meaningful messages, atomic commits
- Local commits always succeed
- Strict JSON output captured for downstream steps

## Prerequisites
- Git repository initialized (`.git` exists)
- Git identity configured (name/email)
- Changes exist in working tree

## Machine Layer (JSON)
- Entrypoint: `scripts/git/commit.sh`
- Actions:
  - `--action analyze`                                   # detect category and list files
  - `--action status`                                    # latest commit summary (hash/subject/timestamp)
  - `--action validate-message --message "feat(...):"`   # validate message format
  - `--action auto [--workflow NAME --context ID]`       # derive message & stage by category
  - `--action commit [--message "..."]` OR `--type/--scope/--subject` (explicit)

Example output (commit/auto):
```json
{
  "ok": true,
  "action": "git.commit",
  "result": {
    "mode": "auto",
    "commit": "9ab369f...",
    "branch": "feat/TASK-001-implement-auth",
    "message": "feat(TASK-001): apply implementation for TASK-001",
    "category": "implementation",
    "stagedFiles": ["src/app.ts","package.json"],
    "pushed": true,
    "amended": false
  },
  "warnings": [
    { "code": "NON_CONVENTIONAL_MESSAGE", "message": "Commit does not match conventional commit format" }
  ],
  "metrics": {
    "branch": "feat/TASK-001-implement-auth",
    "counts": { "staged": 2, "total": 2 }
  }
}
```

Example output (status):
```json
{
  "ok": true,
  "action": "git.commit",
  "result": {
    "hash": "9ab369f...",
    "subject": "feat(scope): subject",
    "committedAt": "2025-08-16T19:55:01+00:00"
  },
  "warnings": [],
  "metrics": {}
}
```

## Process (Thin Invocation)

This workflow acts as a thin caller for the `scripts/git/commit.sh` script, which handles all the core logic.

### 1. Analyze Changes

To analyze changes in the working tree and get suggested commit categories:

```bash
scripts/git/commit.sh --action analyze
```

### 2. Auto Commit

To automatically stage files by category, derive a commit message from the workflow context, and commit:

```bash
scripts/git/commit.sh --action auto --workflow <workflow-name> --context <context-id> [options]
```
- `<workflow-name>`: The name of the current workflow (e.g., `task-implement`, `state-update`).
- `<context-id>`: An identifier relevant to the workflow (e.g., `TASK-001`, `FEATURE-002`).
- `[options]`: Optional overrides for the derived message: `--type`, `--scope`, `--subject`, `--body`.
- Use `--amend` to amend the last commit (non-destructive, reuses staged files).
- Use `--no-push` to skip pushing to the remote.

Example:
```bash
# Auto commit for a task implementation
scripts/git/commit.sh --action auto --workflow "task-implement" --context "TASK-001"
```

### 3. Explicit Commit

To create a commit with an explicitly defined message:

```bash
scripts/git/commit.sh --action commit --type <type> --scope <scope> --subject "<subject>" [--body "<body>"] [options]
```
- `<type>`: (e.g., `feat`, `fix`, `docs`, `chore`).
- `<scope>`: (e.g., `api`, `ui`, `auth`).
- `<subject>`: A concise description of the change.
- `[--body "<body>"]` (optional): Detailed commit message body.
- Alternatively, use `--message "<full-conventional-message>"` to provide the entire message string.
- Use `--amend` to amend the last commit.
- Use `--no-push` to skip pushing to the remote.

Example:
```bash
# Explicit commit with conventional fields
scripts/git/commit.sh --action commit --type "feat" --scope "auth" --subject "add JWT authentication" --body "- implement strategy\n- add guards"

# Explicit commit with precomposed message
scripts/git/commit.sh --action commit --message "fix(api): handle null pointer in TaskService"
```

### 4. Validate Message Text

To validate a commit message text against conventional commit format rules without committing:

```bash
scripts/git/commit.sh --action validate-message --message "<message-text>"
```
Example:
```bash
scripts/git/commit.sh --action validate-message --message "feat(ui): improve dashboard"
```

### 5. Get Latest Commit Status

To get a summary of the latest commit:

```bash
scripts/git/commit.sh --action status
```

## Arguments
- Common:
  - `--amend` amend last commit
  - `--no-push` skip push (local only)
  - `--stage` force category: `specification|implementation|state|documentation|workflow|all`
- Auto:
  - `--workflow NAME` (e.g., `task-implement`, `state-update`)
  - `--context ID` (e.g., `TASK-001`)
  - Optional overrides: `--type/--scope/--subject/--body`
- Commit (explicit):
  - Either `--message "type(scope): subject"` OR `--type t --scope s --subject "text"` (plus optional `--body`)

## Error Handling
Script emits strict JSON with error codes:
- `NOT_A_GIT_REPO` — Not a git repository
- `NO_CHANGES` — Nothing staged/changed to commit
- `BAD_REQUEST` — Missing/invalid arguments (e.g., missing `--message` for validate)
- `BAD_ACTION` — Unknown `--action`

Consume stdout JSON only; ignore stderr logs (human diagnostics).

## Notes
- Conventional commit validation is warning-only to preserve flow; warnings are returned in JSON.
- Staging is inferred from change category with cross-cutting safe defaults; use `--stage` to override.
- Push is attempted but non-blocking; local commits always succeed. Use `--no-push` to skip remote attempts.
- Message derivation for auto mode follows workflow → context mapping; explicit flags override derived values.
