---
name: hc-issue-tracker-local
description: Local issue-tracker provider for HoliCode. Manages epic/story/task/TD tracking directly in .holicode/specs and WORK_SPEC without external tracker dependency.
compatibility: Designed for Claude Code, Codex, OpenCode, and Gemini skills format.
metadata:
  owner: holicode
  provider: local
  scope: issue-tracking-only
---

# Local Issue Tracker Provider

Use this skill when `.holicode/state/techContext.md` has `issue_tracker: local`.

## Responsibilities

- Manage work items locally in `.holicode/specs/**` and `.holicode/state/WORK_SPEC.md`.
- Create and update issue-equivalent records (epic, story, task, technical-design, spike).
- Maintain local hierarchy and dependency references between local IDs.
- Provide normalized data for `issue-sync` without external API calls.

## Local ID Convention

- `EPIC-001`, `STORY-001`, `TASK-001`, `TD-001`, `SPIKE-001`.
- Keep IDs stable and human-readable.
- Reuse existing IDs when updating; allocate next numeric ID when creating.

## Storage Mapping

- `epic` -> `.holicode/specs/epics/EPIC-*.md`
- `story` -> `.holicode/specs/stories/STORY-*.md`
- `task` -> `.holicode/specs/tasks/TASK-*.md`
- `technical-design` -> `.holicode/specs/technical-design/TD-*.md`
- `spike` -> `.holicode/specs/tasks/SPIKE-*.md`

## Operation Mapping

1. `create_issue(...)` -> create/update the mapped local spec file + add entry in `WORK_SPEC.md`.
2. `update_issue(...)` -> update title/status/description in local file + refresh `WORK_SPEC.md` entry.
3. `resolve_issue_ref(...)` -> resolve to local file path and canonical local ID.
4. `set_parent(...)` -> update parent reference fields/links in local specs.
5. `set_relation(...)` -> update dependency references (`Blocks`, `Blocked By`, related links).

## Git-Aware Behavior (Optional)

When `.holicode` is tracked by git, this skill may also support local sync hygiene:

- Stage changed local tracking files (`.holicode/specs/**`, `.holicode/state/**` as needed).
- Optionally create a focused sync commit when explicitly requested by the user or workflow policy.
- Before branch switches/handoffs, verify local tracking changes are not silently stranded.

Do not auto-commit without explicit instruction.
