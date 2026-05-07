---
name: spec-workflow
description: Orchestrate the full spec-driven workflow (business → functional → technical design → implementation plan → implement) with gates and handoffs.
---

# Spec Orchestrator Workflow

## Agent Identity
Role: Orchestrator for 5-phase spec-driven flow (Business → Functional → Technical Design → Implementation Plan → Implementation)
Responsibilities:
- Validate DoR/DoD at each phase
- Spin a new task for each specialist workflow (human-supervised execution)
- Generate/ensure required artifacts and handoffs
Success Criteria:
- End-to-end artifacts created for selected feature
- All gates passed; tasks and component specs co-located with code
What/Why/How Focus:
- Coordinates Why (Business), What (Functional), How-Architecture (Technical Design), What-Deliverables (Implementation Plan), How-Code (Implementation)

## Inputs
- Business brief (short description provided at invocation)
- Feature slug (defaults to date-based)
- Runtime choice (PoC: Option A Node + ts-node + vitest)
- Formality level (optional: `rapid` | `standard` | `thorough` — see Graduated Formality Awareness)

## DoR/DoD Integration
Definition of Ready:
- .holicode/ state exists or state-init executed
- ActiveContext states feature under work OR brief is provided
- Templates available (state, specs, component SPEC.md)
Definition of Done:
- Feature directory created with FEATURE_SPEC.md, USER_STORIES.md, TASKS.md
- Implementation tasks (at least 2 XS/S) executed with tests
- Component-level SPEC.md updated with change log
- QA report (lightweight) added
Quality Gates:
- Presence checks and regex validations for required sections
- Minimal unit tests exist and pass (for PoC demo)

## Memory Bank Integration
Required Files (read):
- .holicode/state/activeContext.md
- .holicode/state/productContext.md (if exists)
- templates/state/*.md
- templates/handoff/handoff-template.md
Updates (write):
- .holicode/state/productContext.md (Business phase)
- .holicode/specs/features/<feature-slug>/* (Functional phase)
- .holicode/handoff/active/* (between phases)
- src/**/SPEC.md (Implementation)

## Graduated Formality Awareness

This orchestrator respects the formality level determined by `intake-triage` (or supplied directly). The formality level controls which phases are mandatory:

| Formality   | Complexity | Phases Executed                                                              |
|-------------|------------|-----------------------------------------------------------------------------|
| **Rapid**   | 0-1        | Skip all spec phases → direct handoff to `task-implement` (rapid mode)      |
| **Standard**| 2-3        | Functional Analyze → Implementation Plan → Task Implement                   |
| **Thorough**| 4-5        | Business Analyze → Functional Analyze → Technical Design → Implementation Plan → Task Implement |

### Formality Resolution
1. **Explicit parameter**: If `formality_level` is provided at invocation, use it directly
2. **Intake-triage output**: If routed from intake-triage, use the `process_depth` from triage result
3. **Default**: If neither is available, infer from brief complexity — default to `standard`

### Phase Skip Rules
- **Rapid**: Skip to step 9 (Implementation Handoff) with `formality_level: rapid` passed to `task-implement`. No planning artifacts are expected — `task-implement` rapid mode accepts a brief directly and enforces 3 non-negotiables (reproducible description, verification step, rollback note). Mandatory backfill via `spec-backfill` after implementation.
- **Standard**: Start at step 4 (Phase B: Functional Analyze). Skip Business Analyze if productContext already exists and is sufficient. Skip Technical Design (step 6) unless architecture decisions are needed.
- **Thorough**: Execute all phases (steps 1-11). All gates enforced. Technical Design (step 6) is included when architecture/techstack/security or other non-functional matters require explicit design.

## Process Steps
1) DoR Validation
- If .holicode/ missing state, instruct to run:
  - scripts/update.sh <repo-path>
  - Run workflows/state-init.md
- Validate that we have a brief or activeContext entry

2) Phase A: Business Analyze
- Create handoff for Business phase
- Create **Workflow-Based Task**: `/business-analyze.md` with the brief
- On completion, ensure .holicode/state/productContext.md updated

3) Gate G1: Business DoD Check
- Validate productContext required sections (WHY, stakeholders, metrics)
- If fail, update handoff with remediation and stop

4) Phase B: Functional Analyze
- Prepare feature folder: .holicode/specs/features/YYYY-MM-DD-task-tracker-create-task
- Create **Workflow-Based Task**: `/functional-analyze.md` to produce:
  - FEATURE_SPEC.md
  - USER_STORIES.md (EARS)
  - nfr-requirements.md
- Optional: api-contracts.md (skipped for runtime A)

5) Gate G2: Functional DoD Check
- Validate USER_STORIES.md has at least one EARS story with Given/When/Then
- Validate TASKS.md draft present or to be created in next step
- If fail, update handoff and stop

6) Phase C: Technical Design (Optional)
- **When to include**: Architecture decisions, tech-stack selection, security design, or other non-functional concerns require explicit design before implementation planning
- **When to skip**: Feature is well-contained within existing architecture and patterns — proceed directly to Phase D
- Create **Workflow-Based Task**: `/technical-design.md` to produce:
  - TD document(s) in `.holicode/specs/technical-design/TD-*.md`
  - TD summary issue in tracker
- On completion, ensure architectural decisions are recorded and referenced

### Gate G2.5: Technical Design DoD Check (if Phase C executed)
- Validate TD document exists with architecture decisions and component boundaries
- Validate NFR considerations addressed (performance, security, scalability as applicable)
- If fail, update handoff with remediation and stop

7) Phase D: Implementation Plan
- Create **Workflow-Based Task**: `/implementation-plan.md` to produce TASKS.md with XS/S tasks and acceptance criteria
- Ensure co-located component SPEC.md requirements included

