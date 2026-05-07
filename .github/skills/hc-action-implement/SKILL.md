---
name: hc-action-implement
description: "Implement code changes in a component using its specification and context. Reads SPEC.md + .index/ from the target path to understand module conventions (architecture, naming, test patterns, DI setup) and produces code + tests aligned with those conventions. Invoke with the component path: /action-implement src/Modules/Configuration"
argument-hint: <component-path>
context: fork
compatibility: Designed for Claude Code, Codex, OpenCode, and Gemini skills format.
metadata:
  owner: holicode
  scope: meta-skill
  archetype: implementer
---

# Action: Implement Component Changes

You are implementing changes in the component at path: **$ARGUMENTS[0]**

Use `$0` as shorthand for `$ARGUMENTS[0]` throughout this skill.

## Step 1: Load Component Index

Read the following files from `$0/` (skip any that don't exist):

1. `$0/SPEC.md` — **primary input**: the technical specification defining what to implement
2. `$0/.index/overview.md` — architecture pattern, purpose, key decisions
3. `$0/.index/dependencies.md` — internal and external dependencies
4. `$0/.index/domain-model.md` — domain entities, aggregates, value objects
5. `$0/AGENTS.md` or `$0/CLAUDE.md` — module-level agent instructions (if any)

**If `$0/SPEC.md` does not exist**: Stop and report that the specification is missing. Implementation requires a specification — the user should create a SPEC.md for the component before invoking this skill.

## Step 2: Discover Quality Aspects

List files in `$0/.index/aspects/`:
- If the directory exists, read each aspect file to understand constraints
- Focus on aspects with `relevance: high` or `relevance: medium`
- These aspects define **rules to follow** during implementation:
  - `security.md` → input validation, auth checks, injection prevention
  - `performance.md` → caching strategy, query patterns, lazy loading
  - `business-rules.md` → domain invariants, validation rules, state transitions
  - `_general.md` → default aggregated constraints (always check if present)
- If no aspects directory exists, rely on SPEC.md and overview.md for constraints

## Step 3: Extract Module Conventions

From the loaded `.index/`, identify and document these conventions before writing any code:

### Architecture Pattern
Determine from `overview.md` or by scanning `$0/`:
- **Layered** (Controllers → Services → Repositories)
- **DDD** (Aggregates, Domain Services, Application Services, Infrastructure)
- **Standard** (flat structure, no layering)
- **Fullstack** (frontend + backend in same module)
- **Custom** (document the observed pattern)

### Naming Conventions
Scan existing source files in `$0/` to determine:
- File naming: PascalCase, camelCase, kebab-case, snake_case
- Class/interface naming: prefix conventions (I for interfaces, Abstract for base classes)
- Method naming: verb-first, get/set patterns
- Test file naming: `*.test.ts`, `*.spec.ts`, `*Tests.cs`, `*_test.go`, etc.

### Dependency Injection
Identify the DI pattern from existing code:
- Constructor injection, module registration, service provider
- Registration location (startup file, module file, DI container config)
- Any DI framework in use (e.g., `inversify`, `tsyringe`, `.NET DI`, `Spring`)

### Test Patterns
Scan existing test files in `$0/` to determine:
- Test framework: Jest, Vitest, xUnit, NUnit, pytest, Go testing, etc.
- Test structure: Arrange-Act-Assert, Given-When-Then, describe/it blocks
- Mocking approach: manual mocks, framework mocks, test doubles
- Test location: co-located (`__tests__/`), mirrored (`test/`), same directory
- Fixture patterns: factories, builders, shared fixtures

## Step 4: Plan Implementation

Before writing code, create a brief implementation plan:

1. **What SPEC.md requires**: List the concrete deliverables from the spec
2. **Files to create/modify**: Map spec requirements to specific files, following the architecture pattern
3. **Dependencies**: Note any new dependencies needed and where to register them
4. **Test coverage**: Map each requirement to at least one test case

Present this plan as a checklist. If the scope is ambiguous, clarify with the user before proceeding.

## Step 5: Implement Code Changes

Write code following all extracted conventions:

1. **Match the architecture pattern** — place code in the correct layer/directory
2. **Follow naming conventions** — match existing file and symbol naming exactly
3. **Register dependencies** — add DI registrations in the established location
4. **Respect aspect constraints** — apply security, performance, and business rule guidelines
5. **Keep changes minimal** — implement only what SPEC.md requires; do not refactor surrounding code
6. **Preserve existing patterns** — if the module uses a specific error handling style, logging pattern, or code structure, follow it

### Implementation Rules

- Do NOT introduce new patterns that conflict with the module's established conventions
- Do NOT add dependencies without checking `$0/.index/dependencies.md` for approved/preferred libraries
- Do NOT create utility abstractions for one-time operations
- Do NOT add features beyond what SPEC.md specifies
- If SPEC.md is ambiguous on a point, match the pattern used by similar existing code in the module

## Step 6: Write Tests

Create tests following the module's test patterns:

1. **Use the same test framework** as existing tests in the module
2. **Follow the same structure** (describe/it, test classes, etc.)
3. **Match the mocking approach** — don't introduce a new mocking library
4. **Place tests in the established location** (co-located, mirrored, etc.)
5. **Cover at minimum**:
   - Happy path for each SPEC.md requirement
   - Edge cases explicitly mentioned in SPEC.md or aspects
   - Error/validation cases from business-rules aspects
6. **Name tests descriptively** — test names should read as behavior specifications

## Step 7: Verify Implementation

After implementing:

1. **Run existing tests** to ensure no regressions: check for a test command in `$0/package.json`, `$0/Makefile`, or project root
2. **Run new tests** to verify they pass
3. **Cross-check against SPEC.md** — verify each requirement has been addressed
4. **Cross-check against aspects** — verify constraints have been respected

Produce a brief verification summary:

```markdown
## Implementation Summary

**Component**: $0
**Date**: [today]
**Spec**: $0/SPEC.md

### Changes Made
| File | Action | Description |
|------|--------|-------------|
| path/to/file | Created/Modified | What was done |

### Test Coverage
| Requirement | Test File | Test Case(s) |
|-------------|-----------|--------------|
| [from SPEC] | path/to/test | test name(s) |

### Aspect Compliance
| Aspect | Status | Notes |
|--------|--------|-------|
| [aspect name] | Followed / N/A | Brief note |

### Verification
- [ ] Existing tests pass (no regressions)
- [ ] New tests pass
- [ ] All SPEC.md requirements addressed
- [ ] Module conventions followed
```

## Constraints

- SPEC.md is the **primary input** — do not implement features not specified
- `.index/` files define **how** to implement, not **what** to implement
- Do NOT modify `.index/` files during implementation — that is `action-analyze`'s responsibility
- Do NOT create `.index/` files if the directory doesn't exist — report that scaffolding is needed
- For modules with >500 files: focus on the specific area described in SPEC.md, don't attempt to understand the entire module
- Progressive disclosure: read overview and SPEC first, deep-read source files only as needed for implementation
- If a test command cannot be found, note it in the verification summary rather than skipping verification
