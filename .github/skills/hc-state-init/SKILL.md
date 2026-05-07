---
name: hc-state-init
description: Initialize `.holicode/` state structure from templates and gather missing project info.
metadata:
  owner: holicode
  scope: state-bootstrap
  origin: workflows/state-init.md
---

# HoliCode State Initialization

Create complete .holicode structure with all necessary state files from templates.

## Project Information Gathering
Use adapted below question, start with possible guesses from available context and/or propose plan to review any existing files possibly containing relevant informations.

<ask_followup_question>
<question>Project initialization details:
- Project name:
- Primary goal/vision:
- Target users:
- Tech stack:
- Success metrics:
</question>
</ask_followup_question>

## Create Directory Structure

This workflow will now leverage `scripts/integrate-project.sh` to ensure the complete and correct HoliCode directory structure is initialized, including `.holicode/state`, `.holicode/analysis`, `docs/`, and `.gitignore` entries.

```bash
./scripts/integrate-project.sh
```

__Important Note__: Scratch is read-only inputs unless explicitly requested. Files under `.holicode/analysis/scratch/` serve as source material and should not be modified during state initialization unless specifically directed.

## Process

IMPORTANT NOTE `.holicode`, `.clinerules`, and `.github` may be hidden by default and not listed in the environment_details etc in your context.

### 1. Validate Environment & User Preferences
- Confirm working directory contains no existing `.holicode/` directory
- Check for Git repository (required for version control)
- Verify necessary tools are available (gh CLI for GitHub integration)
- **Clarify initialization scope with user:**
  - **Content-Only**: Create and populate state files only (recommended for most cases)
  - **Full Scaffolding**: Additionally create directory structure, templates, and integration scripts

### 2. Initialize Git Repository
Set up Git repository if not already initialized:
```bash
# Check if Git repo exists
if [ ! -d .git ]; then
    # Initialize with main as default branch
    git init -b main
    
    # Configure Git identity
    git config user.name "${GIT_USER_NAME:-HoliCode Agent}"
    git config user.email "${GIT_USER_EMAIL:-agent@holicode.local}"
    
    # Create initial .gitignore if not exists
    if [ ! -f .gitignore ]; then
        cat > .gitignore << 'EOF'
# Dependencies
node_modules/
*.log

# Build outputs  
dist/
build/

# IDE
.idea/
.vscode/
*.swp
*.swo

# Environment
.env
.env.local
.env.*.local

# OS
.DS_Store
._*
Thumbs.db

# HoliCode temporary
.holicode/analysis/scratch/
.holicode/cache/
.holicode/docs-cache/
EOF
    fi
    
    echo "Git repository initialized"
fi
```

### 3. Initialize State Files from Templates

Use `write_to_file` tool for each state file, customizing template content with project-specific information:

- **projectbrief.md** - Customize with gathered project information
- **productContext.md** - Customize with target users and business context  
- **systemPatterns.md** - Customize with architectural patterns
- **techContext.md** - Customize with tech stack and constraints
- **activeContext.md** - Customize with initial context
- **retro-inbox.md** - Initialize with basic structure
- **tracker-mapping.md** - Initialize with issue tracker configuration structure (optional, tracker-dependent)
- **issueTrackerBootstrap.md** - Temporary non-blocking checklist for tag/label taxonomy verification
- **delegationContext.md** - Initialize decision delegation settings (NEW)
  - Set all defaults to require_human_approval
  - Initialize maturity indicators as 'low'/'exploratory'/'beginner'
- **gitContext.md** - Initialize Git context state file (NEW)
  - Track current branch, commits, and Git configuration
  - Set up for semantic commits and branch conventions

### 4. Create Initial Git Commit
After all state files are created:
```bash
# Stage all HoliCode files
git add .holicode/ .clinerules/ .github/ .gitignore

# Create initial commit
git commit -m "chore(init): initialize HoliCode framework

- Set up directory structure
- Create initial state files
- Configure Git repository
- Add workflow templates"

# Attempt to push (non-blocking)
if [ -n "${GITHUB_REPO_URL}" ]; then
    git remote add origin "${GITHUB_REPO_URL}" 2>/dev/null || true
    git push -u origin main 2>/dev/null || echo "Note: Push deferred (offline or no remote)"
fi
```

### 5. Update Progress Tracking (Final Step)
**IMPORTANT**: Update `progress.md` LAST to avoid auto-updates and keep state init reference brief:

```
<write_to_file>
<path>.holicode/state/progress.md</path>
<content>
[Customize templates/state/progress.md with initial project status and brief note: "HoliCode state initialization complete"]
</content>
</write_to_file>
```

- Set appropriate next steps in `activeContext.md`
- Update overall status from "Initializing" to next appropriate phase
- Log successful setup for future reference

### 6. Optional Tracker Taxonomy Bootstrap (Non-Blocking)

If `issue_tracker` is external (`vibe_kanban` or `github`):
- Use `.holicode/state/issueTrackerBootstrap.md` to verify preferred tags/labels exist (`epic`, `story`, `task`, `technical-design`, `spike`, `bug`)
- Missing tags/labels are not blocking; record fallback to title/description conventions if needed

If `issue_tracker` is `local`:
- Mark external taxonomy checks as N/A and confirm local ID conventions

After completion:
- Remove `.holicode/state/issueTrackerBootstrap.md` to keep state lean

### 7. Confirm Completion Boundary
- **Explicitly state that state initialization is complete**
- **Confirm Definition of Done has been met**
- **Ask user for explicit approval before proceeding to any additional workflows**
- Clearly indicate what would be logical next steps (but don't execute them automatically)

## Validation
<invoke_workflow>
<workflow>state-health-check.md</workflow>
</invoke_workflow>
