---
name: inbox-process
description: Process awareness-inbox and retro-inbox entries — classify, route, codify, rotate, archive.
---

# Inbox Lifecycle Processing

## Agent Identity
Role: Inbox Lifecycle Processor — classifies, routes, codifies, and rotates inbox entries to keep state files healthy.
Responsibilities:
- Read and classify each entry in awareness-inbox.md and/or retro-inbox.md
- Route entries to appropriate destinations (state files, tracker issues, patterns, archive)
- Codify retro-inbox patterns into durable knowledge artifacts
- Rotate oversized inboxes by archiving processed entries
- Present classification proposals to user for approval before acting (interactive)

Success Criteria:
- All entries classified with a disposition (route / archive / keep)
- Routed entries appear in their target destinations
- Processed entries removed from active inbox and preserved in archive
- Post-processing file sizes within thresholds (awareness <15KB, retro <20KB)
- No data loss — every processed entry preserved in archive or destination

## Mode & Boundaries
- Mode: STATE_MAINTENANCE (reads/writes state files, creates tracker issues, creates archive files)
- Guardrails:
  - Do NOT modify `src/**` code files
  - Do NOT create specification artifacts (specs, stories, TDs)
  - Do NOT execute implementation workflows — only create tracker issues for actionable items
  - Do NOT auto-process without user confirmation of classifications
  - PRESERVE all processed entries in archive before removing from inbox
  - RESPECT zone markers in state files (GENERATED zones: skip; APPEND-ONLY zones: append at top)

## Definition of Ready (DoR)
- [ ] At least one inbox file exists and is non-empty
- [ ] `.holicode/state/` files accessible (activeContext.md, techContext.md, progress.md)
- [ ] Issue tracker accessible (for creating issues from actionable items)
- [ ] Target scope confirmed: `awareness` | `retro` | `both` (default: `both`)

<validation_checkpoint type="dor_gate">
**DoR Self-Assessment**

1. **Inbox File(s) Present**
   - Status: YES / NO
   - awareness-inbox.md: {{awareness_exists}}
   - retro-inbox.md: {{retro_exists}}

2. **State Files Accessible**
   - Status: YES / NO

3. **Issue Tracker Available**
   - Status: YES / NO
   - Provider: {{tracker_provider}}

4. **Scope Confirmed**
   - Status: YES / NO
   - Target: {{target_scope}}

**DoR Compliance**: _/4 criteria met
**Proceed?**: If <4, resolve gaps
</validation_checkpoint>

## Definition of Done (DoD)
- [ ] All entries classified and dispositioned (none left unclassified)
- [ ] Routed entries written to their destination files
- [ ] Tracker issues created for actionable items (issue IDs recorded)
- [ ] Archive file(s) created for processed entries
- [ ] Inbox files trimmed to contain only unprocessed entries + structural skeleton
- [ ] File sizes checked against thresholds (awareness <15KB, retro <20KB)
- [ ] State files updated: activeContext.md -> retro-inbox.md -> progress.md (in order)

---

## Process

### Step 1: Load Inbox State and Determine Scope

Read both inbox files. Compute current file sizes and entry counts.

```bash
# Measure inbox sizes
stat -c "%s" .holicode/state/awareness-inbox.md 2>/dev/null || echo "0"
stat -c "%s" .holicode/state/retro-inbox.md 2>/dev/null || echo "0"
```

Parse entry boundaries:
- **awareness-inbox.md**: Entries delimited by `## YYYY-MM-DD:` headers or `---` separators
- **retro-inbox.md**: Entries delimited by `### YYYY-MM-DD:` headers. Also contains structural sections (`## Pattern Library`, `## Key Learnings`, `## Action Items`, etc.) that are processed differently.

Report status:

```yaml
inbox_status:
  awareness_inbox:
    size_kb: {{size_kb}}
    threshold_kb: 15
    over_threshold: {{true/false}}
    entry_count: {{count}}
  retro_inbox:
    size_kb: {{size_kb}}
    threshold_kb: 20
    over_threshold: {{true/false}}
    entry_count: {{count}}
```

Present scope selection to user:

<ask_followup_question>
<question>Inbox status:
- awareness-inbox: {{size_kb}}KB (threshold 15KB) — {{entry_count}} entries — {{over/under}}
- retro-inbox: {{size_kb}}KB (threshold 20KB) — {{entry_count}} entries — {{over/under}}

