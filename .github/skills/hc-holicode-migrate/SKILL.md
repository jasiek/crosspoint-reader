---
name: hc-holicode-migrate
description: "Framework migration and reconciliation skill. Orchestrates multi-tier update (sync + refresh + drift review) with before/after reporting. Accepts migration guidance as input or auto-detects changes."
compatibility: Designed for Claude Code, Codex, OpenCode, and Gemini skills format.
metadata:
  owner: holicode
  scope: framework-migration
---

# HoliCode Migrate (Framework Update Reconciliation)

Orchestrate a guided framework update by running a multi-tier synchronization sequence, analyzing what changed, and producing a structured migration report. This skill is the recommended entry point when updating a HoliCode project instance to a new framework version or when reconciling after merging upstream framework changes.

## When to Use

- After pulling/merging upstream framework changes that affect skills, workflows, or config
- When upgrading a HoliCode project to a new framework version
- When the user provides release notes or migration instructions for a framework update
- When the agent detects significant drift in `.github/` after a framework update
- User asks to "migrate", "upgrade framework", or "reconcile framework changes"
- Periodic framework reconciliation (e.g., at sprint or milestone boundaries)

**Context**: This skill runs in **HoliCode-driven projects** (target projects, not the framework repo itself). The external `scripts/update.sh` script (run from the framework repo) has already synced framework files to `.github/` before this skill executes. This skill performs post-sync reconciliation: state refresh, drift review, and reporting.

## When NOT to Use

- Running in the framework repo itself — the framework repo uses `holicode-sync` directly for self-management, not `holicode-migrate`
- `scripts/update.sh` has not been run yet — this skill assumes framework files are already in `.github/`
- Mid-implementation of a feature task — migration should happen between work items, not during active coding
- No `.holicode/` directory exists — not a HoliCode-enabled project

## Constraints (Hard Rules)

1. **Composable delegation**: Never duplicate logic from `holicode-sync`, `task-init`, or `issue-sync`. Delegate to them.
2. **Dry-run default**: The skill runs in dry-run mode unless the user explicitly requests execution. Dry-run produces the full report without mutations.
3. **Project-specific preservation**: Never delete project-specific items from `.github/` (inherited from `holicode-sync` safety).
4. **No external dependencies for Tier 1**: Tier 1 is local-only. Tier 2 may use the issue tracker (read-only for analysis, write-only for follow-up tasks with confirmation).
5. **Report always produced**: Both dry-run and execute modes generate a migration report.
6. **Time budget**: Tier 1 should complete in under 5 minutes. Tier 2 should complete in under 15 minutes.
7. **Follow-up task creation requires confirmation**: Even in execute mode, never auto-create tracker issues without user approval.

## Input Modes

The skill accepts optional migration guidance to inform the update process. When no guidance is provided, it records the current `.github/` inventory and state file freshness without attempting drift detection (since `scripts/update.sh` already synced files before this skill runs).

### Mode 1: Release Notes (Structured)

The user provides structured release notes (changelog-style). The skill extracts:
- New skills/workflows added
- Modified skills/workflows (with change summaries)
- Removed/deprecated items
- Breaking changes requiring manual intervention
- Configuration changes

Detection heuristic: Input contains version headers (`## v0.3.0`), changelog sections (`### Added`, `### Changed`, `### Breaking`), or explicit `release_notes:` YAML.

### Mode 2: Migration Markdown (Narrative)

The user provides prose migration instructions (e.g., a migration guide or analysis document). The skill extracts:
- Step-by-step migration actions
- Files to review manually
- Configuration changes needed
- Known issues or workarounds

Detection heuristic: Input contains migration-specific language ("migrate", "upgrade", "manual step required", "breaking change") in prose form without changelog structure.

### Mode 3: Auto-Detect (No Input)

No guidance provided. The skill inventories `.github/` and records current state. The migration report will note that no guidance was provided and recommend reviewing changes manually against the framework release notes or changelog.

### Input Routing

1. If input is provided, scan the first 20 lines for structural markers
2. Release notes markers: version headers, changelog sections (`### Added` / `### Changed`), bullet lists of changes
3. Migration markdown markers: prose instructions, numbered steps, "breaking change" callouts
4. If markers are ambiguous, treat as migration markdown (more permissive)
5. If no input provided, proceed in auto-detect mode
6. Store the detected input mode and extracted guidance for report generation

## Standard Procedure

Execute phases in order. The skill operates in two tiers with a confirmation gate between them.

### Phase 0: Pre-Flight

**Step 0.1: Verify prerequisites**

Check that the project is HoliCode-enabled and framework files have been delivered:

1. Check for `.holicode/` — must exist. If missing, abort: not a HoliCode-enabled project.
2. Check for `.github/` — must exist. If missing, abort: no framework instance found.
3. Check for `.github/skills/` and `.github/agents/` — both must exist. If missing, abort: `scripts/update.sh` has not been run yet (no framework files delivered).

