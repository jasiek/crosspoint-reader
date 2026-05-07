---
name: task-implement
description: Execute a selected TASK by generating code, running tests, validating acceptance criteria, and updating state.
---

# Task Implementation Workflow (Generic Implementation Engine)

## Agent Identity
Role: Generic implementation engine that transforms specifications into working code
Responsibilities:
- Discover artifacts using canonical precedence order (invocationPaths → taskFilePath → specPaths → featureRoot → auto-detect)
- Proactively apply runtime configurations (tsconfig.json, package.json merge, vitest.config.ts)
- Generate/update source code in src/ based on live SPEC.md (canonical source of truth)
- Validate SPEC adherence (API Contract, Data Model, Edge Cases compliance)
- Execute tests and validate acceptance criteria
- Create implementation summary artifact for traceability
- Update HoliCode state files with progress
- Support graduated formality: rapid mode bypasses spec-phase DoR, enforces safety gate, emits capture report for backfill

Success Criteria:
- Working code implementation matches live SPEC.md requirements exactly
- All SPEC-defined fields and behaviors implemented (e.g., createdAt in models)
- Tests pass with >90% success rate
- Implementation summary artifact created
- State files reflect completed work
- Idempotent execution (safe re-runs)
- Rapid mode: Safety gate enforced, capture report emitted, backfill handoff prepared

## Formality Resolution

Determine the formality level for this execution. Formality controls which DoR criteria are enforced and whether spec-phase artifacts are required upfront.

### Resolution Order
1. **Explicit parameter**: If invoked with `--formality rapid|standard|thorough`, use that value
2. **Handoff context**: If a handoff block is present with `process_depth`, map it:
   - `rapid` → formality_level: rapid
   - `standard` → formality_level: standard
   - `thorough` → formality_level: thorough
3. **Issue metadata**: If the tracker issue description contains `formality: <level>`, use that
4. **Default**: `standard` (full ceremony, backward compatible)

```yaml
formality_level:
  rapid:
    description: "Fast-track for low-complexity changes (0-1). Skip spec-phase DoR, enforce safety gate."
    dor_overrides:
      task_issue_in_tracker: RECOMMENDED  # Auto-created during backfill
      component_specs_present: SKIPPED     # Generated during backfill
      state_files_accessible: REQUIRED     # Non-negotiable
      runtime_stack_known: REQUIRED        # Non-negotiable
    dod_overrides:
      spec_change_log: DEFERRED           # Handled by spec-backfill
      component_spec_colocated: DEFERRED  # Handled by spec-backfill
    post_implementation: CAPTURE_REPORT_REQUIRED

  standard:
    description: "Normal spec-driven flow. All DoR/DoD criteria enforced."
    dor_overrides: none  # All criteria required
    dod_overrides: none  # All criteria required
    post_implementation: NORMAL

  thorough:
    description: "Full ceremony with additional review gates."
    dor_overrides: none  # All criteria required + additional rigor
    dod_overrides: none  # All criteria required + additional rigor
    post_implementation: NORMAL
```

**Resolved formality**: {{formality_level}} (source: {{resolution_source}})

## Definition of Ready (DoR)
- [ ] **Task issue** available in tracker with acceptance criteria (via issue ID or auto-discovery) — _RAPID: RECOMMENDED (safety gate substitutes)_
- [ ] **Component SPECs exist** under `src/**/SPEC.md` with contracts/models/deps defined — _RAPID: SKIPPED (deferred to backfill)_
- [ ] **.holicode/state files accessible**: activeContext.md, progress.md, techContext.md — _ALL MODES: REQUIRED_
- [ ] **Runtime indicated**: techContext.md specifies technology stack for proactive config — _ALL MODES: REQUIRED_
- [ ] Pre-flight validation checks passed

<validation_checkpoint type="dor_gate">
**DoR Self-Assessment** (Formality: {{formality_level}})

Before proceeding with implementation, verify:

