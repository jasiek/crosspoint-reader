---
name: hc-state-health-check
description: Validate HoliCode state files for completeness, consistency, and correct cross-references.
metadata:
  owner: holicode
  scope: state-health
  origin: workflows/state-health-check.md
---

# HoliCode State Health Check

Comprehensive validation following systematic checklist approach.

## Metadata Validation
```bash
# Check all state files have consistent metadata
find .holicode/state -name "*.md" -exec grep -l "mb_meta:" {} \;
```

### Metadata Consistency Check
- [ ] All files have consistent `projectID` and `version` fields
- [ ] `lastUpdated` timestamps are logically sequenced  
- [ ] `planVersion` is consistent across all documents
- [ ] `completion` percentages align with described status

## Cross-Reference Validation
### Link Validation
- [ ] All document references (e.g., "See techContext.md") resolve correctly
- [ ] No circular dependencies or contradictory information
- [ ] Implementation plan references match actual files

### Content Consistency
- [ ] Information between documents is consistent
- [ ] No conflicting status reports or completion percentages
- [ ] Architectural decisions align across systemPatterns.md and techContext.md

## Content Validation
### Comprehensive Content Analysis

#### Input-Output Alignment
- [ ] **Source Material Coverage**: Are all key concepts from input sources represented in outputs?
- [ ] **Transformation Quality**: Has information been appropriately synthesized, not just copied?
- [ ] **Priority Preservation**: Are the most important aspects from inputs given appropriate weight?
- [ ] **Context Preservation**: Is essential context maintained through the transformation?

#### Content Coherence & Quality
- [ ] **Internal Consistency**: Do all parts of the content align and support each other?
- [ ] **Logical Flow**: Is there a clear narrative and logical progression?
- [ ] **Appropriate Depth**: Is the level of detail suitable for the intended purpose and audience?
- [ ] **Clarity of Purpose**: Is it clear what each piece of content is meant to achieve?

#### Actionability & Utility
- [ ] **Decision Support**: Does the content enable informed decision-making?
- [ ] **Clear Next Steps**: Are future actions and priorities evident?
- [ ] **Practical Application**: Can the content be directly applied to work?
- [ ] **Knowledge Gaps**: Are unknowns and assumptions clearly identified?

#### Temporal & Evolutionary Aspects
- [ ] **Currency**: Is the content reflecting the current state or outdated information?
- [ ] **Evolution Tracking**: Can you see how the content has evolved over time?
- [ ] **Future-Oriented**: Does content consider upcoming needs and changes?
- [ ] **Historical Context**: Is past context preserved where valuable?

#### Meta-Level Considerations
- [ ] **Abstraction Balance**: Is there appropriate balance between high-level concepts and specific details?
- [ ] **Pattern Recognition**: Are recurring themes and patterns identified and leveraged?
- [ ] **Cross-Domain Integration**: Are insights from different areas properly connected?
- [ ] **Learning Capture**: Are lessons learned and insights properly documented?

## Content Freshness Validation
### Timestamp Analysis
```bash
# Find most recently updated file
find .holicode/state -name "*.md" -exec stat -c "%Y %n" {} \; | sort -nr | head -1
```

- [ ] Identify most recently modified file and changes
- [ ] Flag any files not updated in last 14 days
- [ ] Verify activeContext.md reflects current focus
- [ ] Confirm progress.md completion percentages match current state
- [ ] **Timestamp Recency**: Check lastUpdated timestamps are within reasonable bounds
- [ ] **State Alignment**: Verify activeContext narrative aligns with progress metrics

## WORK_SPEC Manifest Integrity Checks
- [ ] **Link Resolution**: All links in WORK_SPEC.md resolve to existing files
- [ ] **Orphan Detection**: Identify references to non-existent specification files
- [ ] **Bidirectional Linking**: Verify specs reference WORK_SPEC.md correctly
- [ ] **Hierarchy Validation**: Feature → Story → Task linking is consistent
- [ ] **ID Uniqueness**: No duplicate IDs across specification chunks

## Spec Hygiene Quick-Check
- [ ] Run spec-sync advisory to report link/format/ID issues
- [ ] Scan WORK_SPEC.md for orphaned chunks (references to non-existent files)
- [ ] Verify all spec files follow SCHEMA.md requirements
- [ ] Check for missing required sections in specs

