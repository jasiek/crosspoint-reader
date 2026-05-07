# TD: `.index/` Front Matter Schema

**Issue**: HOL-389 (schema origin), HOL-508 (renamed from `.context/` → `.index/`)
**Status**: active
**Related**: HOL-358 (directory structure, originally `.context/`), HOL-363 (quality aspects framework), HOL-378 (meta-skills pattern), HOL-508 (rename + hard cutover)

## Overview

Defines the YAML front matter schema for all `.index/` file types. The `.index/` directory is a per-component mini memory bank that provides scoped knowledge for meta-skills and quality analysis workflows.

**Naming note**: Previously named `.context/`. Renamed to `.index/` in HOL-508 (2026-04-23) to avoid semantic collision with LLM "context" (agent instruction files, context window, activeContext state) and to align with the Karpathy LLM Wiki Query operation framing. Hard cutover — no `.context/` fallback.

## Directory Structure

```
src/Modules/{Name}/.index/
├── overview.md           # Component overview, architecture pattern, key decisions
├── dependencies.md       # Internal/external dependency map
├── domain-model.md       # Domain entities, aggregates, bounded context
└── aspects/              # Quality aspect files (HOL-363)
    ├── _general.md       # Default aggregated aspect (always exists)
    ├── security.md       # Split out when security is critical
    ├── performance.md    # Split out when perf matters
    └── business-rules.md # Split out when domain rules are complex
```

## Schema Definitions

### Common Fields

All `.index/` files share these base fields:

| Field | Type | Required | Allowed Values | Description |
|-------|------|----------|----------------|-------------|
| `confidence` | enum | yes | `scaffold`, `partial`, `complete`, `verified` | Analysis confidence level |
| `last_analyzed` | date | yes | ISO 8601 date (`YYYY-MM-DD`) | When the file was last analyzed/updated |
| `analyzed_by` | string | no | Free text (e.g., `human`, `action-analyze`, `mixed`) | Who/what performed the analysis |

#### Confidence Levels

- **`scaffold`**: Auto-generated stub, minimal or no analysis performed
- **`partial`**: Some sections analyzed, others still placeholder or incomplete
- **`complete`**: All sections filled with analysis, not yet peer-reviewed
- **`verified`**: Analysis reviewed and confirmed by human or secondary agent

### `overview.md`

The main entry point for a component's index. Includes an aspect index for quick discovery.

```yaml
---
confidence: partial
last_analyzed: 2026-03-26
analyzed_by: action-analyze
analysis_scope: full | incremental | targeted
aspects:
  - name: _general
    status: draft
    relevance: high
  - name: security
    status: not-analyzed
    relevance: medium
---
```

| Field | Type | Required | Allowed Values | Description |
|-------|------|----------|----------------|-------------|
| `confidence` | enum | yes | see Common Fields | |
| `last_analyzed` | date | yes | ISO 8601 | |
| `analyzed_by` | string | no | free text | |
| `analysis_scope` | enum | yes | `full`, `incremental`, `targeted` | Scope of the most recent analysis pass |
| `aspects` | list | no | see below | Index of known quality aspects for this component |

**`aspects[]` items:**

| Field | Type | Required | Allowed Values |
|-------|------|----------|----------------|
| `name` | string | yes | Aspect filename without `.md` (e.g., `_general`, `security`) |
| `status` | enum | yes | `not-analyzed`, `draft`, `reviewed`, `verified` |
| `relevance` | enum | yes | `high`, `medium`, `low`, `not-applicable` |

**`analysis_scope` values:**
- **`full`**: Complete component analysis from scratch
- **`incremental`**: Updated since last analysis, focused on changes
- **`targeted`**: Analyzed specific area only (e.g., after a bug fix or migration)

### `dependencies.md`

Maps internal and external dependencies for the component.

```yaml
---
confidence: scaffold
last_analyzed: 2026-03-26
analyzed_by: action-analyze
---
```

| Field | Type | Required | Allowed Values |
|-------|------|----------|----------------|
| `confidence` | enum | yes | see Common Fields |
| `last_analyzed` | date | yes | ISO 8601 |
| `analyzed_by` | string | no | free text |

No additional fields beyond the common set.

### `domain-model.md`

Documents domain entities, aggregates, and bounded context boundaries.

```yaml
---
confidence: scaffold
last_analyzed: 2026-03-26
analyzed_by: action-analyze
---
```

