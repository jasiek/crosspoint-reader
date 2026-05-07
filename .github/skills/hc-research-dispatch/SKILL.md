---
name: hc-research-dispatch
description: "Conduct a deep-dive research analysis of an external tool, framework, repo, or concept. Use when someone shares a GitHub repo, article, X post, LinkedIn post, or PDF and asks \"how does this relate to HoliCode / what can we learn from this?\" Produces a structured .holicode/analysis/ artifact. Works in two modes: (A) Executor mode — you are already inside a workspace, run the analysis directly. (B) Orchestrator mode — you are dispatching workspaces for others to run. Trigger: user shares a URL, post, or attachment and asks to research/analyze it."
compatibility: Designed for Claude Code with Vibe Kanban MCP tools.
metadata:
  owner: holicode
  scope: research-orchestration
---

# Research Dispatch

Conduct and connect deep-dive research on external tools and concepts — producing
analysis artifacts that compound into HoliCode's institutional knowledge.

## When To Use

- User shares a GitHub repo / article / X post / LinkedIn post / PDF to analyze
- User asks "how does this relate to HoliCode?" or "can we learn from this?"
- Starting a spike on a new concept, tool, or framework in the agentic/AI/RAG/memory space
- Building context before making architectural decisions

## When NOT To Use

- Implementing features — use `task-implement` workflow
- Researching HoliCode's own codebase — use `action-analyze` skill
- Harvesting completed research workspaces — use `workspace-harvest` skill

## Two Modes

### Mode A — Executor (you are already in a workspace)

You are already inside a workspace/worktree. Run the research directly.
This is the primary mode — follow Parts 1–3, 5–8 below.

### Mode B — Orchestrator (dispatching workspaces for others)

You are in an orchestrating session and want to spin up workspace(s) for research.
Use Part 4 (Orchestrator Templates) to construct the issue description and workspace prompt,
then dispatch via `workspace-orchestrate` skill.

---

## Part 0 — Ensure Issue Linkage

**Before any research work**, ensure this workspace is anchored to a tracker issue.

Invoke the `workspace-init` skill:
- If the workspace already has an owning issue → proceed
- If not → `workspace-init` will find or create one, link it, and set the title

This is a hard pre-condition. Do not produce analysis artifacts in an unlinked workspace.

---

## Part 1 — Pre-Flight: Read the Source Material

Before analyzing, read the source material:

1. **For attachments**: Check `.vibe-attachments/` in the current worktree for any files
   shared in the conversation or referenced in the owning issue. Read them.
2. **For repos**: Clone to `/tmp/<repo>` and read core source files, not just README.
3. **For X/LinkedIn posts**: The text in the issue description or conversation is the source — extract key claims.

Understand the core thesis before starting analysis.

---

## Part 2 — Analysis Framework

Every research dispatch should evaluate the topic through these axes. Not all axes apply
to every topic — select the relevant subset and document which ones you're skipping and why.

### Axis 1 — What Does It Actually Do? [O]

Tarski object-level verification: what does the code/implementation actually do,
independent of what the README/marketing claims?

- Clone the repo if it exists: `gh repo clone <owner>/<repo> /tmp/<repo>`
- Read core source files, not just README
- Verify: architecture, data model, key algorithms, API surface, dependencies

### Axis 2 — Landscape (Alternatives + Uniqueness)

- Web search for alternatives: `"<tool>" alternatives comparison 2025 2026`
- Build a comparison table: `| Tool | Coverage | Backend | Stars | Key differentiator |`
- Uniqueness verdict: is the combination/approach genuinely novel, or a recombination?
- The verdict must be **falsifiable** — state what evidence would change it

### Axis 3 — HoliCode SDLC Fit

HoliCode covers: `business analysis → functional specs → technical design → implementation → code → PR → merge`

Map the topic to this span:
- Which stage does it address?
- Which skills or agents could use it?
- What would need to change in HoliCode to adopt it?
- Is there a natural skill to create? (name it)

### Axis 4 — Standard Connection Matrix

