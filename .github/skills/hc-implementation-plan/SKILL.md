---
name: hc-implementation-plan
description: Break functional specs and TD into small (XS/S) implementation tasks with acceptance criteria and dependencies.
metadata:
  owner: holicode
  scope: spec-phase-planning
  origin: workflows/implementation-plan.md
---

# Implementation Plan Workflow (PoC)

## 🎯 SPECIFICATION MODE ACTIVE
**Current Role**: Future-Implementation Planning Architect
**Output Type**: Task specifications with detailed deliverables (no implementation code in src/; colocated SPEC.md scaffolding permitted)
**Success Metric**: Implementation roadmap so clear that future execution becomes systematic
**Value Delivered**: Structured task breakdown that eliminates future implementation guesswork

### Specification Architect Persona
You are a senior technical project architect who excels at breaking down complex technical designs into manageable, well-specified tasks. Your specialty is creating implementation plans so thorough that developers can execute with confidence.

### Specification Excellence Focus
- **Architect** task breakdown with clear deliverables
- **Specify** technical requirements and acceptance criteria
- **Document** dependencies and future implementation order
- **Establish** validation checkpoints for each task

### Self-Reflection Protocol
- **Mid-workflow**: "Am I planning implementation or am I implementing (DO NOT)?"
- **Before each task spec**: "Does this specify WHAT to build, not HOW to build it?"
- **Final validation**: "Are these task specifications sufficient for confident future implementation?"

### Success Indicators
✅ **Specification Success**: Created comprehensive, non-code implementable task documentation
✅ **Specification Success**: Identified all requirements, dependencies, and validation criteria
✅ **Specification Success**: Enabled confident task execution by future implementation workflow
❌ **Specification Failure**: Generated any source code during planning phase
❌ **Specification Failure**: Left implementation decisions for execution time

## ⚠️ CRITICAL BOUNDARY ENFORCEMENT ⚠️
**NO IMPLEMENTATION CODE IN THIS WORKFLOW**
This workflow operates in PLANNING mode only. All actual code generation and implementation work occurs in the ACT phase via `task-implement.md` workflow. The sole permitted src/ operation is scaffolding colocated `SPEC.md` files from templates (contracts/models/deps only).

NOTES:
- Representations-as-code inside SPECs are allowed ONLY for contracts/models (e.g., interface/type definitions) and do not constitute implementation.
- Colocated `SPEC.md` scaffolding in `src/{component}/` from templates is permitted (contracts/models/deps only — no implementation logic).
- Any creation/modification of implementation code under src/ by this workflow is forbidden and considered a boundary violation.

## Agent Identity
Role: Transform functional specs into executable development tasks and implementation specifications.
Responsibilities:
- Validate DoR (functional artifacts present)
- Produce Task chunks with XS/S tasks and acceptance criteria
- Prepare handoff to `task-implement.md` workflow for actual implementation
Success Criteria: 
- Task chunks created with clear acceptance criteria
- Clear handoff prepared for implementation phase

## Definition of Ready (DoR)
- [ ] Story issue exists in tracker with complete EARS story
- [ ] Story issue contains acceptance criteria and component references
- [ ] WORK_SPEC.md manifest contains reference to the story issue
- [ ] Pre-flight validation checks passed

