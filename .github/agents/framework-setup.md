---
name: framework-setup
description: Initialize HoliCode framework repository structure (scripts, templates, workflows) and validate setup.
---

# Framework Repository Setup

Initialize the complete HoliCode framework repository structure for development and distribution.

## Framework Type Detection

<ask_followup_question>
<question>What are you setting up?

1. **User Installation** - Install HoliCode framework for personal use
2. **Project Integration** - Add HoliCode to existing project

Please specify which setup you need.</question>
</ask_followup_question>

## Setup Execution

### For User Installation

```bash
# Execute user installation script  
./scripts/install-user-framework.sh
```

### For Project Integration

```bash
# Execute project integration script
./scripts/integrate-project.sh
```

## Repository Structure Validation

```bash
# Validate repository structure
./scripts/validate-framework-structure.sh
```

Expected structure validation:
- [ ] `workflows/` directory with all core workflows
- [ ] `templates/` directory with state and handoff templates  
- [ ] `scripts/` directory with utility scripts
- [ ] `docs/` directory with documentation
- [ ] `install/` directory with installation scripts

## Template Verification

```bash
# Verify all required templates exist
find templates/ -name "*.md" -type f | sort
```

Required templates check:
- [ ] `templates/state/projectbrief.md`
- [ ] `templates/state/productContext.md`
- [ ] `templates/state/systemPatterns.md`
- [ ] `templates/state/techContext.md`
- [ ] `templates/state/activeContext.md`
- [ ] `templates/state/progress.md`
- [ ] `templates/handoff/handoff-template.md`

## Documentation Structure Verification

```bash
# Verify documentation structure
find docs/ -type f -name "*.md" | sort
find .holicode/analysis/ -type d | sort
```

Documentation structure check:
- [ ] `docs/README.md` exists
- [ ] `docs/decisions/` directory exists  
- [ ] `.holicode/analysis/research/` directory exists
- [ ] `.holicode/analysis/decisions/` directory exists
- [ ] `.gitignore` includes HoliCode exclusions

## Workflow Verification

```bash
# Verify all core workflows exist
find workflows/ -maxdepth 1 -name "*.md" -type f | sort
```

Required workflows check:
- [ ] `context-verify.md`
- [ ] `state-health-check.md`
- [ ] `state-init.md`
- [ ] `state-update.md`
- [ ] `task-handoff.md`
- [ ] `task-init` skill
- [ ] `state-review.md`

## Git Repository Initialization

<ask_followup_question>
<question>Should I initialize this as a Git repository and create the initial commit?</question>
</ask_followup_question>

```bash
# Initialize git repository if requested
./scripts/git-init-framework.sh
```

## Documentation Generation

```bash
# Generate framework documentation
./scripts/generate-docs.sh
```

Documentation generation:
- [ ] `README.md` - Framework overview and quick start
- [ ] `docs/getting-started.md` - Installation and setup guide
- [ ] `docs/workflow-reference.md` - Complete workflow documentation
- [ ] `docs/architecture.md` - Framework architecture overview

## Setup Completion Summary

Framework setup completed successfully:

- **Repository Type**: {{setup_type}}
- **Structure**: ✅ Complete
- **Templates**: ✅ {{template_count}} templates installed
- **Workflows**: ✅ {{workflow_count}} workflows installed
- **Scripts**: ✅ {{script_count}} utility scripts installed
- **Documentation**: ✅ Generated

## Next Steps

Based on setup type:

### For User Installation:
1. Test installation: `./scripts/test-installation.sh`
2. Initialize first project: `/state-init`
3. Read getting started guide

### For Project Integration:
1. Verify integration: `/state-health-check`
2. Initialize project state: `/state-init`
3. Set up team workflows

---

**Framework setup complete!** 🚀

Repository structure initialized and ready for {{setup_type}}.