Always check these prior issues/analyses for relevance. Skip with `n/a + reason` if not applicable.

#### Memory / RAG (if topic is memory, retrieval, knowledge management)
| Issue | Topic | Relevance |
|-------|-------|-----------|
| HOL-116 | Hybrid search (sqlite-vss + FTS5) | Does topic replace/complement? |
| HOL-291 | Always-on memory agent | Could topic be the memory backend? |
| HOL-310 | Adaptive memory lifecycle | Memory tiering approach |
| HOL-365 | Self-compaction | Can the system prune/compress itself? |
| HOL-369 | Cross-workspace propagation | Memory scope |

#### Knowledge Graph / Context Structure (if topic involves graphs, wikis, knowledge bases)
| Issue | Topic | Relevance |
|-------|-------|-----------|
| HOL-516 | Karpathy LLM Wiki pattern (raw sources → wiki index → agents) | Is topic the ingestion layer? |
| arscontexta issue | Company Graph (wikilinks + markdown as org-level traversable graph) | Same pattern at different scope? |
| repowise issue | Codebase intelligence graph (tree-sitter, git history, ADRs) | Different layer of the same graph? |

#### Personal Project (PKB / knowledge systems)
| Issue | Topic | Relevance |
|-------|-------|-----------|
| PER-36 | PKB Hybrid Agentic Retrieval | Architecture impact? |
| PER-38 | SOTA hybrid retrieval research | Does topic update recommendations? |
| PER-39 | Storage + vectorDB selection | New contender? |
| PER-60 | Vault integration | Integration path? |

> **How to find issue IDs**: Use `list_issues(project_id, search: "HOL-116")` for holicode-meta
> and `list_issues(project_id, search: "PER-36")` for the personal project.

### Axis 5 — [O]/[M] Separation (Tarski Use-Mention)

Explicitly separate:
- **[O] Object level** — verified from source code, benchmarks, actual examples
- **[M] Meta level** — what the README, post, or marketing says

Flag gaps. A large [O]/[M] gap is important signal.

### Axis 6 — "Steal This" Ideas

3–5 concrete, actionable ideas worth adopting in HoliCode. Each should be:
- Specific (not "improve memory" but "add Decision Trace write-back after every task-implement run")
- Scoped (skill? workflow step? state file change? MCP tool?)
- Graded (high/medium/low priority)

---

## Part 3 — Execute Analysis

With source material read (Part 1) and axes selected (Part 2), execute the research:

### 3a. Clone and Inspect (for repos)

```bash
gh repo clone <owner>/<repo> /tmp/<repo>
```

Read core source files. Prioritize: data model, main entry point, API surface, dependencies.
Do not rely solely on README.

### 3b. Web Research

Run targeted searches — be specific, not generic:
- `"<tool name>" <specific angle> 2025 2026`
- `"<tool name>" vs "<main alternative>" comparison`
- `<topic-specific search>`

Build comparison tables inline.

### 3c. Connection Matrix Evaluation

For each relevant HOL/PER issue from Axis 4 table: resolve the issue via
`list_issues(project_id, search: "HOL-XXX")`, read its description, and assess
how the current topic relates.

### 3d. HoliCode Codebase Cross-Reference

Read `.holicode/state/techContext.md` and `.holicode/state/productContext.md` for context.
Check which existing skills, specs, or workflows the topic touches.

### 3e. Issue Relationships

After completing analysis, create `related` relationships from the owning issue to:
- Any directly relevant existing research issues
- HOL-516 (if topic involves knowledge graphs, wikis, or context structure)
- Any HOL memory issues (if topic involves memory/retrieval)

```
create_issue_relationship(issue_id, related_issue_id, "related")
```

---

## Part 4 — Reliability Criterion

Every research analysis must open with a **Reliability Criterion** — a falsifiable statement
of what makes this analysis trustworthy. Write it at the top of the output file.

### Pattern

