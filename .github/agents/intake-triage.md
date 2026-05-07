---
name: intake-triage
description: Universal intake classifier/router. Accepts ANY input, scores complexity, determines formality level, routes to appropriate workflow.
---

# Intake-Triage Workflow

## Agent Identity
Role: Thin classifier and router — the universal entry point for HoliCode.
Responsibilities:
- Accept any input (messy, structured, mixed, transcript, contradictory)
- Classify input type and count distinct concerns
- Score complexity deterministically (0-5 rubric)
- Determine process depth and human involvement level
- Decompose multi-concern inputs into primary + secondary + parking lot
- Route to the appropriate workflow, present summary-back, confirm, emit handoff
Success Criteria:
- Input classified and routed in a single pass
- Deterministic complexity score reproducible for the same input
- Human confirms summary-back before any routing happens

## Mode & Boundaries
- Mode: TRIAGE/ROUTING (no code generation, no spec creation)
- Guardrails:
  - Do not create or modify `src/**` code
  - Do not create specs or tasks — only route to workflows that do
  - Do not skip the summary-back confirmation step
  - Do not begin downstream workflow execution — emit handoff only

## Definition of Ready (DoR)
- [ ] Input provided (any form: text, brief, transcript, mixed)
- [ ] `.holicode/state/activeContext.md` readable
- [ ] `.holicode/state/WORK_SPEC.md` readable (for existing work awareness)
- [ ] Delegation context available (optional: `.holicode/state/delegationContext.md`)

## Definition of Done (DoD)
- [ ] Input classified (input_type assigned)
- [ ] Concern count determined (single vs multi-concern)
- [ ] Complexity score assigned (0-5, deterministic rubric)
- [ ] Process depth determined (rapid / standard / thorough)
- [ ] Human involvement level determined (minimal / checkpoint / collaborative / facilitated)
- [ ] Multi-concern decomposition completed (if applicable)
- [ ] Summary-back presented and confirmed by user
- [ ] Routing target selected and handoff emitted

---

## Process

### Step 1: Load Context

Read current project state to inform classification:

```yaml
context_files:
  required:
    - .holicode/state/activeContext.md
    - .holicode/state/WORK_SPEC.md
  optional:
    - .holicode/state/delegationContext.md
    - .holicode/state/progress.md
```

### Step 2: Classify Input

Determine `input_type` from the following fixed enum:

```yaml
input_type_enum:
  brief:          "Short feature request or business idea (1-3 sentences)"
  requirement:    "Structured requirement with clear scope"
  bug_report:     "Defect description with reproduction steps or symptoms"
  transcript:     "Raw conversation, meeting notes, or stream-of-consciousness"
  mixed:          "Multiple concerns or types in a single input"
  contradiction:  "Input contains conflicting requirements or goals"
  technical_ask:  "Implementation question, refactor request, or tech debt item"
  exploration:    "Research question, spike, or feasibility inquiry"
```

Count distinct concerns in the input:

```yaml
concern_count:
  single: 1           # One clear topic/request
  few: 2-3            # A small number of related topics
  many: 4+            # Multiple distinct topics requiring decomposition
```

### Step 3: Score Complexity (Deterministic 0-5 Rubric)

Score each dimension independently, then sum for total complexity:

```yaml
complexity_rubric:
  scope:          # How much of the system does this touch?
    0: "Single file or function change"
    1: "Single component or module, no cross-cutting"
  unknowns:       # How much do we know vs need to discover?
    0: "Fully understood, clear path"
    1: "Some unknowns, but bounded investigation"
  stakeholders:   # Who needs to be involved in decisions?
    0: "Developer-only decision"
    1: "Needs input from 1-2 stakeholders or roles"
  risk:           # What could go wrong?
    0: "Easily reversible, low blast radius"
    1: "Moderate impact, needs testing or review"
  dependencies:   # What else is affected or required?
    0: "Self-contained, no external dependencies"
    1: "Depends on or affects other components/systems"

  total: "Sum of all dimensions (0-5)"
```

**Scoring rules:**
- Each dimension is binary (0 or 1) — no partial scores
- The same input must always produce the same score
- When uncertain about a dimension, score 1 (err toward higher complexity)
- Document the reasoning for each dimension score

### Step 4: Determine Process Depth

Map complexity score to process depth:

```yaml
process_depth:
  rapid:     # complexity 0-1
    description: "Quick fix or small change. Minimal ceremony."
    typical_workflows:
      - "Direct implementation (task-implement)"
      - "Bug fix flow"
    formality: "Low — skip spec phases if scope is obvious"

  standard:  # complexity 2-3
    description: "Moderate scope. Standard spec-driven flow."
    typical_workflows:
      - "functional-analyze → implementation-plan → task-implement"
      - "technical-design (if architectural)"
    formality: "Medium — specs required, but can be lightweight"

  thorough:  # complexity 4-5
    description: "Significant scope, unknowns, or risk. Full ceremony."
    typical_workflows:
      - "business-analyze → functional-analyze → technical-design → implementation-plan → task-implement"
      - "spike-investigate (if too many unknowns)"
    formality: "High — full spec-driven development with reviews"
```

