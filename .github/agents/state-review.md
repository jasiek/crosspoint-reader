---
name: state-review
description: Perform a comprehensive review of all HoliCode state files and summarize project status.
---

# Comprehensive HoliCode State Review

Systematic review of all state files following the structured review template from original memory bank methodology.

## Load All State Context

<read_file>
<path>.holicode/state/projectbrief.md</path>
</read_file>

<read_file>
<path>.holicode/state/productContext.md</path>
</read_file>

<read_file>
<path>.holicode/state/systemPatterns.md</path>
</read_file>

<read_file>
<path>.holicode/state/techContext.md</path>
</read_file>

<read_file>
<path>.holicode/state/activeContext.md</path>
</read_file>

<read_file>
<path>.holicode/state/progress.md</path>
</read_file>

## Project Status Overview

### Current State Summary
- **According to [progress.md]**: {{extract_completion_status}}
- **According to [activeContext.md]**: {{extract_current_focus}}
- **According to [projectbrief.md]**: {{extract_objectives_summary}}

### Business Alignment Check
- **Vision alignment**: {{check_vision_alignment}}
- **Success metrics tracking**: {{check_metrics_progress}}
- **User needs satisfaction**: {{check_user_needs}}

## Recent Developments Analysis

### Timeline Analysis
```bash
# Check file modification times
find .holicode/state -name "*.md" -exec stat -c "%Y %n" {} \; | sort -nr
```

- **Most recent update**: {{most_recent_file}} at {{timestamp}}
- **Update frequency**: {{update_pattern_analysis}}
- **Staleness check**: {{files_not_updated_recently}}

### Key Changes Summary
- **Architectural changes**: {{architectural_updates}}
- **Scope modifications**: {{scope_changes}}
- **Technical decisions**: {{tech_decisions}}
- **Process improvements**: {{process_changes}}

## Documentation Quality Assessment

### Consistency Analysis
- **Cross-file consistency**: {{consistency_rating}}
  - projectbrief.md ↔ productContext.md: {{consistency_check_1}}
  - systemPatterns.md ↔ techContext.md: {{consistency_check_2}}
  - activeContext.md ↔ progress.md: {{consistency_check_3}}

- **Information completeness**: {{completeness_assessment}}
  - Missing information gaps: {{identified_gaps}}
  - Underspecified areas: {{underspecified_areas}}

### Citation and Reference Quality
- **Internal references**: {{internal_ref_check}}
- **External dependencies**: {{external_ref_check}}
- **Link validity**: {{broken_links_check}}

## Risk Assessment

### Technical Risks
Based on systemPatterns.md and techContext.md:
- **Architecture risks**: {{architectural_risks}}
- **Technology risks**: {{technology_risks}}
- **Performance risks**: {{performance_risks}}
- **Security considerations**: {{security_risks}}

### Project Risks
Based on activeContext.md and progress.md:
- **Delivery risks**: {{delivery_risks}}
- **Resource constraints**: {{resource_risks}}
- **Dependency risks**: {{dependency_risks}}
- **Quality risks**: {{quality_risks}}

### Documentation Risks
- **Knowledge gaps**: {{knowledge_gaps}}
- **Single points of failure**: {{spof_knowledge}}
- **Outdated information**: {{outdated_info_risks}}

## Recommended Next Steps

### Immediate Actions (Next 1-2 sprints)
1. **{{priority_1_action}}** - {{priority_1_rationale}}
2. **{{priority_2_action}}** - {{priority_2_rationale}}
3. **{{priority_3_action}}** - {{priority_3_rationale}}

### Medium-term Objectives (Next month)
1. **{{medium_term_1}}** - {{mt_1_rationale}}
2. **{{medium_term_2}}** - {{mt_2_rationale}}

### Strategic Improvements
1. **Process improvements**: {{process_recommendations}}
2. **Documentation improvements**: {{doc_recommendations}}
3. **Architecture improvements**: {{arch_recommendations}}

## State Health Score

### Overall Assessment
- **Health Score**: {{calculated_health_score}}/10
- **Confidence Level**: {{confidence_assessment}}
- **Readiness for Scale**: {{scale_readiness}}

### Component Scores
- **Vision Clarity**: {{vision_score}}/10
- **Technical Soundness**: {{technical_score}}/10
- **Documentation Quality**: {{documentation_score}}/10
- **Process Maturity**: {{process_score}}/10

## Review Report Generation

```bash
# Generate formal review report
cat > .holicode/review/report-$(date +%Y%m%d).md << EOF
# HoliCode State Review Report - $(date +%Y-%m-%d)

## Executive Summary
- **Overall Health**: {{health_score}}/10
- **Key Strengths**: {{key_strengths}}
- **Critical Issues**: {{critical_issues}}
- **Recommended Focus**: {{recommended_focus}}

## Detailed Findings
{{detailed_findings_summary}}

## Action Items
{{prioritized_action_items}}

## Next Review**: {{next_review_date}}
EOF
```

<ask_followup_question>
<question>Review complete. Should I:
1. Execute any immediate recommended actions
2. Create tasks for identified improvements  
3. Update any state files based on findings
4. Schedule follow-up reviews</question>
</ask_followup_question>
```
