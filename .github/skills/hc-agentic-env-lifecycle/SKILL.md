---
name: hc-agentic-env-lifecycle
description: Workspace session lifecycle for Coder+Vibe Kanban cloud environments. Guides agents through push → PR → merge → new workspace handoff.
compatibility: Designed for Claude Code, Codex, OpenCode, and Gemini skills format.
metadata:
  owner: holicode
  scope: workspace-lifecycle-orchestration
  uses: agent-session-protocol
---

# Agentic Environment Lifecycle

This skill documents and guides the workspace session lifecycle in a Coder + Vibe Kanban cloud development environment. It is **optional** — only relevant when the runtime environment provides Coder workspaces and Vibe Kanban MCP tools.

> **Abstraction boundary**: The **abstract conventions** this skill enforces (feature branch lifecycle, session end protocol, PR discipline, review output contract) are defined in `holicode.md § Agentic Git Workflow Conventions` and apply regardless of provider. This skill is the **Coder + Vibe Kanban implementation** of those conventions. Other environments (GitHub Codespaces, Gitpod, local worktrees, etc.) need equivalent lifecycle skills that enforce the same abstract rules with their own tooling.

## When This Skill Applies

- You are running inside a Coder workspace (check: `/tmp/coder-agent` exists or `CODER_AGENT_TOKEN` is set)
- Vibe Kanban MCP tools are available (`mcp__vibe_kanban__*`)
- The project uses git with a remote GitHub repository

## Workspace Session Lifecycle

A workspace session follows this dependency chain. Each step requires the previous one to complete.

```
1. commit (local)
   → 2. push (requires remote access)
      → 3. PR create (requires gh auth)
         → 4. PR review (human gate)
            → 4.5 code review (optional, cross-agent via code-review skill)
               → 5. PR merge (human or auto-merge)
                  → 6. new workspace (requires merged base branch)
```

### Feature Branch Lifecycle

For multi-task epics/stories, task branches merge into a feature branch, which then merges into main:

```
main ─────────────────────────────────────────── merge commit ←─┐
  └── feature/<slug> ── squash ←─ task-1 PR                    │
                      ── squash ←─ task-2 PR                    │
                      ── squash ←─ task-3 PR ── roll-up PR ─────┘
```

- Task PRs target `feature/<slug>` (squash merge)
- A single roll-up PR merges `feature/<slug>` into `main` (merge commit)

### Step 1: Commit

Use the `git-commit-manager` workflow or commit directly with conventional format.

```bash
git add <specific-files>
git commit -m "type(scope): description"
```

### Step 2: Push

Push the workspace branch to the remote.

```bash
git push -u origin <branch-name>
```

**Pre-flight Validation:**
- Verify no uncommitted changes: `git status` (should be clean)
- Verify branch name is valid: `git branch --show-current`
- For feature branches, verify they exist on remote: `git ls-remote --heads origin feature/<slug>`
- Verify auth is available: `ssh -T git@github.com` should succeed (not prompt)

Branch naming is provider-specific — Vibe Kanban uses `vk/<workspace-short-id>-<slug>`; other providers use their own conventions (e.g., `feat/TASK-id` for manual worktrees).

**Error Recovery:**
- **Push fails (auth/network)**: Check auth with `git ls-remote origin HEAD`. If timeout, retry after 30s or flag for manual push.
- **Push rejected (non-fast-forward)**: `git fetch origin <branch>` to pull remote, then rebase: `git rebase origin/<branch>` and retry with `git push --force-with-lease`.
- **Branch doesn't exist on remote yet**: Normal on first push with `-u` flag. If push hangs, verify network; try `git push -u origin <branch> --dry-run` first.

### Step 3: PR Create

**Pre-flight Validation Checklist:**
- [ ] `gh auth status` returns authenticated (not "not logged in")
- [ ] Target branch exists on remote: `git ls-remote --heads origin <target-branch>`
- [ ] Current branch is pushed: `git ls-remote --heads origin <current-branch>`
- [ ] No local commits exist that aren't pushed: `git log origin/<current-branch>..HEAD` (should be empty)

