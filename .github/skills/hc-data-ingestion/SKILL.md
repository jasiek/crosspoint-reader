---
name: hc-data-ingestion
description: Pre-processor/normalizer for unstructured inputs. Detects format (transcript, chat dump, free text), extracts signals, and produces a normalized block for intake-triage consumption.
compatibility: Designed for Claude Code, Codex, OpenCode, and Gemini skills format.
metadata:
  owner: holicode
  scope: input-normalization
---

# Data Ingestion Skill

Normalizes raw unstructured inputs into a signal-enriched structured block before they reach intake-triage. This skill detects the input **format** (transcript, chat dump, free text) and extracts actionable signals — it does NOT classify the input **type** (brief, requirement, bug report, etc.), which remains intake-triage's responsibility.

## When to Use

Invoke this skill when ANY of these conditions apply:

1. **Raw meeting content**: Pasted meeting notes, Zoom/Teams transcript, standup recording output
2. **Chat export**: Slack, Teams, Discord, or messaging app copy-paste
3. **Stream-of-consciousness**: Personal notes, brainstorm dumps, voice memo transcriptions
4. **Mixed format paste**: Input that blends conversational and prose styles
5. **Noisy input**: Text with excessive metadata, timestamps, filler words, or formatting artifacts that would reduce triage accuracy

## When NOT to Use

Skip this skill when:

- The input is already a clean brief, requirement, or bug report (no normalization needed)
- The user explicitly names a workflow to run (e.g. "run business-analyze")
- The input is a simple question or information request (not actionable work)
- The input is structured markdown or YAML (already normalized)

## Scope Boundaries