1. **Task Issue in Tracker**
   - _If formality_level == "rapid": N/A (rapid mode — safety gate substitutes). Tracker issue will be created during spec-backfill._
   - _Otherwise:_
   - Status: YES / NO / PARTIAL
   - Evidence: [Issue ID, e.g. GIF-20]
   - Gap: [If NO, what's missing]

2. **Component SPECs Present**
   - _If formality_level == "rapid": N/A (rapid mode — SPEC deferred to backfill)._
   - _Otherwise:_
   - Status: YES / NO / PARTIAL
   - Evidence: [List SPEC.md file paths]
   - Gap: [If NO, which SPECs are missing]

3. **State Files Accessible** (ALL MODES)
   - Status: YES / NO / PARTIAL
   - Evidence: [Confirm activeContext.md, progress.md, techContext.md exist]
   - Gap: [If NO, which files are missing]

4. **Runtime Stack Known** (ALL MODES)
   - Status: YES / NO / PARTIAL
   - Evidence: [Quote from techContext.md]
   - Gap: [If NO, runtime not specified]

_If formality_level == "rapid":_
**DoR Compliance**: _/2 non-negotiable criteria met
**Proceed?**: If <2, resolve gaps. Safety gate check follows.

_Otherwise:_
**DoR Compliance**: _/4 criteria met
**Proceed?**: If <4, resolve gaps before implementation
</validation_checkpoint>

### Rapid Safety Gate (formality_level == "rapid" ONLY)

_Skip this section entirely if formality_level is standard or thorough._

<validation_checkpoint type="rapid_safety_gate">
**3 Non-Negotiable Safety Requirements**

Before proceeding with rapid implementation, ALL THREE must be satisfied:

1. **Reproducible Description**
   - Intent: [WHAT is being implemented — 1-2 sentences]
   - Scope boundary: [What is IN scope / OUT of scope]
   - Expected outcome: [Observable result when complete]
   - Status: DOCUMENTED / MISSING

2. **Verification Step**
   - Verification method: [test command / manual check / observable behavior]
   - Expected result: [What "success" looks like]
   - Status: DOCUMENTED / MISSING

3. **Rollback Note**
   - Pre-implementation commit (baseline): [auto-captured via `git rev-parse HEAD` before coding starts]
   - Rollback command: `git revert {{post_impl_commit}}` (reverts the implementation commit itself)
   - Additional rollback steps: [config changes, env vars, etc. — or "none"]
   - Status: DOCUMENTED / MISSING

**Safety Gate Compliance**: _/3 requirements met
**Proceed?**: ALL 3 must be met. No exceptions in rapid mode.
</validation_checkpoint>

## Definition of Done (DoD)
- [ ] **Working source code files** created/updated in src/ with actual implementation (NOT EMPTY FILES)
- [ ] **Live SPEC.md Change Log updated** with implementation details (contracts unchanged except minor corrections) — _RAPID: DEFERRED to spec-backfill_
- [ ] **All task acceptance criteria satisfied** from input task chunk — _RAPID: verified against safety gate description_
- [ ] **Tests pass** with >90% success rate covering SPEC-defined functionality
- [ ] **Configuration applied proactively**: package.json merged, tsconfig.json exists, vitest.config.ts applied
- [ ] **Dependencies installed successfully**: npm install executed without errors
- [ ] **State batch update completed** (respect zone markers — see holicode.md Zone-Based Update Rules):
  - [ ] activeContext.md: APPEND to `## Recent Changes` (format: `- [ISO_DATE ISSUE_ID] description`). Do NOT edit Generated zones.
  - [ ] retro-inbox.md updated with Tricky Problems and key learnings only
  - [ ] progress.md updated LAST to confirm completion
- [ ] **Update validation**: Confirmed all state files reflect actual completion status
- [ ] **Component SPEC co-located and updated** under src/**/SPEC.md — _RAPID: DEFERRED to spec-backfill_

## Process

```markdown
### 1. Validate DoR and Load Task from Tracker

```

1. Get task issue from tracker (via MCP tools, e.g. `list_issues(query: "GIF-20")` then `get_issue`)
2. Check task metadata in issue:
   - If `Coupling: sequential`, verify blocking tasks are complete
   - Respect Blocks/Blocked By relationships
3. Load Component SPECs referenced in task
4. Check .holicode/state/ files for current context

```javascript

**Discovery Process**: Load planning artifacts using path-agnostic discovery, prioritizing TASK chunk as primary input.
```

### 2. Simplified Tracker-First Discovery

**Discovery Order (Simple & Clear)**:
1. **Task Issue in Tracker** (PRIMARY):
   - If issue ID provided (e.g. `GIF-20`): Use MCP `list_issues(query: "GIF-20")` to resolve UUID, then `get_issue(id)`
   - Otherwise: Use MCP `list_issues(project_id)` and filter for "To do" status tasks
   - Select most recent ready task
2. **Component SPECs**: Load from `src/{component}/SPEC.md` referenced in task
3. **Fallback Only**: If MCP unavailable, check WORK_SPEC.md for task references

**Key Principles**:
- Issue tracker is ALWAYS the primary source
- Component SPECs are ALWAYS local (`src/**/SPEC.md`)
- No automatic dual state management
- simple_id (e.g. `GIF-20`) is the human-readable reference; UUID resolved on-demand

**Discovery Implementation**:
```bash
# Simplified discovery (pseudocode)
if task_simple_id_provided:
    results = list_issues(query: simple_id)
    task = get_issue(results[0].id)
else:
    all_tasks = list_issues(project_id)
    ready_tasks = filter(status == "To do")
    if ready_tasks.length == 1:
        task = ready_tasks[0]
    elif ready_tasks.length > 1:
        ask_user_which_task(ready_tasks)
    else:
        ask_user_for_task_id()

# Load component SPECs from task references
component_specs = load_specs_from_task_references(task)
```

**Ambiguity Resolution**:
If discovery fails or multiple candidates found:

<ask_followup_question>
<question>Which task should I implement?

Found the following ready tasks:
[List tasks with simple_ids and titles]

Please provide the issue ID (e.g. GIF-20) for the task you want implemented.</question>
<options>[List task simple_ids]</options>
</ask_followup_question>

### 3.5. Contextual Problem Tracking (Enhanced TPP)

Initialize contextual problem tracking with environment-aware thresholds:

```yaml
problem_tracker:
  current_issue: null
  attempt_count: 0
  approaches_tried: []
  
  # Contextual thresholds based on issue type
  thresholds:
    environmental_issues: 1-2  # Fast escalation
    logic_issues: 3            # Standard threshold
    integration_issues: 2-3     # Medium escalation
  
  # Environmental issue detection patterns
  environmental_patterns:
    - "command not found"
    - "permission denied"
    - "architecture mismatch"
    - "ENOENT"
    - "MODULE_NOT_FOUND"
    - "pnpm dlx"
    - "npm ERR!"
    - "graceful-fs"
    
on_error:
  # Detect issue type
  if any(pattern in error for pattern in environmental_patterns):
    issue_type = "environmental"
    max_attempts = 2  # Escalate faster
  else:
    issue_type = "logic"
    max_attempts = 3  # Standard threshold
    
  if error_matches(current_issue):
    attempt_count++
    approaches_tried.append(current_approach)
    if attempt_count >= max_attempts:
      trigger: contextual_escalation
  else:
    current_issue = new_error
    attempt_count = 1
```

### Automated Environment Snapshot Collection
When TPP activates for environmental issues, automatically collect diagnostic data:

```yaml
environment_snapshot:
  if issue_type == "environmental":
    collect:
      - os_info: uname -a
      - node_version: node -v  
      - pnpm_version: pnpm -v
      - path_env: echo $PATH
      - root_bin: ls -l node_modules/.bin
      - app_bin: ls -l apps/*/node_modules/.bin (if applicable)
      
  format_for_report: |
    Format collected data as code blocks in escalation
```

Execute environment collection commands before escalation:

<execute_command>
<command>uname -a</command>
<requires_approval>false</requires_approval>
</execute_command>

<execute_command>
<command>node -v</command>
<requires_approval>false</requires_approval>
</execute_command>

<execute_command>
<command>pnpm -v 2>/dev/null || npm -v</command>
<requires_approval>false</requires_approval>
</execute_command>

<execute_command>
<command>echo $PATH</command>
<requires_approval>false</requires_approval>
</execute_command>

<execute_command>
<command>ls -l node_modules/.bin 2>/dev/null | head -20</command>
<requires_approval>false</requires_approval>
</execute_command>

When triggering contextual escalation:

<ask_followup_question>
<question>I've encountered a persistent [issue_type] issue after [attempt_count] attempts:

**Problem**: [Concise description]

**Attempted Solutions**:
1. [Exact command/action tried] - Failed: [Brief outcome]
2. [Exact command/action tried] - Failed: [Brief outcome]
[3. [Exact command/action tried] - Failed: [Brief outcome] if applicable]

**Symptoms**:
```
[Relevant error messages]
[Log snippets]
[Observable behaviors]
```

**Environment Context** (auto-collected):
```bash
# OS & Architecture
[OS info from uname -a]

# Node.js Version
[Node version from node -v]

# Package Manager
[pnpm/npm version]

# PATH Environment
[PATH variable]

# Available Binaries (node_modules/.bin)
[Relevant binary listings]
```

**Current Hypothesis**: [Best guess at root cause based on symptoms and environment data]

**Proposed Next Steps**:
- **Provide specific guidance/command**: If you know the exact solution
- **Suggest a different diagnostic path**: If my hypothesis seems wrong
- **Approve SPIKE task for deeper research**: If this needs investigation
- **Approve workaround [specific workaround]**: If we should bypass this issue

How would you like me to proceed?</question>
<options>["Provide specific guidance/command", "Suggest different diagnostic path", "Approve SPIKE task", "Approve workaround", "Skip for now"]</options>
</ask_followup_question>

### 3. Proactive Runtime Configuration (Addresses PoC Issue: Reactive Config)

**CRITICAL**: Proactively apply all required runtime configurations based on techContext.md, preventing the reactive tsconfig.json fix identified in PoC iteration 04.

**Node.js + TypeScript Runtime Requirements** (when detected in techContext.md):
1. **tsconfig.json** (ALWAYS ensure exists):
   ```json
   {
     "compilerOptions": {
       "target": "ES2020",
       "module": "commonjs", 
       "moduleResolution": "node",
       "esModuleInterop": true,
       "allowSyntheticDefaultImports": true,
       "strict": true,
       "skipLibCheck": true,
       "forceConsistentCasingInFileNames": true,
       "declaration": true,
       "outDir": "./dist"
     },
     "include": ["src/**/*"],
     "exclude": ["node_modules", "dist"]
   }
   ```

2. **package.json Deterministic Merge Strategy**:
   - **READ existing package.json** (if exists)
   - **PRESERVE existing keys**: name, version, description, author, license
   - **MERGE scripts**: Add new scripts, preserve existing ones (no overwrite)
   - **MERGE devDependencies**: Add required deps, preserve existing versions
   - **ADD missing standard scripts**: 
     - `"test": "vitest"`
     - `"start": "ts-node src/index.ts"`
     - `"build": "tsc"`
   - **Required devDependencies**: typescript, ts-node, vitest, @types/node, uuid (if needed per SPEC)

3. **vitest.config.ts Application**:
   - Copy from scaffolds if available
   - Generate minimal config if not found:
   ```typescript
   import { defineConfig } from 'vitest/config'
   
   export default defineConfig({
     test: {
       environment: 'node'
     }
   })
   ```

4. **Dependencies Installation**:
   - Execute `npm install` after all config changes
   - Validate successful installation before proceeding

### 4. Source Code Generation

<workflow_state>
{
  "workflow": "task-implement",
  "phase": "implementation",
  "completed": ["dor_validation", "discovery", "config_setup"],
  "next": "code_generation",
  "confidence": 0.85
}
</workflow_state>

<validation_checkpoint type="stuck_pattern">
**Progress Health Check**

Before proceeding with code generation, assess implementation progress:

**Recent Actions** (last 3 attempts):
1. [Action 1] → [Outcome]
2. [Action 2] → [Outcome]  
3. [Action 3] → [Outcome]

**Pattern Analysis**:
- Semantic similarity? YES / NO (same type of approach repeated)
- Error repetition? YES / NO (same error message recurring)
- Progress made? YES / NO (measurable code advancement)

**Detection Triggers**:
- 3 semantically similar attempts = STUCK
- Same error 2+ times = STUCK
- No progress over 5 interactions = STUCK

**If STUCK**:
1. Document problem in retro-inbox.md
2. Trigger enhanced TPP with environment snapshot
3. Propose: SPIKE task / Alternative approach / Safe default workaround
</validation_checkpoint>

<validation_checkpoint type="scope_alignment">
**Scope Drift Assessment**

**Baseline Scope** (from tracker task issue):
[Original task description and acceptance criteria]

**Current Work Summary**:
[What has been implemented so far]

**Alignment Questions**:
1. Still solving the original problem? YES / PARTIALLY / NO
2. New requirements emerged? NONE / MINOR / SIGNIFICANT
3. Complexity vs estimate? SAME / +20% / +50% / DOUBLED

**Drift Magnitude**:
- All YES/NONE/SAME → MINOR (<10%) - Note and continue
- Mix with PARTIALLY/MINOR/+20% → MODERATE (10-30%) - Ask user confirmation
- Any NO/SIGNIFICANT/+50%/DOUBLED → MAJOR (>30%) - STOP and escalate

**Action Required**: [Based on drift magnitude assessment]
</validation_checkpoint>

#### Discovery & Unknowns Assessment
Before implementation, validate readiness:
- Contract completeness: Are API contracts and data models sufficient in SPEC.md?
- Implementation approach: Is the technical approach clear?
- Test strategy: Are scenarios and coverage goals defined?
- Dependencies: Are required packages/tools available?

If blockers exist:
- Create a SPIKE task or request refinement in planning/design
- Do not proceed until contracts are clarified

1. **SPEC-as-Input Checkpoint**
   - CRITICAL: Component SPECs must exist BEFORE implementation
   - Verify required `src/{component}/SPEC.md` files are present
   - If contracts are incomplete, pause and request TD/IP update

2. **Create source directory structure**
   - Ensure src/ directory exists
   - Create component/service subdirectories as specified in TASK chunk

3. **Generate implementation files**
   - Copy and adapt scaffold code files (e.g., TaskService.ts, TaskService.test.ts)
   - Apply any customizations specified in TASK chunk
   - Ensure code matches functional specifications
   - Implementation MUST conform to SPEC.md contracts

**Implementation Guidance**: Follow the Logic → Router → SPEC → Integrate pattern. See [.holicode/patterns/service-implementation.md](.holicode/patterns/service-implementation.md) for systematic implementation approach.

4. **Update Component SPECs (Change Logs Only)**
   ```markdown
   # SPEC.md Update Rules:
   1. SPECs are INPUT, not OUTPUT of implementation
   2. Only allowed updates:
      - Append to Change Log section with implementation details
      - Minor corrections to existing contracts (typos, clarifications)
      - Fill placeholder sections (e.g., Testing Strategy) if empty
   3. Major contract changes require new analysis:
      - Stop implementation
      - Document required changes
      - Request technical design/functional analysis update

   # Change Log Entry Format:
   ### {{ISO_DATE}} - TASK-{id}
   **Changes**: [Describe what was implemented]
   **Author**: task-implement workflow
   **Validation**: Tests pass, acceptance criteria met
   ```

4. **Bidirectional Linking Pattern**
   ```markdown
   ## Linked Specifications
   **Task**: [TASK-{id}](../../.holicode/specs/tasks/TASK-{id}.md)
   **Story**: [STORY-{id}](../../.holicode/specs/stories/STORY-{id}.md)
   **Feature**: [FEATURE-{id}](../../.holicode/specs/features/FEATURE-{id}.md)
   
   ## Change Log
   ### {{ISO_DATE}} - TASK-{id}
   **Changes**: [Describe implementation changes]
   **Author**: task-implement workflow
   **Validation**: Tests pass, acceptance criteria met
   ```

### 5. Test Execution and Validation

#### Standardized Verification Protocol
Before marking any implementation complete, execute this verification checklist:

**Pre-Verification Checklist:**
- [ ] All source files contain actual implementation (not stubs/placeholders)
- [ ] Component SPECs have been updated with change logs
- [ ] Dependencies installed successfully (`npm install` or equivalent)
- [ ] Configuration files present and valid (tsconfig.json, package.json, etc.)

**Verification Execution:**
- [ ] Unit or preferably integration/e2e tests written for all new functionality
- [ ] Test suite executes without failures
- [ ] Coverage meets minimum threshold (>90% for critical paths)
- [ ] Manual smoke test confirms basic functionality
- [ ] Error handling verified for edge cases

**Post-Verification:**
- [ ] Acceptance criteria from TASK chunk validated
- [ ] Performance requirements met (if specified)
- [ ] Security considerations addressed
- [ ] Documentation/comments adequate

#### Contract-First Testing Approach
1. **Extract contracts from SPEC.md**
   - Identify all public interfaces
   - Define expected behaviors
   - Create contract test stubs

2. **Write contract tests first**
   ```typescript
   // Example: Contract test for interface compliance
   describe('Component Contract Tests', () => {
     it('should implement all SPEC-defined methods', () => {
       // Test each method exists and has correct signature
     });
   });
   ```

3. **Implement against contracts**
   - Build implementation to satisfy contracts
   - Ensure all SPEC requirements met

4. **Implementation Pivot Strategy**
   When implementation gets stuck or encounters persistent issues, apply systematic pivot strategy:
   - **Pivot Threshold**: After N unsuccessful attempts (default: 5, configurable in [`holicode.md`](../holicode.md) Workflow Config), expand hypothesis scope to include:
     - External factors (configuration, dependencies, runtime environments)
     - System-level configurations
     - Cross-component interactions and dependencies
     - Infrastructure and tooling issues
   - **Containerization Issues**: Refer to [`.holicode/patterns/docker-containerization.md`](.holicode/patterns/docker-containerization.md) for systematic troubleshooting
   - **Debugging Strategy**: Shift from micro-iterations to higher level structured problem analysis
   - **Root Cause Analysis**: Look beyond immediate implementation to environmental and systemic factors
   - **Dedicated Investigation Task**: consider raising a need for dedicated task to debug/investigate and run down the problem and only then get back unless any major decisions to shift the approach

5. **Bundled Error Resolution**
   When tests fail:
   - Collect ALL error messages
   - Analyze patterns and root causes
   - Plan comprehensive fix
   - Apply all fixes in single operation
   - Re-run tests once

5. **Validation Criteria**
   - All contract tests pass
   - Implementation tests pass
   - Coverage meets requirements (>90%)
   - No empty/stub implementations

6. **Performance Testing Advisory**
   If the task includes performance criteria, prefer running the project-defined performance profile command (opt-in) instead of the default test suite to avoid CI flakiness. Examples (advisory, tech-agnostic):
   Use only when explicitly specified by TASK/SPECs or techContext; keep perf runs out of default CI.

**Verification Guidance**: See [.holicode/patterns/verification-tools.md](.holicode/patterns/verification-tools.md) for comprehensive verification strategies and tool selection matrix.

### 6. Commit Implementation Changes

Create a semantic commit for the implementation:
```bash
# Call git-commit-manager workflow
/git-commit-manager.md \
  --type "feat" \
  --scope "[TASK_ID]" \
  --subject "implement [task description]" \
  --workflow "task-implement"
```

### 7. Create Implementation PR

After successful implementation and testing:

```bash
# Check if PR already exists
if ! gh pr view 2>/dev/null; then
    echo "Creating pull request for implementation..."
    
    # Trigger PR creation workflow
    /github-pr-create.md
    
    # Note: PR creation is non-blocking
    # If it fails (offline, no auth), continue with local work
else
    echo "PR already exists for this branch"
    
    # Update PR with test results
    /github-pr-update.md --action "add-testing"
fi
```

#### Special Handling for FIX Tasks
If this is a FIX task (task ID starts with "FIX-"):
```bash
# For FIX tasks, push to the same branch as parent PR
if [[ "$TASK_ID" == FIX-* ]]; then
    # Extract parent PR number from task ID (e.g., FIX-PR123-001 -> 123)
    PARENT_PR=$(echo "$TASK_ID" | sed 's/FIX-PR\([0-9]*\)-.*/\1/')
    
    # Get parent PR branch
    PARENT_BRANCH=$(gh pr view "$PARENT_PR" --json headRefName -q .headRefName)
    
    # Ensure we're on the parent branch
    git checkout "$PARENT_BRANCH"
    
    # Commit the fix
    git add -A
    git commit -m "fix: address review feedback from PR #$PARENT_PR

Task: $TASK_ID
Resolves review comments"
    
    # Push to update the existing PR
    git push origin "$PARENT_BRANCH"
    
    # Add comment to PR about fixes
    gh pr comment "$PARENT_PR" --body "✅ Fixed issues from review:
- Task: $TASK_ID
- Changes pushed to this PR
- Please re-review the updated code"
    
    # Request re-review from original reviewers
    /github-pr-update.md --pr "$PARENT_PR" --action "request-review"
fi
```

### 7.5 Monitor PR Feedback (Optional)
After PR creation or update, optionally monitor for review feedback:
```bash
# Check if PR has reviews
PR_NUMBER=$(gh pr view --json number -q .number 2>/dev/null)
if [ -n "$PR_NUMBER" ]; then
    echo "Checking PR #$PR_NUMBER for review feedback..."
    
    # Check review status
    REVIEW_STATUS=$(gh pr view "$PR_NUMBER" --json reviewDecision -q '.reviewDecision')
    
    if [ "$REVIEW_STATUS" = "CHANGES_REQUESTED" ]; then
        echo "⚠️ Changes requested on PR #$PR_NUMBER"
        echo "To generate fix tasks, run:"
        echo "/github-pr-review.md --pr $PR_NUMBER"
    elif [ "$REVIEW_STATUS" = "APPROVED" ]; then
        echo "✅ PR #$PR_NUMBER is approved and ready to merge"
    fi
fi
```

### 8. State Updates & Success Validation

1. **Update Task Issue in Tracker**
   - Use `issue-tracker` skill (`update_issue`) to set status to **"In review"** (never "Done" — see holicode.md Issue Lifecycle)
   - "Done" requires PR merge + QA validation; the agent's terminal state is always "In review"
   - Update description with implementation summary if needed

2. **Update progress.md**
   - Mark implementation tasks as complete
   - Update completion percentages with actual results
   - Note any issues or deviations from planning

3. **Update activeContext.md** (respect zone markers):
   - APPEND to `## Recent Changes`: `- [ISO_DATE ISSUE_ID] description of completed work`
   - Do NOT edit Generated zones (`Current Focus`, `Ready to Start`, `Blockers`) — task-init regenerates these

4. **Critical Success Validation**
   - **Verify working code exists** (not empty files)
   - **Confirm tests pass** with documented results
   - **Validate acceptance criteria met** from tracker task issue
   - **Check SPEC.md change log updated** with implementation details

### 8.5. Capture Report & Backfill Handoff (rapid mode ONLY)

_Skip this section if formality_level is standard or thorough._

**MANDATORY**: After rapid implementation completes, generate and present the Capture Report before marking workflow complete.

#### Capture Report Generation

Collect the following automatically:

```yaml
capture_report:
  title: "Rapid Implementation Capture Report"
  timestamp: "{{ISO_DATETIME}}"
  task_reference: "{{task_id_or_description}}"

  implementation_summary:
    what_was_built: "{{safety_gate_description}}"
    files_changed:
      - "{{list all files created/modified}}"
    tests_written:
      - "{{list test files}}"
    commit_hash: "{{post_impl_commit}}"

  safety_gate_compliance:
    reproducible_description: "{{the intent/scope/outcome from safety gate}}"
    verification_result: "PASS / FAIL"
    verification_evidence: "{{test output or manual check result}}"
    rollback_note: "git revert {{post_impl_commit}}"

  complexity_assessment:
    original_score: "{{from handoff or manual assessment}}"
    actual_complexity_observed: "{{isolated_change | cross_cutting | architectural}}"
    complexity_rationale: "{{why — e.g., touched 1 file vs 5 files, new deps, etc.}}"

  backfill_recommendation:
    complexity_profile: "{{isolated_change | cross_cutting | architectural}}"
    required_artifacts: "{{list based on complexity_profile}}"
    recommended_workflow: "spec-backfill"
```

#### Complexity Profile Mapping

Use these criteria to determine the `actual_complexity_observed`:

| Profile | Criteria |
|---------|----------|
| `isolated_change` | Single file/function, no cross-cutting concerns, no new dependencies |
| `cross_cutting` | Multiple components affected, some dependency changes, integration points |
| `architectural` | System-wide impact, new patterns, external dependencies, design decisions |

#### Present as Sync Checkpoint

<ask_followup_question>
<question>**Rapid Implementation Complete — Capture Report**

Here's a summary of what was built:

**What**: {{what_was_built}}
**Files Changed**: {{files_changed_count}} files
**Tests**: {{tests_written_count}} tests ({{verification_result}})
**Commit**: {{commit_hash}}

**Observed Complexity**: {{actual_complexity_observed}}
**Rationale**: {{complexity_rationale}}

**Backfill Recommendation**: {{complexity_profile}} profile
Required artifacts:
{{required_artifacts_list}}

How would you like to proceed?</question>
<options>["Proceed with spec-backfill (recommended)", "Adjust complexity profile", "Defer backfill with justification (traceability gap until resolved)", "Review capture report details"]</options>
</ask_followup_question>

#### Backfill Handoff

If user confirms backfill, emit handoff context for `/spec-backfill.md`.

If user defers backfill, require a written justification and log it in `retro-inbox.md` as backfill debt. The capture report is preserved in the commit history for future backfill.

```yaml
backfill_handoff:
  target_workflow: "spec-backfill"
  context:
    capture_report: "{{full capture report}}"
    complexity_profile: "{{confirmed profile}}"
    implementation_commit: "{{commit_hash}}"
    files_changed: "{{file list}}"
    source_formality: "rapid"
```

### CRITICAL CHECKPOINT

<validation_checkpoint type="dod_compliance">
**DoD Self-Assessment** (Formality: {{formality_level}})

Before marking implementation complete, systematically verify ALL applicable DoD criteria:

1. **Working source code created/updated**
   - Status: YES / NO / PARTIAL
   - Evidence: [List file paths, summarize changes]
   - Gap: [If NO, what code is missing]
   - Confidence: _/5

2. **Live SPEC.md Change Log updated** _(RAPID: DEFERRED — mark N/A)_
   - Status: YES / NO / PARTIAL / N/A (rapid)
   - Evidence: [Quote change log entry]
   - Gap: [If NO, what's not documented]
   - Confidence: _/5

3. **All task acceptance criteria satisfied**
   - Status: YES / NO / PARTIAL
   - Evidence: [Quote from tracker task showing completion]
   - Gap: [If NO, which criteria failed]
   - Confidence: _/5

4. **Tests pass with >90% success rate**
   - Status: YES / NO / PARTIAL
   - Evidence: [Test output showing pass rate]
   - Gap: [If NO, which tests failed]
   - Confidence: _/5

5. **Configuration applied proactively**
   - Status: YES / NO / PARTIAL
   - Evidence: [Confirm package.json, tsconfig.json, vitest.config.ts]
   - Gap: [If NO, which configs missing]
   - Confidence: _/5

6. **Dependencies installed successfully**
   - Status: YES / NO / PARTIAL
   - Evidence: [npm install output]
   - Gap: [If NO, what errors occurred]
   - Confidence: _/5

7. **State files updated**
   - Status: YES / NO / PARTIAL
   - Evidence: [Confirm activeContext, retro-inbox, progress updated]
   - Gap: [If NO, which files not updated]
   - Confidence: _/5

8. **Component SPEC co-located and updated** _(RAPID: DEFERRED — mark N/A)_
   - Status: YES / NO / PARTIAL / N/A (rapid)
   - Evidence: [Path to SPEC.md with change log]
   - Gap: [If NO, what's missing]
   - Confidence: _/5

_If formality_level == "rapid":_

9. **Capture Report generated and presented**
   - Status: YES / NO
   - Evidence: [Capture report shown to user, response captured]
   - Confidence: _/5

**Overall DoD Compliance (rapid)**: _/7 applicable criteria met (_%)
_(Criteria 2 and 8 deferred to spec-backfill; criterion 9 added for rapid mode)_

_Otherwise:_

**Overall DoD Compliance**: _/8 criteria met (_%)

**Self-Reflection**:
- Are all source files actual implementations? YES / NO
- Did tests actually execute? YES / NO
- Are acceptance criteria genuinely met? YES / NO
- Is state accurately reflecting completion? YES / NO

**Proceed to completion?**: If <90% compliance, resolve gaps before marking complete
</validation_checkpoint>

<intent_check>
**Final Alignment Verification**
- Implementation aligned with original task? YES / NO
- Following all workflow procedures? YES / NO
- Within approved scope boundaries? YES / NO
- Ready for user review? YES / NO
- Confidence in completion: _/5

**If any NO**: [Explain issue and corrective action needed]
</intent_check>

Before marking DoD complete, ensure:
- [ ] Source files contain actual implementation code (not empty/placeholder)
- [ ] Tests execute and pass (addresses PoC iteration 03 complete failure)
- [ ] All TASK acceptance criteria satisfied from tracker issue
- [ ] Tracker issue set to "In review" (not "Done" — Done requires PR merge + QA validation)
- [ ] State files accurately reflect completed work

## Error Handling

- **Missing artifacts**: Report specific missing files and suggest running prior workflows
- **Dependency conflicts**: Report conflicts and suggest resolution strategies  
- **Test failures**: Provide detailed failure information and suggested fixes
- **File conflicts**: Handle existing files gracefully with merge/overwrite options

## Integration Points

### Input Sources
- Task issue from tracker (primary): Retrieved via MCP tools
- Component SPECs: Local `src/**/SPEC.md` files referenced in task
- .holicode/state/activeContext.md (current context)
- .holicode/state/techContext.md (tech stack info)
- reference scaffolds (optional fallback only when a live SPEC is missing)

### Output Targets (task-implement outputs)
- src/**/* (created/updated implementation files driven by tasks/specs)
- src/**/SPEC.md (live component specs updated with Change Log)
- package.json (devDependencies and scripts merged/updated)
- vitest.config.ts (applied/merged if needed)
- Test results (npm run test executed and validated)
- .holicode/state/progress.md (implementation status updated)
- .holicode/state/activeContext.md (current focus/status updated)
- Capture report (rapid mode only): Presented as sync checkpoint
- Backfill handoff context (rapid mode only): Emitted for spec-backfill workflow

## Quality Gates

- [ ] All TASK chunk acceptance criteria satisfied
- [ ] Unit tests pass with >90% success rate
- [ ] Live SPEC.md change log updated with current changes
- [ ] Code follows patterns from systemPatterns.md
- [ ] Implementation matches functional specifications

## Usage Example

```bash
# After spec-workflow.md completes planning phase:
# 1. Run this workflow
/story-implementation.md

# 2. Specify feature to implement
# 3. Workflow handles:
#    - Tech stack setup
#    - Code generation  
#    - Test execution
#    - State updates
```

## Handoff Integration

This workflow is designed to be called from `spec-workflow.md` step 8-9, after technical design and implementation planning is complete. It can also be run standalone when planning artifacts exist.

Next workflow in chain:
- **Rapid mode**: `/spec-backfill.md` (mandatory backfill of story/SPEC/TD artifacts)
- **Standard/Thorough mode**: QA validation or feature completion workflows

## Core Workflow Standards Reference
This workflow follows the Core Workflow Standards defined in holicode.md:
- Generic Workflows, Specific Specifications principle
- DoR/DoD gates enforcement
- Atomic state update patterns
- Tricky Problem Protocol for complex issues
- CI-first & PR-first policies (advisory)