**Prerequisite**: `gh` CLI must be authenticated. Check with `gh auth status`.

If `gh` is not authenticated:
- Try: `coder external-auth access-token github | gh auth login --with-token` (if Coder external auth is configured)
- Or: set `GITHUB_TOKEN` env var from a workspace secret
- Or: flag to the user that PR must be created manually

**Determine PR target branch:**
- If a **feature branch** is active for the parent epic/story (e.g., `feature/<slug>`), the PR MUST target the feature branch — not `main`.
- If no feature branch exists (standalone task), the PR targets the default integration branch (usually `main`).
- Never target another task's workspace branch directly.

**PR title format**: `type(scope): HOL-XX description` — the tracker issue ID MUST appear in the title.

When `gh` is available:
```bash
gh pr create --base <target-branch> --title "type(scope): HOL-XX description" --body "<description>"
```

Use the `github-pr-create` workflow for template-based PR creation.

**Error Recovery:**
- **`gh auth status` fails**: Try `gh auth login` or `gh auth logout && gh auth login` to re-authenticate. If Coder external auth is configured: `coder external-auth access-token github | gh auth login --with-token`
- **Target branch doesn't exist**: Verify branch name is correct. If merging a task to a feature branch, ensure feature branch was created first (may need to create it manually: `git push origin HEAD:refs/heads/feature/<slug>`)
- **PR creation fails (e.g., branch not on remote)**: Verify push completed: `git push -u origin <branch>` and retry `gh pr create`
- **PR already exists for this branch**: Check: `gh pr view` or `gh pr list --head <branch>`. If PR exists, use `gh pr edit` to update description rather than creating a duplicate.

**Manual fallback**: If gh is not available, output the PR creation URL:
```
https://github.com/<org>/<repo>/pull/new/<branch-name>
```

### Step 4: PR Review (Human Gate)

This step is human-controlled. The agent should:
1. Note that PR review is needed before proceeding
2. Optionally check PR status: `gh pr view --json state,reviews,statusCheckRollup`
3. Wait for user instruction before proceeding

### Step 4.5: Cross-Agent Code Review (Optional)

Before merging, optionally dispatch an independent code review to a different AI executor using the `code-review` skill. This adds a structured second opinion from a fresh context.

**When to trigger:**
- High-priority or high-risk changes (security, data integrity, public API)
- Complex multi-file diffs where a fresh perspective adds confidence
- User explicitly requests cross-agent review
- New patterns or architectural decisions that benefit from validation

**When to skip:**
- Trivial changes (config, docs, single-line fixes)
- CI already covers the primary concerns (lint, type-check, tests pass)
- Human reviewer has already approved

**How to dispatch:**
Use the `code-review` skill, which handles executor selection, context bootstrapping, and the findings-only output contract. Review findings are posted as a PR comment or returned directly from the review workspace.

**After review completes:**
- Triage findings by severity (Critical/High must be addressed before merge)
- Implement fixes in the original workspace (not the reviewer session)
- Proceed to Step 5 when all Critical/High findings are resolved

### Step 5: PR Merge

Typically human-initiated. If the agent has permission, use the merge strategy matching the PR type (see `holicode.md § PR Discipline`).

**Pre-flight Validation:**
- [ ] Verify PR is open: `gh pr view <pr-number> --json state`
- [ ] Verify CI checks pass: `gh pr view <pr-number> --json statusCheckRollup`
- [ ] Verify no unresolved reviews: `gh pr view <pr-number> --json reviews` (approved or no reviews)
- [ ] Verify base branch is not deleted or stale

**Merge Strategy (Enforce):**

The merge strategy depends on the PR's purpose. **Always enforce the correct strategy** — using the wrong merge type creates confusing git history and violates the feature branch discipline.

- **Task PR → feature branch**: ALWAYS **squash merge** (collapses task commits into one)
  ```bash
  gh pr merge --squash --delete-branch
  ```
  *Rationale*: Task branches are implementation details. Squashing makes feature branch history clean and traceable.

