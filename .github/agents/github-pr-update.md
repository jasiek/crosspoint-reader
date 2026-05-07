---
name: github-pr-update
description: Update an existing PR (labels/reviewers/comments/status) via the repo's PR update script.
---

# GitHub PR Update Workflow (Slim Caller)

## Agent Identity
Role: Pull request update and status management specialist  
Responsibilities:
- Update PR description and metadata via machine-layer script
- Synchronize PR status with HoliCode state (optional, in follow-ups)
- Add comments, labels, reviewers, and issue links

Success Criteria:
- Requested PR update is applied
- JSON result captured for downstream steps
- Non-destructive operations with robust error reporting

## Prerequisites
- Existing PR for current branch or explicit `--pr` number
- GitHub CLI authenticated (`gh auth status`)

## Definition of Ready (DoR)
- [ ] Update type determined (description/labels/reviewers/comment/link-issues/status)
- [ ] Necessary arguments provided (e.g., `--labels`, `--reviewers`, `--text`, `--issues`)

## Definition of Done (DoD)
- [ ] PR updated successfully per action
- [ ] JSON saved to `.holicode/tmp/last_pr_update.json` (when captured)

## Machine Layer (JSON)
- Entrypoint: `scripts/pr/update.sh`
- Actions supported:
  - `status`
  - `labels --op add|remove|set --labels "a,b"`
  - `reviewers --op add|remove --reviewers "user1,user2"`
  - `comment --text "message"`
  - `link-issues --link-type closes|fixes|related --issues "1,2"`

Example output (for `status`):
```json
{
  "ok": true,
  "action": "pr.update",
  "result": {
    "number": 123,
    "state": "OPEN",
    "mergeStateStatus": "BLOCKED",
    "reviews": [/* ... */],
    "statusCheckRollup": [/* ... */],
    "url": "https://github.com/org/repo/pull/123"
  },
  "warnings": [],
  "metrics": {}
}
```

## Process (Thin Invocation)
```bash
mkdir -p .holicode/tmp

# Get PR status for current branch PR (auto-detected)
scripts/pr/update.sh --action status > .holicode/tmp/last_pr_update.json

# Add labels
scripts/pr/update.sh --action labels --op add --labels "ready-for-review,implementation"

# Remove labels
scripts/pr/update.sh --action labels --op remove --labels "blocked"

# Set labels (replace all)
scripts/pr/update.sh --action labels --op set --labels "ready-for-merge"

# Add reviewers
scripts/pr/update.sh --action reviewers --op add --reviewers "@tech-lead,@architect"

# Add a comment
scripts/pr/update.sh --action comment --text "CI is green; ready for review."

# Link issues
scripts/pr/update.sh --action link-issues --link-type closes --issues "456,789"
```

## Error Handling
- Script emits strict JSON with error codes:
  - GH_CLI_MISSING / GH_AUTH_MISSING / GH_NOT_READY
  - NO_PR / BAD_REQUEST / BAD_ACTION / BAD_OP / MISSING_* (arguments)
- Parse stdout JSON only; ignore stderr logs (human diagnostics).

## Notes
- Keep this workflow lean; all logic lives in `scripts/pr/update.sh`.
- Combine with CI monitor and review workflows as needed.
