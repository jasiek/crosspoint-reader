---
name: hc-orchestrate-story
description: Bridge from a Story to ready Tasks and hand off to task-implement (advisory; no code changes).
context: fork
metadata:
  owner: holicode
  scope: orchestration-story
  origin: workflows/orchestrate-story.md
---

# Story → Implementation Bridge (Generic, Advisory)

## Purpose
Thin, generic connector to navigate from Story chunks to implementation execution. It discovers ready Task chunks for a given Story, proposes execution candidates (respecting coupling metadata if present), or advises to run implementation-plan when tasks are missing. No code generation, no task creation.

## Mode & Boundaries
- Mode: PLAN/ORCHESTRATION (no code generation)
- Guardrails:
  - Do not create/modify src/** code
  - Do not create tasks; only suggest existing ones or advise planning
  - Handoff to `/task-implement.md` for actual implementation

## Definition of Ready (DoR)
- [ ] `.holicode/state/WORK_SPEC.md` exists
- [ ] `.holicode/specs/stories/STORY-{id}.md` exists and is linked from the manifest

## Definition of Done (DoD)
- [ ] Bridge evaluated Story → discovered ready Task chunks (if any)
- [ ] Clear advisory next step:
  - Proposed TASK(s) for execution, or
  - Advisory to run `/implementation-plan.md` for missing tasks
- [ ] User selection captured for TASK(s) to implement
- [ ] Handoff instruction emitted to run `/task-implement.md` with selected TASK(s)

## Discovery
1) Load `.holicode/state/WORK_SPEC.md`
2) Validate that the target Story is listed under “Active Stories”
3) Find `.holicode/specs/tasks/TASK-*.md` linked from the Story and/or manifest
4) For each TASK candidate:
   - Extract status (prefer Status in table: [ready|active])
   - Read optional coupling metadata from template (if present):
     - allowedCoupling: solo | can_combine_with: [TASK-IDs]
     - executionOrder: sequential | parallel (advisory)
   - Prefer TASKs with Status = ready

## Behavior
- If ready TASKs exist:
  - Present list with coupling hints
  - Respect coupling metadata when proposing combinations
  - Ask the user to confirm selected TASK(s)
- If no tasks exist for the Story:
  - Advisory: “Run `/implementation-plan.md` for STORY-{id} to create XS/S TASK chunk(s).”
  - Optionally suggest minimal stub task names based on Story ACs (advisory only)

## Confirmation
Once candidates are identified:

<ask_followup_question>
<question>Select TASK(s) to implement for the Story. Respecting coupling metadata (if present), choose one of:
- Execute a single TASK now
- Execute a compatible set (if allowedCoupling permits)
- Defer and run implementation-plan to create tasks first</question>
<options>["Execute single TASK", "Execute compatible set", "Defer and run implementation-plan"]</options>
</ask_followup_question>

If “Execute single TASK” or “Execute compatible set” is chosen, proceed to Handoff.

## Handoff
- Direct the user to invoke `/task-implement.md` with selected TASK chunks
- Path-agnostic handoff (advice):
  - Provide the paths to chosen `.holicode/specs/tasks/TASK-{id}.md`
  - Remind that `task-implement.md` will resolve live `src/**/SPEC.md` via its discovery precedence
- Example instruction:
```
Next step: run /task-implement.md with:
- taskChunkPath: .holicode/specs/tasks/TASK-XYZ.md
- (optional) invocationPaths/specPaths if you want to direct discovery more strictly
```

## Guardrails & Notes
- No task creation/editing here; this is a navigation bridge
- No code execution; defer all implementation to task-implement
- Maintain KISS and “Generic Workflows, Specific Specifications”
- Coupling metadata is advisory; defer conflicts to user confirmation

## Minimal Operational Steps (for the agent)
1. Parse manifest to confirm Story presence and collect candidate TASK links
2. Parse each TASK file for:
   - Status, Size, linked Story
   - Optional coupling fields if template supports:
     - allowedCoupling: solo | can_combine_with: [TASK-IDs]
     - executionOrder: sequential | parallel
3. Build proposal set:
   - Ready task(s)
   - Compatible combinations (only when allowedCoupling permits)
4. Prompt user to select; then present the exact handoff instruction to task-implement

## Example Output (advisory)
```
Story: STORY-123 (ready)
Discovered ready tasks:
- TASK-201 (Size: S) — allowedCoupling: solo
- TASK-202 (Size: XS) — allowedCoupling: can_combine_with [TASK-203]
- TASK-203 (Size: XS) — allowedCoupling: can_combine_with [TASK-202]

Recommended options:
- Execute TASK-201 alone
- Execute TASK-202 + TASK-203 together (parallel)

Select a path. Then run:
/task-implement.md
Inputs:
- taskChunkPath(s): 
  - .holicode/specs/tasks/TASK-202.md
  - .holicode/specs/tasks/TASK-203.md
```

## Failure Modes
- Story missing in manifest → Advise to update `WORK_SPEC.md` or run `/functional-analyze.md`
- No ready tasks found → Advise running `/implementation-plan.md` for the Story
- Ambiguity in discovery → Ask for explicit Story ID and/or task selection

## Retro Hook (advisory)
After completion, consider logging key insights in `.holicode/state/retro-inbox.md` (e.g., coupling clarity, missing metadata in tasks).