Same schema as `dependencies.md` — common fields only.

### `aspects/*.md`

Individual quality aspect files. Created when the `_general.md` aspect grows too large or when a specific concern requires dedicated tracking (HOL-363 threshold-based splitting).

```yaml
---
aspect: security
status: draft
relevance: high
last_analyzed: 2026-03-26
analyzed_by: action-analyze
---
```

| Field | Type | Required | Allowed Values | Description |
|-------|------|----------|----------------|-------------|
| `aspect` | string | yes | Matches filename without `.md` | Aspect identifier |
| `status` | enum | yes | `not-analyzed`, `draft`, `reviewed`, `verified` | Analysis maturity |
| `relevance` | enum | yes | `high`, `medium`, `low`, `not-applicable` | Importance to this component |
| `last_analyzed` | date | yes | ISO 8601 | |
| `analyzed_by` | string | no | free text | |

**Note**: Aspect files do NOT have a `confidence` field — they use `status` instead, which serves the same graduated-maturity purpose but with aspect-specific semantics aligned to HOL-363.

#### Standard Aspect Names

These are recommended aspect names. Projects may define additional aspects as needed.

| Aspect | Filename | When to Split |
|--------|----------|---------------|
| General | `_general.md` | Always exists (default aggregated) |
| Security | `security.md` | Auth, encryption, input validation, OWASP concerns |
| Performance | `performance.md` | Latency-sensitive, high-throughput, caching strategies |
| Business Rules | `business-rules.md` | Complex domain logic, validation rules, state machines |
| Accessibility | `accessibility.md` | UI components with WCAG requirements |
| Data Integrity | `data-integrity.md` | Financial data, audit trails, consistency guarantees |
| Migration | `migration.md` | Legacy code patterns, migration paths, compatibility |

## Integration with HOL-363 Aspects Framework

The aspect lifecycle defined in HOL-363 maps directly to the front matter:

1. **Default state**: Only `_general.md` exists with `status: draft`
2. **Threshold detection**: When `_general.md` grows beyond threshold, `aspect-manager` skill proposes split
3. **Split**: New `aspects/{name}.md` created with `status: not-analyzed`, added to `overview.md` aspects index
4. **Analysis**: Meta-skill (e.g., `action-analyze`) fills content, updates `status` to `draft`
5. **Review**: Human or secondary agent reviews, promotes to `reviewed` or `verified`

The `overview.md` `aspects[]` array serves as the aspect index — it is the authoritative list of which aspects are tracked for a component, their current status, and relevance.

## Integration with Meta-Skills (HOL-378)

Meta-skills read `.index/` files to parameterize behavior:

1. **Discovery**: Meta-skill receives component path, looks for `.index/` directory
2. **Front matter parsing**: Reads YAML front matter to determine analysis state and scope
3. **Conditional behavior**: Skips `not-analyzed` or `not-applicable` aspects; prioritizes `high` relevance
4. **Output**: Updates `last_analyzed`, `analyzed_by`, and promotes `confidence`/`status` as appropriate

## Validation

A validation script at `.holicode/scripts/validate-context-frontmatter.sh` checks:

1. Required fields present for each file type
2. Enum values are within allowed sets
3. `last_analyzed` is a valid ISO 8601 date
4. Aspect names in `overview.md` index match actual files in `aspects/`
5. Aspect file `aspect` field matches its filename

See the script for usage: `bash .holicode/scripts/validate-context-frontmatter.sh <path-to-.index-dir>`

**Script filename note**: The script file is named `validate-context-frontmatter.sh` for historical reasons (HOL-389); it validates frontmatter schema, not a directory name. Its internal path references use `.index/` post-HOL-508. File rename is out of scope for HOL-508 — tracked as a possible follow-up if the drift becomes confusing.

## Templates

Scaffold templates are in `.holicode/templates/index/`:

```
templates/index/
├── overview.md           # Scaffold — indexes only _general
├── dependencies.md       # Scaffold
├── domain-model.md       # Scaffold
└── aspects/
    ├── _general.md       # Scaffold — always created
    └── _examples/        # Reference only — NOT scaffolded by default
        ├── security.md
        └── business-rules.md
```

When scaffolding a new `.index/` directory, copy only the top-level files and `aspects/_general.md`. The `_examples/` templates are references for when aspects are split out later via the HOL-363 lifecycle.
