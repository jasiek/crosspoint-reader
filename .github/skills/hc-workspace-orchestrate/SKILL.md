---
name: hc-workspace-orchestrate
description: Start work on tracked issues by spinning up workspace sessions. Supports single or parallel dispatch across executors (Claude Code, OpenCode, etc.).
compatibility: Designed for Claude Code, Codex, OpenCode, and Gemini skills format.
metadata:
  owner: holicode
  scope: workspace-dispatch
  uses: agent-session-protocol
---

# Workspace Orchestrate

Dispatch workspace sessions for tracked issues. This skill bridges issue tracking to execution — given one or more issue references, it starts workspace sessions with the right executor, base branch, and repo configuration.

> **Provider note**: This skill currently implements dispatch via **Vibe Kanban** MCP tools (`start_workspace_session`, `get_context`). The abstract branching and PR conventions it enforces come from `holicode.md § Agentic Git Workflow Conventions` and apply regardless of provider. Other workspace providers (GitHub Codespaces, Gitpod, manual worktrees) need equivalent dispatch skills that enforce the same abstract conventions.

## When This Skill Applies

- Agent needs to start implementation work on a tracked issue
- Agent wants to dispatch parallel sessions (same or different executors)
- Agent is planning a batch of work across multiple issues
- User asks to "start work on [issue]" or "spin up a workspace for [issue]"

## When NOT to Use

- Issue has unresolved blocking dependencies — resolve blockers first
- You want to implement code directly — use `task-implement` workflow instead
- Workspace dispatch tools are unavailable — fall back to manual branch creation
- Issue doesn't exist yet — use `issue-tracker` skill to create it first

## Scope Boundaries

- This skill dispatches workspaces only — it does not implement code
- Issue tracking operations (create, update, resolve references) belong to `issue-tracker` skill
- Session lifecycle (push, PR, merge) belongs to `agentic-env-lifecycle` skill
- This skill does NOT decide what to work on — the agent or user selects issues

## Prerequisites

- Workspace dispatch tools available (Vibe Kanban: `mcp__vibe_kanban__get_context` and `mcp__vibe_kanban__start_workspace_session`; other providers: equivalent dispatch API)
- At least one repo configured in the project
- Issue must exist in the tracker (use `issue-tracker` to create if needed)

## Standard Procedure

### 1. Pre-Flight

Before dispatching, validate:

1. **Dispatch tool available**: Call `get_context()` to confirm workspace tooling is reachable. If unavailable, abort with a clear message.

2. **Issue status check**: Use the `issue-tracker` skill to resolve the issue reference and check current status:
   - If already "In progress" — warn the user. Another session may already be working on it. Ask to confirm before proceeding.
   - If "Done" — warn: re-dispatching a completed issue is unusual. Confirm intent.

3. **Feature branch check**: If the issue belongs to a multi-task epic/story, verify a feature branch exists on the remote:
   ```bash
   git ls-remote --heads origin feature/<epic-or-story-slug>
   ```
   - If the feature branch does not exist, create it from the integration branch and push before dispatching.
   - If the issue is a standalone task (no parent, no siblings), skip this check — it branches directly off `main`.

4. **Base branch exists**: If using a non-default or chained branch, verify it exists on the remote:
   ```bash
   git ls-remote --heads origin <base-branch>
   ```
   If not found, abort — the branch may not have been pushed yet.

### 2. Resolve Context

Get the project's repo and default base branch from the workspace context:

```
context = get_context()
repo_id = context.workspace_repos[0].repo_id
default_base = context.workspace_repos[0].target_branch
```

### 3. Resolve Issue

Use the `issue-tracker` skill to resolve the human-readable issue reference (e.g., `HOL-32`, `#123`) to the provider-native ID needed for dispatch.

### 4. Select Executor

| Executor | Use When |
|----------|----------|
| `CLAUDE_CODE` | Default. Best for spec-driven, multi-file implementation |
| `OPENCODE` | Alternative perspective, cross-validation |
| `GEMINI` | Gemini-native tasks if configured |

Default to `CLAUDE_CODE` unless explicitly told otherwise.

### 4b. Select Variant (optional)

Variants select the executor profile (model + config). Available variants depend on the instance — read `~/.local/share/vibe-kanban/profiles.json` to enumerate what's configured, or check `techContext.md` if documented there.

| Variant | Use When |
|---------|----------|
| `null` (omit) | Standard implementation tasks — uses DEFAULT profile |
| `SONNET_1_M` | Full-context analysis: session mining, large file cross-reference, deep transcript analysis |
| `SONNET_1_M_100_K_IN` | Same as above + 100K file read override — **preferred for JSONL/large file corpus analysis** |
| `OPUS_1_M` | Complex reasoning tasks requiring both depth and full context (highest quality, highest cost) |
| `PLAN` | Planning/spec work only, no code execution |
| `APPROVALS` | Human-in-the-loop approval workflow required |

