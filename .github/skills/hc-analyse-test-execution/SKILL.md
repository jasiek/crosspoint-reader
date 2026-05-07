---
name: hc-analyse-test-execution
description: Analyze SDD PoC test execution artifacts and produce a concise improvement report with recommendations.
metadata:
  owner: holicode
  scope: test-analysis
  origin: workflows/analyse-test-execution.md
---

# Analyse Test Execution Workflow

## Agent Identity
Role: Systematically analyze test execution results from Spec-Driven Development PoC iterations.
Responsibilities:
- Check for existing collected artifacts or trigger collection from GitHub repositories
- Review conversation reports for workflow execution details, challenges, and learnings
- Analyze HoliCode state consistency across workflow runs
- Validate the generated codebase against specifications and intended outcomes
- Compare with previous iterations to identify evolutionary patterns
- Identify gaps, inconsistencies, and areas for improvement in workflows and instructions
- Formulate concrete recommendations based on the `spec_driven_exec_plan.md` and the principle of generic workflows driven by specific specifications
Success Criteria: Produce a detailed, insightful analysis report (`test-execution-analysis-s<stage>-i<iteration>.md`) that informs subsequent workflow refinements or `task-implement` efforts.

## Inputs
- `sdd_poc_series` (required): The series number (01-99)
- `sdd_poc_iteration` (required): The iteration number (01-99)
- `github_repo` (optional): GitHub repository in org/name format (e.g., `holicode-testing/s04-i01`)
- `git_ref` (optional): Git reference (branch/tag/commit) to analyze (default: main)
- `phase_suffix` (optional): Phase identifier for multi-phase analysis (e.g., `p01`, `p02`)

## Definition of Ready (DoR)
- [ ] Series and iteration numbers provided
- [ ] Either existing artifacts in `.holicode/analysis/scratch/sdd-poc-sXX-iYY/` OR GitHub repository specified
- [ ] Previous iteration reports available for comparison (if not first iteration)
- [ ] Implementation specifications available (if analyzing Stage 4+)

## Process Steps

### 1. Initialization & Context Setup
```bash
# Format series and iteration with zero-padding
printf -v SERIES "%02d" "$sdd_poc_series"
printf -v ITERATION "%02d" "$sdd_poc_iteration"
SCRATCH_DIR=".holicode/analysis/scratch/sdd-poc-s${SERIES}-i${ITERATION}"
CACHE_DIR=".holicode/cache/repos"
```

### 2. Check for Existing Artifacts
```bash
# Check if artifacts already collected
if [[ -d "$SCRATCH_DIR" ]]; then
    echo "✓ Artifacts already collected in $SCRATCH_DIR"
    # Optionally verify completeness
    if [[ -f "$SCRATCH_DIR/collection-metadata.json" ]]; then
        echo "  Collection metadata present"
    fi
else
    echo "✗ No existing artifacts found"
fi
```

### 3. Conditional Artifact Collection
If artifacts don't exist and GitHub repository is specified:

```bash
# Execute collection script only if needed
if [[ ! -d "$SCRATCH_DIR" && -n "$github_repo" ]]; then
    echo "Collecting artifacts from GitHub repository..."
    .holicode/scripts/collect-sdd-poc-artifacts-github.sh \
        -s "$sdd_poc_series" \
        -i "$sdd_poc_iteration" \
        -r "$github_repo" \
        -p "${git_ref:-main}"
    
    if [[ $? -ne 0 ]]; then
        echo "Error: Collection failed. Check logs above."
        exit 1
    fi
fi
```

### 4. Verify Artifact Availability
```
<list_files>
<path>{{SCRATCH_DIR}}</path>
</list_files>

# Verify required files exist
<list_files>
<path>{{SCRATCH_DIR}}/reports</path>
</list_files>

# Check for text archives
for file in holicode-state.txt holicode-specs.txt codebase.txt; do
    <read_file>
    <path>{{SCRATCH_DIR}}/$file</path>
    </read_file>
done
```

