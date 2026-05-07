---
name: framework-integration-test
description: Run an end-to-end integration test of the HoliCode workflow set and report findings.
---

# Framework Integration Test Workflow

## Agent Identity
Role: Framework Integration Tester  
Responsibilities: Validate all Stage 4 enhancements work together cohesively  
Success Criteria: All integration points tested, metrics collected, report generated

## Purpose
Test the complete Stage 4 enhanced framework through a full cycle:
- Decision delegation framework
- Interactive participation points
- Tricky problem protocol
- Pattern library usage
- Complexity management
- SPIKE handling
- Tech review process

## Test Scenarios

### Scenario 1: Simple Feature (Low Complexity)
1. Start with basic business brief
2. Test delegation bypass (explicit opt-out)
3. Generate single feature → story → task
4. Apply patterns from library
5. Complete without SPIKEs
6. Measure: Time, interactions, pattern reuse

### Scenario 2: Complex Feature (High Complexity)
1. Start with ambiguous business brief
2. Test interactive participation points
3. Generate multiple stories with dependencies
4. Trigger complexity scoring > 5
5. Create and execute SPIKE
6. Test tricky problem protocol
7. Complete tech review with YELLOW status
8. Measure: Escalations, SPIKE effectiveness, review accuracy

### Scenario 3: Problem Resolution
1. Simulate TypeScript augmentation issue
2. Apply pattern from library
3. If pattern fails, trigger tricky problem protocol
4. After 3 attempts, test escalation
5. Document new pattern
6. Measure: Time to resolution, pattern effectiveness

## Process

### 1. Setup Test Environment
```bash
# Create test workspace
mkdir -p test-stage4-integration
cd test-stage4-integration

# Initialize HoliCode
/state-init.md

# Configure delegation settings
# Set some decisions to delegated, others to require approval
```

### 2. Execute Test Scenarios
For each scenario:
1. Document starting conditions
2. Execute workflow sequence
3. Capture all interaction points
4. Measure key metrics
5. Validate outputs against expected

### 3. Collect Metrics
```markdown
## Metrics Collection

### Efficiency Metrics
- Time from brief to implementation plan
- Number of user interactions required
- Pattern reuse percentage
- SPIKE time box adherence

### Quality Metrics
- Specification completeness scores
- Rationale documentation coverage
- Security/reliability finding accuracy
- Tech review alignment scores

### Process Metrics
- Escalation trigger accuracy
- Problem resolution time
- Pattern library hit rate
- Delegation override frequency
```

### 4. Generate Integration Report
```markdown
## Integration Test Report
Location: `.holicode/analysis/reports/stage4-integration-test.md`

### Test Results
- Scenario 1: [PASS/FAIL] - [Details]
- Scenario 2: [PASS/FAIL] - [Details]
- Scenario 3: [PASS/FAIL] - [Details]

### Integration Points Validated
- [ ] Delegation framework → Workflows
- [ ] Interactive points → User responses
- [ ] Complexity scoring → SPIKE creation
- [ ] Pattern library → Problem resolution
- [ ] Tech review → Implementation gate
- [ ] Tricky protocol → Escalation

### Issues Found
[List any integration issues]

### Performance Comparison
| Metric | Stage 3 Baseline | Stage 4 Result | Improvement |
|--------|------------------|----------------|-------------|
| Stuck time | 60+ min | [X] min | [Y]% |
| User interactions | [N/A] | [X] | [New] |
| Pattern reuse | 0% | [X]% | [New] |
```

## DoD
- [ ] All test scenarios executed
- [ ] Metrics collected and analyzed
- [ ] Integration report generated
- [ ] Issues documented in retro-inbox
- [ ] Recommendations provided

## Test Scenario Details

### Scenario 1 Execution: Simple Feature
```yaml
test_case: "Simple User Login Feature"
complexity: Low (1-2)

steps:
  1_setup:
    - Initialize fresh workspace
    - Configure delegation (business: false, technical: true)
    
  2_business_analyze:
    - Input: "User login with email and password"
    - Expected: Feature chunk created
    - Measure: Time to complete, delegation checks
    
  3_functional_analyze:
    - Expected: Single story with acceptance criteria
    - Measure: Pattern references used
    
  4_technical_design:
    - Expected: TD-001, TD-003, TD-005 created
    - Delegation: Should auto-proceed (technical delegated)
    
  5_tech_review:
    - Expected: GREEN status (>80% alignment)
    - Measure: Review accuracy
    
  6_implementation_plan:
    - Expected: 2-3 XS tasks
    - Complexity: All tasks score 1-2
    - No SPIKEs needed

validation:
  - No escalations triggered
  - Pattern library used 2+ times
  - Total time < 30 minutes
```