**Note**: This skill assumes `scripts/update.sh` (run from the framework repo) has already synced framework files to `.github/` before migration begins.

**Step 0.2: Determine execution mode**

- If user said "dry-run", "preview", or "what would change" → `DRY_RUN`
- If user said "execute", "apply", or "run migration" → `EXECUTE` (still confirms before mutations)
- Default: `DRY_RUN`

**Step 0.3: Parse migration guidance input (if provided)**

Apply the input mode detection from the Input Modes section above:
1. Check if the user provided migration guidance (release notes, markdown, or a file path)
2. If a file path is provided, read the file
3. Detect input mode via structural markers
4. Extract structured guidance: changes list, breaking changes, manual steps
5. Store as `migration_guidance` for later phases

**Step 0.4: Capture "before" snapshot**

Record the current state for before/after comparison:

Framework instance inventories (`.github/`):
- Skill directories in `.github/skills/` (directory names + count)
- Workflow files in `.github/agents/` (file names + count)
- Config files in `.clinerules/config/` (file names + count)
- Framework version from `.clinerules/holicode.md` (parse `mb_meta.version` from YAML frontmatter, or fall back to first heading)

State file metadata:
- Last-modified dates for `activeContext.md`, `progress.md`, `WORK_SPEC.md` (from git log or file stats)

This snapshot becomes the "Before" column in the migration report.

**Note on report template**: The template has "source" and "instance" rows in the Before/After Summary table. Since this skill runs in target projects (no framework source at root), populate "source" rows with "N/A" and use "instance" rows for the `.github/` inventory above.

### Phase 1: Tier 1 — State Refresh

Tier 1 performs state file refresh from the issue tracker. Framework files were already synced by `scripts/update.sh` before this skill runs.

**Step 1.0: Pre-mutation confirmation (EXECUTE mode only)**

In `EXECUTE` mode, before any mutations occur:
1. Present instance inventory from Step 0.4 (current skill/workflow/config counts)
2. Note that state files will be refreshed from the tracker
3. Ask: "This will refresh state files (activeContext, WORK_SPEC, progress) from the tracker. Proceed? [y/n]"
   - If "n" or user declines: Switch to `DRY_RUN` mode and continue (report still generated)
   - If "y": Continue in `EXECUTE` mode

In `DRY_RUN` mode: Skip this step.

**Step 1.1: Invoke task-init (state refresh)**

Delegate to the `task-init` skill for session state refresh.

- In `DRY_RUN` mode: Execute `task-init` step 1 only (Load state files). Present current state without modifying generated zones. Record state file freshness.
- In `EXECUTE` mode: Execute the full `task-init` procedure (steps 1-6). This regenerates activeContext zones, refreshes WORK_SPEC via issue-sync, and runs board triage.

**Step 1.2: Capture "after Tier 1" snapshot**

Record the same inventory categories as Phase 0 Step 0.4. Framework file counts in `.github/` are unchanged by Tier 1 (files were synced by `scripts/update.sh` before this skill ran). The snapshot captures state file freshness after `task-init`.

**Step 1.3: Present Tier 1 results and gate**

Present a summary of Tier 1 findings:
- Framework inventory: N skills, M workflows, P config files (unchanged — synced by `scripts/update.sh` prior to migration)
- State file freshness assessment (activeContext, progress, WORK_SPEC)
- Board orientation (if tracker available)

Then ask: "Tier 1 complete. Proceed to Tier 2 (drift review + follow-up task creation)? [y/n]"
- If "n" or user declines: Skip to Phase 3 (report generation) with Tier 2 marked as "skipped"
- If "y": Continue to Phase 2

### Phase 2: Tier 2 — Recommended Reconciliation

Tier 2 performs deeper drift analysis and creates actionable follow-up items.

**Step 2.1: Issue-sync refresh (conditional)**

`task-init` (Step 1.1) already invokes `issue-sync` as part of its full procedure. Only re-invoke `issue-sync` here if:
- `scripts/update.sh` updated tracker-related config (e.g., `issue-tracker.md` changed)
- The `issue-sync` skill itself was updated by `scripts/update.sh`

If neither condition is met, skip — WORK_SPEC.md is already fresh from the `task-init` run.

In `DRY_RUN` mode: Always skip — state was already captured in Tier 1.

**Step 2.2: Drift review**

Cross-reference the instance inventory (from Step 0.4) against migration guidance (if provided). If no guidance was provided, review symlinks, state file freshness, and framework version. Flag items where manual adjustment may be needed.

Categorize findings into three tiers:

**Action Required** — Items needing manual intervention:
- Breaking changes from migration guidance that affect project-specific customizations
- `blocked` symlinks (regular file/directory where symlink expected)
- Entry point files (CLAUDE.md, AGENTS.md) that were overwritten with full inventories — should be lightweight bootstraps instead (see Entry Point Contract)

