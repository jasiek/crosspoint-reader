---
name: github-pr-create
description: Create a pull request using the repo's PR creation script and a selected template.
---

# GitHub PR Create Workflow (Slim Caller)

## Agent Identity
Role: Pull request creation and management specialist  
Responsibilities:
- Create PRs using machine-layer script
- Apply template-based body
- Set metadata (labels, reviewers) via follow-up update workflow if needed

Success Criteria:
- PR created with proper title/body
- JSON result captured for downstream steps
- Non-blocking operation with robust error reporting

## Prerequisites
- Git repository with remote configured
- Current branch contains commits not in base
- GitHub CLI authenticated (`gh auth status`)

## Machine Layer (JSON)
- Entrypoint: `scripts/pr/create.sh`
- Output JSON (example):
```json
{
  "ok": true,
  "action": "pr.create",
  "result": {
    "prNumber": 123,
    "url": "https://github.com/org/repo/pull/123",
    "type": "implementation",
    "branch": "feat/TASK-001-thing",
    "base": "main"
  },
  "warnings": [],
  "metrics": { "durationMs": 0, "retries": 0 }
}
```

## Process

### 1. Determine Branch Type and Template
- Get current branch: `git branch --show-current`
- Match pattern to select template:
  - `spec/*` → `templates/github/pr-spec-template.md`
  - `feat/*` → `templates/github/pr-impl-template.md`
  - `fix/*` → `templates/github/pr-fix-template.md`

#### Fallback Decision Tree (when pattern doesn't match):
1. Check branch prefix (strict match first)
2. If no match → analyze commit messages (majority type wins)
3. If still ambiguous → analyze changed files:
   - src/* changes → implementation template
   - .holicode/specs/* → specification template
   - Only docs/* → documentation (use spec template)
4. Last resort → ask user or use general implementation template

### 2. Derive PR Title
#### Title Priority Rules (explicit hierarchy):
1. If single commit AND conventional format → use it
2. If multiple commits → analyze all commits + branch name:
   - Extract task/feature ID from branch if present
   - Synthesize description from commit patterns
   - Format: `<type>(<task-id>): <synthesized-description>`
3. If branch has task ID → always include it: `feat(TASK-123): <description>`
4. If commits don't match branch type → prefer branch type for consistency
5. When ambiguous → escalate to user with best guess proposal

### 3. Populate Template
- Extract task/feature ID from branch name
- Analyze ALL commits in branch for context
- Fill placeholders with context from:
  - Commit messages (all, not just latest)
  - Changed files summary
  - Task specification (if .holicode/specs/tasks/TASK-ID.md exists)
- Leave human-required fields as placeholders (screenshots, risk assessment)

### 4. Create PR
```bash
gh pr create \
    --title "<derived-title>" \
    --body-file "<populated-template>" \
    --base "${BASE_BRANCH:-main}" \
    --label "<appropriate-labels>"
```

### 5. Machine Layer (Script Invocation)
```bash
# For automated script-based PR creation
mkdir -p .holicode/tmp
scripts/pr/create.sh > .holicode/tmp/last_pr.json

# Optional: add metadata after creation
# scripts/pr/update.sh --action labels --op add --labels "ready-for-review"
# scripts/pr/update.sh --action reviewers --op add --reviewers "@me"
```

## Error Handling
- Script emits strict JSON with error codes:
  - GH_CLI_MISSING / GH_AUTH_MISSING / GH_NOT_READY
  - NOT_A_GIT_REPO / NO_COMMITS / PR_CREATE_FAILED / PR_INFO_MISSING
- Do not parse stderr; consume stdout JSON only.

## Notes
- Keep human-facing guidance here; delegate all logic to `scripts/pr/create.sh`.
- For advanced configuration (base branch override), set env `BASE_BRANCH=main` when invoking.