- **Feature branch roll-up PR → main**: ALWAYS **merge commit** (preserves task history)
  ```bash
  gh pr merge --no-squash --delete-branch
  ```
  *Rationale*: Main branch history must show all tasks and their relationships. Merge commits preserve the feature branch as a logical unit.

**Never deviate**: Using `--squash` on a roll-up PR or `--merge` on a task PR breaks the history structure and makes future debugging harder.

**Error Recovery:**
- **Merge blocked by CI failures**: Don't force merge. Identify CI failure, fix in a new commit, push to same branch, and re-run CI. Then retry merge.
- **Merge blocked by conflicts**: See **Conflict Resolution** section below. Resolve conflicts locally, push to branch, and retry merge.
- **Merge fails with "Branch out of date"**: Fetch latest: `git fetch origin`. If feature branch, rebase: `git rebase origin/<target-branch>`. If main branch, same process.
- **Branch cannot be deleted after merge**: Check for required status checks or branch protection rules. May need to manually delete: `git push origin --delete <branch>`

### Step 6: New Workspace

After the PR is merged into the base branch, a new workspace can be started for the next issue.

> **Vibe Kanban implementation** (other providers use equivalent dispatch):

```
mcp__vibe_kanban__start_workspace_session(
  title: "<issue title>",
  executor: "CLAUDE_CODE",  # or OPENCODE, GEMINI, etc.
  repos: [{repo_id: "<repo-id>", base_branch: "<merged-base-branch>"}],
  issue_id: "<next-issue-id>"
)
```

**Important**: The new workspace branches from the base branch. If the base branch does not include the merged changes yet, the new workspace will be missing them. Always verify the merge is complete before starting a new workspace.

## Conflict Resolution Strategy

When merge conflicts arise, follow this protocol to resolve safely and consistently.

### Task PR Conflicts with Feature Branch

**Scenario**: A task PR to `feature/<slug>` has conflicts with the latest state of the feature branch.

**Resolution steps:**
1. Fetch latest: `git fetch origin feature/<slug>`
2. Rebase task branch onto feature branch: `git rebase origin/feature/<slug>`
3. Resolve conflicts in your editor (Git will mark conflict regions with `<<<<<<<`, `=======`, `>>>>>>>`)
4. Stage resolved files: `git add <resolved-files>`
5. Continue rebase: `git rebase --continue`
6. Verify resolution: `git diff origin/feature/<slug>..HEAD` (should show only your changes)
7. Force-push with safety: `git push --force-with-lease origin <task-branch>`
8. Retry PR merge in GitHub UI

**Important**: Always use `--force-with-lease` (safe) instead of `--force`. This prevents overwriting remote work you don't know about.

### Feature Branch Conflicts with Main

**Scenario**: A feature branch roll-up PR to `main` has conflicts because `main` has new commits.

**Resolution steps:**
1. Fetch latest: `git fetch origin main`
2. Rebase feature branch onto main: `git rebase origin/main`
3. Resolve conflicts in your editor
4. Stage resolved files: `git add <resolved-files>`
5. Continue rebase: `git rebase --continue`
6. Force-push with safety: `git push --force-with-lease origin feature/<slug>`
7. Update PR description with **Conflict Resolution** note: "Rebased onto latest main. Conflicts resolved: [list files]. Verify full diff before merge."
8. Retry PR merge in GitHub UI

**Important**: After resolving conflicts, always verify the full diff is correct before merging. Use `gh pr view --json body` and `gh pr diff` to inspect.

### Never Auto-Merge on Conflicts

**Rule**: Do NOT use `--no-edit` or auto-merge when conflicts exist. Always:
- Manually verify conflict resolution in a text editor
- Review the full diff in the PR before merging
- Test locally if possible (run tests, build, etc.)

Conflicts indicate concurrent changes that need human judgment. Automating around them creates silent bugs.

### Conflict Resolution Checklist

Before committing a conflict resolution:
- [ ] All conflict markers (`<<<<<<<`, `=======`, `>>>>>>>`) are removed
- [ ] Logic is correct (not just arbitrarily choosing one side)
- [ ] Tests still pass: `npm run test`
- [ ] Build still succeeds: `npm run build`
- [ ] Diff makes sense: `git diff origin/<base>..HEAD` reviewed
- [ ] PR description updated with conflict notes (for team visibility)