## Inbox Health & Spec-Backfill Monitoring

### Inbox Size Thresholds
```bash
# Check inbox file sizes (compare raw bytes for accuracy, display KB for readability)
AWARENESS_SIZE=$(stat -c "%s" .holicode/state/awareness-inbox.md 2>/dev/null || echo "0")
RETRO_SIZE=$(stat -c "%s" .holicode/state/retro-inbox.md 2>/dev/null || echo "0")
AWARENESS_THRESHOLD=$((15 * 1024))  # 15KB in bytes
RETRO_THRESHOLD=$((20 * 1024))      # 20KB in bytes

# Display with one decimal for clarity (avoids integer truncation masking near-limit files)
echo "awareness-inbox.md: $(awk "BEGIN {printf \"%.1f\", ${AWARENESS_SIZE}/1024}")KB / 15KB — $([ "$AWARENESS_SIZE" -gt "$AWARENESS_THRESHOLD" ] && echo "OVER" || echo "OK")"
echo "retro-inbox.md: $(awk "BEGIN {printf \"%.1f\", ${RETRO_SIZE}/1024}")KB / 20KB — $([ "$RETRO_SIZE" -gt "$RETRO_THRESHOLD" ] && echo "OVER" || echo "OK")"
```

- [ ] **awareness-inbox.md** size is below 15KB (15360 bytes) threshold
  - If over threshold: Flag as **NEEDS ROTATION** — run `/inbox-process` workflow
  - Current size: {{AWARENESS_SIZE}} bytes ({{AWARENESS_KB}}KB)
- [ ] **retro-inbox.md** size is below 20KB (20480 bytes) threshold
  - If over threshold: Flag as **NEEDS ROTATION** — run `/inbox-process` workflow
  - Current size: {{RETRO_SIZE}} bytes ({{RETRO_KB}}KB)
- [ ] Inbox entry counts are reasonable (awareness <20 entries, retro <30 entries)

### Spec-Backfill Outstanding Items
- [ ] No outstanding spec-backfill deferrals older than 7 days
- [ ] All rapid implementations have corresponding backfill completion entries
- [ ] No "backfill debt" entries in retro-inbox without corresponding tracker issues

### Inbox Processing Recency
- [ ] Last inbox processing date is within 14 days (check archive directory timestamps or activeContext `Recent Changes`)
- [ ] Archive directory (`.holicode/analysis/archive/`) exists and contains recent archives if inboxes have been rotated

## Validation Report Generation

Use write_to_file tool to create validation report at `.holicode/analysis/reports/validation-report-[YYYYMMDD].md`:

<write_to_file>
<path>.holicode/analysis/reports/validation-report-[YYYYMMDD].md</path>
<content>
# State Validation Report

**Generated**: {{ISO_DATE}}
**Report Type**: State Health Check

## Summary
- Files analyzed: N/A (Manual check)
- Issues detected: N/A (Manual check)
- Last updated: N/A (Manual check)

## Technical Validation Results
### Metadata Issues
[List metadata inconsistencies with file citations]

### Cross-Reference Issues
[List broken links, circular dependencies, contradictions]

### Inbox Health
- awareness-inbox.md: {{AWARENESS_SIZE}} bytes ({{AWARENESS_KB}}KB) / 15360 bytes (15KB) — {{OVER or OK}}
- retro-inbox.md: {{RETRO_SIZE}} bytes ({{RETRO_KB}}KB) / 20480 bytes (20KB) — {{OVER or OK}}
- Outstanding backfill items: {{count}}
- Last inbox processing: {{date or "never"}}

## Content Validation Results
[Apply the Comprehensive Content Analysis framework defined above and report findings]

## Content Freshness Analysis
- Most recent update: N/A (Manual check)
- Potentially outdated: N/A (Manual check)
- Context alignment: N/A (Manual check)

## Recommended Actions
[Prioritized list of suggested updates with rationale]

## Next Steps
[Immediate actions needed and follow-up validation schedule]

---
*This validation ensures state files effectively serve their purpose of maintaining project context and guiding development.*
</content>
</write_to_file>

<ask_followup_question>
<question>Validation complete. Should I address any identified issues immediately or create tasks for later resolution?</question>
</ask_followup_question>
