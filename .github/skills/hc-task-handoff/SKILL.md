---
name: hc-task-handoff
description: Create a structured handoff between tasks/conversations using the handoff template.
metadata:
  owner: holicode
  scope: handoff-template
  origin: workflows/task-handoff.md
---

# Create Task Handoff

Generate structured handoff between conversations or team members.

<read_file>
<path>.holicode/templates/handoff/handoff-template.md</path>
</read_file>

<ask_followup_question>
<question>Handoff details:
- From: (conversation/person)
- To: (conversation/person)  
- Task: (brief description)
- Context files needed: (list)
- Success criteria: (how to know it's done)
</question>
</ask_followup_question>

## Create Handoff File
[Generate handoff using template with provided details]

## Update Active Context
[Add handoff to activeContext.md active handoffs list]

## Archive Management
[Instructions for archiving completed handoffs]
