---
name: hc-intake-triage-sensor
description: Lightweight pre-triage sensor. Detects when user input is ambiguous, multi-concern, or needs routing — and recommends invoking the full intake-triage workflow.
compatibility: Designed for Claude Code, Codex, OpenCode, and Gemini skills format.
metadata:
  owner: holicode
  scope: input-classification
---

# Intake-Triage Skill (Pre-Triage Sensor)

This skill is a lightweight classifier that agents should consult early in a conversation when user input is ambiguous, unstructured, or potentially multi-concern. It determines whether the full `/intake-triage.md` workflow should be invoked.

## When to Use

Invoke this skill when ANY of these signals are present:

1. **Ambiguous scope**: The user's request could be a quick fix OR a large initiative — unclear which
2. **Multiple concerns**: The input contains more than one distinct request or topic
3. **Unstructured input**: Raw text, transcript, stream-of-consciousness, or meeting notes — for these, consider invoking the `data-ingestion` skill first to normalize and extract signals before assessment
4. **Contradictory constraints**: The request contains conflicting goals or requirements
5. **Unknown complexity**: You cannot confidently assess whether this is trivial or significant
6. **No active context match**: The request doesn't clearly relate to any in-progress work in `activeContext.md`

## When NOT to Use

Skip this skill when:

- The user explicitly names a workflow to run (e.g. "run business-analyze")
- The request is a clear, scoped implementation task with an existing tracker issue
- The user is continuing work on an already-triaged item
- The input is a simple question or information request (not actionable work)

## Pre-Triage Assessment

Perform a quick (< 30 seconds) assessment:

```yaml
pre_triage:
  signals:
    ambiguous_scope: true/false    # Could be trivial or significant
    multi_concern: true/false      # Contains 2+ distinct topics
    unstructured: true/false       # Raw/messy input format
    contradictory: true/false      # Conflicting requirements detected
    unknown_complexity: true/false # Cannot confidently size this
    no_context_match: true/false   # Doesn't match active work

  recommendation:
    if signal_count >= 2: "Invoke /intake-triage.md workflow"
    if signal_count == 1: "Flag the signal and ask one clarifying question"
    if signal_count == 0: "Proceed directly — no triage needed"
```

## Output

When recommending triage, present concisely:

> I notice this input has [signal descriptions]. I'd recommend running intake-triage to classify complexity and pick the right workflow. Want me to proceed with triage?

If the user declines, proceed with best-effort routing using the agent's own judgment.

## Relationship to Workflow

- **This skill**: Quick sensor — "should we triage?" (seconds)
- **`/intake-triage.md` workflow**: Full classifier/router — "what is this, how complex, where does it go?" (minutes)

The skill is the guard; the workflow is the engine.

### Relationship to Data Ingestion

- **`data-ingestion` skill**: Upstream normalizer — "clean up raw input and extract signals" (seconds)
- **This skill**: Quick sensor — "should we triage?" (seconds)
- When `unstructured: true` fires, invoking `data-ingestion` first produces a cleaner, signal-enriched input that improves triage accuracy.
