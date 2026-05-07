---
name: hc-code-review
description: Cross-agent code review skill. Dispatches a review session to a different executor and produces a structured findings report. Reviewer MUST NOT implement fixes.
compatibility: Designed for Claude Code, Codex, OpenCode, and Gemini skills format.
metadata:
  owner: holicode
  scope: cross-agent-review
---

# Code Review

Dispatch a code review session to a different AI executor than the one that wrote the code. The reviewer produces a structured findings report — it MUST NOT implement fixes or modify source files.

This skill is **integration-agnostic** — the abstract process (output contract, executor selection, dispatch description, findings triage) works across any workspace provider. Provider-specific dispatch mechanics are documented separately at the end.

## When to Use

- Implementation work is complete and ready for review before PR merge
- User requests a cross-agent review ("get a second opinion", "review this with Codex")
- High-value or high-risk changes where a fresh perspective adds confidence
- After `task-implement` completes, before marking the issue "Done"
- Multi-file changes that benefit from holistic review beyond CI checks

## When NOT to Use

- **Trivial changes**: Single-line fixes, typo corrections, config-only changes — CI and human review suffice
- **Already reviewed by CI**: If the only concern is lint/test/build and CI passes, skip
- **No diff to review**: If the branch has no commits beyond the base branch
- **Mid-implementation**: Wait until the work is complete; reviewing incomplete work wastes tokens
- **Same-session self-review**: The originating session reviewing its own code has the same blind spots — dispatch to a different executor or fresh session

## Scope Boundaries

- This skill dispatches a review session and defines the output contract — it does not perform the review itself
- Code fixes belong to the implementation session, not the reviewer
- PR creation and lifecycle belong to `agentic-env-lifecycle` skill
- Issue status updates belong to `issue-tracker` skill

## Prerequisites

- Workspace dispatch capability available (see Provider-Specific Dispatch below)
- Changes committed and pushed to a remote branch
- An issue ID linked to the work being reviewed (for context)

## Findings-Only Output Contract

The reviewer session MUST adhere to these rules:

1. **DO NOT** modify any source files, config files, or state files
2. **DO NOT** create commits or push changes
3. **DO NOT** run `git add`, `git commit`, or any write operations
4. **DO** read all changed files, specs, and acceptance criteria
5. **DO** produce a single structured findings report (format below)
6. **DO** reference specific `file:line` locations for every finding

The reviewer is an auditor, not an implementer. Its only artifact is the findings report.

## Executor Selection

Prefer a **different executor** than the one that produced the implementation:

| Original Executor | Preferred Reviewer | Rationale |
|---|---|---|
| `CLAUDE_CODE` | `CODEX` or `OPENCODE` | Different model avoids shared blind spots |
| `CODEX` | `CLAUDE_CODE` | Claude's strength in spec adherence complements Codex |
| `OPENCODE` | `CLAUDE_CODE` | Cross-vendor diversity |
| `GEMINI` | `CLAUDE_CODE` | Cross-vendor diversity |

**Fallback**: If the preferred reviewer executor is unavailable, a fresh `CLAUDE_CODE` session (different workspace, no shared context) is an acceptable fallback. The key property is **no shared context with the implementation session**.

## Standard Procedure

### 1. Pre-Flight

Before dispatching a review:

1. **Verify changes are pushed**: `git log origin/<branch> --oneline -5` — if the remote branch is behind, push first
2. **Verify diff exists**: `git diff <base-branch>...<branch> --stat` — if empty, abort ("nothing to review")
3. **Collect dispatch context**:
   - Issue ID (human-readable, e.g. `HOL-42`)
   - Branch name
   - Base branch
   - PR URL (if PR already exists): `gh pr view --json url -q .url` or note "PR not yet created"
   - Brief summary of what was implemented (1-3 sentences)

### 2. Compose Dispatch Description

The dispatch description is the reviewer's only briefing. It MUST include all context needed to perform the review autonomously. Use this template:

```
## Code Review Request

**Issue**: <issue-id> — <issue-title>
**Branch**: <branch-name> (base: <base-branch>)
**PR**: <pr-url or "not yet created">

### What was implemented
<1-3 sentence summary of the changes>

### Review instructions
1. Fetch the issue details using the issue tracker to understand acceptance criteria
2. Run `git diff <base-branch>...HEAD --stat` to see changed files
3. Read all changed files completely
4. If component SPECs exist (`src/**/SPEC.md`), read them and verify compliance
5. Check for: correctness, security issues, missing edge cases, spec violations, naming/convention issues
6. Produce a findings report in the format below — DO NOT modify any files

### Findings report format
Write your findings report as a markdown comment on the PR (if PR exists) or output it directly. Use this structure:

#### Code Review Findings: <issue-id>

**Reviewer**: <executor-name>
**Branch**: <branch-name>
**Date**: <ISO date>
**Verdict**: APPROVE | REQUEST_CHANGES | COMMENT

##### Summary
<2-3 sentence overall assessment>

##### Findings

| # | Severity | File:Line | Description | Suggested Fix |
|---|----------|-----------|-------------|---------------|
| 1 | Critical/High/Medium/Low | `path/to/file:42` | What the issue is | How to fix it (text description only, no code patches) |

##### Checklist
- [ ] Acceptance criteria met
- [ ] No security issues (OWASP top 10)
- [ ] Error handling adequate
- [ ] Naming conventions followed
- [ ] No dead code or debug artifacts
- [ ] SPEC compliance verified (if applicable)

**IMPORTANT**: You are a reviewer. Do NOT modify any files. Your only output is this findings report.
```

