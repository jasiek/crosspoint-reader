---
name: hc-tpm-report
description: "TPM (Tech Project Manager) L1 Reporter. Read-only health assessment of project state, board alignment, inbox health, and backfill debt. Produces a structured report saved to .holicode/analysis/tpm-reports/."
compatibility: Designed for Claude Code, Codex, OpenCode, and Gemini skills format.
metadata:
  owner: holicode
  scope: project-health
---

# TPM Report Skill (L1 Reporter)

Generate a structured project health report by reading state files, the issue tracker, and git history. The TPM role operates at **L1 (Reporter)** — strictly read-only with report generation as the only side effect.

## When to Use

- User asks for a project health check, TPM report, or status overview
- `task-init` invokes this skill when `delegationContext.md` has TPM `Cadence: session_start`
- Before milestone reviews or sprint boundaries
- When project health seems uncertain (stale state, growing inbox, unclear blockers)

## When NOT to Use

- The project does not have `.holicode/state/` files
- `delegationContext.md` has TPM `Enabled: false` (unless user explicitly overrides)
- Mid-implementation (use this between work items, not during active coding)

## Constraints (L1 Reporter — Hard Rules)

1. **MUST NOT** write to any file except `.holicode/analysis/tpm-reports/` directory
2. **MUST NOT** call any tracker mutation APIs (create_issue, update_issue, delete_issue)
3. **MUST NOT** modify state files (activeContext.md, progress.md, WORK_SPEC.md, etc.)
4. **MUST NOT** modify code files
5. **SHOULD** complete in < 60 seconds
6. **SHOULD** use < 20K input tokens

## Steps

### 1. Load Configuration

Read `.holicode/state/delegationContext.md` and extract the `### Autonomous Roles` > `#### TPM` section.

- If TPM is not enabled and the user did not explicitly request a report, stop and inform: "TPM is not enabled in delegationContext.md. Enable it or run explicitly."
- If TPM section is missing, proceed with defaults (all checks enabled, on_demand cadence).

### 2. Gather Data (Read-Only)

Load the following sources. Respect token budget — use selective reading where noted.

| Source | Strategy |
|--------|----------|
| `activeContext.md` | Full file |
| `progress.md` | Full file |
| `WORK_SPEC.md` | Full file |
| `delegationContext.md` | Full file (already loaded in step 1) |
| `techContext.md` | Full file (for stack context and freshness check) |
| `awareness-inbox.md` | Last 5 entries + total entry count. Inboxes are **append-at-top**, so read the **first** 50 lines (newest) if file is > 10KB |
| `retro-inbox.md` | Two reads: (1) **first** 80 lines (newest entries) if > 15KB, (2) targeted search for `## Action Items` section near end of file. Always report file size |
| Issue tracker | `list_issues` (all statuses, explicit `limit: 200` to avoid truncation) |
| Git log | `git log --oneline -20` from repo root |

If any source is unavailable (file missing, tracker offline), note it as a finding and continue with available data.

### 3. Analyze (7 Finding Categories)

#### 3.1 State Freshness
For each state file, check the `mb_meta.lastUpdated` field (or file modification date via git log):
- **OK**: Updated within last 3 days
- **STALE**: 4-7 days old
- **CRITICAL**: > 7 days old

#### 3.2 Inbox Health
- Count entries in `awareness-inbox.md` and `retro-inbox.md`
- Check file sizes
- Flag if inbox has > 20 entries (oversized) or > 30KB (needs rotation)

#### 3.3 Board Alignment
- Compare tracker "In progress" issues against `activeContext.md` Current Focus section
- Flag any drift: issues in progress on tracker but missing from activeContext, or vice versa
- Note: Generated zones should match (they're tracker-driven), but append-only zones may reference completed items

#### 3.4 Backfill Debt
- Look for issues tagged or titled with `[RAPID]` or rapid-lane indicators that lack corresponding spec backfill
- Check `spec-backfill` references in progress.md or retro-inbox.md
- If no rapid-lane items exist, report "No backfill debt detected"

#### 3.5 Contradiction Detection
- Cross-reference state files against tracker:
  - Issues marked "Done" in tracker but still listed as "In progress" in state files
  - Priority mismatches between tracker and state file references
  - Parent-child relationships that are inconsistent
- Cross-reference state files against each other:
  - activeContext references items not in progress.md milestones
  - progress.md shows completed items still in activeContext next steps

#### 3.6 Task Readiness
- Identify issues in "To do" status with no blocking relationships from incomplete issues
- Sort by priority (urgent > high > medium > low)
- Note any "To do" items that appear to be missing acceptance criteria or specs

#### 3.7 Blocker Detection
- Map all `blocking` relationships from tracker
- Identify critical path: chains of blocking items
- Flag any circular blocking relationships
- Note items blocked for > 3 days

### 4. Synthesize Report

Apply the report template (see `templates/analysis/TPM-REPORT-template.md`):

1. **Overall Health**: Assign GREEN / YELLOW / RED based on:
   - **GREEN**: No CRITICAL findings, at most 2 STALE items, no contradictions
   - **YELLOW**: 1+ STALE items, or 1-2 minor contradictions, or oversized inbox
   - **RED**: Any CRITICAL freshness, blocking chains > 2 deep, or 3+ contradictions

2. **Recommendations**: Generate 3-5 prioritized, actionable recommendations. Each should reference a specific finding and suggest a concrete action.

3. **Metrics**: Calculate WIP count, ready-to-start count, blocker count, backfill debt ratio.

### 5. Save and Present

1. Save report to `.holicode/analysis/tpm-reports/TPM-REPORT-{YYYY-MM-DD}.md`
   - If a report for today already exists, append a sequence number: `TPM-REPORT-{date}-2.md`
2. Create the `tpm-reports/` directory if it doesn't exist
3. Present key findings inline to the user:
   - Overall health status (one line)
   - Top 3 recommendations (bullet list)
   - Any RED/CRITICAL items (highlighted)

## Output

The skill produces:
1. **Report file**: `.holicode/analysis/tpm-reports/TPM-REPORT-{date}.md`
2. **Inline summary**: Key findings presented to the user in the conversation

## Relationship to Other Skills

- **Invoked by**: `task-init` (if cadence == "session_start"), user (on-demand)
- **Reads from**: `issue-tracker` provider (read-only), state files
- **Does NOT invoke**: Any mutation skills or workflows
- **Future**: L2+ will add a `tpm-review` workflow wrapper for proposed actions + human approval