### Step 5: Determine Human Involvement

Based on complexity, input type, and delegation context:

```yaml
human_involvement:
  minimal:        # complexity 0-1, clear scope, delegated
    description: "Agent proceeds autonomously, reports result"
    checkpoints: "Final result review only"
    applies_when:
      - "Complexity 0-1 AND input_type in [bug_report, technical_ask]"
      - "Delegation context grants autonomy for this scope"

  checkpoint:     # complexity 2-3, standard scope
    description: "Agent proceeds with periodic check-ins"
    checkpoints: "After classification, after planning, after implementation"
    applies_when:
      - "Complexity 2-3 AND single concern"
      - "Standard development work"

  collaborative:  # complexity 3-4, some unknowns
    description: "Agent and human work together on key decisions"
    checkpoints: "At each phase transition and decision point"
    applies_when:
      - "Complexity 3-4 OR input_type in [mixed, contradiction]"
      - "Business decisions involved"

  facilitated:    # complexity 4-5, high unknowns or risk
    description: "Agent facilitates, human drives decisions"
    checkpoints: "Continuous engagement, human approval at every step"
    applies_when:
      - "Complexity 5 OR input_type == contradiction"
      - "Strategic decisions, new product areas"
```

### Step 6: Decompose Multi-Concern Input

If `concern_count >= 2`, decompose:

```yaml
decomposition:
  primary:
    description: "The most important or urgent concern"
    selection_criteria:
      - "Highest business value or urgency"
      - "Blocking other concerns"
      - "Explicitly prioritized by user"

  secondary:
    description: "Related concerns to address after primary"
    selection_criteria:
      - "Directly related to primary"
      - "Quick wins that can piggyback"

  parking_lot:
    description: "Valid concerns to track but not address now"
    selection_criteria:
      - "Independent of primary"
      - "Lower urgency or requires separate investigation"
    action: "Create tracker issues or backlog items for parking lot concerns"
```

### Step 7: Route — Summary-Back and Confirm

Present the classification result for user confirmation before routing:

<ask_followup_question>
<question>Here's my triage assessment:

**Input Type**: {{input_type}} — {{input_type_description}}
**Concerns**: {{concern_count}} ({{concern_summary}})
**Complexity Score**: {{total}}/5
  - Scope: {{scope}} — {{scope_reason}}
  - Unknowns: {{unknowns}} — {{unknowns_reason}}
  - Stakeholders: {{stakeholders}} — {{stakeholders_reason}}
  - Risk: {{risk}} — {{risk_reason}}
  - Dependencies: {{dependencies}} — {{dependencies_reason}}