```markdown
## Reliability Criterion

This analysis is reliable when: (1) <code-level verification requirement>,
(2) <comparison/landscape requirement — grounded in data, not memory>,
and (3) <HoliCode connection requirement — references specific HOL/PER issue numbers,
not vague claims>.
```

The criterion must be **specific to the topic**. Not "I read the code" but
"I verified the graph data model from the Rust source in `src/graph/model.rs`."

---

## Part 4b — Orchestrator Templates (Mode B only)

When dispatching research to a separate workspace (Mode B), use these templates
for the issue description and workspace prompt. Skip this part in executor mode (Mode A).

### Issue Title Pattern

```
Research: <Tool/Concept Name> — <one-line description of what makes it interesting>
```

### Issue Description Template

Include in the issue description:
1. **Source Material** — repo URL, attachment full paths, post text
2. **Primary Research Questions** — 1–2 specific questions
3. **Research Scope** — specific search queries, specific files to inspect, specific HOL/PER issues to check
4. **Output** — artifact path and commit message

### Workspace Prompt Template

Structure the workspace prompt as:
1. Reliability Criterion (always first)
2. Task description with source material
3. Numbered steps: Clone → Web Research → Connection Matrix → HoliCode Relationship → [O]/[M]
4. Output instructions

**Critical**: Include full file paths for any attachments. Include HoliCode context
(see Part 5) so the agent doesn't waste time discovering it.

---

## Part 5 — Meta: HoliCode Context to Always Load

When writing workspace prompts, include this context so the agent doesn't have to discover it:

```
HoliCode's SDLC span: business analysis → functional specs → technical design →
implementation → code → PR → merge.

Architecture: MCP-first; skills are runtime-injectable Markdown; workspace orchestration
via Vibe Kanban MCP; state in .holicode/state/; specs in .holicode/specs/.

Codebase: /var/tmp/vibe-kanban/worktrees/<worktree>/holicode (read-only reference in dispatch workspaces).
```

---

## Part 6 — Philosophical Lenses (Optional but Valuable)

From the epistemic contracts framework — apply when the topic warrants it:

| Lens | When to apply | Key question |
|------|---------------|--------------|
| **Tarski [O]/[M]** | Always | What does it claim vs. what does the code do? |
| **Ashby complexity** | Architecture decisions | Is the tool's complexity appropriate to the problem it solves? |
| **Lakatos** | Evaluating frameworks | What is the hard core (unfalsifiable) vs. protective belt (adjustable) of the tool's claims? |
| **Peirce abduction** | Gap analysis | What's the best explanation for the gap between claimed and observed behavior? |
| **Steel Manning** | Comparing tools | What is the strongest possible case for the alternative/competitor? |

---

## Part 7 — Output Naming Conventions

| Artifact | Location | Pattern |
|----------|----------|---------|
| Analysis file | `.holicode/analysis/` | `<tool-slug>-analysis.md` |
| Comparison table | inside analysis file | inline markdown table |
| Issue | holicode-meta project | `Research: <Name> — <tagline>` |
| Workspace | Vibe Kanban | `HOL-<short name>: <brief description>` |
| Commit | holicode repo | `docs(analysis): <tool> <brief>` |

---

## Relationship to Other Skills

- **Pre-condition**: `workspace-init` (ensure workspace has owning issue — hard dependency)
- **Uses (Mode B)**: `workspace-orchestrate` (dispatch conventions), `issue-tracker` (issue creation)
- **Feeds into**: `workspace-harvest` (collect results when complete)
- **Complements**: `spike-investigate` agent workflow (for time-boxed internal spikes)
- **Aggregates into**: `session-retrospective` (end-of-session synthesis of all research done)

## Constraints

- **Hard pre-condition**: workspace must have an owning issue (invoke `workspace-init` first)
- Read source material before analyzing — analysis quality depends on understanding the topic
- Full attachment paths must be included when dispatching (Mode B) — not just filenames
- Uniqueness verdicts must be falsifiable — state what evidence would change them
- Always create issue relationships to connected prior research — this builds the research graph
- In Mode B: the orchestrating agent should not repeat the research — dispatch to the workspace