### 5. Repository Cache Verification (Optional)
If GitHub repository was used, verify cached repository for specific commit analysis:

```bash
# Check cached repository if needed for deeper analysis
if [[ -n "$github_repo" ]]; then
    REPO_CACHE="$CACHE_DIR/${github_repo}"
    if [[ -d "$REPO_CACHE/.git" ]]; then
        echo "Repository cache available at: $REPO_CACHE"
        cd "$REPO_CACHE"
        echo "Current commit: $(git rev-parse --short HEAD)"
        echo "Commit message: $(git log -1 --pretty=%B)"
    fi
fi
```

### 6. Load Comparison Data
#### A. Previous Iteration Reports
```
# Find previous iteration report
PREV_SERIES=$((10#$SERIES))
PREV_ITERATION=$((10#$ITERATION - 1))

# Check same series, previous iteration
if [[ $PREV_ITERATION -gt 0 ]]; then
    printf -v PREV_ITER "%02d" "$PREV_ITERATION"
    PREV_REPORT=".holicode/analysis/reports/test-execution-analysis-s${SERIES}-i${PREV_ITER}*.md"
else
    # Check previous series if this is iteration 01
    if [[ $PREV_SERIES -gt 1 ]]; then
        PREV_SERIES=$((PREV_SERIES - 1))
        printf -v PREV_SER "%02d" "$PREV_SERIES"
        # Find last iteration of previous series
        PREV_REPORT=".holicode/analysis/reports/test-execution-analysis-s${PREV_SER}-i*.md"
    fi
fi

# Read previous report if found
for report in $PREV_REPORT; do
    if [[ -f "$report" ]]; then
        <read_file>
        <path>$report</path>
        </read_file>
        break
    fi
done
```

#### B. Implementation Specifications
```
# For Stage 4+, load relevant implementation specs
if [[ $((10#$SERIES)) -ge 4 ]]; then
    # Load Stage 4 implementation specifications
    <read_file>
    <path>.holicode/analysis/reports/stage4-phase1-implementation-spec.md</path>
    </read_file>
    <read_file>
    <path>.holicode/analysis/reports/stage4-phase2-implementation-spec.md</path>
    </read_file>
    <read_file>
    <path>.holicode/analysis/reports/stage4-phase3-implementation-spec.md</path>
    </read_file>
fi
```

#### C. Reference Plan
```
<read_file>
<path>.holicode/analysis/scratch/spec_driven_exec_plan.md</path>
</read_file>
```

### 7. Analyze Workflow Execution Reports
For each report in `{{SCRATCH_DIR}}/reports/`:
- Extract workflow type and execution context
- Identify successes and challenges
- Document learnings and meta-observations
- Note any deviations from expected flow

Key analysis points:
- Workflow execution sequence
- Decision points and user interactions
- Error recovery patterns
- Time spent on different phases
- Pattern reuse effectiveness

### 8. Analyze HoliCode State Consistency
From `{{SCRATCH_DIR}}/holicode-state.txt`:
- Verify state file completeness
- Check cross-references between files
- Validate progress tracking accuracy
- Assess context currency
- Identify any state drift

### 9. Analyze Codebase Output
From `{{SCRATCH_DIR}}/codebase.txt`:
- Validate generated code structure
- Check specification adherence
- Verify test coverage
- Assess code quality indicators
- Confirm configuration completeness

### 10. Comparative Analysis
Compare with previous iteration:
- Conversation count changes
- Codebase size evolution
- Quality metrics trends
- New patterns emerged
- Resolved vs new challenges

Calculate improvement metrics:
```
# Example metrics calculation
CURRENT_CONVERSATIONS=$(ls {{SCRATCH_DIR}}/reports/*.md | wc -l)
CURRENT_FILES=$(grep -c "~~~~" {{SCRATCH_DIR}}/codebase.txt || echo "0")

# Compare with previous if available
if [[ -n "$PREV_CONVERSATIONS" ]]; then
    CONVERSATION_CHANGE=$(( (CURRENT_CONVERSATIONS - PREV_CONVERSATIONS) * 100 / PREV_CONVERSATIONS ))
    echo "Conversation change: ${CONVERSATION_CHANGE}%"
fi
```

