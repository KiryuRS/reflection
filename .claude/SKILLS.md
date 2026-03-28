# Git Workflow

## Platform

GitHub — remote: `git@github.com:KiryuRS/reflection.git`

PR creation requires the `gh` CLI (`gh auth status` to verify). If `gh` is not found, stop and ask the user to install it (`gh` via https://cli.github.com) before proceeding with the MR step.

---

## When to trigger this workflow

After completing a coding task — once the implementation is done and working — commit and raise a PR without being asked. The user handles branching; just commit on whatever branch is currently checked out.

---

## Step 1 — Stage files

Add all modified/new files **except**:
- `build/`
- `.cache/`
- `CMakeUserPresets.json`

These are typically already in `.gitignore`, but double-check with `git status` before staging.

```bash
git status                  # review what changed
git add <specific files>    # add only relevant files, never use git add -A blindly
git status                  # confirm staging looks right before committing
```

---

## Step 2 — Commit

Message format:
```
[CLAUDE-CODE] <short imperative summary of what was done>
```

Examples:
```
[CLAUDE-CODE] Add YAML encode support for optional members
[CLAUDE-CODE] Fix descriptor generation for inherited classes
[CLAUDE-CODE] Add unit tests for argparse vector parsing
```

Rules:
- Summary is plain English, imperative mood, no period at the end.
- Keep it under 72 characters total (including the `[CLAUDE-CODE]` prefix).
- No body needed in the commit message — detail goes in the PR description.

```bash
git commit -m "[CLAUDE-CODE] <summary>"
```

---

## Step 3 — Push

```bash
git push
```

If the branch has no upstream yet:
```bash
git push -u origin <branch-name>
```

---

## Step 4 — Raise a Pull Request

Use `gh pr create` with the following description structure:

```
gh pr create --title "[CLAUDE-CODE] <same summary as commit>" --body "$(cat <<'EOF'
## Summary
<2-4 sentences. What this PR does and why. Provide enough context for a reviewer
who wasn't present — motivation, problem solved, or feature added.>

## Implementation Details
<High-level patch notes of what changed. Bullet points per logical change:>
- Added / modified / removed X because Y
- Changed behaviour of Z: before → after
- New file / deleted file and why

## Unit Testing
<Bullet list of what was tested and what each test covers.
If no tests were added or modified, write "N/A — no testable logic changes.">
EOF
)"
```

### PR title

Same `[CLAUDE-CODE] <summary>` format as the commit message.

### Section guidance

**Summary** — the "why" and "what" at a glance. Assumes the reviewer has not seen the task description.

**Implementation Details** — think release patch notes. One bullet per meaningful change. Mention files or functions only if they help the reviewer navigate the diff.

**Unit Testing** — list each new or modified test case with one line on what it asserts. If the feature has no testable logic changes (e.g. a pure refactor with no behaviour change, or a docs-only change), write `N/A`.
