---
name: hc-session-retrospective
description: Produce a structured retrospective report summarizing decisions, learnings, and next steps from a session.
metadata:
  owner: holicode
  scope: retrospective-report
  origin: workflows/session-retrospective.md
---

# Context Report - Conversation Retrospective

Generate an executive detailed report capturing all valuable insights, learnings, and meta-conclusions from a conversation or task completion.

## Report Structure

### 1. Executive Summary
<ask_followup_question>
<question>Please provide a brief summary of what this conversation/task was about (2-3 sentences), key priorities from your (user) point of view</question>
<options>
["Just refer to task definition and carefully follow the conversation", "I'll provode requested information"]
</options>

</ask_followup_question>

### 2. Conversation Metadata
```
---
Date: $(date -u +%Y-%m-%dT%H:%M:%SZ)
Conversation Duration: [estimated from timestamps]
Primary Task Type: [e.g., state initialization, bug fix, feature development]
Participants: Human Developer, AI Assistant
---
```

### 3. Initial Context & Objectives
Document the starting state:
- What was the initial request/task?
- What were the explicit and implicit objectives?
- What context was available at the start?
- What assumptions were made?

### 4. Actions Performed
Create a chronological summary of key actions:

```markdown
## Actions Timeline
1. **[Timestamp]** - [Action description]
   - Type: [e.g., Technical (tool use), Decision, Discussion, Clarification, Analysis]
   - Tool used (if applicable): [e.g., read_file, write_to_file, execute_command]
   - Reasoning/Context: [Why this action was taken]
   - Outcome: [Success/Challenge/Pivot]
   
2. **[Timestamp]** - [Action description]
   - Type: [action type]
   - Tool used (if applicable): [tool name or "Discussion/Analysis"]
   - Reasoning/Context: [reasoning]
   - Outcome: [result]
```

### 5. Challenges & Resolutions
Document obstacles encountered and how they were resolved based on conversation analysis.

### 6. Key Decisions & Rationale
Capture important decisions made during the conversation:
- What alternatives were considered?
- Why were specific approaches chosen?
- What trade-offs were accepted?

### 7. Learning & Insights
Extract valuable learnings for future reference:

#### Technical Learnings
- Tool limitations discovered
- Effective patterns identified
- Anti-patterns to avoid

#### Process Learnings
- Workflow improvements needed
- Communication patterns that worked well
- Areas needing clarification

#### Meta-Observations
- Framework implications
- Broader patterns recognized
- Systemic improvements suggested

### 8. Deliverables & Outcomes
List all concrete outputs from the conversation:
- Files created/modified
- Documentation generated
- State changes
- Process improvements identified

### 9. Impact Assessment
Evaluate the conversation's impact:
- Immediate value delivered
- Long-term implications
- Dependencies created or resolved
- Technical debt introduced or paid down

### 10. Follow-up Actions
AI identifies pending items or future work from conversation analysis, then requests user input.

### 11. Recommendations for Framework Evolution
Based on this conversation, suggest improvements:
- Workflow enhancements
- Tool improvements needed  
- Documentation gaps identified
- Process optimizations

## User Input Collection

The AI should present their analysis and ask for additional user perspective in a single response:

```markdown
Based on my analysis of our conversation, I've identified the following key points:

**Executive Summary**: [AI's summary]
**Key Challenges**: [AI's identified challenges] 
**Follow-up Actions**: [AI's identified next steps]
**Framework Recommendations**: [AI's suggestions]

Please provide any additional perspective you'd like to add:
- Any priorities or nuances I may have missed?
- Additional challenges or successes you want to emphasize?
- Other follow-up actions or recommendations?
- Any meta-observations about this conversation process itself?
```

## Report Generation

Use write_to_file tool to generate the comprehensive report:

```markdown
# Conversation Context Report

**Generated**: [timestamp]
**Report Type**: Conversation Retrospective

## Executive Summary
[Combined AI analysis + user input]

## Task Overview
- **Primary Objective**: [objective]
- **Actual Outcome**: [outcome]
- **Success Metrics**: [metrics]

## Key Actions & Timeline
[Chronological actions with reasoning and outcomes]

## Challenges Faced
[Documented obstacles and resolutions]

## Critical Decisions
[Key decisions with rationale]

## Learnings & Insights
### Technical
[Technical learnings]

### Process  
[Process learnings]

### Meta-Observations
[Framework implications and patterns]

## Deliverables
[Concrete outputs]

## Impact Assessment
- **Immediate Impact**: [immediate value]
- **Long-term Value**: [future value]
- **Framework Evolution**: [implications]

## Next Steps
[Follow-up actions]

## Recommendations
[Framework improvements]

---
*This report captures the complete context and learnings from the conversation for future analysis and framework improvement.*
```

## Usage Examples

### Example from Current Conversation

```markdown
## Executive Summary
This conversation focused on initializing and validating HoliCode framework state files, discovering the importance of content validation beyond technical checks, and establishing patterns for comprehensive conversation retrospectives.

## Key Learnings
1. State validation requires both technical and content analysis
2. Interactive refinement with human guidance catches nuances automation misses
3. Real-time documentation of process insights preserves valuable learnings
4. Conversation retrospectives themselves are valuable framework artifacts

## Deliverables
- 7 updated state files with corrected timestamps
- Comprehensive validation process documentation
- New workflow for conversation retrospectives
```

## Integration with HoliCode

<ask_followup_question>
<question>Should this report be automatically triggered at conversation end, or manually invoked? Should it update any state files?</question>
<options>
["Auto-trigger with option to skip", "Manual invocation only", "Auto-trigger and update retro-inbox", "Manual with state update option"]
</options>
</ask_followup_question>

## Storage & Analysis

Reports should be stored in format `conversation-YYYYMMDD-HHMMSS_<workflow-name_OR_main_topic>.md`:
```
.holicode/analysis/reports/
├── conversation-20250725-001234_<workflow-name>.md
├── conversation-20250725-134567_<workflow-name>.md
└── ...
```

For batch analysis:
```bash
# Analyze patterns across multiple conversations
find .holicode/analysis/reports -name "*.md" -exec grep -H "Meta-Observations" {} \; > meta-patterns.txt
```

## Quality Checklist
- [ ] Executive summary is clear and concise
- [ ] Timeline is chronological and complete
- [ ] Challenges are honestly documented
- [ ] Learnings are actionable
- [ ] Recommendations are specific
- [ ] Report is self-contained for future reference