### 11. Generate Comprehensive Report
Create the analysis report with enhanced structure:

```
<write_to_file>
<path>.holicode/analysis/reports/test-execution-analysis-s{{SERIES}}-i{{ITERATION}}{{phase_suffix}}.md</path>
<content>
# Test Execution Analysis Report - SDD PoC Stage {{SERIES}} Iteration {{ITERATION}}

**Generated**: {{ISO_TIMESTAMP}}
**Analyst**: analyse-test-execution workflow
**Source**: {{github_repo || "Local artifacts"}}
**Dataset Size**: {{conversation_count}} conversations
**Comparison Base**: {{previous_report || "First iteration"}}

## Executive Summary
[High-level overview of iteration success, key findings, and evolution from previous iteration]

## I. Dataset Overview & Collection Method
### Collection Statistics
- Conversations: {{current}} (Previous: {{previous}}, Change: {{change}}%)
- Codebase Files: {{current}} (Previous: {{previous}}, Change: {{change}}%)
- Specifications: {{completeness}}
- State Files: {{status}}

### Collection Method
{{If GitHub: Repository details, commit, branch}}
{{If Local: Source path and collection date}}

## II. Workflow Execution Analysis
### Conversation Distribution
[Table showing workflow types and conversation counts]

### Execution Patterns
[Identified patterns in workflow execution]

### Challenges & Resolutions
[Key challenges faced and how resolved]

## III. HoliCode State Consistency
### State Evolution
[How state files evolved through conversations]

### Consistency Metrics
- Update Frequency: {{metric}}
- Completeness: {{metric}}
- Accuracy: {{metric}}

## IV. Codebase Validation
### Code Generation Metrics
[Statistics on generated code]

### Quality Indicators
[Code quality assessment]

### Specification Adherence
[How well code matches specifications]

## V. Comparative Analysis with Previous Iteration
### Quantitative Comparison
| Metric | Previous (s{{PREV_S}}-i{{PREV_I}}) | Current | Change |
|--------|------------|---------|--------|
| Conversations | {{prev}} | {{curr}} | {{change}}% |
| Code Files | {{prev}} | {{curr}} | {{change}}% |
| Test Coverage | {{prev}} | {{curr}} | {{change}}% |

### Qualitative Evolution
[How the approach/quality evolved]

### Pattern Evolution
[New patterns vs refined patterns]

## VI. Stage {{SERIES}} Implementation Validation
{{If Stage 4+: Validation against implementation specifications}}

### Phase Alignment
[How well execution aligns with planned phases]

### Feature Completeness
[Implementation completeness assessment]

## VII. Key Findings & Insights
### Strengths
[What worked well]

### Areas for Improvement
[What needs enhancement]

### Unexpected Discoveries
[Surprising findings]

## VIII. Recommendations
### Immediate Actions
[Quick wins and fixes]

### Framework Enhancements
[Longer-term improvements]

### Process Improvements
[Workflow and process recommendations]

## IX. Pattern Library Contributions
### New Patterns Identified
[Patterns to add to library]

### Pattern Refinements
[Updates to existing patterns]

## X. Success Metrics
### Achievement Summary
- ✅ [Achieved metrics]
- ⚠️ [Partial achievements]
- ❌ [Missed targets]

## XI. Next Steps
### For Next Iteration
[Specific guidance for next iteration]

### For Framework Evolution
[Strategic recommendations]

## Appendix: Technical Details
### Collection Information
- Script Version: {{version}}
- Collection Date: {{date}}
- Repository: {{repo_details}}
- Artifacts: {{sizes}}

### Analysis Methodology
- Comparison Base: {{previous_iterations}}
- Reference Specs: {{spec_documents}}
- Validation Criteria: {{criteria}}

---
*This analysis completes the evaluation of Stage {{SERIES}} Iteration {{ITERATION}}, providing actionable insights for continued framework evolution.*