Which inbox(es) should I process?</question>
<options>["Both inboxes", "Awareness-inbox only", "Retro-inbox only"]</options>
</ask_followup_question>

### Step 2: Awareness-Inbox Classification

_Skip if scope is "retro only"._

Parse each awareness-inbox entry. For each entry, apply the Classification Algorithm (see below) to assign a disposition.

Present the full classification table to the user for approval BEFORE taking any routing action:

```markdown
| # | Entry (Date: Title) | Classification | Proposed Destination | Confidence |
|---|---------------------|----------------|---------------------|------------|
| 1 | {{date}}: {{title}} | {{classification}} | {{destination}} | {{confidence}} |
| 2 | ... | ... | ... | ... |
```

<ask_followup_question>
<question>Here is the classification for {{entry_count}} awareness-inbox entries:

{{classification_table}}

Approve this classification?</question>
<options>["Yes, proceed with routing", "Adjust specific entries (I'll specify)", "Reclassify all"]</options>
</ask_followup_question>

### Step 3: Awareness-Inbox Routing

For each confirmed classification, execute the routing action:

- **actionable**: Create a tracker issue using the configured issue-tracker provider. Extract the core request as issue title and relevant detail as description. Record the created issue ID.
- **knowledge**: Append relevant content to the appropriate state file using the routing table:
  - Technical knowledge (APIs, tools, configs) -> `techContext.md`
  - Architecture patterns, conventions -> `systemPatterns.md`
  - Business context, user needs -> `productContext.md`
  - Infrastructure details -> `techContext.md`
- **learning**: Append the entry to `retro-inbox.md` (at the top of the dated entries section, preserving reverse-chronological order).
- **stale**: Move to archive directly (no routing needed).
- **mixed**: Decompose into sub-items. Classify and route each sub-item individually.

After routing each entry, mark it as processed.

### Step 4: Retro-Inbox Processing

_Skip if scope is "awareness only"._

Retro-inbox requires a different processing model due to its structural complexity.

#### 4a. Scan for Codifiable Patterns

Identify entries describing reusable patterns, architectural decisions, or process improvements that should be elevated to durable knowledge:

- **Patterns** -> Create or update files in `.holicode/patterns/` or `docs/patterns/`
- **Process improvements** -> Create tracker issues or update relevant workflow documentation
- **Architectural decisions** -> Create or update records in `.holicode/analysis/decisions/`

#### 4b. Scan for Unresolved Action Items

Find unresolved items: `- [ ]` checkboxes, "Action Items" sections, "TODO", "NEEDED" markers.

For each unresolved item:
- If still relevant -> create a tracker issue
- If already addressed elsewhere -> mark as resolved and archive

#### 4c. Classify Remaining Entries

For each dated entry and structural section:

- **codified**: Pattern extracted and preserved elsewhere — safe to archive
- **actionable**: Unresolved action item — create tracker issue, then archive entry
- **reference**: Historical value, fully processed — archive
- **stale**: No longer relevant — archive
- **keep**: Still actively referenced or too recent to process — leave in inbox

Present retro-inbox classification table for user approval (same interactive pattern as Step 2).

### Step 5: Rotation Check and Archive

After processing, check whether post-processing sizes are within thresholds:

```yaml
rotation_check:
  awareness_inbox:
    pre_process_kb: {{pre_size}}
    post_process_kb: {{post_size}}
    threshold_kb: 15
    needs_forced_rotation: {{true/false}}
  retro_inbox:
    pre_process_kb: {{pre_size}}
    post_process_kb: {{post_size}}
    threshold_kb: 20
    needs_forced_rotation: {{true/false}}
```

**Rotation strategy:**
1. Processing itself reduces size (entries removed and archived)
2. If post-processing size is STILL above threshold, perform forced rotation:
   - Identify the oldest N entries needed to bring size below threshold
   - Present forced rotation list to user for approval
   - Archive those entries

**Archive file creation:**
- Destination: `.holicode/analysis/archive/`
- Naming: `{inbox-type}-archive-{YYYY-MM-DD}.md`
  - Example: `awareness-inbox-archive-2026-02-16.md`
  - Example: `retro-inbox-archive-2026-02-16.md`
- If same-day collision: append sequence number (e.g., `retro-inbox-archive-2026-02-16-02.md`)

