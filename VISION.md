# NEXUS — Vision Document

> **A systems programming language built for the age of AI agents.**
> Fast as C. Safer than Rust. Designed for agents, by agents.

---

## 1. The Problem We're Solving

### 1.1 The Language Gap in AI Systems

Every AI infrastructure team today operates in two worlds:

**Python/TypeScript** — where agents live:
- Rich ML ecosystem, easy to prototype
- Garbage collection introduces unpredictable 10-100ms pauses
- 50-100x slower than C for CPU-bound work (tokenization, data transforms, inference kernels)
- Memory overhead makes edge deployment impossible

**C/Rust/C++** — where performance lives:
- Deterministic, zero-overhead execution
- No native constructs for agent workflows, tool orchestration, or structured concurrency
- Rust's borrow checker has a 2-6 month learning curve (per industry surveys)
- C has no memory safety — ~70% of security vulnerabilities in systems software are memory safety issues

**The result:** Teams write performance-critical agent infrastructure in C/Rust and glue it with Python. This creates fragile multi-language systems with FFI overhead, serialization costs, and cognitive friction at every boundary.

### 1.2 What's Wrong with Existing Languages

**C:**
- No memory safety (buffer overflows, use-after-free, data races)
- No type safety (implicit casts, void* everywhere)
- No module system (header files in 2026)
- No error handling (error codes, errno, manual cleanup)
- No concurrency primitives (pthreads are error-prone)
- Undefined behavior in 200+ cases (per the C spec)

**Rust:**
- Borrow checker is too strict for rapid iteration (fighting the compiler)
- Lifetime annotations are pervasive and confusing (`'a`, `'b`, `'static`)
- Async ecosystem is fragmented (tokio vs async-std, pin/unpin complexity)
- Compile times are 5-10x slower than C
- No native agent/workflow constructs (everything is a library)
- Pin/Unpin for self-referential types is a known soundness hole (99+ open unsoundness issues)
- Error messages, while improved, still require deep type system knowledge

