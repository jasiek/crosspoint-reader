---
name: hc-holicode-sync
description: Sync framework skills, workflows, and config from source (skills/, workflows/) into project instance (.github/agents/, .github/skills/) and refresh agent entry points. Hot-reload for HoliCode.
compatibility: Designed for Claude Code, Codex, OpenCode, and Gemini skills format.
metadata:
  owner: holicode
  scope: framework-instance-sync
---

# HoliCode Sync (Hot Reload)

Synchronize HoliCode framework source files into the project's active instance directories. This is the in-session equivalent of `scripts/update.sh` — a "hot reload" that ensures new or modified skills, workflows, and config are immediately available to all agent harnesses without leaving the session.

## When This Skill Applies

- A new skill or workflow was added/modified in framework source (`skills/`, `workflows/`)
- Agent entry points (CLAUDE.md, AGENTS.md) need updating after framework changes
- Skills or workflows are missing from `.github/` but present in source
- After merging framework updates that add new capabilities
- User asks to "sync", "refresh", or "hot reload" the framework
- After applying a process learning that changed a skill or workflow source file

## Scope Boundaries

- Syncs **framework artifacts only** — does not modify application code
- State files (`.holicode/state/*`) are NOT part of sync — they are instance-specific
- Does not push to remote or create PRs — use `agentic-env-lifecycle` for that
- For full external sync (from a separate framework repo), use `scripts/update.sh` instead
- Templates (`.holicode/templates/`) and specs (`.holicode/specs/`) are NOT synced by this skill — use `scripts/update.sh` for those

## Architecture

```
Framework Source (canonical)        Project Instance (active)
─────────────────────────          ──────────────────────────
skills/                       →    .github/skills/
workflows/                    →    .github/agents/
config/                       →    .clinerules/config/
holicode.md                   →    .clinerules/holicode.md
agent-boot/CLAUDE.md          →    CLAUDE.md (root)        [cautious]
agent-boot/AGENTS.md          →    AGENTS.md (root)        [cautious]

Symlinks (agent discovery):
  .claude/skills    → ../.github/skills
  .claude/agents    → ../.github/agents
  .opencode/skills  → ../.github/skills
  .opencode/agents  → ../.github/agents
  .gemini/skills    → ../.github/skills
  .gemini/agents    → ../.github/agents
  .qwen/skills      → ../.github/skills
  .qwen/agents      → ../.github/agents
  .agents/skills    → ../.github/skills
```

## Standard Procedure

Execute steps 1-8 in order. Collect results as you go for the final drift report.

### 1. Pre-Flight

Verify framework source directories exist:
- `skills/` must exist (required)
- `workflows/` must exist (required)
- `config/` may exist (optional)
- `holicode.md` must exist (required)

If `skills/` or `workflows/` is missing, abort with error — this is not a HoliCode framework repo.

Verify instance target exists:
- `.github/` must exist as a directory (not a file)
- Create `.github/skills/`, `.github/agents/`, `.clinerules/config/` if missing

### 2. Detect Drift

Compare framework source with instance targets to build a drift inventory.

**Skills drift**: List directories in `skills/` vs `.github/skills/`. For each:
- Present in source but not instance → mark as `new` (will sync)
- Present in both → compare `SKILL.md` files. If different → mark as `updated` (will sync)
- Present in both, identical → mark as `current` (skip)
- Present in instance but not source → mark as `project-specific` (preserve, flag in report)

**Workflows drift**: List files in `workflows/` vs `.github/agents/`. Same classification.

**Config drift**: If `config/` exists, list files in `config/` vs `.clinerules/config/`. Same classification.

**Core rules drift**: Compare `holicode.md` with `.clinerules/holicode.md`. Mark as `updated` or `current`.

### 3. Sync Skills

For each skill classified as `new` or `updated` in step 2:
- Copy the entire skill directory: `skills/<name>/` → `.github/skills/<name>/`
- Use recursive copy to include all files (SKILL.md + any supporting files)

**Safety**: Never delete directories in `.github/skills/` that are not in `skills/`. These are project-specific additions.

### 4. Sync Workflows

For each workflow file classified as `new` or `updated` in step 2:
- Copy the file: `workflows/<name>.md` → `.github/agents/<name>.md`

**Safety**: Never delete files in `.github/agents/` that are not in `workflows/`. These may be project-specific additions (e.g., a workflow added directly to the instance).

### 5. Sync Config and Core Rules

If `config/` exists in framework source:
- For each config file classified as `new` or `updated`: copy `config/<file>` → `.clinerules/config/<file>`

