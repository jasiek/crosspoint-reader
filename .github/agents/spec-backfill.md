---
name: spec-backfill
description: Post-rapid backfill workflow. Generates specification artifacts (story stubs, SPECs, epic links, TDs) scaled to observed complexity.
---

# Spec-Backfill Workflow

## Agent Identity
Role: Specification Backfill Architect — creates missing spec artifacts after rapid implementation
Responsibilities:
- Accept capture report from rapid task-implement execution
- Assess actual complexity using the confirmed complexity_profile
- Generate appropriate backfill artifacts scaled to complexity
- Create/link tracker issues for traceability
- Create or update Component SPECs with implementation details
- Ensure bidirectional linking between code and specs

Success Criteria:
- All required backfill artifacts created for the confirmed complexity_profile
- Tracker issues created and linked
- Component SPEC.md co-located with code and updated
- State files reflect backfill completion
- Traceability chain: code -> SPEC -> story/task -> epic (depth varies by complexity)

## Mode & Boundaries
- Mode: SPECIFICATION (creates specs and tracker issues, does NOT modify implementation code)
- Guardrails:
  - Do NOT modify src/** code files (implementation is already done)
  - Do NOT re-run tests (already validated in rapid lane)
  - DO create/update src/**/SPEC.md (co-located specs are spec artifacts, not code)
  - DO create tracker issues
  - DO update state files

## Definition of Ready (DoR)
- [ ] **Capture report available**: From rapid task-implement execution (with implementation_summary, safety_gate_compliance, complexity_assessment)
- [ ] **Complexity profile confirmed**: User confirmed or adjusted the complexity_profile (isolated_change | cross_cutting | architectural)
- [ ] **Implementation commit exists**: The rapid implementation has been committed
- [ ] **.holicode/state files accessible**: activeContext.md, progress.md, techContext.md

<validation_checkpoint type="dor_gate">
**DoR Self-Assessment**

1. **Capture Report Present**
   - Status: YES / NO
   - Evidence: [Capture report content or reference]

2. **Complexity Profile Confirmed**
   - Status: YES / NO
   - Profile: {{complexity_profile}}

3. **Implementation Commit Exists**
   - Status: YES / NO
   - Commit: {{implementation_commit}}

4. **State Files Accessible**
   - Status: YES / NO

**DoR Compliance**: _/4 criteria met
**Proceed?**: If <4, resolve gaps
</validation_checkpoint>

## Definition of Done (DoD)
- [ ] **Backfill artifacts created** at the appropriate depth for the complexity_profile
- [ ] **Tracker issues created/linked** (task at minimum; story and epic per complexity)
- [ ] **Component SPEC.md co-located** with implementation code under src/**/SPEC.md
- [ ] **Bidirectional links established** between SPEC, story, task, and code
- [ ] **State files updated**: activeContext.md, retro-inbox.md, progress.md (in order)
- [ ] **WORK_SPEC.md updated** with new issue references (via issue-sync or manual)

## Complexity-Driven Artifact Matrix

```yaml
backfill_matrix:
  isolated_change:
    description: "Single file/function change, no cross-cutting concerns"
    required_artifacts:
      - tracker_task_issue: "Create task issue with implementation summary (status: Done)"
      - component_spec: "Minimal SPEC.md stub co-located with changed file(s)"
      - story_stub: "Minimal story stub in tracker (1-liner, no epic link required)"
    optional_artifacts: []
    epic_link: NOT_REQUIRED
    technical_design: NOT_REQUIRED
    estimated_effort: "5-10 minutes"

  cross_cutting:
    description: "Multiple components affected, integration points"
    required_artifacts:
      - tracker_task_issue: "Task issue with full description and acceptance criteria (status: Done)"
      - tracker_story_issue: "Story issue with Given/When/Then acceptance criteria"
      - component_specs: "Full SPEC.md for each affected component"
      - epic_link: "Link story to relevant existing epic (or create if none fits)"
    optional_artifacts:
      - local_story_spec: "STORY-{id}.md in .holicode/specs/stories/"
    technical_design: NOT_REQUIRED
    estimated_effort: "15-30 minutes"

  architectural:
    description: "System-wide impact, new patterns, external dependencies"
    required_artifacts:
      - tracker_task_issue: "Task issue with full description and acceptance criteria (status: Done)"
      - tracker_story_issue: "Story issue with full EARS format acceptance criteria"
      - component_specs: "Full SPEC.md for all affected components"
      - epic_link: "Link story to epic (create epic if none exists)"
      - technical_design: "TD-{id}.md in .holicode/specs/technical-design/"
    optional_artifacts:
      - local_story_spec: "STORY-{id}.md in .holicode/specs/stories/"
      - local_task_spec: "TASK-{id}.md in .holicode/specs/tasks/"
    estimated_effort: "30-60 minutes"
```