**1M Context Pattern**: When a task requires loading large corpora (session transcripts > 200KB, multi-file cross-reference > 150KB combined, or full session retrospective), create a **dedicated sub-issue** scoped to that analysis and dispatch it with `SONNET_1_M_100_K_IN` (recommended) or `OPUS_1_M`. The 100K file read variant is critical for JSONL sessions — without it the Read tool caps at 25K tokens/call, requiring ~20+ reads for a single 593KB file. The sub-issue DoR should specify exact file paths, output artifact location, and instruct the agent to skip standard state-loading boilerplate.

### 5. Select Base Branch

- **Default**: Use the project's integration branch (from context), typically `main`
- **Feature branch active**: If the issue belongs to a multi-task epic/story with a feature branch, use `feature/<slug>` as the base branch — not `main`
- **Sub-task chaining**: For sequentially dependent sub-tasks, base each workspace off the **previous sub-task's merged branch**. Wait for the previous PR to merge into the feature branch before dispatching the next sub-task.
- **Caution**: Basing off an unmerged task branch creates fragile merge dependencies. Prefer waiting for the PR to merge. Only chain unmerged branches when explicitly instructed and the sub-tasks are truly independent.

### 6. Dispatch

> The examples below use **Vibe Kanban** MCP tools. Other providers should implement equivalent dispatch with the same semantic parameters (title, executor, base branch, issue link).

**Single session (standard):**
```
start_workspace_session(
  title: "<issue title>",
  executor: "CLAUDE_CODE",
  variant: null,
  repos: [{repo_id: "<repo-id>", base_branch: "<base-branch>"}],
  issue_id: "<issue-id>"
)
```

**Single session (1M context — for large corpus / JSONL analysis):**
```
start_workspace_session(
  title: "<sub-issue title> [1M context]",
  executor: "CLAUDE_CODE",
  variant: "SONNET_1_M_100_K_IN",
  repos: [{repo_id: "<repo-id>", base_branch: "<base-branch>"}],
  issue_id: "<sub-issue-id>"
)
```

**Parallel sessions** (independent issues only):
```
# Session A
start_workspace_session(
  title: "Issue A title",
  executor: "CLAUDE_CODE",
  variant: null,
  repos: [{repo_id: "<repo-id>", base_branch: "<base-branch>"}],
  issue_id: "<issue-a-id>"
)

# Session B (parallel, independent work)
start_workspace_session(
  title: "Issue B title",
  executor: "OPENCODE",
  variant: null,
  repos: [{repo_id: "<repo-id>", base_branch: "<base-branch>"}],
  issue_id: "<issue-b-id>"
)
```

For parallel dispatch, verify issues are independent — no shared file coupling or parent-child relationship. When in doubt, dispatch sequentially.

### 7. Post-Dispatch

- **Update issue status**: Delegate to `issue-tracker` skill to set status to "In progress"
- **Log dispatch**: Record what was started (issue ref, executor, base branch, workspace ID/branch)
- **If parallel**: Note which sessions are independent vs. need sequencing
- **Register A2A callback** (optional — see §7b below)

### 7b. Register A2A Callback (optional)

If you want the dispatched workspace to send a completion report back when done, register a callback per `agent-session-protocol`. The **preferred path** is to embed A2A front matter directly in `start_workspace`'s `prompt` parameter so the new session sees the A2A message as its first prompt at boot.

**When to use**: Orchestrating parallel workspaces, automating handoff chains, or anytime you want a structured signal when implementation is complete.

**When to skip**: Single ad-hoc workspace where you'll manually check the result.

#### Preferred: A2A in the dispatch prompt (single MCP call)

```python
# 1. Get your own session ID
ctx = mcp__vibe_kanban__get_context()
my_session_id = ctx.session_id

# 2. Generate a unique invocation_id
invocation_id = f"{datetime.now().isoformat()}Z-{random_4_chars}"

# 3. Dispatch with embedded A2A front matter — this is the new session's first message
mcp__vibe_kanban__start_workspace(
    name="<issue title>",
    executor="CLAUDE_CODE",
    repositories=[{"repo_id": "<repo-id>", "branch": "<base-branch>"}],
    issue_id="<issue-id>",
    prompt=f"""---
a2a_version: "1.0"
invocation_id: "{invocation_id}"
from_session_id: "{my_session_id}"
intent: dispatch
callback_session_id: "{my_session_id}"
max_hops: 1
---

You have been dispatched to implement the linked issue. Because this initial
prompt replaces the default issue-title/description bootstrap, you MUST first:

  1. Call `mcp__vibe_kanban__get_context()` → capture `issue_id`
  2. Call `mcp__vibe_kanban__get_issue(issue_id=<id>)` and read the full issue
     description, acceptance criteria, and any linked specs
  3. Then proceed per your standard workflows

When done (after push + PR creation per `agentic-env-lifecycle`), send a
completion callback to session `{my_session_id}` per the `agent-session-protocol`
skill (step: A2A Completion Callback).
"""
)
```

