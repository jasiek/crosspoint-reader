---
name: hc-issue-tracker-github-issues
description: GitHub Issues provider for HoliCode issue tracking only. Handles issue create/update/sync while leaving PR operations to dedicated git/github workflows.
compatibility: Requires gh CLI and/or GitHub MCP access configured for the repository.
metadata:
  owner: holicode
  provider: github
  scope: issues-only
---

# GitHub Issues Tracker Provider

Use this skill when `.holicode/state/techContext.md` has `issue_tracker: github`.

## Responsibilities

- Create and update GitHub Issues for epics/stories/tasks.
- Resolve issue references (for example `#123`) and maintain hierarchy.
- Manage labels/type mapping and issue relationships for dependency tracking.

## Scope Boundaries

- This skill is limited to GitHub Issues.
- Pull request creation/review/update is handled by dedicated GitHub PR workflows/skills.

## Tooling

- Primary: `gh issue` commands and/or GitHub MCP issue tools.
- Use repository/project identifiers from `.holicode/state/techContext.md`.

## Standard Procedure

1. Read `.holicode/state/techContext.md` and confirm `issue_tracker: github`.
2. Resolve user issue refs (`#123`) before updates.
3. Prefer label-based type classification (`epic`, `story`, `task`, `technical-design`, `spike`, `bug`) and link relationships.
4. If labels are missing/unavailable, continue with deterministic fallback (title/body conventions) and report fallback usage.
5. Report resulting issue number, URL, status, and labels.

## Bootstrap Check (Recommended)

At project setup, check whether preferred labels exist.

- Missing labels are not blocking.
- Create them when possible, or proceed with fallback classification and note assumptions.

Use `.holicode/state/issueTrackerBootstrap.md` as a temporary setup checklist.

## Sync Support Contract

When `issue-sync` requests data, return normalized fields:
- `id_ref`: `#<number>`
- `status`: open/closed mapped to project status vocabulary
- `type`: from labels (fallback to title/body conventions)
- `parent_ref`: from configured parent-link convention
- `relations`: from linked issue references
- `tags`: label names

Keep `WORK_SPEC.md` as cache/reference, never source of truth.
