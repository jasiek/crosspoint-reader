---
name: hc-issue-sync
description: Tracker-agnostic sync skill that updates WORK_SPEC.md from the configured issue provider by delegating provider-specific issue operations to the active issue-tracker provider skill.
compatibility: Designed for Claude Code, Codex, OpenCode, and Gemini skills format.
metadata:
  owner: holicode
  scope: issue-sync-only
---

# Issue Sync

This skill synchronizes issue provider state into `.holicode/state/WORK_SPEC.md`.

## Design Goal

- Keep sync logic tracker-agnostic.
- Keep provider-specific logic in provider skills only.
- Keep `WORK_SPEC.md` as a local read model, never a source of truth.

## Delegation Model

1. Read `.holicode/state/techContext.md` and `.clinerules/config/issue-tracker.md`.
2. Resolve active provider skill through `issue_tracker`.
3. Delegate tracker access and normalization to the provider skill:
   - `issue-tracker-vibe-kanban`
   - `issue-tracker-github-issues`
   - `issue-tracker-local`
4. Render normalized issues into `WORK_SPEC.md` sections.

## Local Mode (`issue_tracker: local`)

In local mode, sync is mostly a consistency pass (no remote fetch):

1. Normalize from local specs (`.holicode/specs/epics|stories|tasks|technical-design`) and `WORK_SPEC.md`.
2. Rebuild or reconcile `WORK_SPEC.md` sections from local canonical files.
3. If nothing changed, report `noop` sync.
4. If `.holicode` is tracked by git, optionally support staging/commit hygiene when explicitly requested.

Recommended branch-safety checks for local mode:
- Ensure local tracking updates are not left uncommitted before branch switches/handoffs.
- Keep local tracking commits small and focused when used.

## Normalized Sync Contract

The provider should return entries with this minimal shape:

```text
id_ref        # human ID (HOL-15, #123)
title
status
type          # epic | story | task | technical-design | spike | meta
priority      # optional: urgent | high | medium | low
parent_ref    # optional
relations[]   # optional: blocking | related | has_duplicate
tags[]        # optional
```

## Rendering Rules

- `Features`: `type == epic`
- `Active Stories`: `type == story`
- `Current Tasks`: `type == task`
- `Technical Design Documents`: `type == technical-design`
- `Completed`: any status that maps to done/completed

Include parent/relationship context inline when available.

## Drift Report

After sync, report:
- Added entries
- Removed entries
- Status changes
- Type/parent/relationship changes

## Taxonomy Assumption Policy

- Preferred external type taxonomy: `epic`, `story`, `task`, `technical-design`, `spike`, `bug`.
- This taxonomy is recommended, not required.
- If tags/labels are missing, sync must continue with deterministic fallback mapping and report assumptions.

## Constraints

- Do not mutate tracker data during sync.
- If provider metadata is incomplete, use deterministic fallback mapping and report assumptions.
- In local mode, do not auto-commit unless explicitly requested.