## Tracker State ↔ Git State Consistency

Maintain alignment between issue tracker status and actual git/PR state to prevent silent mismatches.

### State Validation Rules

| Tracker Status | Expected Git State | Validation Command |
|---|---|---|
| **To do** | Branch should NOT exist on remote | `git ls-remote --heads origin <branch>` — should return empty |
| **In progress** | Task branch exists on remote; no PR yet | `gh pr list --head <branch>` — should be empty |
| **In review** | PR exists, not yet merged | `gh pr view <pr-id> --json state` — should be `OPEN` |
| **QA** | PR merged; branch deleted or archived | `gh pr view <pr-id> --json state` — should be `MERGED`; branch not on remote |
| **Done** | PR merged; no pending work | Same as QA; issue should have all acceptance criteria met |

### When to Check

- **Before dispatch**: Verify issue is "To do" and no orphan branch exists
- **After push**: Verify issue is (or will be moved to) "In progress"
- **After PR creation**: Verify issue is (or will be moved to) "In review"
- **After merge**: Verify issue advances to "QA" (not directly to "Done" unless QA is implicit)

### Common Mismatches & Recovery

| Mismatch | Symptom | Fix |
|---|---|---|
| PR merged but tracker still "In review" | Issue seems unresolved | Manually advance tracker to "QA" or "Done" |
| Branch exists but issue is "To do" | Orphan workspace | Delete branch: `git push origin --delete <branch>` |
| Issue "In progress" but no remote branch | Lost work | Create new workspace on base branch; issue stays "In progress" |
| Multiple PRs for one issue | Duplicate work | Close duplicate PRs; keep only the latest |

## Session End Protocol

These steps implement the abstract session-end rules from `holicode.md § PR Discipline`. Before ending a workspace session, the agent MUST:

1. **Commit**: Ensure all changes are committed (no dirty working tree)
2. **Push**: Push the branch to remote
3. **PR create (mandatory)**: Create a PR before ending the session. This is not optional — do not leave work on an unpushed or un-PR'd branch.
   - Target the feature branch if one is active; otherwise target `main`
   - Use title format: `type(scope): ISSUE-ID description`
   - If `gh` is unavailable, output the manual PR URL and flag it clearly
4. **Issue status**: Set the linked issue to **"In review"** — NOT "Done". The issue is "In review" because a PR exists and awaits human code review. Done requires PR merge + QA validation. (Vibe Kanban: via MCP update; other trackers: via their respective APIs)
5. **Handoff summary**: Provide complete handoff information (see **Handoff Completeness Validation** below)

### A2A Completion Callback (optional)

If this session was started via an A2A `dispatch` call (detected by `task-init` Step 0 per `agent-session-protocol`), send a completion callback **after** completing all steps above (commit → push → PR → issue status update).

**How to detect**: `task-init` parsed any A2A front matter at session start (or from a later `run_session_prompt`) and stored `callback_session_id`, original `invocation_id`, and original `max_hops` in working context.

**Skip the callback if**:
- `callback_session_id` is absent, OR
- `callback_session_id` equals your own session ID (self-loop guard), OR
- Original `max_hops <= 0` (no further A2A emissions allowed in this chain)

**How to send the callback**:

```python
ctx = mcp__vibe_kanban__get_context()
my_session_id = ctx.session_id

# New unique invocation_id per agent-session-protocol — never reuse the original
new_invocation_id = f"{datetime.now().isoformat()}Z-{random_4_chars}"

# Decrement max_hops by 1 (per agent-session-protocol Loop Control)
new_max_hops = original_max_hops - 1

mcp__vibe_kanban__run_session_prompt(
    session_id = original_callback_session_id,
    prompt = f"""---
a2a_version: "1.0"
invocation_id: "{new_invocation_id}"
in_response_to: "{original_invocation_id}"
from_session_id: "{my_session_id}"
intent: callback
status: complete
max_hops: {new_max_hops}
---

## Completion Report

**Issue**: [ISSUE-ID] — [issue title]
**PR**: [PR URL]
**Status**: [Ready to merge / Awaiting review]
**Branch**: [branch name]

**Modified Files**:
[list from git diff --name-only]

**Test Coverage**: [passed/failed count]
**CI Status**: [green / failures with details]

**Notes**: [any blocking issues, known gaps, or dependencies]
"""
)
```