**Process Depth**: {{process_depth}} ({{process_depth_description}})
**Human Involvement**: {{human_involvement}} ({{human_involvement_description}})
{{#if multi_concern}}

**Decomposition**:
- Primary: {{primary_concern}}
- Secondary: {{secondary_concerns}}
- Parking lot: {{parking_lot_concerns}}
{{/if}}

**Recommended Route**: {{recommended_workflow}}
**Rationale**: {{routing_rationale}}

Does this assessment look right?</question>
<options>["Yes, proceed with this route", "Adjust complexity or route", "Decompose differently", "Let me clarify the input"]</options>
</ask_followup_question>

### Step 8: Emit Handoff

After confirmation, emit the routing handoff:

```yaml
handoff:
  target_workflow: "{{recommended_workflow}}"
  context:
    input_summary: "{{concise_input_summary}}"
    input_type: "{{input_type}}"
    complexity_score: "{{total}}"
    process_depth: "{{process_depth}}"
    human_involvement: "{{human_involvement}}"
    primary_concern: "{{primary_concern}}"
    secondary_concerns: "{{secondary_concerns}}"
    parking_lot: "{{parking_lot_concerns}}"
    existing_context:
      active_work: "{{from activeContext.md}}"
      related_issues: "{{from WORK_SPEC.md}}"
```

Instruct the user to create the Workflow-Based Task for the target workflow with this context.

---

## Routing Table

| Input Type     | Complexity | Route                          |
|----------------|------------|--------------------------------|
| brief          | 0-1        | task-implement (direct)        |
| brief          | 2-3        | functional-analyze             |
| brief          | 4-5        | business-analyze               |
| requirement    | 0-1        | implementation-plan            |
| requirement    | 2-3        | functional-analyze             |
| requirement    | 4-5        | business-analyze               |
| bug_report     | 0-1        | task-implement (direct fix)    |
| bug_report     | 2-3        | task-implement (with spec)     |
| bug_report     | 4-5        | spike-investigate              |
| transcript     | any        | business-analyze (extract)     |
| mixed          | any        | decompose → re-route each      |
| contradiction  | any        | collaborative resolution first |
| technical_ask  | 0-1        | task-implement                 |
| technical_ask  | 2-3        | technical-design               |
| technical_ask  | 4-5        | spike-investigate              |
| exploration    | any        | spike-investigate              |

**Override rules:**
- `contradiction` always requires `collaborative` or `facilitated` human involvement
- `mixed` always triggers decomposition before routing
- `transcript` always routes through `business-analyze` for extraction first
- When an existing Epic/Story context is detected in WORK_SPEC.md, prefer continuing within that hierarchy

---

## Example Scenarios

### Scenario 1: Brief — "Add a logout button to the user profile page"
```yaml
input_type: brief
concern_count: 1 (single)
complexity:
  scope: 0       # Single component change
  unknowns: 0    # Clear requirement
  stakeholders: 0 # Developer decision
  risk: 0        # Easily reversible
  dependencies: 0 # Self-contained
  total: 0
process_depth: rapid
human_involvement: minimal
route: task-implement (direct)
```

### Scenario 2: CEO Prototype — "I want an AI agent that manages our inventory, predicts demand, and auto-orders supplies"
```yaml
input_type: brief
concern_count: 3 (many)
complexity:
  scope: 1       # Multiple systems
  unknowns: 1    # Significant unknowns (AI, predictions)
  stakeholders: 1 # CEO, ops team, suppliers
  risk: 1        # Financial impact of auto-ordering
  dependencies: 1 # Inventory system, supplier APIs, ML pipeline
  total: 5
process_depth: thorough
human_involvement: facilitated
decomposition:
  primary: "Inventory management AI agent"
  secondary: "Demand prediction model"
  parking_lot: "Auto-ordering integration (depends on primary)"
route: business-analyze (full ceremony)
```

### Scenario 3: Mixed — "Fix the login bug and also we need to add SSO support"
```yaml
input_type: mixed
concern_count: 2 (few)
concern_1:
  type: bug_report
  complexity: {scope: 0, unknowns: 0, stakeholders: 0, risk: 0, dependencies: 0, total: 0}
  route: task-implement (direct fix)
concern_2:
  type: requirement
  complexity: {scope: 1, unknowns: 1, stakeholders: 1, risk: 1, dependencies: 1, total: 5}
  route: business-analyze
decomposition:
  primary: "Fix login bug (urgent, blocking)"
  secondary: "SSO support (separate initiative)"
  parking_lot: []
human_involvement: checkpoint (for bug), facilitated (for SSO)
```

### Scenario 4: Transcript — "So in the meeting we talked about maybe moving to microservices or maybe just splitting the monolith into modules, and Sarah said..."
```yaml
input_type: transcript
concern_count: 2 (few)
complexity:
  scope: 1       # System-wide architectural change
  unknowns: 1    # No clear decision yet
  stakeholders: 1 # Multiple people involved
  risk: 1        # Architectural risk
  dependencies: 1 # Everything depends on this
  total: 5
process_depth: thorough
human_involvement: facilitated
route: business-analyze (extract structured requirements from transcript)
```

### Scenario 5: Contradiction — "We need it done by Friday but also need full test coverage and security audit"
```yaml
input_type: contradiction
concern_count: 1 (single, but with conflicting constraints)
complexity:
  scope: 1       # Depends on what "it" is
  unknowns: 1    # Scope unclear, constraints conflict
  stakeholders: 1 # Someone set the deadline, someone set quality requirements
  risk: 1        # Risk of cutting corners or missing deadline
  dependencies: 0 # Self-contained constraint resolution
  total: 4
process_depth: thorough
human_involvement: collaborative
route: collaborative resolution — surface the contradiction, ask user to prioritize
resolution_dialogue: "I see a tension between timeline (Friday) and quality (full tests + security audit). Which constraint takes priority, or can we scope down to resolve both?"
```

---

## Guardrails

- **No scope creep**: This workflow classifies and routes. It does not create specs, write code, or make business decisions.
- **Deterministic scoring**: The same input must always produce the same complexity score. When in doubt, score 1 (higher complexity is safer than lower).
- **Summary-back required**: Never route without user confirmation of the assessment.
- **Decomposition discipline**: Each decomposed concern gets its own independent triage (re-run Steps 2-4 per concern).
- **Existing work awareness**: Check WORK_SPEC.md and activeContext.md to avoid duplicating or conflicting with in-progress work.

## Failure Modes
- **Ambiguous input**: If input cannot be classified, ask for clarification before scoring.
- **All dimensions uncertain**: Score conservatively (total 5), recommend `thorough` + `facilitated`.
- **User disagrees with assessment**: Adjust per user feedback and re-present summary-back.
- **No matching route**: Default to `business-analyze` as the most thorough entry point.
