---
name: hc-dev-review-loop
description: "Post-implementation iterative peer review loop. A second agent session (different executor when available) reviews the changes, the implementing session validates each finding with AGREE/DISAGREE/PARTIAL, fixes are applied, and the loop repeats until the reviewer issues an explicit sign-off — at which point the PR is opened."
compatibility: Designed for Claude Code, Codex, OpenCode, and Gemini skills format. Requires either a workspace session dispatch capability (e.g. vibe_kanban MCP) OR a Task sub-agent capability.
metadata:
  owner: holicode
  scope: iterative-peer-review
  uses: agent-session-protocol
---

# Dev Review Loop

After implementation is complete and committed, spin up a second agent session in the same workspace to perform a peer code review. The two sessions exchange findings and rebuttals in structured rounds until the review session issues an explicit sign-off ("No further changes requested"), at which point the implementing session opens the PR.

This skill is **integration-agnostic** — the abstract protocol (round dispatch, AGREE/DISAGREE validation, sign-off contract) works across any workspace provider. Provider-specific dispatch mechanics are documented inline in each option.

## When to Use

- After `task-implement` (or any manual implementation) is committed and the branch is ready for review
- User says `/dev-review-loop` or "start a review loop"
- As a step in a PR creation workflow when a second agent perspective is wanted before human review
- When `code-review` (single-pass) is insufficient and a live back-and-forth with a separate model is desired

## When NOT to Use

- For reviewing an existing PR's human comments → use `github-pr-review` workflow instead
- For a single-pass static analysis with read-only reviewer → use `code-review` skill instead
- When you want a full-system QA scan against acceptance criteria → use `quality-validate` workflow instead
- When neither workspace session dispatch nor Task sub-agents are available → fall back to `code-review`

## Relationship to Other Skills and Workflows

| Tool | Relationship |
|------|-------------|
| `code-review` (skill) | Simpler alternative. `code-review` is single-agent, single-pass, read-only reviewer. This skill is multi-agent, multi-round, with iterative validation. Use `code-review` when speed matters; use this skill when thoroughness matters. |
| `agentic-env-lifecycle` (skill) | Sequential — this skill runs before the PR is opened. Once sign-off is reached, hand off to `agentic-env-lifecycle` for the push → PR → merge flow. |
| `github-pr-create` (workflow) | Sequential — invoked after sign-off if you prefer the workflow over inline `gh` commands. |
| `github-pr-review` (workflow) | Sequential — after this skill creates the PR, `github-pr-review` processes human reviewer comments. |
| `task-implement` (workflow) | Upstream. This skill picks up where `task-implement` finishes. |
| `quality-validate` (workflow) | Adjacent. Use `quality-validate` for acceptance-criteria QA; use this skill for code-level peer review with rebuttal. |

## Inputs

| Input | Required | Default |
|-------|----------|---------|
| `executor` | No | A different executor than the implementing session, preferring `CODEX` when available; otherwise any executor present in the workspace (e.g. `OPENCODE`, `GEMINI`, `AMP`, `CLAUDE_CODE`) |
| `issue_id` | No | Auto-detect from workspace context (`get_context` if vibe_kanban MCP is available) |
| `base` | No | `main` — the branch to diff against |
| `changed_files` | No | Auto-detect from `git diff <base>...HEAD --name-only` |
| `context` | No | None — task description / acceptance criteria to sharpen the review |
| `max_rounds` | No | `3` — stop and surface unresolved findings to user after this many rounds |

## Protocol Overview

**Option A — workspace sessions** (two persistent agents communicating via session resume, e.g. `mcp__vibe_kanban__run_session_prompt`):

```
Implementing session (you)          Review session (different executor)
─────────────────────────           ────────────────────────────────────
1. Commit implementation
2. Create review session ──────────► reads changed files
   (no monitoring needed —
    session resumes you when done)
3. [idle — awaiting resume] ◄──────── resumes implementing session
                                       with structured findings + severity
4. Validate each finding
   AGREE / DISAGREE + reason
5. Apply agreed fixes, commit
6. Dispatch round N+1 ────────────► re-reads files, checks fixes
   (no monitoring needed)      ◄──── resumes implementing session with verdict
7. "No further changes requested" OR new findings → loop back to 4
8. Open PR
```

