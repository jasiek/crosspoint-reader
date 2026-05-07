---
name: hc-agent-session-protocol
description: "A2A inter-session communication protocol using YAML front matter. CHECK THIS SKILL whenever you receive a message that begins with a `---` YAML block containing `a2a_version:` — at session boot OR mid-session via run_session_prompt. Defines front matter format for agent-to-agent session calls, callbacks, and completion reports. Integrate with workspace-orchestrate, dev-review-loop, and agentic-env-lifecycle for structured inter-session comms."
compatibility: Designed for Claude Code, Codex, OpenCode, and Gemini skills format.
metadata:
  owner: holicode
  scope: inter-session-protocol
---

# Agent Session Protocol (A2A)

A lightweight YAML front matter protocol for structured agent-to-agent communication via `run_session_prompt`. Enables callbacks, completion reports, loop detection, and chain-depth control without changing the underlying session transport.

## When to Use

- **You are receiving a message**: If **any** message you receive (at session boot OR mid-session) starts with a YAML front matter block (`---`) containing `a2a_version:`, parse and apply this protocol **before doing anything else**
- **You are calling another session**: Prepend A2A front matter to any `run_session_prompt` call when you want a structured callback
- **Workspace dispatch with callback**: When starting a workspace and wanting a completion report back (use `start_workspace`'s `prompt` parameter — see Pattern 1)
- **dev-review-loop rounds**: Between implementing and review sessions
- **Any orchestration scenario**: Any time you need to know "who called me and where to send the result"

## When NOT to Use

- Plain user messages (no front matter → human-originated → normal handling)
- Task sub-agent calls (no persistent session ID → skip protocol, use plain text)
- Single-turn status pings where no callback is needed and no chain-depth control matters (plain text is fine)

---

## Protocol Specification

### Front Matter Schema (every A2A message)

```yaml
---
a2a_version: "1.0"
invocation_id: "<unique per message; ISO8601-ts + 4 random chars>"
from_session_id: "<sender's session ID>"
intent: <dispatch | review | harvest | ping | callback>
in_response_to: "<invocation_id of the message you're replying to>"   # required for callbacks
callback_session_id: "<session to resume when done>"                  # optional; for forward calls
status: <complete | partial | error>                                  # required for callbacks
max_hops: <integer ≥ 0>                                               # remaining-budget (see below)
---

<message body — plain text below the front matter>
```

### Field Semantics

| Field | Required | Notes |
|-------|----------|-------|
| `a2a_version` | yes | Always `"1.0"` for this version |
| `invocation_id` | yes | **Unique per message**. Used for idempotency/dedupe. Format: `<ISO8601-ts>-<4-random-chars>` |
| `from_session_id` | yes | Sender's session ID, from `mcp__vibe_kanban__get_context().session_id` |
| `intent` | yes | One of: `dispatch`, `review`, `harvest`, `ping`, `callback` |
| `in_response_to` | callbacks | The `invocation_id` of the message being responded to |
| `callback_session_id` | optional | Where to send the result; if absent, no callback expected |
| `status` | callbacks | `complete` (final), `partial` (interim update), `error` |
| `max_hops` | yes | Budget for further A2A emissions in this chain — see Loop Control below |

### Intent Values

| Intent | Use When |
|--------|----------|
| `dispatch` | Starting a workspace and registering a callback |
| `review` | Requesting a code review round |
| `harvest` | Requesting a status or output summary |
| `ping` | Lightweight liveness check |
| `callback` | Responding to any of the above |

---

## Loop Control: `max_hops` Semantics

**Definition**: `max_hops` is the **remaining budget** of A2A messages allowed to be emitted in this chain *after* the current message is received.

**Decrement rule**: When emitting an A2A message in response to or chained from an incoming A2A message, set `max_hops = incoming_max_hops - 1`.

**Block rule**: If incoming `max_hops <= 0`, the receiver **MUST NOT** emit any A2A follow-up (callback, forward, ping, etc.). Handle the work locally and stop the chain.

**Default**: `max_hops: 1` for typical dispatch + single-callback patterns. Use higher values only when chains are explicitly needed.

**Examples**:

| Scenario | Initial `max_hops` | Step | `max_hops` Outgoing |
|----------|---|---|---|
| Dispatch → work → callback | `1` | Receiver emits final callback | `0` |
| Dispatch → mid-status + final callback | `2` | Receiver emits partial callback | `1` |
| ↓ | | Receiver emits final callback | `0` |
| Fire-and-forget | `0` | Receiver MUST NOT emit any A2A | (none) |

**No special cases**: Every A2A emission decrements; every A2A receipt with `<= 0` budget stops the chain. No per-skill "skip if X" overrides.

**Idempotency**: If you receive two A2A messages with the same `invocation_id`, treat the duplicate as a redelivery and ignore. Never emit on duplicates.

**Self-loop guard**: Never emit a callback to your own `from_session_id`; treat it as a misconfigured chain and log a warning.

---

## Receiving an A2A Message — Procedure

**A2A detection runs on every incoming message, not only at session boot.** If a message you just received starts with `---` and contains `a2a_version:`, treat it as A2A regardless of when in the session it arrives.

1. **Detect front matter**: Does the message start with `---` and contain `a2a_version:`? If yes, this is an A2A message.
2. **Parse fields**: Extract `invocation_id`, `from_session_id`, `intent`, `callback_session_id`, `max_hops`, `in_response_to`, `status` (whichever are present).
3. **Idempotency check**: Have you already processed this `invocation_id` in this session? If yes, ignore.
4. **Execute the intent**: Perform the requested work as described in the message body.
5. **Send callback** (if `callback_session_id` is set AND incoming `max_hops > 0`):
   - Get your session ID: `mcp__vibe_kanban__get_context()` → `session_id`
   - Generate a **new unique** `invocation_id` for the callback
   - Set `in_response_to = <incoming invocation_id>`
   - Set `max_hops = <incoming max_hops> - 1`
   - Set `intent: callback`, `status: complete | partial | error`
   - Call `mcp__vibe_kanban__run_session_prompt(session_id=callback_session_id, prompt=<callback front matter + body>)`

**Skip the callback if**: `callback_session_id` absent, or `callback_session_id == your own session_id` (self-loop), or incoming `max_hops <= 0`.

---

## Sending an A2A Message — Checklist

1. **Get your own session ID**: `mcp__vibe_kanban__get_context()` → `session_id`
2. **Generate a unique invocation_id** for this message: `<ISO8601-timestamp>-<4-random-chars>`
3. **Set callback_session_id** if you want a result back (typically your own `session_id`)
4. **Set max_hops** as a budget — `1` for "I expect one callback", `0` for fire-and-forget
5. **Set in_response_to** if this is a callback (the previous `invocation_id`)
6. **Prepend front matter** to the `run_session_prompt` prompt string
7. **Track invocation_id** in working context to match the incoming callback

---

## Session ID Sourcing

```
# Vibe Kanban (primary)
ctx = mcp__vibe_kanban__get_context()
my_session_id = ctx.session_id

# Task sub-agents (no persistent session ID)
# → skip A2A protocol; use plain-text prompts and return findings directly
```

---

## Patterns

### Pattern 1: Dispatch workspace + expect completion report (PREFERRED — single MCP call)

The cleanest path: put the A2A front matter directly in `start_workspace`'s `prompt` parameter. The new session's first message is the A2A dispatch, so `task-init` detects it at boot.

```python
ctx = mcp__vibe_kanban__get_context()
my_session_id = ctx.session_id
invocation_id = "2026-05-02T14:30:00Z-a1b2"

mcp__vibe_kanban__start_workspace(
    name="HOL-XX: <description>",
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
  3. Then proceed per your standard task-implement workflow

When done, send a completion callback to session `{my_session_id}` per the
agent-session-protocol skill (see agentic-env-lifecycle § A2A Completion Callback).
"""
)
```

**Note on overriding the default first prompt**: Without `prompt`, `start_workspace` uses the linked issue's title/description as the new session's first message. Embedding A2A here replaces that default — the dispatch body MUST explicitly tell the new session to fetch the full issue context (step 2 above), or the dispatched agent would start with less context than in a non-A2A dispatch.

**Fallback** (when `start_workspace` doesn't accept a prompt or the workspace was started without one): send a `run_session_prompt` immediately after dispatch with the same A2A front matter. The receiving session detects A2A on its earliest non-boot message via the protocol's "any message" detection rule.

---

### Pattern 2: Review round (dev-review-loop)

Per-round dispatch with a single callback. Each round is a fresh chain (own `invocation_id`, fresh `max_hops`).

See `dev-review-loop` § Round Prompt Template — A2A headers are embedded there.

---

### Pattern 3: Ping / harvest

```python
mcp__vibe_kanban__run_session_prompt(
    session_id="<target-session-id>",
    prompt=f"""---
a2a_version: "1.0"
invocation_id: "2026-05-02T14:31:00Z-c3d4"
from_session_id: "{my_session_id}"
intent: harvest
callback_session_id: "{my_session_id}"
max_hops: 1
---

Quick status: what have you completed so far? Report in under 200 words.
"""
)
```

The target session receives `max_hops: 1`, decrements to `0` in the callback, and stops the chain there.

---

## Relationship to Other Skills

| Skill | Integration |
|-------|-------------|
| `dev-review-loop` | Uses A2A front matter in every `run_session_prompt` call between implementing and review sessions |
| `agentic-env-lifecycle` | Checks for `callback_session_id` at session end; sends completion report (with `max_hops` decremented) |
| `workspace-orchestrate` | Passes A2A dispatch via `start_workspace`'s `prompt` parameter (preferred) or post-dispatch `run_session_prompt` (fallback) |
| `workspace-harvest` | Can use `intent: harvest` ping instead of plain-text status checks |