Compare `holicode.md` with `.clinerules/holicode.md`:
- If different, overwrite `.clinerules/holicode.md` with framework source
- `holicode.md` is framework-owned and should always match source

### 6. Verify and Repair Symlinks

Check all expected agent discovery symlinks exist and point to the correct targets:

```
Expected symlinks:
  .claude/skills    → ../.github/skills
  .claude/agents    → ../.github/agents
  .opencode/skills  → ../.github/skills
  .opencode/agents  → ../.github/agents
  .gemini/skills    → ../.github/skills
  .gemini/agents    → ../.github/agents
  .qwen/skills      → ../.github/skills
  .qwen/agents      → ../.github/agents
  .agents/skills    → ../.github/skills
```

For each expected symlink:
1. Create parent directory if it doesn't exist (e.g., `mkdir -p .claude`)
2. If path exists and is a correct symlink → `ok`
3. If path exists but points to wrong target → remove and recreate → `repaired`
4. If path doesn't exist → create symlink → `created`
5. If path exists but is a regular file/directory (not a symlink) → `blocked` (report, don't delete)

### 7. Check Entry Points (Cautious — Report, Don't Overwrite)

Compare `agent-boot/CLAUDE.md` with root `CLAUDE.md`:
- If files are identical → `in-sync`
- If `agent-boot/CLAUDE.md` does not exist → skip
- If files differ → `diverged` — report the divergence but do NOT auto-overwrite

Do the same for `agent-boot/AGENTS.md` vs root `AGENTS.md`.

**Rationale**: Root entry points may contain project-specific customizations (extra commands, test instructions, etc.). Auto-overwriting would destroy those. Always report divergence and let the human decide.

### 8. Generate Drift Report

After all sync operations complete, output a structured drift report:

```
## HoliCode Sync Report

### Skills
| Skill | Status |
|-------|--------|
| <name> | new / updated / current / project-specific |

### Workflows
| Workflow | Status |
|----------|--------|
| <name> | new / updated / current / project-specific |

### Config
| File | Status |
|------|--------|
| <name> | new / updated / current |

### Core Rules
- holicode.md: updated / current

### Symlinks
| Path | Status |
|------|--------|
| .claude/skills | ok / created / repaired / blocked |
| .claude/agents | ok / created / repaired / blocked |
| ... | ... |

### Entry Points
| File | Status | Action |
|------|--------|--------|
| CLAUDE.md | in-sync / diverged | none / manual merge recommended |
| AGENTS.md | in-sync / diverged | none / manual merge recommended |

### Summary
- Synced: X skills, Y workflows, Z config files
- Preserved: N project-specific items
- Symlinks: M ok, P created/repaired, Q blocked
- Entry points: [status]
- Warnings: [any issues]
```

## Apply Learning → Sync → Available (Meta-Update Cycle)

When a session learning implies a framework change:

1. **Apply** the learning to the canonical source file (`skills/<name>/SKILL.md` or `workflows/<name>.md`)
2. **Re-invoke this skill** to propagate the change to the instance
3. The updated skill/workflow is immediately available to all agent harnesses via symlinks

This creates a tight feedback loop: session observation → framework update → hot reload → immediate availability. No need to leave the session or run external scripts.

## Quick Sync (Single Item)

For simple cases where only one new skill or workflow was added:

```
# For a new skill:
1. Copy skills/<new-skill>/ → .github/skills/<new-skill>/
2. Symlinks already point to .github/skills/ — discovery is automatic

# For a new workflow:
1. Copy workflows/<new>.md → .github/agents/<new>.md
2. Symlinks already point to .github/agents/ — discovery is automatic
```

No full drift detection needed for single-item additions.

## Safety Checks

- **Never delete** items from `.github/` that don't exist in source — they may be project-specific
- **Never auto-overwrite** root CLAUDE.md or AGENTS.md — report divergence only
- **Always report** what changed via the drift report before any git operations
- **Stage explicitly**: if running inside a workspace, stage only the changed files (never `git add .` or `git add -A`)
- **No external dependencies**: this skill uses only local file operations (read, compare, copy, symlink)

## Constraints

- Local-only operation — no network calls, no API calls, no remote dependencies
- Framework source directories (`skills/`, `workflows/`, `config/`, `holicode.md`) are the canonical source of truth
- `.github/` is the active instance consumed by all agent harnesses via symlinks
- State files (`.holicode/state/*`) are never part of sync — they belong to the project instance
- Works in any workspace session (Coder, local, CI) without special tooling