---

### Handoff Completeness Validation

A session ends with a structured handoff. Before concluding, verify:

**Handoff Checklist:**
- [ ] PR URL is clearly stated (e.g., `https://github.com/org/repo/pull/123`)
- [ ] Merge status documented (e.g., "Ready to merge" or "Awaiting review")
- [ ] List of modified files included (for code review context): `git diff origin/<base>..HEAD --name-only`
- [ ] Tracker issue is linked to PR (check PR description)
- [ ] Tracker issue status is set to "In review" (via MCP or tracker API)
- [ ] Any blocking issues or dependencies documented (e.g., "Blocked by HOL-49")
- [ ] Test coverage summary provided:
  - Test pass count: `npm run test 2>&1 | grep -E "passed|failed"`
  - Failure count (if any): enumerate specific test failures
- [ ] Build/CI status confirmed:
  - All checks green: `gh pr view --json statusCheckRollup`
  - Or explicit blockers: list CI failures and remediation steps

**Handoff Format Example:**

```
## Session End Handoff

**PR**: https://github.com/org/repo/pull/123
**Status**: Ready to merge (all CI checks passing)
**Modified Files**:
- src/components/Button.tsx
- src/styles/button.css

**Tracker**: Linked issue HOL-42 set to "In review"
**Dependencies**: None blocking

**Test Coverage**:
- Passed: 24/24
- No failures

**CI Status**: All checks green (lint, type-check, build, test)

**Notes**: Implementation complete. No outstanding issues. Ready for human review and merge.
```

## Review Session Output Contract

When an agent session is performing **code review** (not implementation):

- Output is **findings-only**: comments, observations, requested changes
- The review agent MUST NOT apply inline fixes to the code
- Fixes are handled in a **separate follow-up workspace** dispatched after the review
- The review output should be structured as actionable findings that can be converted to sub-tasks

### Post-Merge Status Transition

After a PR is merged (typically by a human), the linked issue should move from "In review" to **"QA"** (not directly to "Done"). QA means the code is merged but awaits validation — deploy check, E2E test, or acceptance review. "Done" requires explicit QA sign-off.

This can happen in two ways:

- **Human-initiated**: The reviewer merges the PR and advances the tracker issue to "QA", then to "Done" after validation
- **Agent-detected**: On the next `task-init` session start, the agent detects issues stuck in "In review" whose PRs have been merged, and recommends advancing to QA or Done

See `holicode.md` → "Issue Lifecycle & Status Flow" for the full abstract process and tracker-specific mapping notes.

**Important**: Agents MUST NOT mark an issue "Done" at session end. The correct terminal state for an agent session is "In review" with a PR open.

## Validation & Safety Reference

This section consolidates pre-flight validation patterns used throughout the lifecycle.

### Branch Existence Verification

Before any operation that references a branch, verify it exists:

```bash
# Verify local branch exists
git branch --list | grep <branch-name>

# Verify remote branch exists
git ls-remote --heads origin <branch-name>

# Verify remote branch points to a valid commit
git ls-remote origin <branch-name> | awk '{print $1}' | git cat-file -t --stdin
```

**When to check:**
- Before `git push` (if pushing to a new branch for the first time, verify auth; subsequent pushes are safe)
- Before PR creation (verify base branch exists)
- Before new workspace dispatch (verify base branch is on remote and fully merged)

### Authentication State Checkpoints

Verify authentication before critical operations:

```bash
# Check git auth (SSH)
ssh -T git@github.com

# Check gh CLI auth
gh auth status

# If auth fails, re-authenticate
gh auth login --with-token
# (Or via Coder: coder external-auth access-token github | gh auth login --with-token)
```

**When to check:**
- Session start (task-init should recommend `gh auth status`)
- Before PR creation (mandatory)
- Before PR merge (if agent has merge permissions)

