# Infrastructure Patterns & Gotchas

Cross-agent reference for known infrastructure constraints and debugging techniques.
Read this before touching `scripts/infra/` or pushing Coder templates.

---

## Coder Templates (`scripts/infra/coder-template/` and `coder-template-x86/`)

Both templates share an identical `main.tf` and `variables.tf`. Architecture-specific
values live in `terraform.tfvars` (per template directory) and the Docker provider
lives in `provider.tf` (per template directory). Drift is enforced by
`scripts/infra/check-template-drift.sh` — run it after editing either template.

### Pre-push Validation (Mandatory)

Coder's workspace-tags parser is stricter than Terraform and gives **no line numbers** on parse failures. Always validate before pushing:

```bash
mkdir -p /tmp/ttest
cp scripts/infra/coder-template/{main.tf,variables.tf,provider.tf,terraform.tfvars} /tmp/ttest/
coder templates push holicode-agentic --directory /tmp/ttest --name test-parse --yes 2>&1 \
  | grep -E 'parse|Invalid|Updated'
```

For x86:
```bash
mkdir -p /tmp/ttest
cp scripts/infra/coder-template-x86/{main.tf,variables.tf,provider.tf,terraform.tfvars} /tmp/ttest/
coder templates push holicode-agentic-x86 --directory /tmp/ttest --name test-parse --yes 2>&1 \
  | grep -E 'parse|Invalid|Updated'
```

### Parser Gotchas

| Pattern | Error | Fix |
|---------|-------|-----|
| `lifecycle { ignore_changes = all }` inline | "Invalid single-argument block definition" | Expand to multi-line block |
| `ephemeral = true` on `coder_parameter` | "Invalid expression" | Remove (unsupported attribute) |
| `$(...)` subshell in a quoted string | "Invalid expression" | Move to heredoc (`<<-EOT`) |
| `${var%pattern}` inside `<<-EOT` heredoc | "Invalid expression" | Use `$${var%%pattern}` |
| `locals {}` referencing data sources | "Invalid expression" | Inline or use env vars |

**Heredoc escaping rules** (applies to ALL `<<-EOT` blocks in main.tf):
- `${...}` → Terraform interpolation. Use `$${...}` for literal shell `${...}`
- `%{...}` → Terraform template directive. Use `%%{...}` for literal `%{`
- Shell `${var%pattern}` **must** be written as `$${var%%pattern}`

### Binary Search for Parse Errors

Parse errors are almost always in `main.tf` (the large shared file). Keep the other
files present in `/tmp/ttest/` and binary-search only `main.tf`:

```bash
mkdir -p /tmp/ttest
cp scripts/infra/coder-template/{variables.tf,provider.tf,terraform.tfvars} /tmp/ttest/
for lines in 100 200 300 400 500 $(wc -l < scripts/infra/coder-template/main.tf); do
  head -$lines scripts/infra/coder-template/main.tf > /tmp/ttest/main.tf
  result=$(coder templates push holicode-agentic --directory /tmp/ttest \
    --name test --yes 2>&1 | grep -oE 'Invalid|Updated')
  echo "Lines $lines: ${result:-ok}"
done
# Then narrow down: bisect the range where ok → Invalid
```

### Workspace Update Patterns

- `coder update <workspace>` only works if workspace is "out of date" (version name changed)
- Same version name → workspace won't update even after `coder templates push`
- `docker_container` with `restart=no` shows as **disabled** in Coder UI — avoid for init logic, use `coder_script` instead

---

## Forgejo Sidecar

Full operational runbook: `scripts/infra/coder-template/SPEC.md`

### Quick Reference

| Symptom | Root Cause | Fix |
|---------|-----------|-----|
| Adoption hangs/times out | Repo has 1000s of tags | Strip tags: `git -C <bare>.git tag \| xargs git -C <bare>.git tag -d` |
| API auth takes 3-4s | pbkdf2$320000 password hash | Set `FORGEJO__security__PASSWORD_HASH_ALGO=bcrypt` env var |
| All Coder resources disabled | One-shot container (`restart=no`) exited non-zero | Use `coder_script` instead; add `|| true` + `exit 0` to container commands |
| Install page on every restart | INSTALL_LOCK not set as env var | Add `FORGEJO__security__INSTALL_LOCK=true` to container env |
| API returns 403 "must change password" | must_change_password flag set | `sqlite3 /data/gitea/gitea.db "UPDATE user SET must_change_password=0 WHERE name='<user>';"` |
| socat dies on restart | pgrep false positive / process group kill | Use `lsof -i :3001` to check, `setsid ... & disown` to start |
| rootless image UID mismatch | `forgejo:9-rootless` ignores USER_UID | Use `forgejo:9` (regular image) — reliably remaps via USER_UID env var |

### Live Debugging Access

```bash
# From workspace or anywhere with SSH access to the Coder host:
ssh root@host.docker.internal "docker exec <container> <command>"
ssh root@host.docker.internal "docker logs <container> --tail 20"
ssh root@host.docker.internal "docker exec <forgejo> sqlite3 /data/gitea/gitea.db '<query>'"
```