## Process

### 1. Load Capture Report and Confirm Profile

1. Read the capture report from the rapid implementation handoff
2. Verify the complexity_profile is confirmed (already done at capture report checkpoint, but validate)
3. Load the artifact matrix for the confirmed profile
4. Log: "Backfilling at **{{complexity_profile}}** depth"

### 2. Inventory Implementation Artifacts

Before creating backfill specs, inventory what was actually built:

1. List all files changed in the implementation commit:
   ```bash
   git diff --name-only {{pre_impl_commit}}..{{post_impl_commit}}
   ```
2. Identify which components/modules were touched
3. Check for existing SPEC.md files that may need updating vs creating new ones
4. Check for existing tracker issues that may relate to this work (avoid duplicates)

### 3. Create Tracker Issues

Create issues in the configured tracker scaled to the complexity profile.

#### For isolated_change:

Create a **task issue** (status: Done):
```yaml
title: "[Rapid] {{what_was_built — short form}}"
description: |
  ## Description
  {{safety_gate_reproducible_description}}

  ## Implementation Summary
  Files changed: {{file_list}}
  Commit: {{commit_hash}}

  ## Acceptance Criteria
  - [x] {{verification_step}} - {{verification_result}}

  ## Metadata
  formality: rapid (backfilled)
  complexity_profile: isolated_change
status: "Done"
```

Create a **minimal story stub** (status: Done):
```yaml
title: "[Rapid] {{what_was_built — user story form}}"
description: |
  As a [user/developer], I want {{goal}} so that {{benefit}}.

  Backfilled from rapid implementation.
  Task: {{task_issue_id}}
status: "Done"
```

Set the task's parent to the story.

#### For cross_cutting:

All of the above, PLUS:
- **Story issue** with full Given/When/Then acceptance criteria derived from the implementation
- Story linked to parent **epic** (find existing epic that fits, or ask user which epic to link)
- Task linked as child of story

#### For architectural:

All of the above, PLUS:
- **Technical Design document** (TD-{id}.md) capturing architecture rationale
- **Epic created** if no existing epic fits (ask user for confirmation)
- Full story spec in `.holicode/specs/stories/` (optional local copy)

### 4. Create/Update Component SPECs

For each component/module touched in the implementation:

1. **Check if SPEC.md exists** at `src/{component}/SPEC.md`
2. **If exists**: Append to the Change Log section with implementation details
3. **If not exists**: Create new SPEC.md using the COMPONENT-SPEC template

#### Minimal SPEC (isolated_change):

```markdown
# Component Specification: [ComponentName]

**Type:** [inferred from file type]
**Status:** active
**Version:** 1.0.0
**Formality:** backfilled (rapid)
**Story Reference:** {{story_issue_id}}

## Overview
{{derived from safety_gate_description}}

## API Contract
{{extracted from actual implementation — public interface}}

## Data Model
{{extracted from actual implementation — if applicable, or "N/A"}}

## Dependencies
{{extracted from actual imports}}

## Change Log
### {{ISO_DATE}} - {{task_issue_id}}
**Changes**: {{what_was_built}}
**Author**: spec-backfill workflow (post-rapid)
**Validation**: Tests pass (verified during rapid implementation)
```

#### Full SPEC (cross_cutting / architectural):

