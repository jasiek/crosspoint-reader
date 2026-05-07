---
name: tech-review-post-planning
description: Review technical design after planning; assess risks, alignment, and readiness to implement.
---

# Post-Planning Technical Review Workflow

## Agent Identity
Role: Principal Technical Reviewer  
Responsibilities: Validate technical decisions against business requirements, security, and operational needs  
Success Criteria: Comprehensive review report with risk assessment and recommendations  
What/Why/How Focus: Validate HOW aligns with WHY across all technical decisions

## Purpose
Provide systematic self-critic review after technical design phase to ensure:
- Business requirements alignment
- Security coverage adequacy
- Reliability requirements met
- Cost implications understood
- Operational complexity manageable

## Definition of Ready (DoR)
- [ ] All TD documents (TD-001 through TD-XXX) created
- [ ] Business context (productContext.md) available
- [ ] Delegation settings checked (techContext.md or delegationContext.md)
- [ ] Success metrics defined in projectbrief.md

## Definition of Done (DoD)
- [ ] Technical alignment score calculated
- [ ] Security review completed with findings
- [ ] Reliability assessment documented
- [ ] Cost analysis provided (if applicable)
- [ ] Operational complexity evaluated
- [ ] Risk matrix generated
- [ ] Recommendations provided
- [ ] Review report created in `.holicode/analysis/reports/`
- [ ] If critical issues found, handoff created for resolution

## Process

### 1. Load and Analyze Technical Context
```markdown
## Context Loading
1. Read all TD documents from `.holicode/specs/technical-design/`
2. Load productContext.md for business requirements
3. Load systemPatterns.md for architectural constraints
4. Check delegation settings to determine approval requirements
```

### 2. Business Alignment Assessment
```markdown
## Business Alignment Checklist
- [ ] Each business goal has corresponding technical solution
- [ ] Success metrics are measurable through technical design
- [ ] User personas' needs addressed by architecture
- [ ] Constraints (budget, timeline, resources) respected
- [ ] MVP scope appropriately bounded

### Alignment Scoring
- **Full Alignment (90-100%)**: All business needs addressed
- **Good Alignment (70-89%)**: Most needs met, minor gaps
- **Partial Alignment (50-69%)**: Significant gaps requiring attention
- **Poor Alignment (<50%)**: Major redesign needed
```

### 3. Security Review
```markdown
## Security Assessment
Using patterns from `.holicode/patterns/security-checklist.md`:

### Authentication & Authorization
- [ ] Authentication method specified
- [ ] Authorization strategy defined
- [ ] Session management addressed
- [ ] Multi-factor authentication considered

### Data Protection
- [ ] Encryption at rest specified
- [ ] Encryption in transit defined
- [ ] PII handling documented
- [ ] Data retention policies noted

### API Security
- [ ] Rate limiting planned
- [ ] Input validation strategy
- [ ] CORS configuration noted
- [ ] API versioning approach

### Compliance
- [ ] Regulatory requirements identified
- [ ] Compliance gaps documented
- [ ] Audit trail capabilities

### Security Risks
For each identified risk:
- Risk description
- Severity (Critical/High/Medium/Low)
- Mitigation strategy or acceptance rationale
- Owner for resolution
```

### 4. Reliability Assessment
```markdown
## Reliability Review
Using patterns from `.holicode/patterns/reliability-patterns.md`:

### Availability
- [ ] Target SLA defined
- [ ] Single points of failure identified
- [ ] Redundancy strategy specified
- [ ] Failover mechanisms planned

### Performance
- [ ] Response time targets set
- [ ] Throughput requirements defined
- [ ] Scaling strategy documented
- [ ] Performance testing approach

### Error Handling
- [ ] Error recovery strategies
- [ ] Retry mechanisms defined
- [ ] Circuit breaker patterns
- [ ] Graceful degradation plans

### Monitoring
- [ ] Health check strategy
- [ ] Metrics collection plan
- [ ] Alerting thresholds
- [ ] Incident response process
```

### 5. Cost & Complexity Analysis
```markdown
## Cost Analysis
### Infrastructure Costs
- Compute resources required
- Storage needs
- Network bandwidth
- Third-party services

### Development Costs
- Estimated person-hours
- Required expertise
- Training needs
- Tool licensing

## Complexity Assessment
### Technical Complexity
- Number of components: [count]
- Integration points: [count]
- External dependencies: [count]
- Complexity score: [1-10]

### Operational Complexity
- Deployment complexity: [Low/Medium/High]
- Maintenance burden: [Low/Medium/High]
- Monitoring requirements: [Low/Medium/High]
- Team expertise needed: [Low/Medium/High]
```

