# Nexus — Project Context for AI Agents

## Project Overview

**Nexus** is an experimental systems programming language designed for AI agent systems. It targets C-level performance with compile-time memory safety, no garbage collection, and purpose-built syntax for agent workflows.

**Repository:** `git@github.com:preciousnwankwo/nexus.git`
**Status:** Phase 1 — Formal Language Specification
**File extension:** `.nx`

## Language Design

- **Performance:** Match or exceed C through ownership-based optimization
- **Memory safety:** Simplified Rust-style ownership (single owner, move semantics, borrowing) — zero runtime cost
- **Type system:** Hindley-Milner inference with generics, ADTs, pattern matching
- **Error handling:** `Result<T, E>` + `?` operator (no exceptions)
- **Bootstrap:** Nexus-0 (C) → Nexus-v1 (self-hosting) → Nexus-v2 (LLVM)
- **Toolchain:** Written entirely in C/assembly — no Rust dependency
- **FFI:** First-class C interop via libclang-based bindgen

## Key Syntax

```nexus
fn fibonacci(n: u32) -> u32 {
    if n <= 1 { return n }
    return fibonacci(n - 1) + fibonacci(n - 2)
}

fn process(data: own Vector<u8>) -> Result<ProcessedData> {
    let parsed = parse(data)
    return transform(parsed)
}

match response {
    Ok(data) => handle(data),
    Err(e) => log("error: {e}"),
}

agent = Agent(model: "gpt-4")
agent |=> search_web(query) |=> summarize() |=> respond()

parallel {
    let a = fetch(url_a)
    let b = fetch(url_b)
}
```

## Development Phases

| Phase | Description | Status |
|---|---|---|
| 1 | Formal Language Specification | **Current** |
| 2 | Bootstrap Compiler (C) | Pending |
| 3 | Memory Safety (ownership/borrowing) | Pending |
| 4 | Full Type System (HM inference) | Pending |
| 5 | LLVM Backend | Pending |
| 6 | Self-Hosting | Pending |
| 7 | Transpilation & FFI | Pending |
| 8 | AI-Agent Features | Pending |
| 9 | Tooling & Ecosystem | Pending |

## Current Work

Writing the formal language specification (`spec/nexus-spec.md`, target ~100-200 pages). The whitepaper (`nexus-whitepaper.md`) and development plan (`plan/nexus-plan.md`) are the foundation.

No compiler code exists yet. The spec must be complete before implementation begins.

## Project Structure

```
nexus/
├── AGENTS.md                  # This file
├── nexus-whitepaper.md        # Language design whitepaper
├── plan/
│   └── nexus-plan.md          # Detailed development plan
├── mcp/
│   └── nexus-agent/           # nexus-planner MCP agent
│       ├── server.py           # MCP server entry point
│       ├── git_tools.py        # Git operations
│       ├── build_tools.py      # Build/source tools
│       ├── nexus_context.py    # Project context loader
│       └── pyproject.toml      # Python dependencies
└── .gitignore
```

## Conventions

- **Commits:** Conventional commits — `feat:`, `fix:`, `docs:`, `chore:`, `refactor:`, `test:`
- **Agent commits:** Use identity `nexus-planner` / `nexus-planner@nexus.local`
- **Push safety:** Main/master blocked by default
- **Merge conflicts:** Manual resolution (agent fails on conflict)

## Using the nexus-planner Agent

The MCP server at `mcp/nexus-agent/server.py` exposes 16 tools for any MCP-compatible AI agent.

### Setup

```bash
cd mcp/nexus-agent
python3 -m venv .venv
source .venv/bin/activate
pip install "mcp[cli]"
```

### Connect from any MCP client

```json
{
  "mcpServers": {
    "nexus": {
      "command": "python3",
      "args": ["/Users/ibold/Programming/experiment/nexus/mcp/nexus-agent/server.py"]
    }
  }
}
```

### Available Tools

| Tool | Description |
|---|---|
| `git_status_tool` | Working tree status |
| `git_diff_tool` | Unstaged changes |
| `git_diff_staged_tool` | Staged changes |
| `git_log_tool` | Recent commits |
| `git_show_commit_tool` | Specific commit diff |
| `git_branch_list_tool` | List branches |
| `git_branch_create_tool` | Create branch |
| `git_branch_delete_tool` | Delete branch |
| `git_preview_commit_tool` | **Preview commit before confirming** |
| `git_commit_tool` | Stage and commit |
| `git_push_tool` | Push to origin (blocks main) |
| `git_pull_tool` | Pull with rebase |
| `git_stash_tool` | Stash changes |
| `git_stash_pop_tool` | Pop stash |
| `git_checkout_tool` | Switch branches |
| `read_source_file_tool` | Read repo file |
| `list_source_files_tool` | List files by pattern |
| `get_project_structure_tool` | Directory tree |
| `run_make_tool` | Run make targets |

### Commit Workflow

1. Agent calls `git_preview_commit_tool("message")` — shows diff and proposed commit
2. User reviews the output
3. Agent calls `git_commit_tool("message")` — stages and commits
4. Agent calls `git_push_tool(branch="feature-x")` — pushes to origin

Main branch is blocked. Use `force=True` to override.