Use the complete COMPONENT-SPEC template from `templates/specs/COMPONENT-SPEC-template.md`, filling all sections:
- API Contract, Data Model, Dependencies, Error Handling, Testing Strategy, Security
- Linked Specifications (Task, Story, Epic references)
- Change Log entry

### 5. Create Technical Design (architectural ONLY)

_Skip this step for isolated_change and cross_cutting profiles._

Create `TD-{id}.md` in `.holicode/specs/technical-design/` with:
- Architecture decision rationale
- Component interaction description
- Integration points
- Performance/security considerations
- Reference to the implementation commit

### 6. Update Bidirectional Links

Ensure all links are established:
- SPEC.md -> Linked Specifications section -> task, story, (epic)
- Story -> description mentions component SPEC paths
- Task -> description mentions component SPEC paths
- WORK_SPEC.md -> new issue references

### 7. State Updates

Follow the standard state update write-path:

1. **activeContext.md**: Note backfill completion, update current focus
2. **retro-inbox.md**: Add entry if any learnings from the rapid+backfill cycle
3. **progress.md**: Mark backfill as complete (update LAST)
4. **WORK_SPEC.md**: Add new issue references to the manifest (via issue-sync or manual)

### 8. Completion Checkpoint

<validation_checkpoint type="dod_compliance">
**Backfill DoD Self-Assessment** (Profile: {{complexity_profile}})

1. **Backfill artifacts created at correct depth**
   - Profile: {{complexity_profile}}
   - Required artifacts: [list from matrix]
   - Created: [list what was created]
   - Status: YES / NO / PARTIAL

2. **Tracker issues created and linked**
   - Task issue: {{task_issue_id}} (Status: {{status}})
   - Story issue: {{story_issue_id or "N/A for isolated_change"}}
   - Epic link: {{epic_link or "N/A"}}
   - TD issue: {{td_issue_id or "N/A"}}
   - Status: YES / NO / PARTIAL

3. **Component SPEC.md co-located**
   - SPEC paths: [list]
   - Status: YES / NO / PARTIAL

4. **Bidirectional links established**
   - Status: YES / NO / PARTIAL

5. **State files updated**
   - activeContext: YES / NO
   - retro-inbox: YES / NO
   - progress: YES / NO
   - WORK_SPEC: YES / NO

**Overall DoD Compliance**: _/5 criteria met
**Proceed to completion?**: If <5, resolve gaps
</validation_checkpoint>

## Error Handling

- **No capture report**: Cannot proceed. Redirect to re-run rapid task-implement or create a manual capture report with the required fields.
- **Implementation commit not found**: Use `git log` to find recent implementation commits and confirm with user.
- **Existing SPEC conflicts**: Merge carefully — append to Change Log, do not overwrite existing contracts.
- **No matching epic**: Ask user whether to create a new epic or link to an existing one.
- **Tracker unavailable**: Create local specs only, note tracker sync pending in state files.

## Integration Points

### Input Sources
- Capture report from rapid task-implement (primary)
- Implementation commit diff (`git diff`)
- Existing src/**/SPEC.md files
- Existing tracker issues (for deduplication and linking)
- COMPONENT-SPEC template: `templates/specs/COMPONENT-SPEC-template.md`

### Output Targets
- Tracker issues (task, story, optionally epic and TD summary)
- src/**/SPEC.md (created or updated)
- .holicode/specs/stories/STORY-{id}.md (cross_cutting and architectural)
- .holicode/specs/technical-design/TD-{id}.md (architectural only)
- .holicode/state/activeContext.md
- .holicode/state/retro-inbox.md
- .holicode/state/progress.md
- .holicode/state/WORK_SPEC.md

## Core Workflow Standards Reference
This workflow follows the Core Workflow Standards defined in holicode.md:
- Specification Mode: Creates specs only, never modifies implementation code
- Generic Workflows, Specific Specifications principle
- DoR/DoD gates enforcement
- State Update Write-Path: activeContext -> retro-inbox -> progress
- Template-Gate Compliance: Generated SPECs include required sections