### 6. Generate Review Report
```markdown
## Review Report Template
Create in: `.holicode/analysis/reports/tech-review-{date}.md`

# Technical Review Report
**Date**: {{ISO_DATE}}
**Reviewer**: tech-review-post-planning workflow
**Phase**: Post-Technical Design

## Executive Summary
[Brief overview of findings]

## Alignment Scores
- Business Alignment: [XX%]
- Security Coverage: [XX%]
- Reliability Readiness: [XX%]
- Cost Efficiency: [Rating]
- Operational Readiness: [Rating]

## Critical Findings
### High Priority Issues
1. [Issue]: [Impact and recommendation]
2. [Issue]: [Impact and recommendation]

### Medium Priority Issues
[List]

### Low Priority Improvements
[List]

## Risk Matrix
| Risk | Likelihood | Impact | Mitigation |
|------|------------|--------|------------|
| [Risk 1] | [H/M/L] | [H/M/L] | [Strategy] |

## Recommendations
### Immediate Actions
1. [Action with owner]
2. [Action with owner]

### Before Implementation
1. [Requirement]
2. [Requirement]

### Future Considerations
1. [Enhancement]
2. [Enhancement]

## Approval Requirements
Based on delegation settings:
- [ ] Business approval needed: [Yes/No]
- [ ] Technical approval needed: [Yes/No]
- [ ] Security approval needed: [Yes/No]
```

### 7. Decision Gates
```markdown
## Review Outcomes
Based on findings, determine next steps:

### GREEN: Proceed to Implementation
- Business alignment > 80%
- No critical security issues
- Reliability requirements met
- Acceptable cost/complexity

### YELLOW: Conditional Proceed
- Business alignment 60-80%
- Minor security issues (mitigatable)
- Some reliability gaps (acceptable)
- Higher complexity (justified)

Actions:
- Document accepted risks
- Create mitigation tasks
- Schedule follow-up review

### RED: Redesign Required
- Business alignment < 60%
- Critical security vulnerabilities
- Major reliability gaps
- Unacceptable cost/complexity

Actions:
- Create redesign tasks
- Schedule stakeholder review
- Update technical designs
```

## Human Interaction Points
```markdown
## When to Engage Stakeholders

### Always Require Human Review
- Critical security findings
- Business alignment < 70%
- Cost overrun > 20%
- Major architectural changes suggested

### Conditional Review (Check Delegation)
- Medium security issues
- Minor reliability gaps
- Alternative approaches identified
- Trade-off decisions needed

### Automated Approval Allowed
- All scores > 85%
- No critical issues
- Within budget/timeline
- Delegation explicitly granted
```

## Error Handling
- Missing TD documents: Report gap and continue with available
- Unclear requirements: Flag for clarification
- Conflicting constraints: Document and escalate

## Example Execution

### Loading Context
```bash
# Read all TD documents
for td in .holicode/specs/technical-design/TD-*.md; do
  echo "Loading: $td"
done

# Check delegation settings
if [ -f ".holicode/state/delegationContext.md" ]; then
  echo "Delegation context loaded"
fi
```

### Creating Review Report
```markdown
# Technical Review Report
**Date**: 2025-08-10
**Reviewer**: tech-review-post-planning workflow
**Phase**: Post-Technical Design

## Executive Summary
Technical design review completed for authentication system. Architecture aligns well with business requirements with minor security enhancements recommended.

## Alignment Scores
- Business Alignment: 85%
- Security Coverage: 75%
- Reliability Readiness: 90%
- Cost Efficiency: Good
- Operational Readiness: Good

## Critical Findings
### High Priority Issues
None identified.

### Medium Priority Issues
1. **MFA not mandatory**: Consider requiring MFA for admin users
2. **Rate limiting undefined**: Specify rate limits for API endpoints

### Low Priority Improvements
1. Consider implementing refresh token rotation
2. Add more detailed logging for audit trail

## Risk Matrix
| Risk | Likelihood | Impact | Mitigation |
|------|------------|--------|------------|
| Session hijacking | Low | High | Implement secure cookies, HTTPS only |
| Brute force attacks | Medium | Medium | Rate limiting, account lockout |

## Recommendations
### Immediate Actions
1. Define rate limiting strategy (Owner: Tech Lead)
2. Document MFA requirements (Owner: Security Team)

### Before Implementation
1. Complete security threat modeling
2. Define monitoring KPIs

### Future Considerations
1. Consider OAuth2 provider integration
2. Evaluate biometric authentication options

## Approval Requirements
Based on delegation settings:
- [ ] Business approval needed: No
- [ ] Technical approval needed: Yes (for MFA decision)
- [ ] Security approval needed: Yes
```

## Integration with Workflow Chain

### Input from technical-design.md
- TD documents in `.holicode/specs/technical-design/`
- Component SPECs in `src/**/SPEC.md`
- Updated WORK_SPEC.md manifest

### Output to implementation-plan.md
- Review report in `.holicode/analysis/reports/`
- Risk mitigation requirements
- Approved/conditional approval status
- Any required design updates

## Workflow Invocation
This workflow is typically invoked after technical-design.md completes:

```bash
# Execute review after technical design
/tech-review-post-planning.md

# Review will:
# 1. Load all TD documents
# 2. Assess against business requirements
# 3. Perform security/reliability review
# 4. Generate comprehensive report
# 5. Determine GREEN/YELLOW/RED status
```

## Success Metrics
- 100% of technical designs reviewed before implementation
- >90% accuracy in finding architectural and security issues
- Clear GREEN/YELLOW/RED decision paths
- Review reports actionable with specific recommendations