**Zig:**
- No memory safety guarantees (manual memory management)
- No borrow checker (arena allocators help but don't prevent all bugs)
- No type inference (explicit types everywhere)
- No agent-specific features
- Small ecosystem, limited adoption

**Go:**
- Garbage collector (unpredictable latency)
- No generics until 1.18 (and they're limited)
- No ownership system (everything is either pointer or value)
- No enum types (until 1.22, limited)
- Not suitable for systems programming (runtime overhead)

### 1.3 The Agentic Programming Gap

Current agent frameworks (LangChain, CrewAI, AutoGen) are all Python libraries. They:
- Can't express agent workflows as first-class language constructs
- Have no compile-time safety for tool composition
- Can't optimize agent pipelines at compile time
- Have no type safety across tool boundaries
- Can't reason about resource usage or concurrency at the language level

**What if the language itself understood agents?**

---

## 2. The Nexus Thesis

**A programming language can be simultaneously:**
- **Faster than C** (through ownership-based alias analysis and domain-specific optimization)
- **Safer than Rust** (simplified ownership, better inference, fewer annotations)
- **More readable than Python** (clean syntax, type inference, first-class agent constructs)
- **Purpose-built for AI agents** (agent composition, tool orchestration, structured concurrency as language primitives)

This is not incremental improvement. It is a fundamental redesign of what a systems language should be when the primary consumer is an AI agent.

---

## 3. Design Principles

### 3.1 Performance is Non-Negotiable

**Target: Match or exceed C on standard benchmarks.**

How:
- **Ownership-based alias analysis:** The compiler knows exactly when memory is accessed, enabling optimizations C can't prove (no `restrict` needed)
- **Zero-cost abstractions:** Ownership, borrowing, and agent pipelines compile to the same machine code as hand-written C
- **Bounds check elimination:** Compile-time proof that bounds checks are unnecessary → removed
- **Arena allocation as a language primitive:** No per-object deallocation cost
- **Monomorphization:** Generic functions specialized at compile time (no runtime dispatch)
- **LLVM backend:** Leverages decades of optimization work + Nexus-specific passes
- **No runtime:** No GC, no reference counting, no hidden allocations

### 3.2 Safety is Compile-Time, Not Runtime

**Target: Memory safety, type safety, and concurrency safety with zero runtime cost.**

How:
- **Simplified ownership model:** Single owner, move semantics, borrowing — but with inference that eliminates 80% of Rust's annotation burden
- **No null pointers:** `Option<T>` is the default, `T` is non-nullable
- **No data races:** Ownership + borrowing prevents concurrent mutable access at compile time
- **Exhaustive pattern matching:** Compiler ensures all cases are handled
- **No implicit conversions:** Every type conversion is explicit
- **Undefined behavior is impossible:** The language spec defines all behavior

### 3.3 Syntax Serves the Domain

**Target: Agent workflows read like the mental model.**

How:
- **Agent pipeline operator (`|=>`):** `agent |=> search(query) |=> summarize() |=> respond()`
- **Structured concurrency (`parallel`):** `parallel { let a = fetch(url_a); let b = fetch(url_b) }`
- **Tool composition as types:** Tools are typed functions, composition is type-checked
- **Result types everywhere:** `Result<T, E>` + `?` operator — no exceptions, no error codes
- **JSON as a first-class type:** `data["user"]["name"]` with compile-time schema checking

### 3.4 The Toolchain is Minimal and Auditable

**Target: A single binary that compiles Nexus to native code.**

How:
- **Bootstrap from C:** ~5,000 lines of C → compiles Nexus subset to C
- **Self-hosting:** Rewrite compiler in Nexus → compiles itself
- **No external dependencies:** No LLVM dependency for bootstrap (C backend)
- **Fast compilation:** Target <1s for incremental builds (like Go, not Rust)
- **LSP from day one:** Language server built into the compiler

### 3.5 FFI is a First-Class Citizen

**Target: Importing C libraries is as natural as writing native code.**

How:
- **libclang-based bindgen:** Auto-generate Nexus declarations from C headers
- **Zero-overhead FFI:** Calling C functions has no overhead vs calling from C
- **Safe wrappers:** Generated bindings are type-safe, with ownership tracking
- **Transpilation:** Can emit C or Rust code for interoperability

---

## 4. Language Design

### 4.1 Ownership System (Simplified Rust)

```nexus
// Ownership transfer (move semantics by default)
fn take(own data: String) { /* owns data */ }

// Borrowing (shared reference)
fn view(data: &String) { /* borrows data */ }

// Mutable borrowing (exclusive reference)
fn modify(data: &mut String) { /* mutates data */ }

// Usage
let s = String::from("hello");
view(&s);           // s is borrowed, still valid
modify(&mut s);     // s is mutably borrowed
take(s);            // s is moved, no longer valid
// view(s);         // COMPILE ERROR: s was moved
```

**Key simplifications over Rust:**
- Lifetime inference requires ~80% fewer annotations (like OCaml/Haskell)
- Borrow checker integrated into type checker (better error messages)
- No `Pin`/`Unpin` (known soundness hole in Rust — 99+ open issues)
- Arena allocation as a language primitive (no need for complex lifetime gymnastics)
- `own` keyword makes ownership transfer explicit and visible

### 4.2 Type System (Hindley-Milner + Extensions)

```nexus
// Full type inference — no annotations needed
fn add(a, b) { a + b }  // inferred: fn(i64, i64) -> i64

// Generics with monomorphization
fn max<T: Ord>(a: T, b: T) -> T { if a > b { a } else { b } }

// Algebraic data types with exhaustive matching
enum Result<T, E> { Ok(T), Err(E) }
enum Option<T> { Some(T), None }

match result {
    Ok(data) => process(data),
    Err(e) => log(e),  // Compiler ensures all cases handled
}

// Type constraints
fn sort<T: Ord + Clone>(items: &mut Vec<T>) { ... }
```

### 4.3 Error Handling (No Exceptions, No Error Codes)

```nexus
fn parse_config(path: &str) -> Result<Config, Error> {
    let content = fs::read(path)?;           // ? propagates errors
    let config = Config::parse(&content)?;   // Early return on error
    return Ok(config);
}

// Usage
match parse_config("app.conf") {
    Ok(config) => start(config),
    Err(e) => log("Failed: {e}"),
}
```

### 4.4 Agent Primitives (The Differentiator)

```nexus
// Agent pipeline operator
agent = Agent(model: "gpt-4")
result = agent |=> search_web(query) |=> summarize() |=> respond()

// Structured concurrency
parallel {
    let embeddings = compute_embeddings(text)
    let metadata = fetch_metadata(id)
    let permissions = check_access(user)
}
// All three complete before continuing

// Tool composition (type-safe)
tool = search_web |=> extract_links |=> fetch_all |=> summarize
result = tool.run(query)

// Agent with tools
agent = Agent(model: "gpt-4")
    .with_tool(search_web)
    .with_tool(fetch_url)
    .with_tool(parse_json)

response = agent.run("Research {topic} and summarize findings")
```

### 4.5 Memory Management

```nexus
// Stack allocation (default)
let x = 42;

// Heap allocation (explicit)
let s = String::from("hello");

// Arena allocation (bulk free)
arena = Arena::new();
let items = arena.alloc<Vec<Item>>();
// All arena memory freed at once when arena goes out of scope

// Ownership transfer (no copying)
let a = String::from("data");
let b = take_ownership(a);  // a is moved to b, a is no longer valid

// Borrowing (no ownership transfer)
fn process(data: &String) { /* read-only access */ }
fn modify(data: &mut String) { /* exclusive write access */ }
```

### 4.6 Concurrency

```nexus
// Structured concurrency (parallel blocks)
parallel {
    let a = fetch(url_a)
    let b = fetch(url_b)
}
// Both complete before continuing

// Async/await for I/O
async fn fetch_data(url: &str) -> Result<Data> {
    let response = http::get(url).await?;
    return Data::parse(&response)
}

// Channels for message passing
channel = Channel::new();
spawn {
    channel.send(compute());
}
let result = channel.recv();

// No data races at compile time
// Ownership system prevents concurrent mutable access
```

---

## 5. Compilation Strategy

### 5.1 Bootstrap Path

```
Stage 0: Nexus-0 (C)          ← CURRENT
  - Lexer, parser, type checker, C codegen
  - ~3,200 lines of C (already written)
  - Compiles Nexus subset → C → native binary
  - Status: WORKING (fibonacci, loops, structs, enums, ownership all compile and run)

Stage 1: Nexus-v1 (Nexus, compiled by Nexus-0)
  - Full type system (HM inference, generics)
  - Ownership checker (simplified borrow checker)
  - Module system
  - Standard library (written in Nexus)
  - Self-hosting: compiles itself

Stage 2: Nexus-v2 (compiled by Nexus-v1)
  - LLVM backend for native performance
  - Agent primitives (|=>, parallel, tools)
  - libclang-based bindgen for C FFI
  - Full optimization pipeline

Stage 3: Nexus-v3 (production)
  - Package manager
  - Language server (LSP)
  - Debugger support
  - Cross-compilation
```

### 5.2 Backend Strategy

- **Bootstrap backend:** C code generation (portable, any C compiler)
- **Production backend:** LLVM IR (optimization, native performance)
- **Alternative backends:** Rust (for Rust ecosystem interop), WASM (for web)

---

## 6. Agent Integration Architecture

### 6.1 MCP (Model Context Protocol) Native Support

Nexus treats MCP as a first-class concept, not an afterthought:

```nexus
// Define an MCP server in Nexus
mcp_server NexusTools {
    tool search_web(query: String) -> Result<String> {
        // Implementation
    }

    tool read_file(path: String) -> Result<String> {
        // Implementation
    }

    tool execute(command: String) -> Result<Output> {
        // Implementation with safety checks
    }
}

// Connect to external MCP servers
let github = McpClient::connect("github")
let result = github.call("search_repos", params)
```

### 6.2 Agent Runtime

```nexus
// Agent definition
agent ResearchAgent {
    model: "gpt-4"
    tools: [search_web, fetch_url, parse_json, summarize]
    max_steps: 10
    timeout: 30s
}

// Agent execution with structured concurrency
parallel {
    let research = ResearchAgent.run("Research {topic}")
    let analysis = AnalysisAgent.run("Analyze {data}")
}
// Both agents run concurrently, results combined when both complete

// Agent pipeline with error handling
result = ResearchAgent::new()
    |=> search_web(query)
    |=> extract_content()
    |=> summarize()
    |=> validate()
```

### 6.3 Tool Safety

```nexus
// Tools are typed and composable
tool SafeSearch = search_web
    |=> rate_limit(100)        // Max 100 calls/minute
    |=> timeout(30s)           // 30 second timeout
    |=> retry(3)               // Retry 3 times on failure
    |=> validate(schema)       // Validate output against schema

// Tool composition is type-checked at compile time
// If output of step N doesn't match input of step N+1 → compile error
```

---

## 7. Addressing Known Issues

### 7.1 C Issues Solved

| C Issue | Nexus Solution |
|---------|---------------|
| Buffer overflows | Bounds checking at compile time (or proven unnecessary) |
| Use-after-free | Ownership system prevents access after move |
| Null pointer dereference | `Option<T>` is the default, `T` is non-nullable |
| Data races | Ownership + borrowing prevents concurrent mutable access |
| Manual memory management | Ownership + RAII + arena allocation |
| No module system | Proper module system with `import`/`export` |
| Error codes | `Result<T, E>` + `?` operator |
| Undefined behavior | All behavior is defined in the spec |
| Implicit type conversions | All conversions are explicit |
| Header files | Modules with proper dependency tracking |

### 7.2 Rust Issues Solved

| Rust Issue | Nexus Solution |
|------------|---------------|
| Borrow checker too strict | Simplified ownership with better inference |
| Lifetime annotations everywhere | Lifetime inference (80% fewer annotations) |
| Pin/Unpin complexity | Arena allocation replaces self-referential types |
| Async fragmentation | Built-in async/await with structured concurrency |
| Slow compilation | Fast incremental compilation (target <1s) |
| No agent primitives | First-class agent, pipeline, and concurrency syntax |
| Complex error types | Simplified `Result<T, E>` with `?` operator |
| No null but `Option` is verbose | Pattern matching + `?` makes it ergonomic |
| Trait bounds are complex | Simpler constraint system |
| No package manager (cargo is external) | Built-in package manager |

### 7.3 Zig Issues Solved

| Zig Issue | Nexus Solution |
|-----------|---------------|
| No memory safety | Compile-time ownership + borrowing |
| Manual memory management | RAII + arena allocation |
| No type inference | Full HM type inference |
| No agent features | First-class agent constructs |
| Comptime is powerful but complex | Simpler metaprogramming model |

---

## 8. Development Plan

### Phase 1: Bootstrap Expansion (NOW — Month 1-2)
**Goal:** Expand the working bootstrap compiler to support real programs.

- [ ] Complete HM type inference with generics
- [ ] Module system (`import`/`export`/`mod`)
- [ ] Standard library (strings, vectors, hash maps, I/O)
- [ ] `match` expressions with exhaustiveness checking
- [ ] `Result<T, E>` type and `?` operator
- [ ] Struct methods and trait system
- [ ] Comprehensive test suite

**Milestone:** Can write real Nexus programs (not just examples).

### Phase 2: Agent Primitives (Month 2-3)
**Goal:** Implement the features that make Nexus unique.

- [ ] Agent pipeline operator (`|=>`)
- [ ] Structured concurrency (`parallel` blocks)
- [ ] Async/await with structured cancellation
- [ ] Tool composition types
- [ ] JSON as a first-class type with schema validation
- [ ] MCP server/client support

**Milestone:** Can express agent workflows as first-class language constructs.

### Phase 3: Self-Hosting (Month 3-5)
**Goal:** Rewrite the compiler in Nexus.

- [ ] Rewrite lexer/parser in Nexus
- [ ] Rewrite type checker in Nexus
- [ ] Rewrite codegen in Nexus
- [ ] Bootstrap verification: Nexus-v1 compiles itself
- [ ] Standard library in Nexus

**Milestone:** Self-hosting compiler that compiles itself.

### Phase 4: Performance (Month 5-7)
**Goal:** Match or exceed C performance.

- [ ] LLVM backend
- [ ] Ownership-based alias analysis optimization
- [ ] Bounds check elimination
- [ ] Arena allocation optimization
- [ ] Benchmark against C, Rust, Zig
- [ ] Profile-guided optimization

**Milestone:** Performance >= C on standard benchmarks.

### Phase 5: Ecosystem (Month 7-9)
**Goal:** Make Nexus usable for real projects.

- [ ] Package manager
- [ ] Language server (LSP)
- [ ] Debugger support (GDB/LLDB integration)
- [ ] Cross-compilation
- [ ] libclang-based bindgen for C FFI
- [ ] Documentation generator
- [ ] Package repository

**Milestone:** Developers can build real projects in Nexus.

### Phase 6: Production (Month 9-12)
**Goal:** Production-ready language.

- [ ] Formal language specification (100+ pages)
- [ ] Security audit
- [ ] Performance optimization
- [ ] Ecosystem growth
- [ ] Community building
- [ ] Production deployments

**Milestone:** Nexus is used in production AI systems.

---

## 9. Success Criteria

### Minimum Viable Product (MVP)
- [ ] Compiles Nexus programs to native binaries (via C backend)
- [ ] Memory safety with zero runtime cost
- [ ] Performance within 10% of C on standard benchmarks
- [ ] Agent pipeline operator and structured concurrency
- [ ] C FFI via libclang bindgen
- [ ] Self-hosting compiler

### Full Success
- [ ] Performance >= C on all benchmarks
- [ ] Self-hosting compiler with LLVM backend
- [ ] Transpilation to C and Rust
- [ ] Growing ecosystem (100+ packages)
- [ ] Active community (1000+ developers)
- [ ] Used in production AI systems

### Aspirational
- [ ] Becomes a standard language for AI agent infrastructure
- [ ] Replaces C/C++ in new AI systems projects
- [ ] Community-driven language evolution
- [ ] Academic adoption for systems programming courses

---

## 10. Risk Analysis

| Risk | Impact | Mitigation |
|------|--------|------------|
| Compiler complexity exceeds estimates | High | Phased approach; ship minimal viable subset first |
| Ownership model too restrictive | Medium | Provide `unsafe` escape hatch; iterate on ergonomics |
| Performance doesn't match C | High | Leverage LLVM; ownership enables optimizations C can't do |
| Bootstrap takes too long | Medium | C backend is straightforward; prioritize correctness |
| Too ambitious for small team | High | Start with Phase 1 (expand bootstrap); iterate incrementally |
| Agent features don't resonate | Medium | Get feedback from AI teams early; iterate on syntax |
| Ecosystem chicken-and-egg | Medium | C FFI means existing libraries are usable from day one |

---

## 11. Why Now?

1. **AI agents are writing more code than ever.** GitHub Copilot, Claude Code, and similar tools are generating millions of lines of code daily. A language optimized for agent consumption is timely.

2. **The safety-performance gap is widening.** AI systems handle increasingly sensitive data and make autonomous decisions. C's memory safety issues are unacceptable; Rust's learning curve is too high for rapid iteration.

3. **MCP is becoming the standard.** Model Context Protocol is emerging as the standard interface for AI tool integration. A language with native MCP support has a first-mover advantage.

4. **Bootstrap technology is proven.** Zig showed that a self-hosting C bootstrap is feasible. Rust showed that ownership works. LLVM showed that backend optimization is solvable. Nexus combines these lessons.

5. **The team has momentum.** A working bootstrap compiler already exists (3,200 lines of C, compiles and runs real programs). The spec is thorough (3,479 lines). The foundation is solid.

---

## 12. Conclusion

Nexus is not "another systems language." It is a language designed from the ground up for the primary consumer of code in 2026: AI agents. By combining C-level performance, compile-time safety, and first-class agent primitives, Nexus aims to eliminate the gap between what agents need and what existing languages provide.

The experiment begins with a working bootstrap compiler. The next step is to expand it into a language that agents can use to build the next generation of AI infrastructure.

**Nexus: Built for agents. Fast as C. Safe by default.**

---

*This is a living document. As the project evolves, so will this vision.*

*Nexus Project — June 2026*
