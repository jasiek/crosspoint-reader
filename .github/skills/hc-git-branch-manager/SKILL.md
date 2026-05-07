---
name: hc-git-branch-manager
description: Manage git branches safely via the repo's branch script (create/switch/cleanup/validate).
metadata:
  owner: holicode
  scope: git-branch
  origin: workflows/git-branch-manager.md
---

# Git Branch Manager (Slim Caller)

## Agent Identity
Role: Systematic branch operations manager  
Responsibilities:
- Delegate all branch operations to machine-layer script
- Keep guidance human-readable and minimal
- Ensure consistent naming and safe switching/cleanup

Success Criteria:
- Zero data loss during switches
- Consistent naming conventions
- Clean Git tree
- Strict JSON output captured for downstream steps

## Prerequisites
- Git repository initialized (`.git` exists)
- Git identity configured (name/email)
- For remote operations: GitHub CLI authenticated (`gh auth status`)

## Machine Layer (JSON)
- Entrypoint: `scripts/git/branch.sh`
- Actions:
  - `--action create` with type/name options
  - `--action switch --branch NAME`
  - `--action cleanup [--base main]`
  - `--action validate [--name NAME]`

Example output (create):
```json
{
  "ok": true,
  "action": "git.branch",
  "result": {
    "mode": "create",
    "branch": "feat/TASK-001-implement-auth",
    "base": "main",
    "pushed": true,
    "previous": "main"
  },
  "warnings": [],
  "metrics": {}
}
```

## Process (Thin Invocation)

This workflow acts as a thin caller for the `scripts/git/branch.sh` script, which handles all the core logic.

### 1. Create Branches

To create a new branch:

```bash
scripts/git/branch.sh --action create --type <type> [options]
```
- `<type>`: `spec`, `feat`, `fix`, `chore`, or `release`.
- `[options]`:
  - For `spec`: `--phase <phase>` (`business|functional|technical|plan`) and `--feature-id <FEATURE-ID>`.
  - For `feat`: `--task-id <TASK-ID>` and optional `--description "<kebab-words>"`.
  - For `fix`: `--issue <issue-number>` and optional `--description "<kebab-words>"`.
  - For `chore`: `--description "<kebab-words>"`.
  - For `release`: `--version <vX.Y.Z>`.
  - Use `--name "<full-branch-name>"` to provide an explicit full branch name (skips composition).
  - Use `--base "<base-branch>"` to specify the base branch (default: `main`).

Example:
```bash
# Create a specification branch
scripts/git/branch.sh --action create --type spec --phase technical --feature-id "FEATURE-001"

# Create a feature branch
scripts/git/branch.sh --action create --type feat --task-id "TASK-001" --description "implement-auth"

# Create a fix branch
scripts/git/branch.sh --action create --type fix --issue 123 --description "resolve-memory-leak"

# Create a chore branch
scripts/git/branch.sh --action create --type chore --description "update-dependencies"

# Create a release branch
scripts/git/branch.sh --action create --type release --version v1.2.3
```

### 2. Switch Branch (Safe)

To safely switch to an existing branch (includes auto-stashing and handling remote updates):

```bash
scripts/git/branch.sh --action switch --branch <branch-name>
```
Example:
```bash
scripts/git/branch.sh --action switch --branch "feat/TASK-001-implement-auth"
```

### 3. Cleanup Merged Branches

To clean up locally merged branches (excluding the specified base branch):

```bash
scripts/git/branch.sh --action cleanup --base <base-branch>
```
Example:
```bash
scripts/git/branch.sh --action cleanup --base main
```

### 4. Validate Branch Name

To validate the current branch name or an explicit branch name against conventions:

```bash
scripts/git/branch.sh --action validate [--name <branch-name>]
```
- `[--name <branch-name>]` (optional): The branch name to validate. If omitted, the current branch is validated.

Example:
```bash
scripts/git/branch.sh --action validate
# or
scripts/git/branch.sh --action validate --name "feat/TASK-001-implement-auth"
```


## Error Handling
Script emits strict JSON with error codes:
- `NOT_A_GIT_REPO` — Not a git repository
- `BAD_REQUEST` — Missing/invalid arguments (e.g., missing `--task-id`)
- `BAD_BRANCH_NAME` — Does not match naming conventions
- `BAD_ACTION` — Unknown `--action`

Consume stdout JSON only; ignore stderr logs (human diagnostics).

## Notes
- Branch naming is validated by project conventions implemented in `scripts/lib/git.sh`:
  - Allowed patterns include:
    - `spec/<phase>/<FEATURE-ID>`
    - `feat/<TASK-ID>-<kebab-description>`
    - `fix/<issue|kebab>`
    - `chore/<kebab>`
    - `release/v<semver>`
- Remote operations are non-blocking: pushes are attempted but failures (offline) do not break local flow.
- Switching uses safe checkout with auto-stash and optional rebase/pull when remote exists.