### Session Resumption & Idempotency

If a workspace session is interrupted (e.g., network failure, agent crash), it can be safely resumed using these idempotent patterns.

**Idempotent Operations:**

| Operation | Why Idempotent | Resume Pattern |
|---|---|---|
| `git push -u` | Safe to retry; verifies remote state before pushing | Retry `git push -u origin <branch>` — Git will skip if already pushed |
| `git commit` (with new message) | Creates a new commit; not idempotent | Check `git log` for the commit; if present, skip rerun |
| `gh pr create` | Not idempotent (creates duplicate if run twice) | Check `gh pr list --head <branch>` before retrying |
| `gh pr merge --squash` | Not idempotent (closes PR if run twice) | Check `gh pr view --json state` before retrying |

**State Preservation & Recovery:**

1. **If push is interrupted**: Verify result before retrying.
   ```bash
   git ls-remote --heads origin <branch>  # Check if pushed
   if [ $? -eq 0 ]; then
     echo "Branch is on remote, proceeding to next step"
   else
     echo "Push did not complete, retrying..."
     git push -u origin <branch>
   fi
   ```

2. **If PR creation is interrupted**: Check for existing PR.
   ```bash
   gh pr list --head <branch> --json number
   # If output is empty, PR was not created; create it now
   # If output has an ID, PR exists; proceed to review
   ```

3. **If a merge attempt is interrupted**: Verify merge result.
   ```bash
   gh pr view <pr-number> --json state
   # If state is MERGED, merge succeeded; delete branch and move on
   # If state is OPEN, merge did not happen; retry after addressing blockers
   ```

**Session Resume Pattern:**

If a workspace session must be resumed (e.g., in a new workspace on the same branch):

1. **Read the tracker issue** to restore context: `gh issue view <issue-id>`
2. **Fetch latest branch state**: `git fetch origin <branch>`
3. **Check git log** to see what commits were already made: `git log <base-branch>..HEAD`
4. **Determine where to resume**:
   - If commits are local only (not pushed), push them: `git push -u origin <branch>`
   - If PR already exists, resume review step (await approval)
   - If PR merged, dispatch new workspace for next issue
5. **Continue from the next logical step** in the lifecycle

**Important**: Resumption works because the branch state is durable (on remote) and the tracker issue is the source of truth. A new workspace can continue where the previous one left off.

### Parallel Dispatch Safety Validation

When dispatching multiple sub-tasks in parallel, verify they are truly file-independent:

```bash
# Compare file sets between two branches
git diff <base> <branch-1> --name-only > /tmp/files-1.txt
git diff <base> <branch-2> --name-only > /tmp/files-2.txt
comm -12 <(sort /tmp/files-1.txt) <(sort /tmp/files-2.txt)  # Intersection — should be EMPTY
```

If the intersection is empty, tasks are file-independent and can be dispatched in parallel.

If the intersection is non-empty, tasks modify overlapping files and MUST be dispatched sequentially (or coordinated).

**When to check:**
- Before dispatching 2+ sub-tasks in parallel
- Document the file-independence rationale in the epic or task descriptions

## Dependencies and Gaps

> Capabilities marked **(VK)** are Vibe Kanban / Coder specific. Other providers need equivalents.

| Capability | Status | Notes |
|-----------|--------|-------|
| Git commit | Available | Via git or git-commit-manager workflow (provider-agnostic) |
| Git push | Available | Standard git, credentials via Coder GIT_ASKPASS **(VK)** |
| gh CLI auth | Gap | Coder external auth doesn't auto-configure gh CLI **(VK)** |
| PR create | Partial | Works when gh is authenticated; manual fallback needed (provider-agnostic) |
| PR merge | Human gate | Agent should not auto-merge without explicit permission (provider-agnostic) |
| New workspace | Available | Via `mcp__vibe_kanban__start_workspace_session` **(VK)** |

## Scope Boundaries

- This skill handles workspace lifecycle orchestration only
- Issue tracking operations are handled by `issue-tracker` skill
- Code implementation is handled by `task-implement` workflow
- This skill does NOT modify holicode core — it is an optional extension
