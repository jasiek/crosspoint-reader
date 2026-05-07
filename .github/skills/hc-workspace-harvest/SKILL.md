---
name: hc-workspace-harvest
description: "Harvest outputs and status from parallel or sibling workspace sessions. Use when you want to check what other running or completed workspaces have found, consolidate findings across parallel spike/research sessions, or synthesize cross-workspace learnings. Trigger when the user asks to \"check on other sessions\", \"what did the other workspaces find\", \"consolidate results\", or \"harvest sibling sessions\"."
compatibility: Designed for Claude Code with Vibe Kanban MCP tools.
metadata:
  owner: holicode
  scope: workspace-orchestration
---

# Workspace Harvest

Collect and synthesize outputs from parallel or sibling workspace sessions.
Use this skill to "check in" on other running or completed workspaces — extracting
their findings without requiring a context switch.

## When To Use

- Multiple workspaces were dispatched in parallel (e.g. several spike/research sessions)
- User asks what the other sessions found or wants a consolidated view
- Orchestrating across sessions and need to merge outputs before continuing
- Periodic check-in on long-running autonomous workspaces

## When NOT To Use

- You only have one workspace — nothing to harvest
- You need the session to actively continue work — use `run_session_prompt` directly
- You want to start new work — use `workspace-orchestrate` instead

## Two Modes

### Mode A — Ping (Active Sessions)

For sessions that are still running or recently active: send a prompt to ask for
current status and key findings. Uses `run_session_prompt` MCP tool.

Best when: you want a live update mid-task, sessions are still active.

### Mode B — JSONL Harvest (Completed Sessions)

For completed or sleeping sessions: read their local JSONL transcript files to
extract the final result and key assistant outputs.

Best when: sessions have finished and you want their full output.

### Mode C — Deep Analysis (Large Transcripts)

For very large transcripts (> 1MB JSONL files): dispatch a sub-issue with the
`SONNET_1_M_100_K_IN` variant to do full transcript analysis.

See `workspace-orchestrate` skill § 1M Context Pattern.

---

## Standard Procedure

### Step 1 — Enumerate Target Workspaces

List workspaces to harvest. Filter by name pattern if the user specified a scope
(e.g. "the spike sessions", "the research workspaces started today").

```
list_workspaces(name_search: "<optional filter>", archived: false)
```

Present the list to confirm scope before proceeding if more than ~5 workspaces match.
Exclude the **current workspace** from harvest targets.

### Step 2 — Get Sessions Per Workspace

For each target workspace, get its sessions:

```
list_sessions(workspace_id: "<workspace-id>")
```

Note: a workspace may have multiple sessions (re-runs, continuations). Usually
focus on the **most recent session** per workspace unless doing a full retrospective.

### Step 3 — Locate JSONL Files

JSONL transcript files live at:

```
~/.local/share/vibe-kanban/sessions/<first-2-chars-of-session-id>/<session-id>/processes/*.jsonl
```

Example: session `df2caff7-80b8-4f64-bbf3-189c7e644ee0` →
`~/.local/share/vibe-kanban/sessions/df/df2caff7-.../processes/*.jsonl`

Check file sizes before reading:

```bash
ls -lh ~/.local/share/vibe-kanban/sessions/<prefix>/<session-id>/processes/
```

Size thresholds:
- **< 500KB**: Read directly in current session
- **500KB–5MB**: Extract tail only (see fast extraction below) or use `SONNET_1_M_100_K_IN`
- **> 5MB**: Always use `SONNET_1_M_100_K_IN` variant (see Mode C)

### Step 4 — Extract Outputs

#### Fast extraction (completed sessions, any size)

The `result` event is always the **last line** of a completed JSONL file.
Read just the tail to get the summary cheaply:

