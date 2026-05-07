---
name: hc-issue-tracker
description: Unified issue-tracker interface for HoliCode. Use for create/update/link/resolve operations across providers. Routes to provider-specific skills based on .holicode/state/techContext.md.
compatibility: Designed for Claude Code, Codex, OpenCode, and Gemini skills format.
metadata:
  owner: holicode
  scope: issue-tracking-only
---

# Issue Tracker Interface

This skill defines a provider-agnostic interface for issue tracking operations in HoliCode.

Use this skill when the user asks to:
- create or refine epic/story/task issues
- update issue status/title/description
- resolve a simple issue ID (for example `GIF-15`) to provider-native IDs
- set parent-child links, type tags, and issue relations

## Scope Boundaries

- This skill handles issue tracking only.
- Pull requests, reviews, and merge workflows are out of scope.
- For GitHub, issue tracking and PR management are intentionally separated.

## Provider Selection

1. Read `.holicode/state/techContext.md`.
2. Resolve `issue_tracker`.
3. Route by provider:
   - `vibe_kanban` -> use `issue-tracker-vibe-kanban`
   - `github` -> use `issue-tracker-github-issues`
   - `local` -> use `issue-tracker-local`
4. If not configured, ask for explicit provider selection and default to `vibe_kanban` only if the project context says so.

## Canonical Operations

All provider skills should implement these operations with provider-native tools:

1. `create_issue(issue_type, title, description, parent_ref?, priority?)`
2. `update_issue(issue_ref, status?, title?, description?, priority?)`
3. `delete_issue(issue_ref)`
4. `resolve_issue_ref(issue_ref)` — resolve human-readable ID to native ID
5. `set_parent(issue_ref, parent_ref)` — pass null to un-nest
6. `set_type(issue_ref, issue_type)`
7. `set_relation(issue_ref, related_ref, relation_type)`
8. `assign_issue(issue_ref, user_ref)`
9. `unassign_issue(issue_ref, user_ref)`

`sync_work_spec()` is handled by the dedicated `issue-sync` skill to keep responsibilities separated.

## Output Contract

When reporting back, always include:
- Provider used
- Human-readable issue ID (`HOL-15`, `#123`, `TASK-001`)
- Native ID if different (UUID, node ID, etc.)
- Changed fields, resulting status, and priority

## Source of Truth

- For external providers (`vibe_kanban`, `github`), the issue tracker is the source of truth.
- For `local`, `.holicode/specs/**` and `WORK_SPEC.md` are the source of truth.