**Option B — Task tool sub-agent** (fallback when workspace session dispatch is unavailable):

```
Implementing session (you)
──────────────────────────────────────────────────────────
1. Commit implementation
2. Launch Task sub-agent ─────────► reads files, returns findings in result
3. Read findings from tool result
4. Validate: AGREE / DISAGREE + reason
5. Apply agreed fixes, commit
6. Launch new Task sub-agent ─────► round N+1 prompt with prior-round outcome
7. Read verdict from tool result
8. Repeat 4-7 until "No further changes requested"
9. Open PR
```

## Steps

### 1. Gather Context

```bash
# Determine changed files vs base
git diff <base>...HEAD --name-only
```

If a workspace session dispatch is available (e.g. vibe_kanban MCP):

```
mcp__vibe_kanban__get_context          # captures your session_id for A2A callbacks
mcp__vibe_kanban__list_sessions        # find your own session ID if not in context
```

Store `my_session_id` from `get_context()` — it is required for the A2A callback field in the round prompt.

Build a **change summary** to include in the review prompt:
- Issue ID and title (from context if available)
- List of changed files with one-line description of what changed in each
- Key design decisions that reviewers should know (not self-evident from the diff)
- Any known constraints or intentional trade-offs

### 2. Create the Review Session

Choose the approach that best matches what is available:

---

#### Option A — workspace session (preferred)

Example using vibe_kanban MCP:

```
mcp__vibe_kanban__create_session(
  workspace_id = <current workspace id>,
  executor     = <resolved executor>,   # see executor resolution below
  name         = "<ISSUE-ID> <executor> Review"
)
```

**Executor resolution** — pick in this order:
1. Explicit `executor` param if provided by the caller
2. `CODEX` — attempt `create_session` with `executor="CODEX"`. If the call succeeds, use that session. If it fails (executor not available), fall back to step 3.
3. Any other executor that differs from the implementing session's executor (prefer `OPENCODE`, `GEMINI`, `AMP` over `CLAUDE_CODE` for independence)
4. `CLAUDE_CODE` as last resort if no other executor is available

**How to detect available executors**: provider listing tools (e.g. `list_sessions`) typically only show existing sessions, not available executors. The reliable method is to **attempt `create_session` with `CODEX`** and treat a success as confirmation of availability. Do not infer executor availability from session listings alone — those reflect what is currently running, not what can be started.

The review session intentionally uses a **different executor than the implementing session** wherever possible, to get an independent perspective. If only one executor type is available, proceed with it — a same-executor review is still valuable.

Save the returned `session_id` — this is the **review session**. Proceed to §3.

For other workspace providers, use the equivalent "create session" call with an explicit executor and a name that includes the issue ID and "Review".

---

#### Option B — Task tool sub-agent (fallback when workspace session dispatch is unavailable)

If no workspace session dispatch is accessible, launch a sub-agent via the Task tool instead:

```
Task(
  subagent_type = "general-purpose",   # or "Explore" for read-only investigation
  description   = "<ISSUE-ID> peer code review",
  prompt        = <round-1 review prompt — see Round Prompt Template below,
                   omitting the "post findings back to session" instruction
                   since there is no session ID to post to>
)
```

**Differences from Option A:**
- The sub-agent returns its findings directly in the tool result rather than posting to a session. Read the result and proceed to §5 (validate findings) immediately.
- There is no separate session ID; all rounds run as sequential Task calls from the implementing session.
- The review prompt should instruct the sub-agent to **return** findings in its response rather than calling a session-resume tool.
- For round N+1, launch a new Task call with the round-N+1 prompt prepended with the prior-round outcome block.

The loop structure (validate → fix → request next round → iterate until clean) is identical to Option A.

### 3. Dispatch Round 1

Send the **round-1 review prompt** to the review session. Example with vibe_kanban MCP:

```
mcp__vibe_kanban__run_session_prompt(
  session_id = <review session id>,
  prompt     = <see Round Prompt Template below>
)
```

The prompt instructs the review session to **resume this implementing session** when it is done, injecting its findings into your context. No polling or active monitoring is required — the review session resumes you directly.