**Archive file format:**

```markdown
---
mb_meta:
  projectID: "{{projectID}}"
  sourceFile: "{{awareness-inbox.md or retro-inbox.md}}"
  archiveDate: "{{ISO_DATE}}"
  entryDateRange: "{{oldest_date}} to {{newest_date}}"
  entryCount: {{count}}
  reason: "inbox-lifecycle-processing"
---

# {{Inbox Type}} Archive — {{ISO_DATE}}

Archived by inbox-process workflow. Entries classified, routed, and preserved.

## Processing Summary

| # | Entry | Classification | Destination |
|---|-------|----------------|-------------|
| 1 | {{date}}: {{title}} | {{classification}} | {{destination_or_archive}} |

---

## Archived Entries

{{full text of archived entries, preserving original markdown formatting}}
```

After archive creation, update the active inbox file:
- Remove archived entries
- Preserve the YAML frontmatter and structural skeleton
- Preserve any entries classified as "keep"

### Step 6: State Updates

Follow the standard State Update Write-Path:

1. **activeContext.md**: APPEND to `## Recent Changes` (APPEND-ONLY zone):
   ```
   - [{{ISO_DATE}} HOL-20] Inbox lifecycle processing: {{N}} awareness entries, {{M}} retro entries processed, {{K}} issues created
   ```

2. **retro-inbox.md**: Add brief learning entry ONLY if the processing itself yielded a process improvement insight. Do not add a routine entry.

3. **progress.md**: APPEND to `Current Milestones — IN PROGRESS` (APPEND-ONLY zone):
   ```
   ### Inbox Lifecycle Processing ({{ISO_DATE}}, HOL-20) — COMPLETED
   -   Processed {{N}} awareness-inbox entries, {{M}} retro-inbox entries
   -   Created {{K}} tracker issues, archived {{A}} entries
   -   Post-processing sizes: awareness {{X}}KB, retro {{Y}}KB
   ```

### Step 7: Completion Checkpoint

<validation_checkpoint type="dod_compliance">
**Inbox Processing DoD Self-Assessment**

1. **All entries classified**
   - Awareness entries: {{classified}}/{{total}}
   - Retro entries: {{classified}}/{{total}}
   - Status: YES / NO

2. **Routing complete**
   - Tracker issues created: {{issue_ids}}
   - State file updates: {{count}} updates
   - Pattern files created: {{count}}
   - Status: YES / NO

3. **Archive files created**
   - Files: {{archive_file_list}}
   - Status: YES / NO

4. **Inbox files trimmed**
   - awareness-inbox post-size: {{size_kb}}KB (threshold: 15KB)
   - retro-inbox post-size: {{size_kb}}KB (threshold: 20KB)
   - Status: YES / NO

5. **State files updated**
   - activeContext: YES / NO
   - retro-inbox: YES / NO (if applicable)
   - progress: YES / NO

**Overall DoD Compliance**: _/5 criteria met
**Proceed to completion?**: If <5, resolve gaps
</validation_checkpoint>

---

## Classification Algorithm

### Awareness-Inbox Classifications

```yaml
classification_rules:
  actionable:
    signals:
      - Contains explicit action items ("must", "should", "need to", "requires", "TODO")
      - Describes a problem that needs a fix or feature
      - Contains unresolved blockers or issues
      - References work that has not been started
    destination: "Create tracker issue via issue-tracker skill"

  knowledge:
    signals:
      - Documents how something works (API, infrastructure, tool, convention)
      - Records configuration details, commands, patterns
      - Captures environment or architecture information
      - Describes established decisions or conventions
    routing_table:
      technical_knowledge: "techContext.md"
      architecture_patterns: "systemPatterns.md"
      business_context: "productContext.md"
      infrastructure_details: "techContext.md"
      tool_configurations: "techContext.md"
      conventions: "systemPatterns.md"

  learning:
    signals:
      - Reflects on what went well or poorly
      - Identifies improvement opportunities
      - Contains meta-observations about process
      - Describes a mistake or unexpected outcome
    destination: "retro-inbox.md (append at top of dated entries)"

  stale:
    signals:
      - Information is superseded by newer entries
      - References completed work with no ongoing relevance
      - Duplicates information already in state files
      - More than 30 days old with no pending actions
    destination: "Archive directly"

  mixed:
    signals:
      - Entry contains multiple types of content
    handling: "Decompose into sub-items, classify each individually"
```

