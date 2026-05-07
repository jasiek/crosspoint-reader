---
name: hc-github-pr-review
description: Analyze PR reviews and checks via script, generate FIX tasks, and emit a JSON summary.
metadata:
  owner: holicode
  scope: github-pr
  origin: workflows/github-pr-review.md
---

# GitHub PR Review Analysis (Slim Caller)

## Agent Identity
Role: PR review feedback processor and learning system integrator  
Responsibilities:
- Fetch and analyze PR reviews, comments, and CI status via machine-layer script
- Categorize feedback by type and priority (script-level)
- Generate FIX tasks for must-fix items (script-level)
- Emit strict JSON summary for downstream use

Success Criteria:
- Review data collected and summarized
- Must-fix items turned into FIX tasks in `.holicode/specs/tasks/`
- JSON summary captured for follow-ups

## Prerequisites
- PR exists (current branch or explicit `--pr` number)
- GitHub CLI authenticated (`gh auth status`)

## Machine Layer (JSON)
- Entrypoint: `scripts/pr/review.sh`
- Output JSON (example):
```json
{
  "ok": true,
  "action": "pr.review",
  "result": {
    "pr": { "title": "feat: add auth", "author": { "login": "user" }, "url": "..." },
    "approvals": 1,
    "changesRequested": 0,
    "totalComments": 5,
    "checks": [ /* gh checks json */ ],
    "mustFix": [ /* detected must-fix items */ ],
    "createdTasks": ["FIX-PR123-001","FIX-PR123-002"]
  },
  "warnings": [],
  "metrics": {}
}
```

## Process

### 1. Fetch PR Data
```bash
gh pr view <pr-number> --json reviews,comments,checks
```

### 2. Categorize Feedback (LLM Analysis)

#### Clear Severity Thresholds:
**Must-Fix (blocking - create FIX tasks):**
- Security vulnerabilities (any severity)
- Breaking changes to public APIs
- Data loss risks
- Failed CI checks
- Explicit "Request Changes" review status
- Comments with keywords: "blocker", "critical", "must fix"

**Should-Fix (strongly recommended):**
- Performance regressions >20%
- Missing error handling in user-facing code
- Direct reviewer requests (comments with "please" or "should")

**Consider (optional):**
- Style preferences
- Alternative approaches
- Performance improvements <20%

**Questions (needs clarification):**
- Any comment ending with "?"
- Comments requesting explanation

### 3. Generate FIX Tasks
For each must-fix item:
- Create `.holicode/specs/tasks/FIX-PR<number>-<seq>.md`
- Link to PR comment
- Group related comments into single task
- Define clear acceptance criteria

### 4. Post Summary
```bash
gh pr comment <pr-number> --body "<summary>"
```

Note: When categorization is unclear, escalate to user with rationale.

### 5. Machine Layer (Script Invocation)
```bash
# For automated script-based review analysis
mkdir -p .holicode/tmp

# Analyze current branch's PR (auto-detected)
scripts/pr/review.sh > .holicode/tmp/last_pr_review.json

# Or target a specific PR:
# scripts/pr/review.sh --pr 123 > .holicode/tmp/last_pr_review.json
```

## Error Handling
- Script emits strict JSON with codes:
  - GH_CLI_MISSING / GH_AUTH_MISSING / GH_NOT_READY
  - NO_PR
- Consume stdout JSON only; ignore stderr logs (human diagnostics).

## Notes
- All categorization and task generation are implemented in `scripts/pr/review.sh`.
- Follow up with PR update/CI monitor workflows as needed.