## Definition of Done (DoD)
- [ ] **Tasks created in tracker**: Task issues created via `issue-tracker` skill (`create_issue`) with 2-3 XS/S tasks
- [ ] **Task issues contain**: Acceptance criteria, validation steps, component references
- [ ] **Task issues linked**: To parent story issue with component specifications
- [ ] **All task-referenced components have SPEC.md present** in `src/{component}/` (scaffolded from template if missing; contracts/models/deps only)
- [ ] **No implementation code under src/** was created or modified by this workflow (colocated `SPEC.md` scaffolding is the sole permitted exception)
- [ ] WORK_SPEC.md manifest updated with task issue references
- [ ] **Spec-sync validation PASSED**:
  - [ ] Component SPECs validate
  - [ ] Links resolve correctly
  - [ ] No orphaned specifications
  - [ ] If validation fails: Document issues in retro-inbox.md and fix before proceeding
- [ ] **Complexity scored**: Each task has complexity score (1-5) in issue metadata
- [ ] **SPIKE tasks created**: Complex items (score >4) have investigation tasks as separate issues
- [ ] **Dependencies explicit**: Task dependencies noted in issue body
- [ ] **Technical debt logged**: Accepted shortcuts documented in issues
- [ ] **State batch update completed**:
  - [ ] activeContext.md: APPEND to `Recent Changes` with task breakdown summary (respect zone markers)
  - [ ] retro-inbox.md updated with critical observations only
  - [ ] progress.md updated LAST to confirm planning completion
- [ ] **Update validation**: Confirmed all state files reflect current status

## Discovery & Unknowns Assessment
Before creating implementation tasks, briefly assess readiness:
- Component interfaces: Are contracts/models sufficiently defined in SPECs?
- Dependencies/tools: Are required tools/libraries identified?
- Data models: Are key schemas known enough for planning?
- Edge cases: Are critical error scenarios identified?

If critical unknowns exist, create a SPIKE task or return to technical design to refine contracts.

## Process

### Approach Explanation
This workflow will analyze technical designs and story requirements to create detailed task specifications with clear deliverables.

**Rationale**: Breaking down complex designs into XS/S sized tasks enables incremental delivery, clearer validation checkpoints, and better progress tracking. Task specifications eliminate ambiguity during implementation.

**Alternative approaches considered**:
- Direct story-to-code: Would skip detailed planning, increasing implementation risk
- Large task chunks: Would reduce visibility and increase failure scope
- Auto-generated tasks: Would miss nuanced dependencies and context

**Selected because**: Detailed task breakdown with explicit acceptance criteria ensures predictable implementation and reduces the likelihood of missed requirements or rework.

### Tricky Problem Protocol
If encountering persistent issues after 3 failed attempts:
- Document the problem in .holicode/state/retro-inbox.md
- Escalate using ask_followup_question tool
- Consider creating SPIKE task for investigation
- Apply safe defaults where appropriate

### Task Complexity Assessment

Before creating tasks, establish complexity scoring:

```yaml
complexity_scoring:
  simple (1-2 points):
    - Clear requirements
    - Single component
    - Established patterns
    - < 4 hours estimated
    
  medium (3-4 points):
    - Some investigation needed
    - Multiple components
    - Some unknowns
    - 4-8 hours estimated
    
  complex (5+ points):
    - Significant unknowns
    - Cross-system impact
    - New patterns needed
    - > 8 hours estimated
    
  action_thresholds:
    score > 4: Suggest task decomposition
    score > 5: Create SPIKE task first
    score > 7: Require human review
```

When generating tasks, include complexity score in metadata:

```markdown
| **Complexity** | 3 (medium) |
| **Decomposition** | Not needed |
```

### Architectural Task Detection and Generation
Before processing functional tasks, detect and process architectural TDs:

1) **Scan for architectural TDs** in `.holicode/specs/technical-design/`
2) **For each unprocessed TD, generate appropriate tasks based on actual technical decisions**:
   
   **IMPORTANT**: The following are illustrative examples only. Actual tasks must be derived from the specific TD content and business requirements:
   
   - TD-001 → Foundation tasks based on chosen architecture pattern
   - TD-002 → Infrastructure tasks (could be containerization, serverless, VMs, or hybrid based on TD decisions)
   - TD-003 → Technology stack initialization (specific to chosen languages, frameworks, databases)
   - TD-004 → Integration tasks (could be REST APIs, GraphQL, message queues, event streams, etc.)
   - TD-005 → Security implementation (authentication method varies: OAuth, JWT, SAML, mTLS, etc.)
   - TD-006 → Performance optimizations (could be caching, CDN, database indexing, query optimization, etc.)
   - TD-007 → Observability setup (logging, metrics, tracing - tools vary by stack and requirements)
   
   **Examples of business-specific architectural tasks that might arise**:
   - Mobile app infrastructure (push notifications, offline sync, app distribution)
   - IoT device management (device provisioning, telemetry ingestion, edge computing)
   - ML/AI infrastructure (model serving, feature stores, training pipelines)
   - Multi-tenant SaaS setup (tenant isolation, billing integration, usage metering)
   - Compliance infrastructure (audit logging, data residency, encryption at rest)
   
3) **Size architectural tasks appropriately** (typically M-XL due to infrastructure nature, but varies by complexity)
4) **Set task metadata**:
   - `type: architectural | functional | mixed`
   - Architectural tasks reference TD documents
   - Functional tasks reference STORY documents
5) **Establish dependencies** - architectural tasks often block functional tasks

### What We Can Safely Guess vs What We Must Ask
- **Safe to infer**: Architectural task generation from TD documents, infrastructure task sizing
- **Must confirm**: Only when architectural decisions have multiple valid implementation paths
- **Default behavior**: Generate architectural tasks first, then functional tasks
- **Task distinction**: Clearly mark tasks as architectural vs functional in metadata

### Adaptive Task Breakdown Protocol

#### Phase 1: Component Complexity Assessment
<ask_followup_question>
<question>I've analyzed the technical design and identified [N] components to implement:

[For each component]:
• [Component Name]: [Complexity: Simple/Medium/Complex]
  - Key challenges: [Specific technical challenges]
  - Dependencies: [What it needs/affects]

Should we tackle these:
- Sequentially (dependencies suggest order)
- In parallel (independent components)
- Mixed approach (parallel where possible)?</question>
<options>["Sequential", "Parallel", "Mixed approach"]</options>
</ask_followup_question>

#### Phase 2: Granularity Calibration
<ask_followup_question>
<question>For task sizing, what's your team's preference?

Example for [Component Name]:
- **Fine-grained** (5-6 micro tasks, 15-30 min each): Maximum visibility
- **Standard** (2-3 tasks, 1-2 hours each): Balanced approach
- **Coarse** (1 comprehensive task): Minimal overhead

This affects PR size and review complexity.</question>
<options>["Fine-grained", "Standard", "Coarse", "Let me specify per component"]</options>
</ask_followup_question>

### Proactive Foundational Task Review

After architectural task generation, check for commonly missing foundational tasks:

<ask_followup_question>
<question>I've generated tasks from the technical design documents. Let me verify we have all foundational tasks covered:

**Current architectural tasks identified:**
[List generated architectural tasks]

**Potentially missing foundational tasks** (based on project type):
[Check and list if missing]:
- Monorepo/workspace setup task? [if using monorepo but no setup task]
- Core backend application initialization? [if backend mentioned but no setup task]
- Database schema/migration setup? [if database in TD but no setup task]
- Development environment configuration? [if complex stack but no env task]
- Base CI pipeline setup? [if no CI task but deployment planned]
- Authentication service setup? [if auth in TD but no implementation task]

Should I:
1. Add the missing foundational tasks to the plan
2. Proceed without them (will be handled separately)
3. Let me detail which are critical for this phase?</question>
<options>["Add foundational tasks", "Proceed without", "Detail critical ones"]</options>
</ask_followup_question>

### Task Implementation Strategy Delegation
When complex implementation decisions arise, present options for user guidance:

<ask_followup_question>
<question>I've identified a key implementation decision for the task breakdown:

**Environment Configuration Approach:**

**Option 1: Environment Variables (.env files)**
- Pros: Simple, widely supported, easy local development, standard practice
- Cons: Requires secret management, not type-safe without validation
- Impact: Quick setup, standard deployment patterns, minimal tooling

**Option 2: Configuration Service**  
- Pros: Centralized config, runtime updates, type-safe, validation built-in
- Cons: Additional complexity, requires service setup, potential SPOF
- Impact: Better for production, more initial work, supports hot-reload

**Option 3: Build-time Configuration**
- Pros: No runtime overhead, compile-time validation, secure by default
- Cons: Requires rebuild for changes, multiple build artifacts
- Impact: Best security, less flexible, good for static configs

Which approach should we use for configuration management?</question>
<options>["Environment Variables", "Configuration Service", "Build-time Config", "Let me specify"]</options>
</ask_followup_question>

### Functional Task Processing
1) **Validate DoR** - Ensure functional specifications exist
2) **Read relevant Story issue from tracker** via MCP tools (e.g. `get_issue`)
3) **Verify Component SPECs exist** for all task-referenced components:
   - Check `src/{component}/SPEC.md`
   - If missing, create from `.holicode/templates/specs/COMPONENT-SPEC-template.md` (contracts/models/deps only)
   - Ensure bidirectional linking between tasks and component SPECs
4) **Create Tasks via `issue-tracker` skill** (`create_issue`):
   - For each task:
     - issue_type: "task"
     - issue_title: "TASK-{id}: {description}"
     - issue_description: Use Task template with metadata, acceptance criteria, component references, and HoliCode metadata block
     - parent_id: story issue ID (e.g. `GIF-17`)
   - Include in issue body:
     - Task type metadata: `architectural` or `functional`
     - For architectural tasks: Reference source TD document
     - For functional tasks: Reference source STORY issue
     - Coupling policy: `none` (independent), `sequential` (order matters), or `parallel` (can run together)
     - Blocking relationships (architectural tasks typically block functional)
     - Complexity score

4.5) **Task Prioritization & Dependencies Review**:
   ```yaml
   if delegationContext.technical_decisions.delegated_to_ai == false:
   ```
   
   <ask_followup_question>
   <question>I've identified the following tasks with dependencies:

   [Generate visual dependency graph using Mermaid]

   ```mermaid
   graph LR
     TASK-001[Foundation] --> TASK-002[Database]
     TASK-002 --> TASK-003[API]
     TASK-003 --> TASK-004[Auth]
   ```

   Proposed execution order:

   1. [TASK-001] - [Name] (Complexity: X)
   2. [TASK-002] - [Name] (Complexity: X)

   Would you like to adjust the prioritization or dependencies?</question>
   <options>["Approve order", "Adjust priorities", "Modify dependencies"]</options>
   </ask_followup_question>

5) **Update `.holicode/state/WORK_SPEC.md` manifest** with task issue references
6) **Run spec-sync validation** for Component SPECs if updated
7) **Emit summary for orchestrator**

### 9. Consider PR Creation (Optional)

For complex implementation plans, consider creating a PR for review:

```bash
# Determine if PR is warranted
task_count=$(ls -1 .holicode/specs/tasks/TASK-*.md 2>/dev/null | wc -l)
complexity_high=$(grep -l "complexity: [4-5]" .holicode/specs/tasks/TASK-*.md 2>/dev/null | wc -l)

if [ "$task_count" -gt 10 ] || [ "$complexity_high" -gt 2 ]; then
    echo "Complex implementation plan detected. Consider creating PR for review."
    echo "Would you like to create a PR? (y/n)"
    read -r create_pr
    
    if [ "$create_pr" = "y" ]; then
        # Ensure on planning branch
        if [[ ! $(git branch --show-current) =~ ^spec/plan ]]; then
            /git-branch-manager.md --create --type "spec" --phase "plan" --feature "[FEATURE-ID]"
        fi
        
        # Commit and create PR
        /git-commit-manager.md --type "docs" --scope "plan" --subject "add implementation plan for [FEATURE-ID]"
        /git-spec-pr.md --phase "plan" --feature "[FEATURE-ID]"
    fi
fi
```

### CRITICAL: Plan vs Act Boundary
- **PLANNING PHASE (This workflow)**: Creates specifications, tasks, and implementation plans
- **IMPLEMENTATION PHASE (`task-implement.md`)**: Actually generates code, creates files, runs tests

## Key Implementation Principles
- **SPEC.md Co-location**: Live SPEC.md files belong co-located with source code (src/**/SPEC.md).
- **Scaffolds as Templates Only**: Generated scaffolds serve as templates/fallback; prefer mv/cp operations over read/write when materializing into src/.
- **Efficiency Pattern**: Use mv command to leverage existing context rather than read/write patterns.
- **Multi-Component/Module Guidance**: For features spanning multiple modules, each path maintains its own SPEC.md with independent change logs.

