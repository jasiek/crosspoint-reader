---
name: hc-index-load
description: Load the .index/ directory for any code unit and produce a structured orientation summary. Use at session start, when switching focus to a new component, or before invoking action-analyze or action-implement. Works for any .index/-bearing path regardless of project type or architecture.
argument-hint: <path>
context: fork
agent: general-purpose
compatibility: Designed for Claude Code, Codex, OpenCode, and Gemini skills format.
metadata:
  owner: holicode
  scope: index-query
  pattern: karpathy-query
---

# Index Load

Load the `.index/` directory for the code unit at `$ARGUMENTS[0]` and produce a structured orientation summary. This is a **read-only query** — it surfaces existing knowledge, it does not analyze or verify.

Use `$0` as shorthand for `$ARGUMENTS[0]` throughout this skill.

## When to Use

- Starting work on a component you haven't touched recently
- Before invoking `action-analyze` (to decide if analysis is even needed)
- Before invoking `action-implement` (quick orientation before deep-reading SPEC.md)
- Switching focus between components mid-session
- Answering "what does this component do?" questions

## Input

- **Path**: `$ARGUMENTS[0]` — relative path to any code unit (module, package, service, library, app)
- The path must contain a `.index/` directory OR at least a `SPEC.md` / `CLAUDE.md` / `AGENTS.md`

## Procedure

### Phase 1: Discover What Exists

1. **Verify the path exists**. If `$ARGUMENTS[0]` is empty or the path doesn't exist, report an error and stop.

2. **Check for `.index/` directory** at `$0/.index/`:
   - If it exists → this is the primary path (Phase 2a)
   - If it does not exist → fallback path (Phase 2b)

### Phase 2a: Load from `.index/` (Primary Path)

3. **Read `$0/.index/overview.md`** — this is the single most important file. Extract:
   - Component purpose and boundaries
   - Architecture pattern (DDD / Layered / Standard / Fullstack / Custom)
   - Public API / entry points
   - Key decisions
   - Front matter: `confidence`, `last_analyzed`, `analysis_scope`

4. **List all other `.index/` files** without reading them yet. Categorize:
   - `dependencies.md` — present / absent
   - `domain-model.md` — present / absent
   - `aspects/_general.md` — present / absent
   - `aspects/*.md` (dedicated) — list names

5. **Read the aspect index** from `overview.md` front matter `aspects[]` array (if present):
   - Note each aspect's `status` and `relevance`
   - Flag any aspect with `relevance: high` and `status: not-analyzed` as a gap

6. **Conditionally read supplementary files** — only if overview.md references them or they are small:
   - `$0/.index/dependencies.md` — read if overview mentions integrations or coupling
   - `$0/.index/domain-model.md` — read if overview mentions DDD or complex domain
   - `$0/.index/aspects/_general.md` — always read if present (it's the quality baseline)

7. **Check for companion files** at `$0/`:
   - `SPEC.md` — note if present (do NOT deep-read; just confirm existence and read title/summary)
   - `CLAUDE.md` or `AGENTS.md` — note if present

### Phase 2b: Fallback (No `.index/` Directory)

8. **Read whatever exists** at `$0/`:
   - `$0/SPEC.md` — read summary section only
   - `$0/CLAUDE.md` or `$0/AGENTS.md` — read fully
   - Scan top-level directory listing for structure clues

9. **Report the gap**: Note that no `.index/` directory exists. Suggest:
   - Run `action-analyze $0 update` to bootstrap a `.index/` directory
   - Or manually scaffold using templates from `.holicode/templates/index/`

### Phase 3: Produce Orientation Summary

10. **Output the summary** in the format below. Present it directly to the calling agent.

## Output Format

```markdown
# Index: <component-name>

**Path**: `$0`
**Has .index/**: yes | no
**Confidence**: <confidence from overview.md frontmatter, or "n/a">
**Last analyzed**: <date, or "never">

## Purpose

<2-3 sentences from overview.md: what this component does and its boundaries>

## Architecture

- **Pattern**: <DDD / Layered / Standard / Fullstack / Custom / Unknown>
- **Entry points**: <key exports or public API surface>
- **Key decisions**: <1-2 most important architectural decisions>

## Index Files

| File | Present | Confidence | Notes |
|------|---------|------------|-------|
| overview.md | yes/no | <level> | <brief note> |
| dependencies.md | yes/no | <level> | <brief note> |
| domain-model.md | yes/no | <level> | <brief note> |
| aspects/_general.md | yes/no | <status> | <brief note> |
| aspects/<name>.md | yes/no | <status> | <relevance> |

## Gaps

<List any notable gaps: missing files, scaffold-only confidence, not-analyzed high-relevance aspects.
If no gaps: "Index is complete. No gaps detected.">

## Companion Files

- **SPEC.md**: present / absent — <title if present>
- **CLAUDE.md / AGENTS.md**: present / absent

## Ready

<One sentence: what the agent should do next based on index state.
Examples:
- "Index is complete. Ready to implement — invoke /action-implement $0."
- "Overview exists but aspects are not analyzed. Consider /action-analyze $0 first."
- "No .index/ directory. Run /action-analyze $0 update to bootstrap.">
```

## Constraints

- **Read-only** — never create, modify, or delete any files
- **Fast** — this is an orientation query, not an analysis. Read selectively, don't scan source code
- **No verification** — do not cross-reference `.index/` claims against source code (that's `action-analyze`'s job)
- **Scope to path** — do not read files outside `$0/` and `$0/.index/`
- **No architecture detection from source** — report the pattern from `.index/overview.md` only; if overview doesn't state it, report "Unknown" (don't infer by scanning code)
- **Progressive disclosure** — read overview first, then decide what else to read; never bulk-read all `.index/` files upfront
- **No legacy `.context/` support** — this skill reads `.index/` only. Projects still on `.context/` must migrate directories via `git mv` before using this skill (HOL-508 hard cutover).

## Relationship to Other Skills

| Skill | Relationship |
|-------|-------------|
| `action-analyze` | **index-load** surfaces what's known; **action-analyze** verifies it against code and fills gaps |
| `action-implement` | **index-load** orients the agent; **action-implement** reads SPEC.md deeply and writes code |
| `task-init` | **task-init** loads HoliCode project state; **index-load** loads per-component `.index/` |

## Extension Points

Project-specific extensions can augment `index-load` by placing additional instructions in:
- `$0/.index/skills/index-load.md` — per-component overrides (e.g., name resolution, extra files to read)
- `$0/AGENTS.md` — agent instructions that supplement the loaded index

Extensions should add to the orientation, not replace the generic procedure. See the issue (HOL-508) for the WebCon extension design as a reference example.
