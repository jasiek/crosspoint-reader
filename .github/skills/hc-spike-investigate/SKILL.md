---
name: hc-spike-investigate
description: Time-boxed investigation workflow to answer unknowns, test hypotheses, and document findings.
metadata:
  owner: holicode
  scope: investigation-spike
  origin: workflows/spike-investigate.md
---

# SPIKE Investigation Workflow

## Agent Identity
Role: Technical Investigator
Responsibilities: Research unknowns, test hypotheses, document findings
Success Criteria: Answer investigation question within time box

## DoR
- [ ] SPIKE task exists with clear question
- [ ] Time box defined (max 4 hours)
- [ ] Success criteria specified

## Process

### 1. Load SPIKE Specification
Read the SPIKE task from `.holicode/specs/tasks/SPIKE-{id}.md`

### 2. Research Phase
- Review existing patterns in `.holicode/patterns/`
- Search relevant documentation
- Analyze similar implementations
- Research external resources

> **Large corpus gate**: If the investigation requires loading session transcripts, workspace logs, or multi-file corpora exceeding ~200KB total, do **not** attempt inline analysis. Instead:
> 1. Create a dedicated sub-issue scoped to that analysis (e.g. "Load and analyse HOL-XX session JSONL [1M context]")
> 2. Dispatch it with `variant: SONNET_1_M_100_K_IN` via `workspace-orchestrate`
> 3. The sub-issue DoR must specify: exact file paths, output artifact path, and "skip standard state-loading boilerplate"
> 4. Pick up the artifact once the workspace completes
>
> This pattern applies equally to `session-retrospective` workflows that need to load raw JSONL transcripts. See `techContext.md § Executor Profiles` for variant selection and read thresholds.

### 3. Hypothesis Testing
- Create minimal PoC if needed
- Test assumptions
- Document what works and what doesn't
- Identify edge cases

### 4. Documentation
Update the SPIKE task with:
- **Findings**: What was learned
- **Recommendation**: How to proceed
- **New patterns**: Any patterns to add to library
- **Remaining unknowns**: What still needs investigation

### 5. Update Parent Task
- Update parent task with findings
- Reduce complexity score based on learnings
- Add implementation approach

### 6. Pattern Extraction
If new patterns discovered:
- Document in relevant `.holicode/patterns/` file
- Update retro-inbox.md with learning

## DoD
- [ ] Question answered or declared unanswerable
- [ ] If investigation involved large corpora (> 200KB): 1M sub-issue dispatch used, not inline loading
- [ ] Findings documented in SPIKE analysis report (`.holicode/analysis/spike-<ISSUE-ID>-<slug>.md`)
- [ ] Parent task updated with findings summary
- [ ] **Actionable follow-ups section** included in analysis report with:
  - Concrete recommendations (prioritized: P0/P1/P2)
  - Suggested follow-up stories/tasks to create (with title, scope sketch, and size estimate)
  - Decision points requiring human input (flagged explicitly)
  - Remaining unknowns that need further investigation
- [ ] Patterns extracted to library (if applicable)
- [ ] Time box respected
- [ ] retro-inbox.md updated with learnings
- [ ] **Spike Outcome Ritual completed** (see `.clinerules/holicode.md` § Spike Outcome Ritual):
  - [ ] Analysis doc has standard header (Issue, Date, Status, Recommendation)
  - [ ] SPIKE-INDEX.md entry added (create from template if first spike)
  - [ ] Knowledge distilled to durable docs (techContext / systemPatterns / patterns / CLAUDE.md as applicable)
  - [ ] Tracker issue updated with final status + analysis doc link

## Time Box Management
```yaml
time_management:
  max_duration: 4_hours
  checkpoints:
    - 1_hour: Assess progress
    - 2_hours: Decide on approach
    - 3_hours: Start documenting
    - 4_hours: HARD STOP
```

## Investigation Techniques

### Pattern Matching
Check if the problem matches any existing patterns:
- Testing issues → `.holicode/patterns/testing-cookbook.md`
- TypeScript problems → `.holicode/patterns/typescript-patterns.md`
- Docker challenges → `.holicode/patterns/docker-patterns.md`
- Configuration issues → `.holicode/patterns/configuration-patterns.md`
- Security concerns → `.holicode/patterns/security-checklist.md`
- Reliability needs → `.holicode/patterns/reliability-patterns.md`

### Minimal PoC Creation
When creating a PoC:
- Keep it minimal (just enough to test hypothesis)
- Document assumptions
- Test edge cases
- Measure performance if relevant

### Documentation Template for Findings
```markdown
## Findings
**Question**: [Original investigation question]
**Answer**: [Clear answer or "Unable to determine"]

### What Worked
- [Approach that succeeded]
- [Key insight gained]

### What Didn't Work
- [Failed approach and why]
- [Incorrect assumption identified]

### Recommendation
[Clear recommendation for how to proceed]

### New Patterns Discovered
[Any reusable patterns to add to library]

### Remaining Unknowns
[What still needs investigation]

### Actionable Follow-Ups
#### Recommended Stories/Tasks to Create
- **[P0] [title]** — [scope sketch] — Size: [XS/S/M/L]
- **[P1] [title]** — [scope sketch] — Size: [XS/S/M/L]

#### Decision Points (Require Human Input)
- [Decision needed and options]

#### Further Investigation Needed
- [Follow-up spike if applicable]

### Time Spent
[Actual time spent on investigation]
```

## Common Investigation Areas

### Performance Issues
- Profile the code
- Identify bottlenecks
- Test optimization strategies
- Document trade-offs

### Integration Problems
- Test API contracts
- Verify data formats
- Check authentication/authorization
- Document integration patterns

### Configuration Mysteries
- Test different configurations
- Document what each setting does
- Identify dependencies between settings
- Create minimal working config

### Testing Challenges
- Try different mocking strategies
- Test isolation vs integration approaches
- Document test patterns that work

## Exit Criteria
Investigation ends when:
1. Question is answered definitively
2. Time box expires
3. Determined unanswerable with current resources
4. Blocked by external dependencies

## Next Steps After Completion
**Workflow Completed**: SPIKE Investigation
**Actions Required**:
1. Complete the **Spike Outcome Ritual** (analysis doc header, SPIKE-INDEX entry, knowledge distillation, tracker update)
2. Update parent task with findings
3. Create follow-up SPIKEs if needed
4. Add patterns to library
5. Update WORK_SPEC.md if scope changed

**Handoff to Implementation**:
If SPIKE successful, parent task should now have:
- Clear implementation approach
- Reduced complexity score
- Known patterns to apply
- Identified risks and mitigations
