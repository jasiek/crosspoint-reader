---
name: hc-workspace-init
description: "Vibe Kanban-specific skill: ensure the current workspace has an owning issue and a meaningful title. Callable anytime — at session start (invoked by task-init), mid-session (user decides to anchor an informal conversation to an issue), or by other skills (research-dispatch, task-implement) as a pre-condition. Gracefully no-ops when not in a Vibe Kanban context. Trigger: any moment when a workspace should be anchored to a tracker issue."
compatibility: Vibe Kanban MCP only. No-op in non-VK HoliCode environments.
metadata:
  owner: holicode
  scope: workspace-lifecycle
---

# Workspace Init

Anchor the current Vibe Kanban workspace to a tracker issue and ensure it has a meaningful title.

An unlinked workspace is an orphan — its output has no traceability back to the board,
no relationships to other work, and no way to find it later. This skill closes that gap.

**VK-specific**: this skill requires Vibe Kanban MCP tools (`get_context`,
`link_workspace_issue`, `update_workspace`). It does nothing in non-VK HoliCode
environments — check for VK context in Step 1 and exit gracefully if absent.

## When To Invoke

- **Session start** (via `task-init`): automatically delegated to when `task-init`
  detects an unlinked workspace in a VK environment
- **Mid-session**: user wants to "make this conversation official" and attach it to an issue
- **Pre-condition**: any skill (`research-dispatch`, `task-implement`, etc.) that needs
  the workspace anchored before producing tracked artifacts
- **Manual**: user explicitly asks to link or create an issue for the current workspace

## When NOT To Invoke

- Not in a Vibe Kanban context — exit immediately (Step 1 guard)
- Workspace already has a linked issue — nothing to do
- User explicitly says they don't want tracking for this session

---

## Procedure

### Step 1 — VK Context Guard

```
context = get_context()
```

**If `get_context()` is unavailable or returns no `workspace_id`**: not in a VK
environment. Exit silently — this skill is a no-op outside VK.

Extract:
- `workspace_id` — the current workspace
- `issue_id` — if non-null, skip to Step 5 (already linked)
- `workspace_repos[0].repo_id` — for issue project inference
- `workspace_branch` — may contain issue hints (e.g. `vk/572d-some-slug`)

If `issue_id` is null, proceed to Step 2.

### Step 2 — Infer Issue from Context

Before creating a new issue, check if an existing issue should own this workspace.

#### 2a. Check conversation context

Look at what the user has asked or shared:
- Did they mention a specific issue ID? (e.g. "work on HOL-123")
- Did they share a URL, attachment, or topic that matches an existing issue?

#### 2b. Search existing issues

If there's a plausible match from conversation context:

```
list_issues(project_id: "<project>", search: "<keyword from context>")
```

Check the results. If a matching issue exists and is in a compatible state
(Backlog, Todo, In progress), it may be the intended owner.

#### 2c. Check branch name for clues

The workspace branch name sometimes encodes issue context:
- `vk/572d-what-is-papercli` → workspace 572d, slug "what-is-papercli"
- The slug may hint at the topic — use it as a search term

#### Decision gate

- **Match found with high confidence** → proceed to Step 4 (link)
- **Ambiguous match** → ask user: "This workspace has no owning issue. Did you mean [issue X]?"
- **No match** → proceed to Step 3 (create)

### Step 3 — Create Issue

Create a new issue in the appropriate project.

#### Determine project

- If workspace repos suggest holicode → use `holicode-meta` project
- If the topic is personal → use `personal` project
- If unclear → ask user

#### Issue title

Derive from conversation context:
- For research topics: `Research: <Name> — <one-line description>`
- For tasks: `[TASK] <imperative description>`
- For spikes: `[SPIKE] <question to answer>`

Keep titles concise (< 80 chars).

#### Issue description

Minimum viable description — capture the intent:

```markdown
## Context

<What the user asked for / what this workspace is doing>

## Source

<URL, attachment paths, or reference to what triggered this workspace>
```

Do not over-engineer the description. The workspace will produce the real output.

```
create_issue(
  project_id: "<project-id>",
  title: "<title>",
  description: "<description>",
  priority: "medium"
)
```

### Step 4 — Link Workspace to Issue

```
link_workspace_issue(
  workspace_id: "<workspace-id>",
  issue_id: "<issue-id>"
)
```

### Step 5 — Update Workspace Title

The workspace title should be: `<simple_id>: <concise description>`

First, get the issue's simple_id:

```
get_issue(issue_id: "<issue-id>")
→ extract simple_id (e.g. "HOL-497")
```

Then update the workspace name:

```
update_workspace(
  workspace_id: "<workspace-id>",
  name: "<simple_id>: <concise title>"
)
```

Examples:
- `HOL-497: edgequake graph-RAG research`
- `HOL-516: LLM Wiki pattern review`
- `PER-72: vault integration spike`

### Step 6 — Create Relationships (if applicable)

If the issue was just created (Step 3) and there are obviously related issues
visible from context, create relationships:

```
create_issue_relationship(issue_id, related_issue_id, "related")
```

Common relationships to check:
- Parent epic or story (if this is a sub-task)
- Sibling research issues from the same exploration session
- HOL issues referenced in the conversation

### Step 7 — Report

Briefly inform the user:

- **Linked existing**: "Linked this workspace to HOL-497 (EdgeQuake research)"
- **Created + linked**: "Created HOL-503 and linked it to this workspace"
- **Already linked**: (say nothing — no action needed)

---

## Edge Cases

| Situation | Action |
|-----------|--------|
| `get_context()` fails or returns no workspace | Not in a Vibe Kanban context — skip silently |
| Multiple plausible issue matches | Present options to user, ask which one |
| User says "don't track this" | Respect; skip and don't re-trigger in this session |
| Workspace already has a title but no issue | Still create/link the issue; optionally update title to add simple_id prefix |
| Issue exists but is Done/Cancelled | Warn user — linking to a closed issue is unusual. Confirm or create new |

---

## Relationship to Other Skills

- **Invoked by**: `task-init` (Step 7 — delegates to this skill when VK context is present)
- **Called by**: `research-dispatch`, `task-implement`, any skill needing workspace traceability
- **Uses**: `issue-tracker` patterns (project resolution, issue creation conventions)
- **Complements**: `workspace-orchestrate` (orchestrate creates workspace WITH issue; this skill handles the gap when that didn't happen)
- **Complements**: `agentic-env-lifecycle` (lifecycle assumes issue linkage for PR creation)
- **MCP tools**: `get_context`, `link_workspace_issue`, `update_workspace`, `create_issue`, `get_issue`, `list_issues`, `create_issue_relationship`

## Constraints

- Never create duplicate issues — always search first
- Never link to an issue owned by a different active workspace without user confirmation
- Workspace title update is non-destructive (can be changed again later)
- Minimal interruption — if the match is obvious, just do it and report