8) Gate G3: Implementation DoR Check
- Confirm tasks sized XS/S
- Confirm component/service SPEC.md templates exist for modules

### Gate G3.5: Technical Review Gate
- **Execute Post-Planning Review**: Run `/tech-review-post-planning.md`
- **Review Outcomes**:
  - GREEN → Proceed to implementation
  - YELLOW → Address conditions, then proceed
  - RED → Return to technical design phase
- **Document Accepted Risks**: Update retro-inbox.md with any accepted risks
- **Update activeContext.md**: Note review completion and any conditions

9) **CRITICAL: Implementation Handoff & Workflow Orchestration**
- **REQUIRED**: Orchestrate transition to implementation execution
- **Orchestrated Workflow Transition**: Prompt user to create **Workflow-Based Task** for `/task-implement.md`
- **Formality-Aware Handoff**:
  - **Standard/Thorough path**: Pass planning outputs — TASKS.md from `.holicode/specs/tasks/`, Component SPECs from `src/**/SPEC.md`, configuration files
  - **Rapid path**: Pass the original brief/description directly — no planning artifacts are expected. Include `formality_level: rapid` so `task-implement` applies the rapid safety gate instead of full DoR
- **State Update**: Update `.holicode/state/activeContext.md` to reflect transition to implementation phase
- **Human Orchestration**: Workflow transitions require user to manually create new task (see **Workflow-Based Task** in holicode.md)

### Implementation Workflow Execution
**Prompt User**: "Ready for implementation. Please create a **Workflow-Based Task** for `/task-implement.md` with the planning context."
**Responsibilities**:
- Automated tech stack setup (package.json, vitest.config.ts, npm install)
- Source code generation in src/ based on TASKS.md and scaffolds (or brief in rapid mode)
- Live SPEC.md creation co-located with code
- Unit test execution and validation
- State file updates (activeContext.md, retro-inbox.md, progress.md)

### CRITICAL SUCCESS FACTOR
The workflow chain MUST complete end-to-end from planning through implementation. **User must execute implementation phase to complete PoC and meet success criteria.**

10) **Gate G4: Implementation DoD Check (CRITICAL VALIDATION)**
- [ ] **vitest tests pass** (>90% success rate target)
- [ ] **Live SPEC.md updated** with change log entry
- [ ] **TASKS.md acceptance criteria satisfied** for all executed tasks
- [ ] **All required source files created** in src/ structure
- [ ] **State files reflect completed work** (progress.md, activeContext.md)
- [ ] **Working code implementation** exists (not empty codebase)

### Implementation Failure Recovery
If Gate G4 fails:
1. **Diagnose root cause** (missing files, test failures, incomplete implementation)
2. **Re-execute task-implement.md** with corrections
3. **Update state files** to reflect remediation efforts
4. **Validate complete end-to-end success** before proceeding

11) **Finalization & Quality Assurance**
- **Generate QA Report**: Create `.holicode/analysis/reports/qa-poc-[feature-name].md`
- **Archive Handoffs**: Move completed handoffs to `.holicode/handoff/archive/`
- **Update Active Context**: Reflect completion status and next steps
- **Preserve Provenance**: Maintain source references and traceability
- **Validate End-to-End Success**: Confirm working code exists from business brief to implementation

### Success Validation Checklist
- [ ] Business context properly analyzed and documented
- [ ] Functional specifications created with EARS format user stories
- [ ] Implementation tasks defined and executed
- [ ] **Working code generated** (not empty files)
- [ ] Tests passing with documented results
- [ ] State files accurately reflect project progress
- [ ] Complete workflow chain executed without manual intervention gaps

## Next-Step Suggestion

After each workflow completion, assess what comes next:

1. **Read Current Context**: Check `.holicode/state/WORK_SPEC.md` and `.holicode/state/activeContext.md`
2. **Propose Next Workflow**: Based on current state, suggest the most likely next step:
   - From Business Analysis → Functional Analysis (if feature chunks exist but no stories)
   - From Functional Analysis → Technical Design (if stories exist but no technical design)
   - From Technical Design → Implementation Planning (if design exists but no tasks)
   - From Implementation Planning → Task Implementation (if tasks exist but no code)
   - From Implementation Planning → Orchestrate Story (if tasks exist but selection/sequence is needed)
   - From Task Implementation → Quality Validation (if code exists but not validated)
3. **Provide Rationale**: One-line explanation of why this is the logical next step
4. **Clear Call-to-Action**: Simple instruction for user to proceed

## Optional State Sync

When a workflow successfully completes major work:

1. **Advisory State Updates**: If `.holicode/state/activeContext.md` or `.holicode/state/progress.md` are already being touched by the workflow, suggest updating:
   - Current focus and completed milestones
   - Next steps and any blockers identified
   - Set lastUpdated to current ISO date if field exists
2. **Scope Control**: Only suggest state updates when files are already part of the workflow's normal output (avoid scope creep)
3. **Keep Lightweight**: Simple status updates, not comprehensive state management

## Tool Usage Patterns
- File operations: create/update markdown and source files
- Test runner: npm scripts with vitest
- Gate checks: simple regex validations

## Human Interaction Points
- Orchestrator prompts to run state-init if absent
- Mid-phase validations: confirm outputs and continue
- Final approval: confirm PoC artifacts and test results

## Error Handling
- On gate failure: write remediation notes to handoff and stop
- On test failure: suggest rerun after fixing; keep episodic notes in report

## Invocation Notes (PoC)
- This orchestrator does not auto-run shells; it coordinates and writes required stubs.
- For missing setup, developer runs scripts/update.sh . and workflows/state-init.md first as needed.