### Scenario 2 Execution: Complex Feature
```yaml
test_case: "Multi-tenant SaaS Platform"
complexity: High (4-5)

steps:
  1_setup:
    - Initialize workspace
    - Configure delegation (all require approval)
    
  2_business_analyze:
    - Input: "Multi-tenant platform with isolation"
    - Maturity: Low (triggers investigation)
    - Expected: Multiple clarification requests
    
  3_functional_analyze:
    - Expected: 5+ stories with dependencies
    - Visual: Dependency diagram generated
    
  4_technical_design:
    - Expected: All 7 standard TDs created
    - Security: Critical requirements identified
    - Reliability: High availability needs
    
  5_tech_review:
    - Expected: YELLOW status
    - Issues: 2-3 medium priority findings
    - Conditions: Security enhancements needed
    
  6_implementation_plan:
    - Complexity: Multiple tasks score 4-5
    - SPIKEs: 2+ investigation tasks created
    - Expected: Phased implementation approach

validation:
  - User interactions > 10
  - SPIKEs created and time-boxed
  - Tech review identifies real issues
  - Total time < 2 hours
```

### Scenario 3 Execution: Problem Resolution
```yaml
test_case: "TypeScript Module Augmentation Issue"
problem: Express Request type extension fails

steps:
  1_simulate_problem:
    - Attempt 1: Direct augmentation (fails)
    - Attempt 2: Global declaration (fails)
    - Attempt 3: Custom namespace (fails)
    
  2_trigger_escalation:
    - After 3 attempts, protocol activates
    - Document in retro-inbox.md
    - Ask user for guidance
    
  3_pattern_check:
    - Check ts-node-typescript-patterns.md
    - Pattern exists but doesn't work
    
  4_resolution:
    - User provides solution
    - Update pattern library
    - Apply workaround
    
  5_knowledge_capture:
    - Document in retro-inbox
    - Update pattern with variation
    - Create reusable solution

validation:
  - Escalation at exactly 3 attempts
  - Pattern library checked first
  - Solution documented for reuse
  - Time to resolution < 15 minutes
```

## Metrics Collection Script Integration

This workflow integrates with the metrics collection script:

```bash
# After test execution
./scripts/collect-stage4-metrics.sh

# Generates report with:
# - Pattern usage counts
# - Complexity distribution
# - Rationale coverage
# - Delegation usage
# - Review results
```

## Report Template

```markdown
# Stage 4 Integration Test Report
**Date**: {{ISO_DATE}}
**Tester**: framework-integration-test workflow
**Framework Version**: Stage 4 Phase 3

## Executive Summary
[Overall assessment of Stage 4 features integration]

## Test Scenario Results

### Scenario 1: Simple Feature
**Result**: PASS
**Execution Time**: X minutes
**Key Observations**:
- Delegation worked as configured
- No unnecessary escalations
- Pattern library effectively used

### Scenario 2: Complex Feature  
**Result**: PASS
**Execution Time**: Y minutes
**Key Observations**:
- Interactive points triggered appropriately
- SPIKEs created for high complexity
- Tech review caught important issues

### Scenario 3: Problem Resolution
**Result**: PASS
**Resolution Time**: Z minutes
**Key Observations**:
- Tricky protocol activated at threshold
- Pattern library updated with solution
- Escalation handled gracefully

## Integration Validation

### Feature Integration Matrix
| Feature | Scenario 1 | Scenario 2 | Scenario 3 | Status |
|---------|------------|------------|------------|--------|
| Delegation Framework | ✅ | ✅ | N/A | PASS |
| User Participation | ✅ | ✅ | ✅ | PASS |
| Tricky Protocol | N/A | ✅ | ✅ | PASS |
| Pattern Library | ✅ | ✅ | ✅ | PASS |
| Complexity Scoring | ✅ | ✅ | N/A | PASS |
| SPIKE Creation | N/A | ✅ | N/A | PASS |
| Tech Review | ✅ | ✅ | N/A | PASS |

### Performance Metrics
| Metric | Target | Achieved | Status |
|--------|--------|----------|--------|
| Problem Resolution | <15 min | X min | [PASS/FAIL] |
| Pattern Reuse | >80% | X% | [PASS/FAIL] |
| Review Accuracy | >90% | X% | [PASS/FAIL] |
| User Satisfaction | Positive | [Rating] | [PASS/FAIL] |

## Issues and Recommendations

### Issues Found
1. [Issue description and impact]
2. [Issue description and impact]

### Recommendations
1. [Improvement suggestion]
2. [Enhancement opportunity]

## Conclusion
[Final assessment of Stage 4 readiness]
```

## Error Handling
- Test failure: Document failure mode and retry
- Integration conflict: Identify conflicting features
- Performance degradation: Compare with baseline
- Missing dependencies: List required fixes