**Review Recommended** — Items to verify but likely fine:
- Skills/workflows updated by `scripts/update.sh` that may affect project-specific usage
- Config changes from the framework that may require project-specific adjustments
- State file timestamps (check if they're stale relative to last framework update)

**Informational** — Changed items requiring no action:
- New skills/workflows added by `scripts/update.sh`
- Framework version bump (e.g., v0.2.0 → v0.2.1)
- Symlinks verified as correct

**Step 2.3: Generate follow-up tasks**

If migration guidance specifies manual steps or if drift review found "Action Required" items:
1. List recommended follow-up tasks with descriptions and priority
2. In `EXECUTE` mode with user confirmation: Create tracker issues for each follow-up task
3. In `EXECUTE` mode without confirmation or in `DRY_RUN` mode: Output the task list as recommendations only

**Step 2.4: "What to Test" analysis**

Based on changed workflows and skills, generate a testing checklist:
- For each `updated` or `new` skill: Suggest a scenario to verify it works
- For each `updated` or `new` workflow: Suggest invoking it on a sample input
- For breaking changes from migration guidance: Specific regression tests
- For entry point divergences: Verify agent discovery still works (check symlink chain)

### Phase 3: Generate Migration Report

**Step 3.1: Render report**

Apply the `templates/analysis/MIGRATION-REPORT-template.md` template. Fill all `{{placeholder}}` values from data collected in Phases 0-2.

In `DRY_RUN` mode: Mark the report header with `[DRY RUN]` and annotate mutation sections with "would be applied".

**Step 3.2: Save report**

Save to `.holicode/analysis/migration-reports/MIGRATION-<YYYY-MM-DD>.md`
- Create the `migration-reports/` directory if it doesn't exist
- If a report for today already exists, append a sequence number: `MIGRATION-<YYYY-MM-DD>-2.md`

**Step 3.3: Present summary**

Output an inline summary:

```
Migration Report ({{MODE}}):
- Framework: {{before_version}} → {{after_version}}
- Synced: X skills, Y workflows, Z config files
- Preserved: N project-specific items
- Entry points: {{status}}
- Tier 2: {{executed / skipped}}
- Follow-up tasks: {{count}} recommended
- Report: .holicode/analysis/migration-reports/MIGRATION-<date>.md
{{- If dry-run: "Re-run with 'execute' to apply changes." }}
```

## Relationship to Other Skills

- **Delegates to**: `task-init` (Tier 1 — state refresh), `issue-sync` (Tier 2 — board refresh, conditional)
- **Does NOT delegate to**: `holicode-sync` — framework files are synced by `scripts/update.sh` before this skill runs
- **Uses**: `issue-tracker` provider (Tier 2 — read-only for analysis, optional write for follow-up tasks)
- **Complements**: `scripts/update.sh` (external script syncs framework files; this skill performs post-sync reconciliation)
- **Produces**: Migration report in `.holicode/analysis/migration-reports/`
- **Template**: `templates/analysis/MIGRATION-REPORT-template.md`

## Safety Checks

- **Dry-run default**: Always dry-run first. Never mutate without explicit user confirmation.
- **Never delete**: Inherits `holicode-sync` safety — project-specific items in `.github/` are preserved.
- **Never auto-overwrite entry points**: CLAUDE.md and AGENTS.md divergences are reported, not auto-fixed. See Entry Point Contract below for what these files should contain.
- **Never rewrite entry points with full inventories**: Entry points are lightweight bootstraps, not skill/workflow catalogs. Agent discovery happens via symlinks (`.claude/skills/`, `.claude/agents/`), not by listing contents in CLAUDE.md or AGENTS.md.
- **Tracker read-only during analysis**: Drift review and analysis phases never mutate tracker data.
- **Follow-up task creation gated**: Even in execute mode, follow-up tracker issues require explicit user confirmation before creation.
- **Report always saved**: Both modes produce and save a report for auditability.
- **Stage explicitly**: If git operations follow, stage only the changed files (never `git add .` or `git add -A`).

## Entry Point Contract

Root entry point files (`CLAUDE.md`, `AGENTS.md`) are **lightweight bootstraps** — they point agents to the framework and list project-specific context. They are NOT skill/workflow inventories.

**What entry points should contain:**
- Pointer to `.clinerules/holicode.md` (the framework rules)
- List of state files to read on session start
- Project-specific commands (test, build, lint)
- Project-specific git conventions
- A short workflow table with 5-7 key workflows (optional, for quick reference only)

**What entry points should NOT contain:**
- Full inventory of all skills and workflows (agents discover these via symlinks)
- Detailed skill descriptions (these live in `SKILL.md` files)
- Content that duplicates `holicode.md`

**Discovery mechanism**: Agents find skills and workflows through directory symlinks:
- `.claude/skills → ../.github/skills` (skill discovery)
- `.claude/agents → ../.github/agents` (workflow discovery)

Entry points may contain project-specific additions — these are valid customizations and should be preserved, not replaced. If `scripts/update.sh` overwrote entry points with framework defaults, manually merge project-specific sections back in.