**Note on overriding the default first prompt**: Without `prompt`, `start_workspace` uses the linked issue's title/description as the new session's first message. Embedding A2A here replaces that default — the dispatch body MUST explicitly tell the new session to fetch the full issue context (step 2 above), or the dispatched agent would start with less context than in a non-A2A dispatch.

The dispatched session's `task-init` Step 0 will parse the A2A front matter from this initial prompt and store the callback obligation. `agentic-env-lifecycle` will emit the callback at session end with `max_hops: 0` (terminates the chain).

#### Fallback: post-dispatch `run_session_prompt`

If for some reason `start_workspace` cannot accept a prompt (e.g., the workspace was already started, or the dispatch path doesn't allow it), send the same A2A front matter via `run_session_prompt` immediately after dispatch. The dispatched session detects A2A on any incoming message (per `agent-session-protocol` "Receiving" rules), not just at boot.

**For parallel dispatch**: each `start_workspace` call gets its own unique `invocation_id` in its prompt. Track them in working context to match incoming callbacks.

## Cross-Validation Patterns (Advanced)

Two distinct patterns exist for getting a second perspective. Choose based on intent:

### Pattern A: Implementation Cross-Validation

Two executors independently implement the **same issue**. Best for spikes or when the optimal approach is uncertain.

1. Start `CLAUDE_CODE` session for the issue
2. Start `OPENCODE` session for the same issue (different workspace, same base branch)
3. After both complete, compare approaches and synthesize the best solution

Both sessions receive the **same issue ID** and implement independently. This is resource-intensive — use only when explicitly requested or for spikes.

### Pattern B: Code Review (Implement + Review)

One executor implements, a different executor reviews the result. Best for quality assurance before merge.

1. Implementation session completes and creates a PR
2. Use the `code-review` skill to dispatch a review session to a **different executor**
3. The reviewer produces a **findings-only report** — no code changes, no commits
4. The implementation session (or a human) triages and addresses findings

Key differences from Pattern A:
- The review session gets a **different context** (review instructions, not implementation instructions)
- The reviewer MUST NOT modify files — its only artifact is the structured findings report
- Executor diversity adds value: e.g. Codex reviewing Claude's work catches different classes of issues than self-review
- Less resource-intensive than Pattern A since only one implementation is produced

See the `code-review` skill for the full dispatch procedure, output contract, and findings format.

## Error Handling

| Error | Recovery |
|-------|----------|
| Dispatch tool unavailable | Check MCP connection; fall back to manual branch creation |
| Issue reference not found | Verify ID with `issue-tracker` skill; check correct project |
| Base branch doesn't exist | Verify with `git ls-remote`; check if source branch was pushed |
| Issue already "In progress" | Confirm with user; may be intentional (re-dispatch) or a conflict |
| No repos in context | Check project configuration in workspace tooling |

## Relationship to Other Skills

- **Delegates to**: `issue-tracker` (reference resolution, status updates)
- **Complements**: `agentic-env-lifecycle` (handles session end: push → PR → merge)
- **Feeds from**: `task-init` (dispatch recommendations from board triage)

## Issue Description Completeness

When creating sub-tasks or dispatching workspaces, ensure each issue description is **self-contained**:

1. **Full implementation scope** in every sub-task description — do not rely on "see parent issue" without quoting the relevant parts.
2. **Link SPEC/spike docs** directly in the description (file paths or URLs).
3. **Include parent context** when relevant — quote the parent epic/story's goal and constraints so the dispatched agent doesn't need to look them up.
4. A workspace should be able to start work from the issue description alone, without manual prompting.

## Dispatching Review Workspaces

When dispatching a workspace for code review (not implementation):

1. **Include explicit context** in the session title: `Review: HOL-XX <description>`
2. **Link the issue** via `issue_id` so the agent can retrieve the issue description
3. **Enrich the issue description** with review-relevant context before dispatch:
   - PR URL to review
   - Key files or areas to focus on
   - What the review should evaluate (correctness, security, performance, etc.)
4. Do not rely on the review agent to self-discover context — it should have everything it needs from the issue and linked PR.

## Output Contract

After dispatch, report:
- Issue reference (human-readable ID)
- Executor used
- Base branch (and whether it's a feature branch)
- Workspace ID and branch name (from dispatch response)
- Any warnings (chained branch, parallel overlap, status conflicts)

## Constraints

- Dispatch only — never implements code or modifies source files
- Requires network access (workspace dispatch is a remote operation)
- Parallel dispatch decisions are advisory — human or orchestrator confirms
- Prefer default integration branch; chain branches only when explicitly needed