```bash
# Get the final result event (last line of JSONL)
tail -1 <file.jsonl> | python3 -c "
import json, sys
outer = json.loads(sys.stdin.read())
inner = json.loads(outer['Stdout'])
if inner.get('type') == 'result':
    r = inner
    print('Status:', 'SUCCESS' if not r.get('is_error') else 'ERROR')
    print('Turns:', r.get('num_turns'))
    print('Cost: \$', round(r.get('total_cost_usd', 0), 4))
    print('Summary:', r.get('result', '(no summary)'))
"
```

#### Full assistant text extraction (small/medium files)

Extract all assistant text messages from a JSONL file:

```bash
python3 -c "
import json, sys
msgs = []
with open(sys.argv[1]) as f:
    for line in f:
        try:
            outer = json.loads(line)
            inner = json.loads(outer.get('Stdout', '{}'))
            if inner.get('type') == 'assistant':
                for block in inner.get('message', {}).get('content', []):
                    if block.get('type') == 'text' and block['text'].strip():
                        msgs.append(block['text'])
        except: pass
# Print last N messages (most relevant output is usually at the end)
for m in msgs[-10:]:
    print('---')
    print(m[:2000])
" <file.jsonl>
```

#### Ping active session (Mode A)

If the session process is still running, send a status request:

```
run_session_prompt(
  session_id: "<session-id>",
  prompt: "Quick status check: what have you found so far? Please give a concise summary of key findings and what you're currently working on. Keep it under 300 words."
)
```

Then use `get_execution(execution_id)` to retrieve the response.

### Step 5 — Synthesize

After collecting outputs from all target sessions, produce a consolidated synthesis:

1. **Per-workspace summary**: workspace name, session status (done/running/error),
   cost, turns, 2–3 sentence summary of key finding
2. **Cross-workspace synthesis**: patterns, agreements, contradictions across sessions
3. **Action items**: what to do next based on the combined findings

Format for output:

```
## Workspace Harvest Report

**Harvested**: <N> workspaces | <date>

### [Workspace Name 1]
- Status: Done | Cost: $X.XX | Turns: N
- Summary: <2-3 sentences>

### [Workspace Name 2]
- Status: Running (pinged) | ...
- Summary: <ping response>

---
## Cross-Session Synthesis

<Key patterns, agreements, contradictions>

## Next Steps / Action Items

- ...
```

---

## JSONL Schema Reference

Each line in a `.jsonl` file is:
```json
{"Stdout": "<escaped-json-string>"}
```

The inner JSON can be:

| `type` | When | Key fields |
|--------|------|-----------|
| `assistant` | Each assistant turn | `message.content[].text` |
| `user` | Each user turn | `message.content[].text` |
| `stream_event` | Streaming chunks (noisy) | skip for output extraction |
| `result` | Final event (last line) | `result`, `num_turns`, `total_cost_usd`, `is_error` |
| `control_request/response` | Claude Code internal | skip |

**Key insight**: the `result` event's `result` field contains the agent's own
summary of what it accomplished — this is usually the most useful single piece
of text to extract from a completed session.

---

## Large Transcript Pattern (Mode C)

When JSONL files exceed 1MB, dispatch a dedicated sub-issue for analysis:

1. Create a sub-issue: `[HARVEST] Analyze session transcripts for <parent issue>`
2. Include exact file paths and extraction goal in the description
3. Dispatch with `SONNET_1_M_100_K_IN` variant (see `workspace-orchestrate` § Variants)
4. The sub-issue agent reads the full transcripts and produces a findings document

This avoids exhausting context in the orchestrating session on raw log parsing.

---

## Relationship to Other Skills

- **Uses**: `workspace-orchestrate` (for dispatch conventions and variant selection)
- **Complements**: `session-retrospective` (retrospective is post-mortem; harvest is mid-flight)
- **Delegates to**: `SONNET_1_M_100_K_IN` sub-issue for large transcript analysis
- **MCP tools**: `list_workspaces`, `list_sessions`, `run_session_prompt`, `get_execution`

## Constraints

- Read-only: does not modify any session or workspace state
- Does not terminate or interrupt running sessions
- Ping prompts (Mode A) consume budget in the target session — use sparingly
- Large files (> 5MB) must use Mode C — do not attempt inline extraction