- **Does**: Detect format, clean text, extract signals, flag gaps
- **Does NOT**: Classify `input_type` from the intake-triage enum (that is intake-triage's job)
- **Does NOT**: Route to workflows, create specs, create issues, or generate code
- **Does NOT**: Resolve ambiguities or make decisions — only surfaces them

## Input Format Detection

Scan the first 10-20 lines for structural markers to determine the primary format:

```yaml
format_detection:
  transcript:
    markers:
      - "Speaker labels (e.g. 'John:', '[Sarah]', 'Speaker 1:')"
      - "Timestamps (e.g. '[10:23]', '00:15:30')"
      - "Turn-taking patterns (alternating speakers)"
      - "Filler words ('um', 'so basically', 'you know')"
      - "Incomplete or fragmented sentences"
    sources: "Meeting recordings, Zoom/Teams transcripts, interview notes, standup recordings"

  chat_dump:
    markers:
      - "Username/handle prefixes (e.g. '@alice', 'alice:', '<alice>')"
      - "Short messages (typically < 280 chars per line)"
      - "Chat-style timestamps (e.g. '2:34 PM', '14:34')"
      - "Thread/reply indicators, emoji, reactions"
      - "Channel/room headers (e.g. '#engineering', '[General]')"
    sources: "Slack exports, Teams chat copies, Discord logs, iMessage/WhatsApp copies"

  free_text:
    markers:
      - "Absence of speaker labels or chat metadata"
      - "Paragraph-form prose"
      - "Personal voice ('I think', 'we should', 'my idea is')"
      - "Bullet lists or stream-of-consciousness without structural markers"
    sources: "Personal notes, voice memo transcriptions, brainstorm dumps, email body pastes"

  detection_rules:
    step_1: "Scan first 10-20 lines for structural markers"
    step_2: "Speaker/username labels present → transcript or chat_dump"
    step_3: "Timestamp patterns → refine transcript (long-form) vs chat_dump (short messages)"
    step_4: "Line length distribution → short lines suggest chat_dump"
    step_5: "No structural markers → free_text"
    step_6: "Mixed signals → assign primary format, note secondary in source metadata"

  confidence:
    high: "3+ format-specific markers found"
    medium: "1-2 markers found"
    low: "No clear markers — defaulting to free_text"
```

## Standard Procedure

### Step 1: Receive Input

Accept the raw input without rejection. This skill is designed to handle messy, noisy, and incomplete input.

### Step 2: Detect Format

Apply the format detection heuristics above. Assign `detected_format` and `format_confidence`.

### Step 3: Clean and Normalize

Apply format-specific cleaning while preserving semantic content:

- **transcript**: Standardize speaker labels to `[Name]:` format. Remove filler words that add no semantic value. Collapse repeated timestamps. Merge fragmented sentences from the same speaker into coherent statements.
- **chat_dump**: Strip platform-specific metadata (emoji reactions, thread indicators, read receipts). Standardize username format. Collapse rapid-fire messages from the same person into coherent blocks. Preserve chronological order.
- **free_text**: Minimal cleaning. Fix obvious transcription errors (for voice memos). Normalize bullet/list formatting. Preserve paragraph structure and personal voice.

### Step 4: Extract Signals

Scan the cleaned text for actionable signals:

- **Intent indicators**: Explicit requests ("we need", "can we", "let's build"), decisions ("decided to", "going with"), complaints ("broken", "doesn't work", "bug"), questions ("should we", "what if").
- **Urgency cues**: Time pressure ("by Friday", "ASAP", "blocking"), severity ("critical", "production down", "customers impacted"). Assign level only when explicit evidence exists.
- **Referenced entities**: People (names, roles, teams), systems (service names, APIs, tools), dates (deadlines, milestones), external parties (vendors, clients).
- **Topic clusters**: Group related statements under distinct topics. Each cluster becomes a potential "concern" for intake-triage decomposition.
- **Action items**: Explicit commitments or assignments mentioned in the input ("Sarah will handle X by Friday").

### Step 5: Identify Gaps

Flag what is missing or ambiguous — do NOT fill in gaps with assumptions:

- Ambiguous scope (could be small or large)
- Missing context (references to prior decisions without explanation)
- Contradictory statements (present both sides)
- Unclear ownership or priority
- Provide a suggested clarifying question for each gap

### Step 6: Assemble Output

Produce the `normalized_input` block per the Output Contract below.

### Step 7: Handoff Decision

- **If invoked standalone**: Present the normalized output and recommend: "This normalized input is ready for intake-triage. Invoke the `intake-triage` skill or `/intake-triage.md` workflow to classify and route."
- **If invoked by intake-triage**: Return the normalized block for the workflow to continue processing at its classification step.

## Output Contract

The skill produces a `normalized_input` YAML block:

```yaml
normalized_input:
  # Source metadata
  source:
    detected_format: "transcript | chat_dump | free_text"
    format_confidence: "high | medium | low"
    estimated_participants: ["name1", "name2"]  # empty list for free_text
    estimated_timespan: "~30 min meeting" | "3 days of chat" | null
    original_length: "~2400 words"

  # Cleaned content (primary payload for intake-triage)
  content:
    cleaned_text: |
      The full input text, cleaned of formatting noise but preserving
      semantic content. Speaker labels standardized, timestamps removed
      unless semantically relevant, chat metadata stripped.
    content_summary: "2-3 sentence summary of what this input is about"

  # Extracted signals (enrichment for intake-triage)
  signals:
    intent_indicators:
      - { signal: "decision needed", evidence: "Sarah said 'we need to decide by Friday'" }
    urgency_cues:
      - { level: "high | medium | low | none", evidence: "quoted text showing urgency" }
    referenced_entities:
      people: ["Sarah", "DevOps team"]
      systems: ["auth service", "billing API"]
      dates: ["Friday", "Q3 launch"]
      external: ["AWS", "Stripe"]
    topic_clusters:
      - { topic: "authentication refactor", evidence_range: "lines 12-45" }
      - { topic: "deployment timeline", evidence_range: "lines 50-62" }
    action_items:
      - { item: "Decide on SSO provider", owner: "Sarah", deadline: "Friday" }

  # Gaps and ambiguities (for intake-triage awareness)
  gaps:
    ambiguous_items:
      - { item: "description of ambiguity", suggested_clarification: "question to resolve it" }
    incomplete_items:
      - { item: "what seems missing", impact: "how this affects triage" }
    confidence_notes: "Overall assessment of input quality for triage purposes"
```

**How this feeds intake-triage:**
- `content.cleaned_text` → intake-triage Step 2 (classify input_type from enum)
- `signals.topic_clusters` → intake-triage `concern_count` determination
- `signals.referenced_entities` → complexity scoring (dependencies dimension)
- `signals.urgency_cues` → complexity scoring (risk dimension)
- `signals.intent_indicators` → input_type classification evidence
- `gaps` → intake-triage aware of limitations before scoring

## Ambiguity Handling

```yaml
ambiguity_handling:
  principle: "Flag, don't fabricate. Surface uncertainty, don't hide it."

  rules:
    - "Never invent participants, dates, or entities not present in the input"
    - "Never assign urgency unless explicit cues exist in the text"
    - "When format detection confidence is 'low', state the assumption explicitly"
    - "When a topic cluster could be 1 concern or 2, present both interpretations"
    - "When input is very short (< 50 words), note that signal extraction is limited"

  sparse_input_protocol:
    threshold: "< 50 words or < 3 sentences"
    action: |
      Produce minimal normalized output with:
      - content.cleaned_text: the input as-is (minimal cleaning)
      - content.content_summary: best-effort summary
      - signals: only what is explicitly present (may be mostly empty)
      - gaps.confidence_notes: "Input is sparse. Signal extraction limited.
        Consider asking for additional context before triage."

  conflicting_signals_protocol:
    action: |
      When the input contains contradictory statements:
      - Flag in gaps.ambiguous_items with both sides of the contradiction
      - In signals.intent_indicators, note the contradiction as a signal
      - Do NOT resolve the contradiction — that is intake-triage's job
        (it may classify as input_type: contradiction)
```

## Integration

### Composability

This skill is designed to work in two modes:

1. **Standalone**: User pastes raw text → agent consults `data-ingestion` → normalized output presented → agent recommends invoking `intake-triage`
2. **Chained with intake-triage**: The `intake-triage` skill detects `unstructured: true` → delegates to `data-ingestion` for normalization → continues with its classification step using the enriched output

### Relationship to Other Skills

- **`intake-triage` skill**: Downstream consumer. May invoke data-ingestion when unstructured input is detected.
- **`/intake-triage.md` workflow**: Downstream consumer. The normalized `content.cleaned_text` feeds its Step 2 (Classify Input). Signals provide pre-extracted evidence for complexity scoring.
- **`business-analyze` workflow**: Indirect consumer. When intake-triage routes transcripts to business-analyze, the normalized signals (topic clusters, action items) accelerate requirement extraction.

## Constraints

- Never assign `input_type` from the intake-triage enum — that is intake-triage's responsibility
- Never invent entities, urgency, dates, or participants not explicitly present in the input
- Never make routing decisions — only normalize and extract
- Preserve the original meaning and intent of the input during cleaning
- When uncertain, include more context rather than less (err toward preservation)