### 4. Receive Findings

When the review session resumes this session, its findings arrive directly in your conversation context. Proceed to Step 5 immediately.

In the vibe_kanban model the review session delivers findings by calling:

```
mcp__vibe_kanban__run_session_prompt(
  session_id = <implementing session id>,   # your session
  prompt     = "<A2A callback front matter + structured findings>"
)
```

The callback will include A2A front matter (per `agent-session-protocol`). Parse the front matter, then read the findings in the body. No follow-up prompts needed. Other providers should use their equivalent "resume session" call.

### 5. Validate Each Finding

For every finding posted by the review session, respond with exactly one of:

- **`AGREE`** — accept the finding as valid; describe the fix to apply
- **`DISAGREE`** — reject the finding with specific reasoning (cite code/spec/legend that contradicts the finding)
- **`PARTIAL AGREE`** — accept the substance but reject part of the suggested fix; describe what you will actually apply

Rules for validation:
- Do not AGREE silently; always state *why* the finding is valid
- Do not DISAGREE without citing a specific reason (spec text, existing code pattern, documented convention, etc.)
- If a finding reveals a genuine gap in business docs or SPEC (not the code), AGREE on the doc fix and note it separately from the code fix

After validation, summarize which findings will be fixed (AGREE/PARTIAL) and which are rejected (DISAGREE).

### 6. Apply Fixes

For each AGREE/PARTIAL AGREE finding, apply the fix and commit. Follow the project's conventional-commit style:

```bash
git add <specific files>
git commit -m "fix(<scope>): apply <ISSUE-ID> review round N (<summary>)

<bullet list of what was fixed and why each finding was valid>"
```

If the project provides commit/branch wrappers (e.g. `scripts/git/commit.sh`), prefer them over raw `git` commands.

### 7. Request Next Round

Send the **round-N+1 prompt** to the review session. Include:
- Summary of what was accepted vs rejected (with reasoning for rejections)
- List of commits applied
- Explicit instruction: reply "No further changes requested" if clean

### 8. Iterate Until Sign-Off

Repeat steps 4–7 until one of:

**a) Review session replies "No further changes requested"** → proceed to Step 9.

**b) `max_rounds` is reached without sign-off** → surface remaining unresolved findings to the user, ask whether to open the PR anyway or continue.

**c) All remaining findings in a round are DISAGREE** → the review session has no accepted findings left; treat this as implicit sign-off and proceed to Step 9.

### 9. Open the PR

Either invoke the `agentic-env-lifecycle` skill (or the `github-pr-create` workflow), or run `gh` directly:

```bash
git push -u origin <branch>
gh pr create \
  --base main \
  --title "<type>(<scope>): <subject> (<ISSUE-ID>)" \
  --body "<PR body — see template below>"
```

If the PR already exists (branch was pushed earlier), update the body:

```bash
gh pr edit <number> --body "<PR body>"
```

If the project provides a PR-creation script (e.g. `scripts/pr/create.sh`), prefer it over raw `gh` calls so policy hooks are honored.

## Round Prompt Template

Use this template for **round 1**. For subsequent rounds, prepend a "Round N context" section describing what was accepted/rejected in the prior round.

> This template uses the `agent-session-protocol` front matter. The front matter tells the review session that it was called by an agent and that it must respond via `run_session_prompt` rather than relying on manual follow-up.

