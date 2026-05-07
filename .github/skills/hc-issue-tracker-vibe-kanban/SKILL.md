---
name: hc-issue-tracker-vibe-kanban
description: Vibe Kanban provider for HoliCode issue tracking. Create/update/resolve/link/tag/relation operations via vibe_kanban MCP tools.
compatibility: Requires vibe_kanban MCP tools and project IDs configured in .holicode/state/techContext.md.
metadata:
  owner: holicode
  provider: vibe_kanban
  scope: issue-tracking-only
---

# Vibe Kanban Issue Tracker Provider

Use this skill when `.holicode/state/techContext.md` has `issue_tracker: vibe_kanban`.

## Responsibilities

- Create epic/story/task issues in the configured VK project.
- Update issue fields, status, and priority.
- Resolve user-facing IDs (for example `HOL-15`) to issue UUIDs.
- Manage hierarchy via native `parent_issue_id`.
- Manage type classification via tags.
- Manage cross-issue relationships (`blocking`, `related`, `has_duplicate`).
- Manage issue assignees.

## MCP Tool Reference

All tools use the `mcp__vibe_kanban__` prefix (e.g., `mcp__vibe_kanban__list_issues`). Below lists the short names.

### Issue CRUD

| Tool | Key Parameters | Notes |
|------|---------------|-------|
| `list_issues` | `project_id`, `simple_id`, `search`, `status`, `priority`, `tag_id`, `tag_name`, `assignee_user_id`, `parent_issue_id`, `limit`, `offset` | Returns `total_count`, `returned_count`. Use `simple_id` for exact ID lookup (e.g., `"HOL-42"`). Use `search` for substring matching. |
| `get_issue` | `issue_id` (UUID) | Returns embedded tags, relationships, and sub-issues. |
| `create_issue` | `title`, `project_id`?, `description`?, `parent_issue_id`?, `priority`? | `project_id` optional only if workspace is linked to a remote project. |
| `update_issue` | `issue_id`, `title`?, `description`?, `status`?, `priority`?, `parent_issue_id`? | Pass `parent_issue_id: null` to un-nest from parent. |
| `delete_issue` | `issue_id` | |

### Tags

| Tool | Key Parameters | Notes |
|------|---------------|-------|
| `list_tags` | `project_id`? | Discover available tags. `project_id` optional if workspace linked. |
| `list_issue_tags` | `issue_id` | Returns tags with `issue_tag_id` needed for removal. |
| `add_issue_tag` | `issue_id`, `tag_id` | Attach a tag to an issue. |
| `remove_issue_tag` | `issue_tag_id` | Uses the `issue_tag_id` from `list_issue_tags`, NOT `issue_id` + `tag_id`. |

### Relationships

| Tool | Key Parameters | Notes |
|------|---------------|-------|
| `create_issue_relationship` | `issue_id`, `related_issue_id`, `relationship_type` | Types: `blocking`, `related`, `has_duplicate`. |
| `delete_issue_relationship` | `relationship_id` | Get `relationship_id` from `get_issue` response or `create_issue_relationship` return value. |

### Assignees

| Tool | Key Parameters | Notes |
|------|---------------|-------|
| `list_issue_assignees` | `issue_id` | Returns entries with `issue_assignee_id`. |
| `assign_issue` | `issue_id`, `user_id` | Use `list_org_members` to discover user IDs. |
| `unassign_issue` | `issue_assignee_id` | Uses `issue_assignee_id` from `list_issue_assignees`, NOT `issue_id` + `user_id`. |

### Context & Discovery

| Tool | Key Parameters | Notes |
|------|---------------|-------|
| `get_context` | (none) | Returns `organization_id`, `project_id`, `issue_id`, `workspace_id`, `workspace_branch`, `workspace_repos`. |
| `list_organizations` | (none) | Discover org UUIDs. |
| `list_org_members` | `organization_id`? | Discover user UUIDs for assignment. |
| `list_projects` | `organization_id` | Discover project UUIDs. |
| `list_issue_priorities` | (none) | Returns allowed values: `urgent`, `high`, `medium`, `low`. |

## Project ID Resolution

`project_id` is required by `list_issues`, `create_issue`, and `list_tags` unless the workspace is linked to a remote project. Use this fallback chain:

1. **Check `get_context`** — if it returns a non-null `project_id`, use it.
2. **Check `.holicode/state/techContext.md`** — if a project UUID is cached there, use it.
3. **Discover via API** — `list_organizations` → pick org → `list_projects(organization_id)` → pick project.
4. Cache the resolved `project_id` in `techContext.md` for future use.

## Standard Procedure

1. Read `.holicode/state/techContext.md` and validate provider/project config.
2. Resolve `project_id` using the fallback chain above.
3. For user references like `HOL-15`, resolve via `list_issues(simple_id: "HOL-15")` before updates.
4. Prefer type tags (`epic`, `story`, `task`, `technical-design`, `spike`, `bug`) and parent via `parent_issue_id` where applicable.
5. Set `priority` when creating/updating issues (values: `urgent`, `high`, `medium`, `low`).
6. If tags are missing/unavailable, continue with deterministic fallback (title prefix and/or metadata block) and report fallback usage.
7. For dependencies, use relationship APIs instead of free-text metadata.
8. Confirm resulting `simple_id`, UUID, status, tags, priority, and parent back to the user.

## Pagination

`list_issues` supports pagination via `limit` and `offset` parameters. The response includes:
- `total_count`: total number of matching issues
- `returned_count`: number of issues in the current page

Default limit is 50. For full board syncs, iterate with increasing `offset` until `returned_count` < `limit` or all `total_count` items are fetched.

## Bootstrap Check (Recommended)

At project setup, check whether preferred tags exist using `list_tags`.

- Missing tags are not blocking.
- Create them when possible, or proceed with fallback classification and note assumptions.

Use `.holicode/state/issueTrackerBootstrap.md` as a temporary setup checklist.

## Sync Support Contract

When `issue-sync` requests data, return normalized fields from native VK data:
- `id_ref`: `simple_id`
- `status`: issue status
- `type`: from tags (fallback to title/description conventions)
- `priority`: from issue priority field
- `parent_ref`: resolve from `parent_issue_id`
- `relations`: map from `relationships`
- `tags`: native tag names

## Notes

- VK exposes tags, sub-issues, relationships, assignees, and priorities via MCP; prefer native fields over description metadata.
- Comment APIs are not currently exposed in this MCP surface.
- Workspace and repo management tools (`list_workspaces`, `start_workspace_session`, etc.) are out of scope for this skill — see `workspace-orchestrate` and `agentic-env-lifecycle`.
- Keep PR operations out of this skill.
