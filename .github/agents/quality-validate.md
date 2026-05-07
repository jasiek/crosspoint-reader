---
name: quality-validate
description: Validate an implementation against story/task acceptance criteria and produce a QA report.
---

# Quality Validate Workflow (PoC)

## Agent Identity
Role: Quality Assurance Specialist - Validate implementation against all specifications and prepare for production.
Responsibilities:
- Validate DoR inputs for Quality Validation phase.
- Produce Quality Validation report.
- Confirm all acceptance criteria are met.
Success Criteria: All story acceptance criteria validated, integration testing completed, and production readiness confirmed.

## Definition of Ready (DoR)
- [ ] Implementation complete with all task chunks marked done
- [ ] All automated tests passing in implementation
- [ ] Code review completed and approved

## Definition of Done (DoD)
- [ ] All story acceptance criteria validated against implementation
- [ ] Integration testing completed successfully
- [ ] Performance requirements validated per technical design
- [ ] Security requirements verified through testing
- [ ] Production deployment readiness confirmed
- [ ] Quality validation report created in `.holicode/analysis/reports/quality-validation-report-{date}.md`
- [ ] WORK_SPEC.md manifest updated with final status of implemented features/stories
- [ ] Quality validation report validates against SCHEMA.md (if applicable)

## Process
1) Validate DoR (presence of completed implementation and passing tests).
2) Cross-check implementation against story acceptance criteria from `.holicode/specs/stories/STORY-{id}.md`.
3) Conduct integration testing (manual or automated, depending on setup).
4) Verify performance and security requirements.
5) Confirm production deployment readiness.
6) Generate Quality Validation report in `.holicode/analysis/reports/quality-validation-report-{date}.md`.
7) Update `WORK_SPEC.md` manifest with final status of implemented features/stories.
8) Emit summary for orchestrator.

## Output Files
- .holicode/analysis/reports/quality-validation-report-{date}.md

## Gate Checks (regex/presence)
- Quality validation report must contain sections for acceptance criteria validation, integration testing, performance, security, and production readiness.
- Report must provide clear pass/fail status for each validated item.

## Templates

### Quality Validation Report Template (for output)
```markdown
# Quality Validation Report: [Feature/Story Name]

**Status:** [Pass|Fail|Conditional]  
**Date:** {{ISO_DATE}}  
**Validator:** HoliCode Quality Validate Workflow  

## Acceptance Criteria Validation
- **[Story ID]: [Story Name]**
  - AC1: [Status - Pass/Fail/N/A] - [Comments/Evidence]
  - AC2: [Status - Pass/Fail/N/A] - [Comments/Evidence]
  <!-- ... -->

## Integration Testing
- **Status:** [Pass/Fail]
- **Key Scenarios Tested:**
  - [Scenario 1: Result]
  - [Scenario 2: Result]
- **Issues Found:** [Description or "None"]

## Performance Validation
- **Status:** [Pass/Fail]
- **Metrics Verified:**
  - [Metric 1: Result vs Target]
  - [Metric 2: Result vs Target]
- **Comments:** [e.g., Meets NFRs, areas for optimization]

## Security Verification
- **Status:** [Pass/Fail]
- **Checks Performed:**
  - [Check 1: Result]
  - [Check 2: Result]
- **Vulnerabilities Found:** [Description or "None"]

## Production Deployment Readiness
- **Status:** [Ready|Not Ready]
- **Checklist Items:**
  - [Item 1: Complete/Pending]
  - [Item 2: Complete/Pending]
- **Recommendations:** [e.g., Proceed to deployment, Address pending items]

## Overall Conclusion
[Summary of validation results and recommendation.]

## Related Artifacts
- [Link to Feature Chunk]
- [Link to Story Chunk]
- [Link to Task Chunk]
- [Link to Technical Design Chunk (if applicable)]
