# Issue Tracker Skill Configuration

This project uses a unified issue-tracker skill interface with provider-specific implementations.

## Skills

- Interface skill: `issue-tracker`
- Sync skill: `issue-sync`
- Active provider skill: `issue-tracker-vibe-kanban`
- Alternate provider skill: `issue-tracker-github-issues`
- Local-only provider skill: `issue-tracker-local`

## Provider Routing

- `issue_tracker: vibe_kanban` -> `issue-tracker-vibe-kanban`
- `issue_tracker: github` -> `issue-tracker-github-issues`
- `issue_tracker: local` -> `issue-tracker-local`

## Type Taxonomy (Recommended, Non-Blocking)

Preferred tags/labels for external trackers:
- `epic`, `story`, `task`, `technical-design`, `spike`, `bug`

If some tags/labels do not exist, operations should continue using deterministic fallbacks (title prefixes/description metadata) and report assumptions.

Use `.holicode/state/issueTrackerBootstrap.md` during initialization, then remove it when confirmed.

## Runtime Source of Truth

- Effective provider is configured in `.holicode/state/techContext.md` via `issue_tracker`.
- If configuration and this file diverge, `.holicode/state/techContext.md` wins.
- Local mode is valid for small projects that manage work entirely in `.holicode/specs/**` + `WORK_SPEC.md`.

## Scope Boundary

- These skills handle issue tracking only.
- Pull request operations remain in dedicated Git/GitHub workflows.
- In local mode, `issue-sync` is typically `noop` unless local spec consistency or git hygiene actions are needed.
