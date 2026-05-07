---
name: hc-action-analyze
description: Analyze a module's architecture, dependencies, quality aspects, and .index/ accuracy by reading its .index/ files and selectively inspecting source code. Use when investigating a module before implementation, reviewing code health, or validating that .index/ documentation matches actual code.
argument-hint: "[module-path]"
context: fork
agent: general-purpose
compatibility: Designed for Claude Code, Codex, OpenCode, and Gemini skills format.
metadata:
  owner: holicode
  scope: module-analysis
---

# Action: Analyze Module

Analyze the module at `$ARGUMENTS[0]` by reading its `.index/` files and selectively inspecting source code. Produce a structured analysis report.

## Input

- **Module path**: `$ARGUMENTS[0]` (relative to repo root)
- **Optional instruction**: `$ARGUMENTS[1]` — if set to `update`, update `.index/` files with findings

## Procedure

### Phase 1: Discovery (Progressive Disclosure — Overview First)

1. **Verify the module path exists**. If `$ARGUMENTS[0]` is empty or the path doesn't exist, report an error and stop.

2. **Read the `.index/` directory listing** at `$ARGUMENTS[0]/.index/`. If no `.index/` directory exists, scan the module's top-level files and report: "No .index/ directory found. Run with `update` argument to bootstrap one."

3. **Read overview files first** — do NOT deep-read everything at once:
   - `$ARGUMENTS[0]/.index/overview.md` — module purpose, architecture pattern, key decisions
   - `$ARGUMENTS[0]/SPEC.md` — component specification (if it exists)

4. **Scan for additional .index/ files** and note their presence without reading them yet:
   - `dependencies.md` — internal and external dependency map
   - `domain-model.md` — domain entities, aggregates, value objects
   - `aspects/_general.md` — default aggregated quality aspect (always exists if `.index/` is scaffolded)
   - `aspects/*.md` — dedicated quality aspects (security, performance, business-rules, etc.)
   - Any other `.md` files in `.index/`

### Phase 2: Targeted Deep-Read

Based on what you learned in Phase 1, selectively read additional files:

5. **Read dependency information** if the overview mentions external integrations or complex coupling:
   - `$ARGUMENTS[0]/.index/dependencies.md`
   - Scan `import` / `require` statements in 2-3 key source files to verify

6. **Read quality aspects** if the overview mentions quality concerns or aspects are indexed:
   - `$ARGUMENTS[0]/.index/aspects/_general.md` — default aggregated quality aspect
   - Any dedicated aspects with `relevance: high` or `relevance: medium` in their frontmatter
   - Check for test files: glob `$ARGUMENTS[0]/**/*.test.*` or `$ARGUMENTS[0]/**/*.spec.*`

7. **Inspect source code selectively** — do NOT read every file. Target:
   - Entry points (index files, main exports)
   - Files referenced in `.index/` docs as key components
   - Files where `.index/` claims seem worth verifying

### Phase 3: Analysis

Produce findings for each analysis dimension:

8. **Architecture Assessment**
   - Module structure and layering (flat, layered, hexagonal, etc.)
   - Key patterns in use (repository, factory, observer, etc.)
   - Boundary enforcement (how well the module encapsulates its internals)
   - Entry point clarity (is the public API obvious?)

9. **Dependency Analysis**
   - Internal dependencies (other modules in the same project)
   - External dependencies (third-party packages)
   - Coupling assessment (tight/loose, direction of dependencies)
   - Circular dependency risk

10. **Quality Aspects**
    - Test presence and apparent coverage strategy
    - Error handling patterns
    - Type safety (TypeScript strictness, runtime validation)
    - Code complexity signals (deeply nested logic, large files)

11. **Index Discrepancies** (critical — this is unique value)
    - Claims in `.index/` files that don't match actual code
    - Documented APIs that no longer exist or have changed signatures
    - Missing documentation for significant code that exists
    - Stale references to removed dependencies or patterns

### Phase 4: Report

12. **Produce the analysis report** in the format below. Present it directly — do not save to a file unless explicitly asked.

### Phase 5: Optional Index Update

13. **If `$ARGUMENTS[1]` is `update`**: Modify `.index/` files to fix discrepancies found in step 11. For each update:
    - State what changed and why
    - Preserve existing accurate content
    - Add missing sections for undocumented aspects
    - If no `.index/` directory exists, create it with `overview.md` and `aspects/_general.md` as a minimal bootstrap (per HOL-389 schema)

## Output Format

```markdown
# Module Analysis: <module-path>

**Date**: <ISO date>
**Analyzed by**: action-analyze skill

## Summary

<2-3 sentence overall assessment of module health and .index/ accuracy>

## Architecture

| Aspect | Finding |
|--------|---------|
| Structure | <flat/layered/hexagonal/other> |
| Key patterns | <patterns observed> |
| Boundary enforcement | <strong/moderate/weak> |
| Entry point clarity | <clear/ambiguous> |

<1-2 paragraphs of architectural observations>

## Dependencies

| Type | Count | Notable |
|------|-------|---------|
| Internal modules | <N> | <key deps> |
| External packages | <N> | <key deps> |
| Coupling | <tight/moderate/loose> | <direction notes> |

<Circular dependency risks or coupling concerns, if any>

## Quality Aspects

| Aspect | Status | Notes |
|--------|--------|-------|
| Test coverage | <present/partial/absent> | <strategy notes> |
| Error handling | <consistent/inconsistent/minimal> | <pattern notes> |
| Type safety | <strict/moderate/loose> | <details> |
| Complexity | <low/moderate/high> | <hotspots> |

## Index Discrepancies

| # | File | Discrepancy | Severity |
|---|------|-------------|----------|
| 1 | <.index/file> | <what's wrong> | High/Medium/Low |

<If no discrepancies: "No discrepancies found. .index/ files accurately reflect the codebase.">

## Recommendations

1. <Actionable recommendation with rationale>
2. <...>
3. <...>
```

## Constraints

- **Read-only by default** — only modify `.index/` files when `update` argument is provided
- **Progressive disclosure** — never bulk-read all source files; start with `.index/` and SPEC, then target reads
- **No code changes** — this skill analyzes, it does not implement fixes
- **Scope to module** — do not analyze code outside the specified module path
- **Verify, don't trust** — cross-reference `.index/` claims against actual source code