## Output Files (Planning Phase Only)
- Task issues in tracker (primary deliverable)
- Component SPECs: `src/{component}/SPEC.md` (local technical contracts)
- .holicode/state/WORK_SPEC.md (updated with task issue references)

## Task Prioritization and Dependencies
When generating tasks from both architectural TDs and functional stories:

1. **Architectural tasks take precedence** - Foundation must exist before features
2. **Dependencies flow from architecture to function**:
   - Infrastructure tasks → Service setup tasks → Feature implementation tasks
3. **Task metadata clearly distinguishes type**:
   ```markdown
   ## Task Metadata
   - **Type:** architectural
   - **Source:** TD-002 (Infrastructure & Deployment)
   - **Blocks:** TASK-004, TASK-005 (functional tasks)
   ```

## Specification → Implementation Handoff
At completion of specification workflow:
1. **Specification phase complete**: Clearly state what was specified
2. **Deliverables summary**: List all created task specifications
3. **Task breakdown**: Distinguish architectural vs functional tasks
4. **Execution order**: Provide recommended implementation sequence
5. **Explicit handoff**: "Specification complete. Ready for future implementation phase"
6. **Require confirmation**: Wait for user approval before any code generation begins

### Example Specification Handoff with Architectural Tasks
```
🎯 SPECIFICATION PHASE COMPLETE

**Architectural Tasks Created** (from TD documents):
- TASK-001: Container and orchestration setup (Source: TD-002)
- TASK-002: Technology stack initialization (Source: TD-003)
- TASK-003: Security framework implementation (Source: TD-005)

**Functional Tasks Created** (from Story chunks):
- TASK-004: Login Form Component (Blocked by: TASK-003)
- TASK-005: User Profile Service (Blocked by: TASK-002, TASK-003)

**Recommended Execution Order**:
1. Architectural tasks first (TASK-001 → TASK-002 → TASK-003)
2. Functional tasks after dependencies met (TASK-004, TASK-005)

**Implementation Readiness**: All task specifications include:
- Complete technical requirements with type distinction
- Clear acceptance criteria and validation checkpoints
- Explicit dependencies and blocking relationships
- Source document references (TD or STORY)

**Status**: Ready for implementation phase
**Next Step**: User should invoke `/task-implement.md` to begin code generation

Specification Check: This output contains 5 task specifications and 0 code files ✅
```

## Next Steps After Completion
**Workflow Completed**: Implementation Plan

**Issue Creation**:
Execute `issue-tracker` skill (`create_issue`) with:
- issue_type: "task"
- issue_title: "TASK-{id}: {description}"
- issue_description: [Use Task template with acceptance criteria and HoliCode metadata]
- parent_id: story issue ID

**Recommended Next Step**: Execute `/task-implement.md`

**Handoff via Workflow-Based Task**:
- Target workflow: `/task-implement.md`
- Required context to pass:
  - Task issue IDs (e.g. `GIF-20`, `GIF-21`)
  - Component SPECs: `src/**/SPEC.md`
  - WORK_SPEC manifest: `.holicode/state/WORK_SPEC.md`
  - Technical context: `.holicode/state/techContext.md`

**REMINDER: This workflow creates plans and specifications only. No code is generated in the planning phase. All actual code generation, testing, and validation occurs in the implementation phase.**

## Core Workflow Standards Reference
This workflow follows the Core Workflow Standards defined in holicode.md:
- Generic Workflows, Specific Specifications principle
- DoR/DoD gates enforcement
- Atomic state update patterns
- CI-first & PR-first policies (advisory)