```
---
a2a_version: "1.0"
invocation_id: "[round-N invocation_id: ISO8601-ts + 4 random chars]"
from_session_id: "[implementing session id]"
intent: review
callback_session_id: "[implementing session id]"
max_hops: 1
---

You are performing round [N] of a peer code review for [ISSUE-ID]: [issue title].

**This is an agent-to-agent call.** When done, resume the implementing session
`[implementing session id]` using `mcp__vibe_kanban__run_session_prompt` with your
findings — per the `agent-session-protocol` skill (use `intent: callback` front matter).
The implementing session is idle until you resume it; no active monitoring needed.

## What was implemented

[Change summary: 2-5 bullet points covering what changed and why]

## Key design decisions

[Any non-obvious choices the reviewer should know before reading the diff]

## Files to review

[List of changed files with one-line description each]

## Review instructions

Read each file in full before forming opinions.

For each finding:
- State: file path and line range
- Severity: BLOCKER (correctness/security) | MAJOR (significant UX or logic gap) | MINOR (polish/style)
- Explain the issue clearly
- Suggest a concrete fix

Focus on:
- Edge cases and error states
- Regression risk to existing callers
- SPEC compliance (read colocated `**/SPEC.md` files first)
- Acceptance-criteria alignment with the linked tracker issue
- Data integrity, security, and authorization invariants relevant to the changed surfaces

When done, call back to `[implementing session id]` using:

  mcp__vibe_kanban__run_session_prompt(
    session_id = "[implementing session id]",
    prompt = """---
a2a_version: "1.0"
invocation_id: "[NEW unique id for the callback — different from the request's]"
in_response_to: "[the request's invocation_id]"
from_session_id: "[your session id]"
intent: callback
status: complete
max_hops: 0
---

[Your complete findings here, or "No further changes requested" if clean]
"""
  )
```

**Note on `max_hops`**: each round is its own A2A chain. The request carries `max_hops: 1` (budget for one callback); the callback decrements to `max_hops: 0` (chain terminates). Round N+1 starts a fresh chain with a new `invocation_id`.

**For round N+1**, prepend after the front matter block:

```
## Round [N] outcome

The following findings from round [N] were:

ACCEPTED (applied in commit [hash]):
- [finding summary] → [fix applied]

REJECTED (with reasoning):
- [finding summary] → [reason for rejection]

Re-review the changed files with the above in mind. Check whether the
accepted fixes introduced any new issues. If you have no remaining concerns,
reply "No further changes requested" in the callback body.
```

## PR Body Template

```markdown
## Summary

- **What:** [one-sentence description of the change]
- **Why:** [user request or business reason — cite issue, ticket, or message]
- **Scope:** [UI-only / backend-only / full-stack / docs-only]

## Changes

| File | Change |
|------|--------|
| `path/to/file` | [what changed] |

## Key design decisions

[Any non-obvious choices explained here]

## Test plan

- [ ] [Step 1]
- [ ] [Step 2]
- [ ] [Step 3]

## Review history

[N] review rounds with [executor] session `[session id]`.
Round-[last] fixes applied in `[commit hash]`:
- [bullet list of fixes]

Closes [ISSUE-ID]
```

## Error Handling

| Situation | Action |
|-----------|--------|
| Workspace context lookup returns no workspace_id | Ask user for workspace ID or fall back to provider list (`list_workspaces` etc.) |
| Workspace session dispatch unavailable | Use Option B (Task tool sub-agent) instead of abandoning the loop. Only fall back to single-pass `code-review` if neither option is available. |
| Review session creation fails | Retry once; if still fails, switch to Option B (Task sub-agent) or `code-review` |
| Review session posts no verdict after multiple follow-up prompts | Surface unresolved state to user; ask whether to proceed with PR anyway |
| `max_rounds` reached | List remaining unresolved findings, ask user: "Open PR anyway or continue?" |
| PR already exists | Use `gh pr edit` to update the body; do not error |
| Review session incorrectly cites a non-existent line or misreads the code | DISAGREE with specific code citation proving the finding is wrong |

## Notes on the AGREE/DISAGREE Protocol

The validation step is the core value of this skill. A few critical behaviors:

**Never rubber-stamp findings.** If the review session flags something that is correct-by-design or already handled elsewhere, DISAGREE with evidence. False positives that cause unnecessary changes are worse than no review.

**Disagreements are informative.** The review session should log whether it accepts or concedes a disagreement. If it maintains its position, escalate to the user rather than deadlocking silently.

**Round 2+ must check fixes.** When dispatching round N+1, explicitly ask the review session whether the accepted fixes from round N introduced any new issues — not just whether the original findings are now resolved.

## Constraints

- The review session is intended to **propose** changes; the implementing session **decides and applies** them. Do not let the review session commit code on your behalf.
- Each round must produce either new findings or an explicit sign-off — silent absence of output is not a verdict.
- Do not skip the validation step (Step 5). Auto-applying every finding undermines the point of the protocol.
- One issue per loop — do not batch multiple unrelated issues into a single review session.