### Retro-Inbox Classifications

```yaml
retro_classification_rules:
  codified:
    signals:
      - Pattern has been extracted to a durable location
      - Learning has been integrated into workflow or state files
      - Decision has been recorded in analysis/decisions/
    destination: "Archive (knowledge preserved elsewhere)"

  actionable:
    signals:
      - Contains unresolved [ ] checkboxes
      - References "TODO", "NEEDED", "Action Items"
      - Describes work not yet completed
    destination: "Create tracker issue, then archive entry"

  reference:
    signals:
      - Historical milestone documentation
      - Completed work retrospective
      - Fully processed — no pending actions
    destination: "Archive"

  stale:
    signals:
      - Superseded by newer decisions or approaches
      - References tools, patterns, or approaches no longer used
      - More than 90 days old with no ongoing relevance
    destination: "Archive"

  keep:
    signals:
      - Less than 14 days old
      - Still actively referenced by current work
      - Contains patterns not yet codified
    destination: "Leave in inbox"
```

### Confidence Levels

```yaml
confidence:
  high: "Clear signals, single classification applies"
  medium: "Some ambiguity, but best classification is identifiable"
  low: "Multiple classifications could apply — flag for user decision"
```

Entries classified with `low` confidence are highlighted in the classification table for explicit user guidance.

---

## Rotation Strategy

```yaml
rotation:
  sequence: "process-then-rotate"
  rationale: "Processing naturally reduces size; forced rotation only if still over threshold"

  thresholds:
    awareness_inbox:
      max_size_kb: 15
      max_entry_count: 20     # advisory, not hard limit
    retro_inbox:
      max_size_kb: 20
      max_entry_count: 30     # advisory, not hard limit

  forced_rotation:
    trigger: "Post-processing size still exceeds threshold"
    strategy: "Archive oldest entries until size is below threshold"
    approval: "Present forced rotation list to user before executing"

  archive_destination: ".holicode/analysis/archive/"
  archive_naming: "{inbox-type}-archive-{YYYY-MM-DD}.md"
  collision_handling: "Append sequence number: -02, -03, etc."
```

---

## Error Handling

- **Inbox file not found**: Skip that inbox, process the other if in scope. Warn user.
- **Tracker unavailable**: Collect actionable items as a list. Present to user with recommendation to create issues manually or retry later. Do not block the rest of the workflow.
- **Entry boundary ambiguous**: When an entry's start/end is unclear, present the ambiguous text to the user and ask for boundary confirmation.
- **Mixed entry decomposition unclear**: Present the full entry and ask user to identify sub-items.
- **Archive directory missing**: Create `.holicode/analysis/archive/` automatically.
- **State file write conflict**: Follow zone rules strictly. If an APPEND-ONLY section has unexpected content, append at top without modifying existing entries.
- **Very large inbox (>50KB)**: Process in batches of 10 entries at a time to keep user interaction manageable.

---

## Integration Points

### Input Sources
- `.holicode/state/awareness-inbox.md` — primary input
- `.holicode/state/retro-inbox.md` — primary input
- `.holicode/state/techContext.md` — for tracker provider and routing context
- `.holicode/state/activeContext.md` — for current work context
- `.holicode/state/progress.md` — for completion tracking context

### Output Targets
- `.holicode/state/techContext.md` — knowledge routing target
- `.holicode/state/systemPatterns.md` — knowledge routing target
- `.holicode/state/productContext.md` — knowledge routing target
- `.holicode/state/retro-inbox.md` — learning routing target + state update
- `.holicode/state/activeContext.md` — state update
- `.holicode/state/progress.md` — state update
- `.holicode/analysis/archive/` — archive files
- `.holicode/patterns/` — codified pattern files (from retro processing)
- `docs/patterns/` — codified pattern files (from retro processing)
- Issue tracker — new issues from actionable items

---

## Core Workflow Standards Reference
This workflow follows the Core Workflow Standards defined in holicode.md:
- State Maintenance Mode: reads/writes state files and creates tracker issues
- Generic Workflows, Specific Specifications principle
- DoR/DoD gates enforcement
- State Update Write-Path: activeContext -> retro-inbox -> progress
- Zone-Based Update Rules (HOL-34): respects GENERATED and APPEND-ONLY markers
- Interactive classification with user approval before routing
