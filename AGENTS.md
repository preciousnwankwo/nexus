# Nexus — Project Context for AI Agents

## Project Overview

**Nexus** is an experimental systems programming language designed for AI agent systems. It targets C-level performance with compile-time memory safety, no garbage collection, and purpose-built syntax for agent workflows.

**Repository:** `git@github.com:preciousnwankwo/nexus.git`
**Status:** Phase 1 — Bootstrap Compiler Expansion
**File extension:** `.nx`

## Language Design

- **Performance:** Match or exceed C through ownership-based optimization
- **Memory safety:** Simplified Rust-style ownership (single owner, move semantics, borrowing) — zero runtime cost
- **Type system:** Hindley-Milner inference with generics, ADTs, pattern matching
- **Error handling:** `Result<T, E>` + `?` operator (no exceptions)
- **Bootstrap:** Nexus-0 (C) → Nexus-v1 (self-hosting) → Nexus-v2 (LLVM)
- **Toolchain:** Written entirely in C/assembly — no Rust dependency
- **FFI:** First-class C interop via libclang-based bindgen
- **Agent primitives:** `|=>` pipeline operator, `parallel` blocks, MCP native support

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
| 1 | Bootstrap Compiler (C) | **CURRENT** — Working, expanding |
| 2 | Agent Primitives (`\|=>`, `parallel`, MCP) | Pending |
| 3 | Self-Hosting | Pending |
| 4 | LLVM Backend | Pending |
| 5 | Ecosystem & Tooling | Pending |

## Current State

The bootstrap compiler (`src/bootstrap/`) is ~3,200 lines of C and WORKS:
- Lexer, parser, type checker, codegen all functional
- Compiles `.nx` files to C code
- Successfully runs: fibonacci, loops, structs, enums, ownership examples
- Build: `make` → `build/nexus-bootstrap`
- Test: `./build/nexus-bootstrap examples/hello.nx /tmp/out.c && cc -std=c11 /tmp/out.c src/runtime/runtime.c -o /tmp/out -I src/runtime && /tmp/out`

## Project Structure

```
nexus/
├── AGENTS.md                  # This file
├── VISION.md                  # Comprehensive vision document
├── nexus-whitepaper.md        # Language design whitepaper
├── spec/nexus-spec.md         # Formal language specification (3,479 lines)
├── src/
│   ├── bootstrap/             # Nexus-0 compiler (C)
│   │   ├── lexer.c/h          # Tokenizer
│   │   ├── parser.c/h         # Recursive descent + Pratt parser
│   │   ├── ast.c/h            # AST definitions
│   │   ├── typecheck.c/h      # Type checker
│   │   ├── codegen.c/h        # C code generator
│   │   ├── symbol_table.c/h   # Symbol table
│   │   ├── string_table.c/h   # String interning
│   │   ├── arena.c/h          # Arena allocator
│   │   └── main.c             # Entry point
│   └── runtime/               # Runtime library
│       ├── runtime.c          # String, slice, I/O builtins
│       └── runtime.h
├── examples/
│   ├── hello.nx               # Fibonacci + loops (working)
│   ├── ownership.nx           # Ownership examples (working)
│   └── full.nx                # Structs, enums, functions (working)
├── mcp/nexus-agent/           # MCP server for AI agents
├── tests/                     # Test suite
└── Makefile
```

## Conventions

- **Commits:** Conventional commits — `feat:`, `fix:`, `docs:`, `chore:`, `refactor:`, `test:`
- **Agent commits:** Use identity `fisher` / `fisher@owl-audit`
- **Push safety:** Main/master blocked by default
- **Merge conflicts:** Manual resolution (agent fails on conflict)

## Key Design Decisions

1. **Simplified ownership vs Rust:** 80% fewer lifetime annotations through better inference
2. **No Pin/Unpin:** Arena allocation replaces self-referential types (avoids Rust's 99+ open soundness issues)
3. **Agent primitives are language features:** Not libraries — `|=>`, `parallel`, MCP are built-in
4. **Bootstrap from C:** Proven approach (Zig, C, Go all did this)
5. **Fast compilation target:** <1s incremental builds (like Go, not Rust)

## Using the nexus-planner Agent

The MCP server at `mcp/nexus-agent/server.py` exposes tools for any MCP-compatible AI agent.

### Setup

```bash
cd mcp/nexus-agent
python3 -m venv .venv
source .venv/bin/activate
pip install "mcp[cli]"
```

### Available Tools

| Tool | Description |
|---|---|
| `git_status_tool` | Working tree status |
| `git_diff_tool` | Unstaged changes |
| `git_commit_tool` | Stage and commit |
| `git_push_tool` | Push to origin (blocks main) |
| `read_source_file_tool` | Read repo file |
| `list_source_files_tool` | List files by pattern |
| `get_project_structure_tool` | Directory tree |
| `run_make_tool` | Run make targets |
| ... | (16 total tools) |
