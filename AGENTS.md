# [Project Name] (HoliCode)

## Entry Point (Mandatory)
- Follow HoliCode framework rules in `.clinerules/holicode.md`.
- If anything in this file conflicts with HoliCode, `.clinerules/holicode.md` wins.

## Project Context (Always Read First)
- `.holicode/state/activeContext.md`
- `.holicode/state/progress.md`
- `.holicode/state/WORK_SPEC.md` (work manifest linking to active tracker issues)

## How We Work Here
- HoliCode is spec-driven: plan/spec workflows produce docs; implementation happens only after specs exist.
- The configured issue tracker is the source of truth for task management; local `.holicode/` stores technical specs/state.

## Workflows (Custom Agents)
- Workflows are defined in `.github/agents/` (canonical path).
- Other agents discover them via symlinks (`.claude/agents/`, `.opencode/agents/`, `.gemini/agents/`, `.qwen/agents/`).
- Core HoliCode workflows (agent names match filenames):
  - `business-analyze`, `functional-analyze`, `technical-design`, `implementation-plan`, `task-implement`, `spec-backfill`, `spec-workflow`

## Skills
- Skills are defined in `.github/skills/` (canonical path).
- Other agents discover them via symlinks (`.claude/skills/`, `.opencode/skills/`, `.gemini/skills/`, `.qwen/skills/`, `.agents/skills/`).
- Skills use the Agent Skills open standard (`SKILL.md` with YAML frontmatter).

## Git Conventions
- Always stage explicitly: `git add <specific-files>` (never `git add .` / `git add -A`).
- Conventional commits: `type(scope): subject`.