### 3. Select Executor

Choose the reviewer executor per the Executor Selection table above. Default to `CODEX` when the original work was done by `CLAUDE_CODE`.

### 4. Create Review Sub-Task

Create a review sub-task issue via the `issue-tracker` skill:
- **Title**: `"Review: <issue-id> — <short-title>"`
- **Description**: The full dispatch description from step 2 (this is the reviewer's only briefing)
- **Parent**: The original implementation issue (if the tracker supports parent-child)

This is required, not optional. The reviewer agent receives context through the issue description — without it, the reviewer starts blind.

### 5. Dispatch Review Session

Use the workspace provider to start a new session for the reviewer. The dispatch MUST include:
- **Issue link**: The review sub-task ID created in step 4
- **Executor**: The selected reviewer executor
- **Base branch**: The **implementation branch** (not main), so the reviewer lands on the code to be reviewed

How the dispatch is performed depends on the workspace provider — see Provider-Specific Dispatch below.

### 6. Post-Dispatch

- Note the review session/workspace ID and branch for tracking
- Inform the user that the review has been dispatched and what to expect
- The review findings will appear as a PR comment or in the review session output

## Handling Review Findings

After the reviewer session completes:

1. **Read the findings report** from the review session or PR comments
2. **Triage findings by severity**:
   - **Critical**: Must fix before merge. Block PR.
   - **High**: Should fix before merge. Discuss if time-constrained.
   - **Medium**: Fix if straightforward, otherwise create follow-up issue.
   - **Low**: Optional. Create follow-up issue or ignore with rationale.
3. **Implement fixes** in the original implementation session (not the reviewer session)
4. **Re-request review** only if Critical/High findings were addressed and scope was significant

## Error Handling

| Error | Recovery |
|-------|----------|
| Dispatch capability unavailable | Fall back to manual review: output the dispatch description for a human or manual agent session |
| Reviewer executor unavailable | Use fresh `CLAUDE_CODE` session as fallback |
| No issue ID available | Create review without issue link; include branch name and summary in description |
| PR does not exist yet | Include "PR not yet created" in dispatch; reviewer outputs findings directly instead of as PR comment |
| Empty diff | Abort — nothing to review |

## Relationship to Other Skills

- **Complements**: `workspace-orchestrate` (shares dispatch pattern, different purpose)
- **Follows**: `task-implement` workflow (review happens after implementation)
- **Precedes**: `agentic-env-lifecycle` (review before PR merge)
- **Delegates to**: `issue-tracker` (for issue reference resolution)

## Constraints

- Reviewer session is **read-only** — no file mutations, no commits
- Reviewer MUST NOT be the same session that implemented the code
- Findings report is the only artifact — no code patches, no auto-fixes
- Dispatch only — this skill does not perform the review itself
- One review per dispatch — do not batch multiple issues into a single review session

---

## Provider-Specific Dispatch

The abstract procedure above is provider-agnostic. Each workspace provider needs an equivalent dispatch mechanism. This section documents known implementations.

### Vibe Kanban (Coder Workspaces)

**Available when**: `mcp__vibe_kanban__start_workspace_session` tool is accessible.

**Dispatch call**:
```
start_workspace_session(
  title: "Review: <issue-id> — <short-title>",
  executor: "<reviewer-executor>",
  repos: [{repo_id: "<repo-id>", base_branch: "<implementation-branch>"}],
  issue_id: "<issue-id>"
)
```

**Context bootstrapping**: The dispatch description from step 2 should be set as the issue description (or a review sub-task description) so the reviewer agent receives it on session start. VK agents start with only the issue title — the description must carry the full review briefing.

**Findings delivery**: Reviewer posts findings as a GitHub PR comment via `gh pr comment` (if PR exists and `gh` is authenticated), or outputs them directly in the session.

### GitHub Codespaces / Manual Sessions

**Available when**: VK dispatch is unavailable but GitHub Codespaces or local sessions can be started manually.

**Dispatch approach**:
1. Create a review sub-issue (via `issue-tracker` skill) with the dispatch description as the issue body
2. Start a Codespace or local agent session on the implementation branch
3. Point the agent at the review sub-issue for instructions

**Findings delivery**: PR comment via `gh pr comment`, or output directly in the session. The reviewer MUST NOT commit files — the read-only contract applies regardless of provider.

### Other Providers (Future)

Any workspace provider that can start a session with:
- A specified executor/agent runtime
- A target branch (the implementation branch)
- An issue or description payload for context

...can implement this dispatch pattern. The abstract procedure (steps 1-6) and the output contract remain identical regardless of provider.
