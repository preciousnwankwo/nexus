# Nexus Language Specification

**Version:** 0.1.0-draft
**Status:** Work in Progress
**Date:** June 2026

---

## Table of Contents

1. [Lexical Structure](#1-lexical-structure)
2. [Identifiers and Keywords](#2-identifiers-and-keywords)
3. [Literals](#3-literals)
4. [Operators and Precedence](#4-operators-and-precedence)
5. [Types](#5-types)
6. [Expressions](#6-expressions)
7. [Statements](#7-statements)
8. [Functions](#8-functions)
9. [Structs and Enums](#9-structs-and-enums)
10. [Module System](#10-module-system)
11. [Agent-Specific Syntax](#11-agent-specific-syntax)
12. [Grammar Summary](#12-grammar-summary)
13. [Ownership and Borrowing Rules](#13-ownership-and-borrowing-rules)
14. [Lifetime Inference](#14-lifetime-inference)
15. [Semantics](#15-semantics)
16. [Runtime Specification](#16-runtime-specification)

---

## 1. Lexical Structure

A Nexus source file is a sequence of Unicode characters (UTF-8 encoded) divided into tokens by the lexer. The lexer is greedy and context-free: each maximal sequence of characters that matches a token pattern is consumed as a single token.

### 1.1 Source File Encoding

- **Encoding:** UTF-8 (mandatory)
- **Line endings:** LF (`\n`), CRLF (`\r\n`), or CR (`\r`) — all normalized to LF during lexing
- **Null bytes:** Not permitted in source files

### 1.2 Whitespace

Whitespace is used to separate tokens. The following characters are whitespace:

| Character | Codepoint | Name |
|-----------|-----------|------|
| ` `       | U+0020    | Space |
| `\t`      | U+0009    | Tab |
| `\n`      | U+000A    | Line feed |
| `\r`      | U+000D    | Carriage return |
| `\r\n`    | U+000D+000A | Carriage return + line feed |

Whitespace is **not significant** in Nexus (no significant indentation). Multiple whitespace characters are equivalent to one for token separation purposes.

### 1.3 Comments

Nexus supports two comment forms:

**Line comments:**
```
// This is a line comment
let x = 5  // Inline comment
```

A line comment starts with `//` and extends to the end of the line. Line comments are terminated by a newline character or end-of-file.

**Block comments:**
```
/* This is a
   block comment */
```

A block comment starts with `/*` and ends at the first subsequent `*/`. Block comments **nest**: each `/*` inside a block comment opens a new level, and each `*/` closes one. An unterminated block comment at end-of-file is a lexer error.

```
/* Outer comment
   /* Nested comment */
   Back to outer
*/
```

**Documentation comments** (future — not lexically distinct in v0):
```
// TODO: support /// and //! doc comments in a future version
```

### 1.4 Token Categories

The lexer produces the following token categories:

| Category | Examples | Description |
|----------|----------|-------------|
| Keyword | `fn`, `let`, `mut`, `if` | Reserved words with special meaning |
| Identifier | `foo`, `bar_baz`, `X` | User-defined names |
| Integer literal | `0`, `42`, `0xff`, `0o77`, `0b1010` | Integer values |
| Float literal | `3.14`, `1.0e10`, `2.5f32` | Floating-point values |
| String literal | `"hello"`, `r"raw"` | String values |
| Character literal | `'a'`, `'\n'`, `'\u{0041}'` | Single character values |
| Boolean literal | `true`, `false` | Boolean values |
| Operator | `+`, `-`, `*`, `->`, `=>` | Symbols that denote operations |
| Delimiter | `(`, `)`, `{`, `}`, `[`, `]` | Structural delimiters |
| Separator | `,`, `;`, `:`, `.` | Punctuation |
| Special | `|=>`, `::` | Agent-specific or path operators |

### 1.5 Token Concatenation

Tokens are consumed greedily. The longest valid token is always chosen. For example:

- `=` is the assignment operator
- `==` is the equality operator
- `=>` is the match arm arrow
- `->` is the return type arrow
- `|=>` is the agent pipeline operator

Given the input `==>`, the lexer produces `==` followed by `=` (not `=` followed by `==`).

---

## 2. Identifiers and Keywords

### 2.1 Identifiers

An identifier is a sequence of characters that names a binding (variable, function, type, module, etc.).

**Grammar:**
```
identifier  = (letter | "_") { letter | digit | "_" }
letter      = "a".."z" | "A".."Z" | unicode_letter
unicode_letter = any Unicode character with category Lu, Ll, Lt, Lm, or Lo
digit       = "0".."9"
```

**Rules:**
- Must start with a letter or underscore (`_`)
- Subsequent characters may be letters, digits, or underscores
- Identifiers are case-sensitive: `Foo` and `foo` are distinct
- Identifiers starting with `_` are allowed but have no special semantics (no "unused" convention like Rust)

**Examples:**
```
x
foo
_bar
snake_case_name
camelCaseName
_unicode_2
MAX_SIZE
```

**Reserved identifiers:** The following identifiers are reserved and cannot be used as user-defined names (they are keywords):

See Section 2.2.

### 2.2 Keywords

Keywords are reserved identifiers with special syntactic meaning. They cannot be used as identifiers.

**Core keywords:**

| Keyword | Description |
|---------|-------------|
| `fn` | Function declaration |
| `let` | Immutable binding |
| `mut` | Mutable binding modifier |
| `if` | Conditional expression |
| `else` | Conditional alternative |
| `match` | Pattern matching expression |
| `return` | Return from function |
| `struct` | Struct declaration |
| `enum` | Enum declaration |
| `import` | Import a module |
| `export` | Export from module |
| `use` | Use/import a name |
| `as` | Alias in use statements |
| `pub` | Public visibility |
| `mod` | Module declaration |
| `self` | Current module reference |
| `super` | Parent module reference |
| `packet` | Root module reference |
| `unsafe` | Unsafe block |
| `arena` | Arena allocation scope |
| `own` | Owned type annotation |
| `borrow` | Borrowed type annotation |
| `async` | Asynchronous function/block |
| `await` | Wait for async completion |
| `parallel` | Parallel execution block |
| `agent` | Agent declaration |
| `type` | Type alias declaration |
| `trait` | Trait declaration |
| `impl` | Trait implementation |
| `where` | Generic constraints |
| `loop` | Infinite loop |
| `while` | Conditional loop |
| `for` | Iterator loop |
| `in` | Iterator binding |
| `break` | Break from loop |
| `continue` | Continue to next iteration |
| `true` | Boolean true literal |
| `false` | Boolean false literal |
| `null` | Null/nil literal |
| `self` | Method receiver |
| `drop` | Explicit destructor call |
| `copy` | Copy trait |

**Soft keywords (context-dependent):**

| Keyword | Context | Description |
|---------|---------|-------------|
| `move` | Variable binding | Explicit move annotation |
| `ref` | Pattern | Reference pattern |
| `mut` | Binding, reference | Mutability modifier |

Soft keywords may be used as identifiers in non-keyword contexts, but this is discouraged.

### 2.3 Naming Conventions

The following conventions are **enforced by convention** (not by the compiler) but are strongly recommended:

| Kind | Convention | Example |
|------|-----------|---------|
| Types | PascalCase | `MyStruct`, `Result`, `Vector` |
| Functions | snake_case | `my_function`, `parse_input` |
| Variables | snake_case | `my_var`, `count` |
| Constants | SCREAMING_SNAKE_CASE | `MAX_SIZE`, `PI` |
| Modules | snake_case | `my_module`, `io` |
| Type parameters | Single uppercase letter or PascalCase | `T`, `E`, `KeyType` |
| Lifetimes | Single lowercase letter or `'name` | `'a`, `'input` |
| Enum variants | PascalCase | `Ok`, `Err`, `Some` |

---

## 3. Literals

Literals are syntactic forms that denote fixed values.

### 3.1 Integer Literals

Integer literals denote whole numbers. They have type `i64` by default, with type suffixes to specify other types.

**Grammar:**
```
integer_literal = decimal_literal | hex_literal | octal_literal | binary_literal
decimal_literal = digit { digit } [ "_" digit { digit } ]
hex_literal     = "0x" hex_digit { hex_digit | "_" }
octal_literal   = "0o" octal_digit { octal_digit | "_" }
binary_literal  = "0b" ("0" | "1") { ("0" | "1") | "_" }
```

**Rules:**
- Leading zeros are not permitted in decimal literals (except `0` itself)
- Underscores may be used as visual separators: `1_000_000`, `0xFF_FF`
- Underscores cannot be adjacent: `1__0` is invalid
- Underscores cannot be leading or trailing: `_5`, `5_` are invalid
- No `+` or `-` sign is part of the literal; negation is an expression (`-5`)

**Type suffixes:**
```
42      // i64 (default)
42u8    // u8
42u16   // u16
42u32   // u32
42u64   // u64
42u128  // u128
42i8    // i8
42i16   // i16
42i32   // i32
42i64   // i64
42i128  // i128
```

**Examples:**
```
0
42
1_000_000
0xFF
0o77
0b1010_0101
42u32
0xff_u8
```

### 3.2 Float Literals

Float literals denote floating-point numbers. They have type `f64` by default.

**Grammar:**
```
float_literal = (decimal_literal "." decimal_literal [exponent]) | (decimal_literal exponent) | (decimal_literal float_suffix)
exponent       = ("e" | "E") ["+" | "-"] decimal_literal
float_suffix   = "f32" | "f64"
```

**Rules:**
- Must contain a `.`, an exponent, or both
- A `.` alone is not sufficient: `5.` is valid, but `5` is an integer
- Exponent is `e` or `E` followed by optional sign and decimal digits
- Underscores allowed as in integer literals

**Type suffixes:**
```
3.14    // f64 (default)
3.14f32 // f32
1.0e10  // f64
1.0e10f32 // f32
```

**Examples:**
```
3.14
3.14f32
1.0e10
1.0E-5
1_000.0_001
.5      // invalid — must have leading digit
5.      // valid — equals 5.0
```

### 3.3 String Literals

String literals denote heap-allocated, UTF-8 encoded strings. The type is `String` (owned) or `&str` (borrowed) depending on context.

**Grammar:**
```
string_literal    = regular_string | raw_string | byte_string
regular_string    = '"' { string_char } '"'
raw_string        = "r" { "#" } '"' { any_char } '"' { "#" }
byte_string       = "b" '"' { byte_char } '"' | "b" "r" { "#" } '"' { any_char } '"' { "#" }
string_char       = normal_char | escape_char
normal_char       = any Unicode character except '"', '\', and control characters
escape_char       = '\' escape_sequence
escape_sequence   = "n" | "r" | "t" | "0" | "\\" | '"' | "\'"
                   | "u{" hex_digit { hex_digit, hex_digit, hex_digit, hex_digit, hex_digit, hex_digit } "}"
                   | "x" hex_digit hex_digit
byte_char         = normal_char | escape_char | byte_escape
byte_escape       = '\x' hex_digit hex_digit
```

**Escape sequences:**

| Sequence | Meaning |
|----------|---------|
| `\n` | Newline (U+000A) |
| `\r` | Carriage return (U+000D) |
| `\t` | Tab (U+0009) |
| `\0` | Null (U+0000) |
| `\\` | Backslash |
| `\"` | Double quote |
| `\'` | Single quote |
| `\u{HHHHHH}` | Unicode escape (1-6 hex digits) |
| `\xHH` | ASCII escape (2 hex digits) |

**Raw strings:** Raw strings do not process escape sequences. They are delimited by `"` with optional `#` count:
```
r"no \escape"          // no escape processing
r#"no "escape"#"       // allows " inside
r##"allows "both"##"   // allows " and #
```

**String interpolation (future):** Not in v0. Planned for a future version.

**Examples:**
```
"hello, world"
"line\nbreak"
"unicode: \u{0041}"
"backslash: \\"
r"raw string"
r#"raw with "quotes"#" 
```

### 3.4 Character Literals

Character literals denote single Unicode scalar values. The type is `char`.

**Grammar:**
```
char_literal = "'" (char_char | escape_char) "'"
char_char    = any Unicode character except "'", '\', and control characters
```

**Examples:**
```
'a'
'\n'
'\u{0041}'
'\\'
'\''
```

### 3.5 Boolean Literals

Boolean literals denote values of type `bool`.

```
true
false
```

### 3.6 Null Literal

The null literal denotes the absence of a value. It is used with nullable types (`?T`).

```
null
```

### 3.7 Array Literals

Array literals denote fixed-size arrays. Type is inferred from elements.

```
[1, 2, 3]              // [i64; 3]
[0; 10]                 // [i64; 10] — ten zeros
["hello", "world"]      // [&str; 2]
```

### 3.8 Tuple Literals

Tuple literals denote tuple values. Type is inferred from elements.

```
()                      // unit type ()
(1, "hello")            // (i64, &str)
(1, 2, 3)               // (i64, i64, i64)
```

---

## 4. Operators and Precedence

Operators are special tokens that denote operations on values. Nexus uses a fixed precedence table — parentheses are used to override precedence.

### 4.1 Operator Precedence Table

Precedence is listed from **highest** (tightest binding) to **lowest** (loosest binding):

| Precedence | Operator | Associativity | Description |
|-----------|----------|---------------|-------------|
| 1 (highest) | `::` | Left | Path separator |
| 2 | `.` | Left | Field access |
| 3 | `()` | Left | Function call |
| 3 | `[]` | Left | Index access |
| 4 | `!` `-` | Right | Unary not, unary minus |
| 5 | `*` `/` `%` | Left | Multiplication, division, modulo |
| 6 | `+` `-` | Left | Addition, subtraction |
| 7 | `<<` `>>` | Left | Bitwise shift |
| 8 | `&` | Left | Bitwise AND |
| 9 | `^` | Left | Bitwise XOR |
| 10 | `\|` | Left | Bitwise OR |
| 11 | `==` `!=` `<` `>` `<=` `>=` | Left | Comparison |
| 12 | `&&` | Left | Logical AND |
| 13 | `\|\|` | Left | Logical OR |
| 14 | `??` | Left | Null coalescing |
| 15 | `=` `+=` `-=` `*=` `/=` `%=` `&=` `\|=` `^=` `<<=` `>>=` | Right | Assignment |
| 16 (lowest) | `=>` | Right | Match arm arrow |
| 16 | `\|=>` | Left | Agent pipeline |

### 4.2 Operator Descriptions

**Unary operators:**
- `-x` — arithmetic negation (works on numeric types)
- `!x` — logical not (works on `bool`) or bitwise not (works on integer types)
- `&x` — address-of (creates a reference)
- `*x` — dereference (accesses pointed-to value)

**Binary operators:**
- `+` — addition, string concatenation (with overloading)
- `-` — subtraction
- `*` — multiplication
- `/` — division (integer division truncates toward zero)
- `%` — modulo (sign follows dividend)
- `==` — equality (deep comparison for structs)
- `!=` — inequality
- `<`, `>`, `<=`, `>=` — ordering (numeric, string lexicographic)
- `&&` — logical and (short-circuit)
- `||` — logical or (short-circuit)
- `&` — bitwise and
- `|` — bitwise or
- `^` — bitwise xor
- `<<` — left shift
- `>>` — right shift
- `??` — null coalescing (unwrap or use default)

**Assignment operators:**
- `=` — assignment (move semantics by default)
- `+=`, `-=`, `*=`, `/=`, `%=` — compound assignment
- `&=`, `|=`, `^=`, `<<=`, `>>=` — compound bitwise assignment

**Special operators:**
- `->` — return type annotation (not an operator; part of function syntax)
- `=>` — match arm separator
- `|=>` — agent pipeline operator
- `::` — path separator (module::item)
- `..` — range operator (inclusive: `1..5` means `[1,2,3,4,5]`)
- `..=` — inclusive range operator
- `..` — exclusive range operator

### 4.3 Operator Overloading

Operators can be overloaded by implementing traits on user-defined types. See Section 9 for trait implementations.

### 4.4 Parentheses

Parentheses `(` `)` may be used to override precedence in expressions:

```
(2 + 3) * 4    // 20, not 14
```

Parentheses are also used for grouping in pattern matching, function parameters, and type expressions.

---

## 5. Types

Types in Nexus describe the shape and ownership semantics of values. The type system is based on Hindley-Milner inference with extensions for ownership and borrowing.

### 5.1 Primitive Types

| Type | Size | Range | Description |
|------|------|-------|-------------|
| `bool` | 1 byte | `true`, `false` | Boolean |
| `char` | 4 bytes | Unicode scalar values | Unicode character |
| `u8` | 1 byte | 0 to 255 | Unsigned 8-bit integer |
| `u16` | 2 bytes | 0 to 65,535 | Unsigned 16-bit integer |
| `u32` | 4 bytes | 0 to 4,294,967,295 | Unsigned 32-bit integer |
| `u64` | 8 bytes | 0 to 2^64 - 1 | Unsigned 64-bit integer |
| `u128` | 16 bytes | 0 to 2^128 - 1 | Unsigned 128-bit integer |
| `i8` | 1 byte | -128 to 127 | Signed 8-bit integer |
| `i16` | 2 bytes | -32,768 to 32,767 | Signed 16-bit integer |
| `i32` | 4 bytes | -2^31 to 2^31 - 1 | Signed 32-bit integer |
| `i64` | 8 bytes | -2^63 to 2^63 - 1 | Signed 64-bit integer (default integer) |
| `i128` | 16 bytes | -2^127 to 2^127 - 1 | Signed 128-bit integer |
| `f32` | 4 bytes | IEEE 754 single | 32-bit float |
| `f64` | 8 bytes | IEEE 754 double | 64-bit float (default float) |
| `str` | — | — | String slice (borrowed) |
| `String` | — | — | Owned string (heap-allocated) |
| `()` | 0 bytes | — | Unit type (zero-sized) |
| `!` | 0 bytes | — | Never type (unreachable) |

**Default types:**
- Integer literals default to `i64` unless annotated
- Float literals default to `f64` unless annotated
- Character literals are always `char`
- String literals are `&str` (borrowed) when borrowed, `String` when owned

### 5.2 Reference Types

References provide borrowed access to values without taking ownership. They are denoted with `&` (shared) or `&mut` (exclusive).

**Grammar:**
```
reference_type = "&" [lifetime] ["mut"] type
lifetime       = "'" identifier
```

**Types:**
- `&T` — shared reference: multiple readers, no writers
- `&mut T` — exclusive reference: one writer, no readers
- `&'a T` — reference with explicit lifetime
- `&'a mut T` — mutable reference with explicit lifetime

**Examples:**
```
&i32             // shared reference to i32
&mut String      // exclusive reference to String
&'a Vec<u8>      // reference with lifetime 'a
&'a mut [u8]     // mutable slice reference with lifetime
```

**Rules:**
- A shared reference may coexist with other shared references
- An exclusive reference cannot coexist with any other reference
- References cannot outlive the value they point to
- References are `Copy` (the reference itself can be copied, not the referent)

### 5.3 Pointer Types

Raw pointers are available for unsafe code. They have no borrow-checking guarantees.

**Grammar:**
```
pointer_type = "*" ("const" | "mut") type
```

**Types:**
- `*const T` — raw immutable pointer
- `*mut T` — raw mutable pointer

Raw pointers:
- Can be null
- Do not imply ownership
- Cannot be dereferenced outside `unsafe` blocks
- Are not `Send` or `Sync`

### 5.4 Array and Slice Types

**Arrays** are fixed-size, stack-allocated sequences:

```
[Type; Length]
```

**Examples:**
```
[i32; 5]        // array of 5 i32s
[bool; 256]      // array of 256 bools
[[f64; 3]; 3]    // 3x3 matrix
```

**Slices** are dynamically-sized views into contiguous sequences:

```
&[Type]          // shared slice
&mut [Type]      // mutable slice
```

**Examples:**
```
&[u8]            // shared byte slice
&mut [i32]       // mutable integer slice
```

**Relationship:** An array can be borrowed as a slice: `&[i32; 5]` coerces to `&[i32]`.

### 5.5 Tuple Types

Tuples are fixed-size collections of potentially different types:

```
(Type1, Type2, ..., TypeN)
```

**Examples:**
```
()                        // unit type
(i32, bool)               // 2-tuple
(i32, String, f64)        // 3-tuple
((i32, i32), (f64, f64))  // nested tuples
```

**Access:** Tuples are accessed by index: `let (a, b, c) = tuple;` or `tuple.0`, `tuple.1`.

### 5.6 Function Types

Function types are written as parameter types followed by return type:

```
(ParamType1, ParamType2, ...) -> ReturnType
```

**Examples:**
```
() -> i32                    // function taking nothing, returning i32
(i32, i32) -> i32           // function taking two i32s
(i32) -> ()                  // function taking i32, returning nothing
(&str) -> String             // function taking string slice, returning owned String
```

### 5.7 Generic Types

Generic types are parameterized by type variables:

```
TypeName<TypeParam1, TypeParam2, ...>
```

**Examples:**
```
Vector<i32>                  // vector of i32
HashMap<String, i32>         // hash map from String to i32
Result<T, E>                 // generic result type
Option<Vector<u8>>           // optional vector of bytes
```

**Type constraints** are specified with `where` or inline:

```
fn max<T: Ord>(a: T, b: T) -> T
fn sort<T: Ord + Copy>(items: &mut [T])
fn process<T>(data: T) where T: Debug + Clone
```

### 5.8 Algebraic Data Types (ADTs)

ADTs are defined with `enum` and can carry associated data:

```
enum Option<T> {
    Some(T),
    None,
}

enum Result<T, E> {
    Ok(T),
    Err(E),
}

enum Shape {
    Circle(f64),                        // radius
    Rectangle(f64, f64),                // width, height
    Triangle(f64, f64, f64),            // three sides
}
```

**Destructuring** in patterns:
```
match shape {
    Shape::Circle(r) => print("radius: {r}"),
    Shape::Rectangle(w, h) => print("{w}x{h}"),
    Shape::Triangle(a, b, c) => print("sides: {a}, {b}, {c}"),
}
```

### 5.9 Type Aliases

Type aliases create alternative names for existing types:

```
type Distance = f64;
type UserId = u64;
type Result<T> = core::result::Result<T, Error>;
```

### 5.10 Nullability

Nullable types are denoted with `?T`:

```
?T              // optional T, can be T or null
?i32            // optional i32
?String         // optional String
?&str           // optional string slice
```

**Unwrapping:**
```
let value: ?i32 = get_value();

// with match
match value {
    Some(v) => use(v),
    null => handle_missing(),
}

// with ?? (null coalescing)
let v = value ?? 0;  // use 0 if null

// with ? (error propagation) — only in functions returning Result
let v = value?;
```

### 5.11 Ownership Qualifiers

Ownership is annotated explicitly in function signatures and type expressions:

```
own T             // owned value (single owner, move semantics)
&T                // borrowed (shared reference)
&mut T            // borrowed (exclusive reference)
```

**In function parameters:**
```
fn process(data: own Vector<u8>)    // takes ownership
fn inspect(data: &Vector<u8>)       // borrows shared
fn modify(data: &mut Vector<u8>)    // borrows exclusively
```

**Move semantics:** Assignment moves ownership by default:
```
let a = Vector::new();
let b = a;           // a is moved to b; a is no longer usable
let c = b;           // b is moved to c
```

**Copy types:** Primitive types and types implementing `Copy` are bitwise-copied instead of moved:
```
let a: i32 = 5;
let b = a;           // a is copied, both a and b are valid
let c = a;           // a is copied again
```

### 5.12 Arena-Allocated Types

Arena allocation is a first-class primitive for batch-deallocated data:

```
arena {
    let temp = Vector::new();   // allocated in arena
    let more = String::from("hello");  // also in arena
}   // all arena allocations freed at once
```

Types within an `arena` block are reference-counted with the arena, not individually freed. The arena itself manages the backing memory.

### 5.13 Trait Types (Future)

Traits define shared interfaces. Planned for a future version of the spec:

```
trait Printable {
    fn print(&self);
}

impl Printable for MyStruct {
    fn print(&self) {
        // implementation
    }
}
```

### 5.14 Type Inference

Nexus uses Hindley-Milner type inference. Type annotations are optional in most cases:

```
let x = 5;              // inferred as i64
let y = 3.14;           // inferred as f64
let s = "hello";        // inferred as &str
let v = Vector::new();  // inferred from usage

fn add(a, b) { a + b }  // fully inferred
```

**Annotations are required when:**
- The compiler cannot determine a unique type
- Function return types are part of the public API
- Generic type parameters are ambiguous
- Integer/float type is ambiguous

```
let x: i32 = 5;         // explicit annotation
let y = 5i32;            // suffix notation
let z: i64 = 5;          // explicit
```

**Bidirectional type checking:** When the expected type is known, the compiler propagates type information inward:

```
fn process() -> Result<Data> {
    let x = compute()?;  // x is inferred as Data from return type
    return Ok(x);
}
```

---

## 6. Expressions

Expressions produce values. Every expression has a type and is evaluated at runtime (or optimized away by the compiler).

### 6.1 Literal Expressions

The simplest expressions are literals:

```
42          // i64
3.14        // f64
true        // bool
'a'         // char
"hello"     // &str
null        // null
[1, 2, 3]  // array
(1, "a")   // tuple
```

### 6.2 Variable Expressions

Identifiers reference bindings in scope:

```
x
my_function
count
```

### 6.3 Arithmetic Expressions

```
a + b       // addition
a - b       // subtraction
a * b       // multiplication
a / b       // division
a % b       // modulo
-a          // negation
```

**Rules:**
- Both operands must be the same numeric type (no implicit widening)
- Integer division by zero is a runtime error
- Float division by zero produces infinity or NaN (IEEE 754)
- Overflow in debug mode is a runtime error; in release mode wraps

### 6.4 Comparison Expressions

```
a == b      // equal
a != b      // not equal
a < b       // less than
a > b       // greater than
a <= b      // less than or equal
a >= b      // greater than or equal
```

**Rules:**
- Both operands must be the same type
- Returns `bool`
- Comparison operators require the type to implement the appropriate trait

### 6.5 Logical Expressions

```
!a          // logical not
a && b      // logical and (short-circuit)
a || b      // logical or (short-circuit)
```

**Rules:**
- Operands must be `bool`
- `&&` and `||` short-circuit: the right operand is not evaluated if the left determines the result

### 6.6 Bitwise Expressions

```
!a          // bitwise not
a & b       // bitwise and
a | b       // bitwise or
a ^ b       // bitwise xor
a << b      // left shift
a >> b      // right shift
```

**Rules:**
- Both operands must be the same integer type
- Shift amount must be less than the bit width (undefined behavior in release mode, error in debug)

### 6.7 Assignment Expressions

```
x = value           // assignment (move)
x += value          // compound assignment
x -= value
x *= value
x /= value
x %= value
x &= value
x |= value
x ^= value
x <<= value
x >>= value
```

**Rules:**
- The left-hand side must be a mutable lvalue (mutable variable, mutable reference, or mutable field)
- Assignment moves ownership by default (no implicit copy)
- Compound assignment desugars: `x += y` becomes `x = x + y` (with move semantics)
- Assignment expressions return `()` 

### 6.8 Function Call Expressions

```
function_name(arg1, arg2, ...)
```

**Examples:**
```
print("hello")
add(1, 2)
vector.push(42)
```

**Rules:**
- Number and types of arguments must match the function signature
- Arguments are evaluated left to right
- By default, arguments are moved (ownership transferred)
- Borrowed parameters use `&` or `&mut` syntax at the call site

**Method call syntax (sugar):**
```
object.method(args)    // desugars to method(&object, args) or method(&mut object, args)
```

### 6.9 Index Expressions

```
collection[index]
```

**Examples:**
```
arr[0]
vec[i]
matrix[row][col]
```

**Rules:**
- Index must be an integer type
- Out-of-bounds access is a runtime error (bounds checked)
- Returns a reference to the element: `&T` for shared, `&mut T` for mutable

### 6.10 Field Access Expressions

```
value.field_name
```

**Examples:**
```
point.x
user.name
result.value
```

**Rules:**
- Field access on a struct returns the field value
- With references: `(&s).field` auto-derefs to `s.field`
- Multiple levels of auto-deref: `(&(&s)).field` works

### 6.11 Method Call Expressions

```
receiver.method_name(args)
```

**Examples:**
```
vec.len()
string.push_str("hello")
option.unwrap()
```

**Rules:**
- Method resolution follows auto-deref rules
- Methods are looked up on the type and its deref chain
- `self` parameter is implicit in method definitions

### 6.12 Block Expressions

A block `{ ... }` is an expression. Its value is the last expression in the block:

```
let x = {
    let a = 5;
    let b = 10;
    a + b    // this is the block's value (15)
};
```

**Rules:**
- The last expression (without a semicolon) is the block's value
- If the last expression has a semicolon, the block returns `()`
- Blocks introduce a new scope

### 6.13 If Expressions

`if` is an expression (not a statement):

```
if condition {
    value_if_true
} else {
    value_if_false
}
```

**Examples:**
```
let x = if y > 0 { y } else { -y };
let result = if score >= 90 { "A" } else if score >= 80 { "B" } else { "C" };
```

**Rules:**
- Condition must be `bool`
- Both branches must have the same type (or the `else` branch can be omitted for `()`)
- `else if` chains are supported
- `else` is required if `if` produces a non-`()` value

### 6.14 Match Expressions

Pattern matching with exhaustive checking:

```
match value {
    pattern1 => expression1,
    pattern2 => expression2,
    pattern3 => {
        // multi-line block
        let x = compute();
        expression3
    },
}
```

**Examples:**
```
match number {
    0 => "zero",
    1 => "one",
    2..=9 => "single digit",
    _ => "other",
}

match result {
    Ok(data) => process(data),
    Err(e) => log("error: {e}"),
}

match shape {
    Shape::Circle(r) => 3.14159 * r * r,
    Shape::Rectangle(w, h) => w * h,
    Shape::Triangle(a, b, c) => {
        let s = (a + b + c) / 2.0;
        (s * (s - a) * (s - b) * (s - c)).sqrt()
    },
}
```

**Pattern forms:**
- Literals: `0`, `"hello"`, `true`
- Variables: `x` (binds the value)
- Wildcards: `_` (matches anything, binds nothing)
- Ranges: `1..=10`, `'a'..='z'`
- Tuples: `(a, b, c)`
- Structs: `Point { x, y }`
- Enums: `Option::Some(x)`, `Result::Ok(v)`
- Guards: `x if x > 0 => ...`
- OR patterns: `1 | 2 | 3 => ...`

**Rules:**
- Patterns must be exhaustive (all cases covered) or have a wildcard/`_`
- Match expressions are evaluated top-to-bottom; first matching arm wins
- Each arm is a block expression

### 6.15 Loop Expressions

**Infinite loop:**
```
loop {
    // runs forever
    if done { break; }
}
```

**While loop:**
```
while condition {
    // runs while condition is true
}
```

**For loop:**
```
for item in collection {
    // runs for each item
}

for i in 0..10 {
    // runs for i = 0, 1, ..., 9
}
```

**Loop as expression:** `loop` returns a value via `break`:

```
let result = loop {
    let value = compute();
    if value.is_valid() {
        break value;    // loop returns this value
    }
};
```

**Rules:**
- `break` exits the innermost loop
- `continue` skips to the next iteration
- `break value` is only valid in `loop` (not `while` or `for`)
- `for` loops use the `IntoIterator` trait

### 6.16 Return Expressions

```
return expression
```

**Rules:**
- Returns the value from the enclosing function
- The expression's type must match the function's return type
- `return` without expression returns `()`
- Implicit return: the last expression in a function body (without `return`) is the return value

```
fn add(a: i32, b: i32) -> i32 {
    a + b    // implicit return
}

fn greet() {
    print("hello");
    // implicit return of ()
}
```

### 6.17 Error Propagation

The `?` operator propagates errors up the call stack:

```
fn read_file(path: &str) -> Result<String> {
    let content = fs::read_to_string(path)?;  // returns Err early if error
    Ok(content)
}
```

**Rules:**
- `?` can only be used in functions returning `Result<T, E>` or `Option<T>`
- On `Err(e)`: the function returns `Err(e.into())` (error is converted via `Into`)
- On `null` (for `?T`): the function returns `null`
- Equivalent to: `match value { Ok(v) => v, Err(e) => return Err(e.into()) }`

### 6.18 Closure Expressions

Closures are anonymous functions:

```
|a, b| a + b              // inferred parameter and return types
|x: i32| -> i32 { x * 2 } // fully annotated
|data| {                    // multi-line closure
    let processed = process(data);
    transform(processed)
}
```

**Capture modes:**
- Closures capture variables by reference by default
- `move` keyword captures by value (ownership):
```
let data = Vector::new();
let closure = move || data.len();  // data is moved into closure
```

### 6.19 Range Expressions

```
start..end         // exclusive range [start, end)
start..=end        // inclusive range [start, end]
```

**Examples:**
```
0..10              // 0, 1, 2, ..., 9
0..=10             // 0, 1, 2, ..., 10
'a'..='z'          // all lowercase letters
```

### 6.20 Type Cast Expressions

Explicit type casting with `as`:

```
let x: i32 = 42;
let y: f64 = x as f64;
let z: u8 = 255u32 as u8;  // truncation
```

**Rules:**
- Numeric types can be cast between each other (may truncate or lose precision)
- `as` is explicit — no implicit widening or narrowing
- Reference types can be cast with `as` (unsafe for most cases)

### 6.21 Unsafe Expressions

Unsafe operations are wrapped in `unsafe { }` blocks:

```
let value = unsafe {
    *raw_pointer    // dereference raw pointer
};
```

**Rules:**
- `unsafe` blocks opt out of borrow-checking guarantees
- Only specific operations require `unsafe`: raw pointer dereference, calling unsafe functions, accessing mutable statics, implementing unsafe traits
- `unsafe` does not disable all checks — only borrow checking and certain memory safety checks

---

## 7. Statements

Statements control execution flow and bind names. Unlike expressions, statements do not produce values (except `let` bindings).

### 7.1 Let Statements

Let statements introduce bindings:

```
let x = 5;                  // immutable binding
let mut y = 10;             // mutable binding
let z: i32 = 42;            // with type annotation
let name = "nexus";         // inferred as &str
let (a, b) = (1, 2);       // destructuring
let Point { x, y } = point; // struct destructuring
```

**Rules:**
- Bindings are immutable by default
- `mut` makes a binding mutable (can be reassigned)
- Type annotation is optional (inferred by default)
- Destructuring patterns must match the value's shape
- `mut` bindings can be reassigned: `x = new_value;`
- Shadowing is allowed: `let x = 5; let x = 10;` (new binding shadows old)

**Move semantics:**
```
let a = Vector::new();
let b = a;          // a is moved to b
// a is no longer usable
```

### 7.2 Expression Statements

Any expression followed by a semicolon becomes a statement:

```
print("hello");     // expression statement
x + y;              // valid but useless (compiler warning)
compute();          // function call as statement
```

**Rules:**
- The expression is evaluated for its side effects
- The result is discarded
- The statement produces `()`

### 7.3 Block Statements

Blocks introduce new scopes:

```
{
    let x = 5;          // x is local to this block
    let y = x + 1;      // y is also local
    // x and y go out of scope here
}
// x and y are not accessible here
```

### 7.4 If Statements

When used as a statement (not expression):

```
if condition {
    do_something();
}

if condition {
    do_something();
} else {
    do_other();
}

if condition1 {
    do_first();
} else if condition2 {
    do_second();
} else {
    do_default();
}
```

### 7.5 Match Statements

When used as a statement:

```
match command {
    "start" => start_process(),
    "stop" => stop_process(),
    "restart" => {
        stop_process();
        start_process();
    },
    _ => unknown_command(),
}
```

### 7.6 Loop Statements

```
loop {
    let input = read_input();
    if input == "quit" { break; }
    process(input);
}

while running {
    tick();
}

for item in items {
    process(item);
}

for i in 0..100 {
    compute(i);
}
```

### 7.7 Return Statements

```
return;              // return ()
return value;        // return value
return Ok(result);   // return with error propagation setup
```

**Implicit return:** The last expression in a function body (without semicolon) is the return value:

```
fn double(x: i32) -> i32 {
    x * 2    // no semicolon — this is the return value
}
```

### 7.8 Break and Continue

```
break;               // exit innermost loop
break value;         // exit loop with value (only in `loop`)
continue;            // skip to next iteration
```

### 7.9 Import Statements

```
import std.io;                // import module
import std.io.{print, read}; // import specific items
use std::io;                  // use alias
use std::io as io;            // renamed import
```

### 7.10 Unsafe Statements

```
unsafe {
    let ptr = &raw as *const i32;
    let value = *ptr;
}
```

### 7.11 Arena Statements

```
arena {
    let data = load_large_dataset();
    let processed = transform(data);
    // processed is available here
}   // all arena allocations freed
// processed is no longer valid
```

### 7.12 Parallel Statements

```
parallel {
    let a = fetch(url_a);
    let b = fetch(url_b);
    let c = fetch(url_c);
}
// all three have completed
```

---

## 8. Functions

Functions are the fundamental unit of code organization. They encapsulate reusable computation.

### 8.1 Function Declarations

```
fn function_name(parameter: Type, parameter: Type) -> ReturnType {
    // body
}
```

**Examples:**
```
fn add(a: i32, b: i32) -> i32 {
    a + b
}

fn greet(name: &str) {
    print("Hello, {name}!");
}

fn no_args() -> bool {
    true
}

fn no_return() {
    print("side effect");
}
```

**Rules:**
- `fn` keyword followed by name, parameters, optional return type, and body
- Body is a block expression
- Return type is optional; omitting it means `()`
- Last expression (without semicolon) is the implicit return value
- `return` can be used for early returns

### 8.2 Parameters

Parameters are a comma-separated list of `name: Type` pairs:

```
fn process(data: &Vec<u8>, verbose: bool) -> Result<String> {
    // ...
}
```

**Parameter patterns:**
```
fn func(x: i32)                    // simple parameter
fn func(x: i32, y: i32)           // multiple parameters
fn func(point: Point { x, y })     // destructuring parameter
fn func(items: &[i32])             // slice reference
fn func(callback: fn(i32) -> i32)  // function pointer
```

**Ownership in parameters:**
```
fn take(own data: Vector<u8>)      // takes ownership
fn borrow(data: &Vector<u8>)       // borrows shared
fn mutate(data: &mut Vector<u8>)   // borrows exclusively
```

### 8.3 Default Parameters

```
fn connect(host: &str, port: u16 = 80, timeout: u64 = 30) {
    // ...
}

connect("example.com");                // uses defaults
connect("example.com", 443);          // overrides port
connect("example.com", 443, 60);      // overrides both
```

**Rules:**
- Default parameters must come after required parameters
- Default values must be compile-time constants
- Callers can skip defaulted parameters from the right

### 8.4 Variadic Parameters

```
fn printf(format: &str, args: ...) {
    // ...
}
```

**Rules:**
- Variadic parameters use `...` as the type
- Must be the last parameter
- Only allowed in `unsafe` functions or with specific compiler support

### 8.5 Generic Functions

Functions can be parameterized by type variables:

```
fn identity<T>(value: T) -> T {
    value
}

fn max<T: Ord>(a: T, b: T) -> T {
    if a > b { a } else { b }
}

fn map<T, U>(items: &[T], f: fn(T) -> U) -> Vec<U> {
    let mut result = Vec::new();
    for item in items {
        result.push(f(item));
    }
    result
}
```

**Type constraints:**
```
fn sort<T: Ord + Copy>(items: &mut [T]) { ... }
fn print_all<T: Printable>(items: &[T]) { ... }
fn process<T>(data: T) where T: Debug + Clone { ... }
```

### 8.6 Closures

Closures are anonymous functions that capture their environment:

```
let add = |a, b| a + b;
let x = 5;
let add_x = |n| n + x;      // captures x

let mut counter = 0;
let mut increment = || { counter += 1; };
increment();
```

**Capture modes:**
- `|args| body` — captures by reference (shared or mutable as needed)
- `move |args| body` — captures by value (moves into closure)

**Closure types:**
- `fn(T) -> U` — function pointer (no capture)
- `impl Fn(T) -> U` — closure trait (shared borrow)
- `impl FnMut(T) -> U` — mutable closure trait (mutable borrow)
- `impl FnOnce(T) -> U` — once closure trait (moves captured values)

### 8.7 Function Pointers

Functions can be passed as values:

```
fn apply(f: fn(i32) -> i32, x: i32) -> i32 {
    f(x)
}

let result = apply(double, 5);  // 10
```

**Function pointer type:** `fn(ParamTypes) -> ReturnType`

### 8.8 Higher-Order Functions

Functions that take or return other functions:

```
fn compose(f: fn(i32) -> i32, g: fn(i32) -> i32) -> fn(i32) -> i32 {
    |x| f(g(x))
}

fn filter<T>(items: &[T], predicate: fn(&T) -> bool) -> Vec<&T> {
    let mut result = Vec::new();
    for item in items {
        if predicate(item) {
            result.push(item);
        }
    }
    result
}
```

### 8.9 Entry Point

The program entry point is the `main` function:

```
fn main() {
    // program starts here
}

fn main() -> Result<()> {
    // can return Result for error handling
    let config = load_config()?;
    run(config)
}
```

**Rules:**
- `main` takes no parameters
- Return type is `()` or `Result<()>`
- If `main` returns `Result<()>`, the exit code is 0 on success, 1 on error

### 8.10 External Functions

Functions defined in other languages via FFI:

```
extern "C" {
    fn printf(format: *const u8, ...) -> i32;
    fn malloc(size: usize) -> *mut u8;
    fn free(ptr: *mut u8);
}
```

**Rules:**
- `extern "C"` declares a C-compatible function
- Calling convention is platform-specific (System V on Unix, Windows x64 on Windows)
- External functions are implicitly `unsafe`
- Parameters and return types must be FFI-safe (no closures, no owned types)

---

## 9. Structs and Enums

Structs and enums are the building blocks of user-defined types.

### 9.1 Struct Declarations

Structs are named, heterogeneous collections of named fields:

```
struct Point {
    x: f64,
    y: f64,
}

struct User {
    name: String,
    email: String,
    age: u32,
    active: bool,
}
```

**Fieldless structs:**
```
struct Marker;
struct Unit;
```

**Generic structs:**
```
struct Vector<T> {
    data: *mut T,
    len: usize,
    cap: usize,
}

struct Pair<A, B> {
    first: A,
    second: B,
}
```

**Tuple structs (unnamed fields):**
```
struct Color(u8, u8, u8);
struct Meters(f64);
struct Matrix4x4([[f64; 4]; 4]);
```

### 9.2 Struct Instantiation

```
let point = Point { x: 1.0, y: 2.0 };
let user = User {
    name: "Alice".into(),
    email: "alice@example.com".into(),
    age: 30,
    active: true,
};
```

**Field init shorthand:** When variable name matches field name:
```
let name = "Alice";
let user = User {
    name: name.into(),    // can shorten to: name: name.into()
    email: "alice@example.com".into(),
    age: 30,
    active: true,
};
```

**Struct update syntax:**
```
let user2 = User {
    active: false,
    ..user    // copy remaining fields from user
};
```

**Rules:**
- All fields must be provided (unless using `..` update syntax)
- Field order does not matter
- Field values are moved by default (unless the type is `Copy`)

### 9.3 Field Access

```
let x = point.x;
point.y = 5.0;        // mutable access
```

**Nested field access:**
```
let name = user.name;
let first_char = user.name.bytes().next();
```

### 9.4 Methods

Methods are functions defined on a struct:

```
impl Point {
    fn new(x: f64, y: f64) -> Self {
        Point { x, y }
    }

    fn distance(&self, other: &Point) -> f64 {
        let dx = self.x - other.x;
        let dy = self.y - other.y;
        (dx * dx + dy * dy).sqrt()
    }

    fn translate(&mut self, dx: f64, dy: f64) {
        self.x += dx;
        self.y += dy;
    }

    fn into_tuple(self) -> (f64, f64) {
        (self.x, self.y)
    }
}
```

**Self parameter conventions:**
- `&self` — borrows the instance (most common)
- `&mut self` — mutably borrows the instance
- `self` — takes ownership (consumes the instance)
- No `self` — associated function (called with `Type::function()`)

**Calling methods:**
```
let p = Point::new(1.0, 2.0);     // associated function
let d = p.distance(&other);        // method call
p.translate(3.0, 4.0);            // mutable method
let tuple = p.into_tuple();       // consuming method
```

### 9.5 Enum Declarations

Enums are sum types — values that can be one of several variants:

**Simple enums:**
```
enum Direction {
    North,
    South,
    East,
    West,
}

enum Color {
    Red,
    Green,
    Blue,
    Custom(u8, u8, u8),
}
```

**Enums with associated data:**
```
enum Shape {
    Circle(f64),                        // radius
    Rectangle(f64, f64),                // width, height
    Triangle(f64, f64, f64),            // three sides
}

enum Result<T, E> {
    Ok(T),
    Err(E),
}

enum Option<T> {
    Some(T),
    None,
}
```

**Named-field enums:**
```
enum Message {
    Quit,
    Move { x: i32, y: i32 },
    Write(String),
    ChangeColor(u8, u8, u8),
}
```

**Generic enums:**
```
enum Tree<T> {
    Leaf(T),
    Node {
        value: T,
        left: Box<Tree<T>>,
        right: Box<Tree<T>>,
    },
}
```

### 9.6 Enum Instantiation

```
let dir = Direction::North;
let shape = Shape::Circle(5.0);
let msg = Message::Move { x: 10, y: 20 };
let color = Color::Custom(255, 128, 0);
```

### 9.7 Pattern Matching on Enums

```
match shape {
    Shape::Circle(r) => {
        print("Circle with radius {r}");
    },
    Shape::Rectangle(w, h) => {
        print("Rectangle {w}x{h}");
    },
    Shape::Triangle(a, b, c) => {
        print("Triangle with sides {a}, {b}, {c}");
    },
}
```

**Exhaustiveness:** The compiler enforces that all variants are handled:
```
match direction {
    Direction::North => "up",
    Direction::South => "down",
    Direction::East => "right",
    Direction::West => "left",
    // No wildcard needed — all variants covered
}
```

### 9.8 Enum Methods

```
impl Shape {
    fn area(&self) -> f64 {
        match self {
            Shape::Circle(r) => 3.14159 * r * r,
            Shape::Rectangle(w, h) => w * h,
            Shape::Triangle(a, b, c) => {
                let s = (a + b + c) / 2.0;
                (s * (s - a) * (s - b) * (s - c)).sqrt()
            },
        }
    }

    fn describe(&self) -> &str {
        match self {
            Shape::Circle(_) => "circle",
            Shape::Rectangle(_, _) => "rectangle",
            Shape::Triangle(_, _, _) => "triangle",
        }
    }
}
```

### 9.9 Operator Overloading

Operators are overloaded by implementing traits:

```
impl Add for Point {
    type Output = Point;

    fn add(self, other: Point) -> Point {
        Point {
            x: self.x + other.x,
            y: self.y + other.y,
        }
    }
}

impl Display for Point {
    fn fmt(&self, f: &mut Formatter) -> Result {
        write!(f, "({}, {})", self.x, self.y)
    }
}
```

**Common overloadable traits:**

| Trait | Operator | Description |
|-------|----------|-------------|
| `Add` | `+` | Addition |
| `Sub` | `-` | Subtraction |
| `Mul` | `*` | Multiplication |
| `Div` | `/` | Division |
| `Rem` | `%` | Remainder |
| `Neg` | `-` (unary) | Negation |
| `Not` | `!` | Logical/bitwise not |
| `And` | `&&` | Logical and |
| `Or` | `\|\|` | Logical or |
| `Eq` | `==` | Equality |
| `Ord` | `<`, `>`, `<=`, `>=` | Ordering |
| `Index` | `[]` | Indexing |
| `Display` | `{}` | String formatting |
| `Debug` | `{:?}` | Debug formatting |

### 9.10 Visibility

By default, struct fields and enum variants are **private** to their module:

```
pub struct Point {
    pub x: f64,       // public field
    y: f64,           // private field
}
```

**Visibility modifiers:**
- `pub` — public (visible everywhere)
- `pub(packet)` — public within the packet
- `pub(super)` — public in the parent module
- `pub(in path)` — public in the specified path
- No modifier — private (visible in current module and its children)

---

## 10. Module System

Nexus uses a file-based module system. The module tree mirrors the directory structure.

### 10.1 Module Files

Each directory can have a `mod.nx` file that defines the module:

```
src/
├── main.nx           // packet root
├── lib.nx            // library root (alternative)
├── io/
│   ├── mod.nx        // io module definition
│   ├── read.nx       // io::read submodule
│   └── write.nx      // io::write submodule
└── utils.nx          // utils module
```

**Module declaration in `main.nx`:**
```
mod io;               // declares io module (looks for io/mod.nx or io.nx)
mod utils;            // declares utils module (looks for utils.nx)
```

### 10.2 File Modules

A single file can be a module:

```
// utils.nx
fn helper() -> i32 {
    42
}

pub fn public_helper() -> i32 {
    helper()
}
```

### 10.3 Nested Modules

Modules can be nested:

```
// mod.nx (root)
mod io {
    pub fn read() -> String {
        // ...
    }

    pub fn write(data: &str) {
        // ...
    }
}
```

Or in separate files:
```
// mod.nx
pub mod read;
pub mod write;

// read.nx
pub fn read_line() -> String {
    // ...
}

// write.nx
pub fn write_line(data: &str) {
    // ...
}
```

### 10.4 Import System

**Module imports:**
```
import std.io;                 // import module
import std.io.{print, read};  // import specific items
import std.io::*;              // import everything (discouraged)
```

**Use declarations:**
```
use std::io;
use std::io::{print, read};
use std::io as io_lib;         // renamed import
```

**Relative imports:**
```
use self::helper;              // current module
use super::parent_helper;      // parent module
use packet::root_helper;        // packet root
```

### 10.5 Export System

By default, items are private. Use `pub` to make them public:

```
pub fn public_function() { }
pub struct PublicStruct { }
pub enum PublicEnum { }
pub type PublicType = i32;
pub const PUBLIC_CONST: i32 = 42;
```

**Re-exports:**
```
pub use io::read;       // re-export read from io
pub use io::*;          // re-export everything from io
```

### 10.6 Predefined Modules

Nexus provides the following standard modules (planned):

| Module | Description |
|--------|-------------|
| `std` | Standard library root |
| `std.io` | Input/output |
| `std.fs` | Filesystem operations |
| `std.net` | Networking |
| `std.collections` | Vector, HashMap, Set, etc. |
| `std.string` | String operations |
| `std.math` | Math functions |
| `std.result` | Result type |
| `std.option` | Option type |
| `std.error` | Error trait |
| `std.fmt` | Formatting |
| `std.env` | Environment variables |
| `std.process` | Process management |
| `std.time` | Time operations |
| `std.json` | JSON parsing/serialization |
| `std.async` | Async/await primitives |

### 10.7 Packet Structure

A Nexus project has a packet root (`main.nx` for binaries, `lib.nx` for libraries):

```
my_project/
├── nexus.toml        // project manifest
├── src/
│   ├── main.nx       // packet root (binary)
│   ├── lib.nx        // packet root (library)
│   ├── io.nx         // module
│   └── utils/        // module directory
│       └── mod.nx
├── tests/            // test files
└── examples/         // example files
```

---

## 11. Agent-Specific Syntax

Nexus includes purpose-built syntax for AI agent workflows. This section defines the agent-specific constructs.

### 11.1 Agent Declaration

Agents are first-class entities:

```
agent = Agent(model: "gpt-4")
agent = Agent(model: "claude-3-opus", temperature: 0.7)
```

**Agent creation:**
```
let agent = Agent {
    model: "gpt-4",
    temperature: 0.7,
    max_tokens: 4096,
    system_prompt: "You are a helpful assistant.",
}
```

### 11.2 Agent Pipeline Operator

The `|=>` operator chains tool calls into a pipeline:

```
agent |=> tool1(arg) |=> tool2() |=> tool3()
```

**Equivalent to:**
```
let result1 = agent.call(tool1, arg);
let result2 = result1.call(tool2);
let result3 = result2.call(tool3);
```

**Examples:**
```
agent |=> search_web(query) |=> summarize() |=> respond()

agent |=> read_file(path) |=> parse_json() |=> validate(schema)

agent |=> fetch_data(url) |=> transform() |=> save(output_path)
```

**Rules:**
- `|=>` left-associates: `a |=> b |=> c` means `(a |=> b) |=> c`
- The result of each tool call is passed as input to the next
- Tool calls can take arguments
- Pipeline results can be bound: `let result = agent |=> tool() |=> tool2()`

### 11.3 Parallel Blocks

Structured concurrency with `parallel`:

```
parallel {
    let a = fetch(url_a)
    let b = fetch(url_b)
    let c = fetch(url_c)
}
```

**Rules:**
- All statements in a `parallel` block execute concurrently
- The block waits for all statements to complete before continuing
- Each statement can bind a variable
- Variables bound inside `parallel` are accessible after the block
- Nested `parallel` blocks are allowed

**Error handling in parallel:**
```
parallel {
    let a = fetch(url_a)?
    let b = fetch(url_b)?
    let c = fetch(url_c)?
}   // if any fails, all are cancelled and error propagates
```

### 11.4 Async/Await

Asynchronous operations:

```
async fn fetch_data(url: &str) -> Result<Data> {
    let response = http_get(url).await?;
    let data = parse(response).await?;
    Ok(data)
}
```

**Rules:**
- `async` before `fn` makes it an async function
- `.await` pauses until the async operation completes
- Async functions return `Future` ( polled by the runtime)
- `async` blocks create anonymous futures:
```
let future = async {
    let a = fetch(url_a).await;
    let b = fetch(url_b).await;
    a + b
};
```

### 11.5 Tool Declarations

Tools are reusable functions that agents can call:

```
tool search_web(query: &str) -> Vec<Result> {
    // implementation
}

tool summarize(text: &str) -> String {
    // implementation
}

tool respond(message: &str) {
    print(message);
}
```

**Rules:**
- `tool` keyword declares a callable tool
- Tools have typed parameters and return types
- Tools can be composed with `|=>`
- Tools are async by default (can use `.await`)

### 11.6 Schema Declarations

Schemas define structured data formats:

```
schema UserSchema {
    name: String,
    age: u32,
    email: String,
}

schema ResponseSchema {
    status: u16,
    data: UserSchema,
    timestamp: u64,
}
```

**Rules:**
- `schema` declares a structured data type
- Schemas can validate JSON/structured data at runtime
- Schema types can be used as regular types
- Compile-time checking where possible

---

## 12. Grammar Summary

This section provides a complete EBNF grammar for the Nexus language.

### 12.1 Lexical Grammar

```ebnf
(* Whitespace and comments *)
whitespace     = " " | "\t" | "\n" | "\r" ;
line_comment   = "//" { !"\n" } ;
block_comment  = "/*" { block_comment | !"*/" } "*/" ;
comment        = line_comment | block_comment ;

(* Identifiers *)
identifier     = ( letter | "_" ) { letter | digit | "_" } ;
letter         = "a".."z" | "A".."Z" | unicode_letter ;
digit          = "0".."9" ;

(* Integer literals *)
integer_literal = decimal_literal | hex_literal | octal_literal | binary_literal ;
decimal_literal = digit { digit | "_" } ;
hex_literal     = "0x" hex_digit { hex_digit | "_" } ;
octal_literal   = "0o" oct_digit { oct_digit | "_" } ;
binary_literal  = "0b" ("0" | "1") { ("0" | "1") | "_" } ;

(* Float literals *)
float_literal  = decimal_literal "." decimal_literal [ exponent ]
               | decimal_literal exponent
               | decimal_literal float_suffix ;
exponent       = ("e" | "E") ["+" | "-"] decimal_literal ;
float_suffix   = "f32" | "f64" ;

(* String literals *)
string_literal = regular_string | raw_string ;
regular_string = '"' { string_char } '"' ;
string_char    = normal_char | escape_char ;
escape_char    = '\' ( "n" | "r" | "t" | "0" | "\\" | '"' | "'"
                    | "u{" hex_digit { hex_digit } "}" | "x" hex_digit hex_digit ) ;
normal_char    = ? any Unicode character except '"', '\', and control characters ? ;
raw_string     = "r" { "#" } '"' { any_char } '"' { "#" } ;

(* Character literals *)
char_literal   = "'" ( char_char | escape_char ) "'" ;
char_char      = ? any Unicode character except "'", '\', and control characters ? ;

(* Boolean literals *)
bool_literal   = "true" | "false" ;

(* Null literal *)
null_literal   = "null" ;
```

### 12.2 Type Grammar

```ebnf
type           = primitive_type | reference_type | pointer_type | array_type
               | slice_type | tuple_type | function_type | generic_type
               | nullable_type | ownership_type ;
primitive_type = "bool" | "char" | "u8" | "u16" | "u32" | "u64" | "u128"
               | "i8" | "i16" | "i32" | "i64" | "i128" | "f32" | "f64"
               | "str" | "String" | "()" | "!" ;
reference_type = "&" [ lifetime ] ["mut"] type ;
lifetime       = "'" identifier ;
pointer_type   = "*" ("const" | "mut") type ;
array_type     = "[" type ";" expr "]" ;
slice_type     = "&" ["mut"] "[" type "]" ;
tuple_type     = "(" [ type { "," type } ] ")" ;
function_type  = "(" [ type { "," type } ] ")" "->" type ;
generic_type   = identifier "<" type { "," type } ">" ;
nullable_type  = "?" type ;
ownership_type = "own" type ;
```

### 12.3 Expression Grammar

```ebnf
expression     = literal_expr | identifier_expr | binary_expr | unary_expr
               | call_expr | index_expr | field_expr | block_expr
               | if_expr | match_expr | loop_expr | return_expr
               | closure_expr | range_expr | cast_expr | unsafe_expr ;

literal_expr   = integer_literal | float_literal | string_literal
               | char_literal | bool_literal | null_literal
               | array_literal | tuple_literal ;
array_literal  = "[" [ expr { "," expr } [ "," ] ] "]" ;
tuple_literal  = "(" [ expr { "," expr } ] ")" ;

binary_expr    = expression binop expression ;
unary_expr     = unary_op expression ;
unary_op       = "-" | "!" | "&" | "*" ;

call_expr      = expression "(" [ expr { "," expr } ] ")" ;
index_expr     = expression "[" expression "]" ;
field_expr     = expression "." identifier ;

block_expr     = "{" statement* expression? "}" ;

if_expr        = "if" expression block_expr [ "else" ( if_expr | block_expr ) ] ;
match_expr     = "match" expression "{" match_arm* "}" ;
match_arm      = pattern [ "if" expression ] "=>" ( expression | block ) "," ;

loop_expr      = loop_block | while_expr | for_expr ;
loop_block     = "loop" block_expr ;
while_expr     = "while" expression block_expr ;
for_expr       = "for" identifier "in" expression block_expr ;

return_expr    = "return" [ expression ] ;

closure_expr   = "|" [ param_list ] "|" [ "->" type ] block_expr
               | "move" "|" [ param_list ] "|" [ "->" type ] block_expr ;

range_expr     = expression ".." expression | expression "..=" expression ;
cast_expr      = expression "as" type ;
unsafe_expr    = "unsafe" block_expr ;
```

### 12.4 Statement Grammar

```ebnf
statement      = let_stmt | expr_stmt | block_stmt | if_stmt
               | match_stmt | loop_stmt | return_stmt
               | break_stmt | continue_stmt | import_stmt
               | unsafe_stmt | arena_stmt | parallel_stmt ;

let_stmt       = "let" ["mut"] pattern [":" type] "=" expression ";" ;
expr_stmt      = expression ";" ;
block_stmt     = block_expr ;
if_stmt        = if_expr ;
match_stmt     = match_expr ;
loop_stmt      = loop_expr | while_expr | for_expr ;
return_stmt    = return_expr ";" ;
break_stmt     = "break" [ expression ] ";" ;
continue_stmt  = "continue" ";" ;
import_stmt    = "import" import_path ["." "{" import_list "}"] ";" ;
unsafe_stmt    = "unsafe" block_expr ;
arena_stmt     = "arena" block_expr ;
parallel_stmt  = "parallel" block_expr ;
```

### 12.5 Declaration Grammar

```ebnf
declaration    = fn_decl | struct_decl | enum_decl | type_decl
               | mod_decl | use_decl | trait_decl | impl_decl
               | agent_decl | tool_decl | schema_decl ;

fn_decl        = "fn" identifier [ generic_params ] "(" [ param_list ] ")"
                 [ "->" type ] [ where_clause ] block_expr ;
struct_decl    = "struct" identifier [ generic_params ]
                 ( "{" field_list "}" | "(" type_list ")" ) ;
enum_decl      = "enum" identifier [ generic_params ]
                 "{" enum_variant { "," enum_variant } [ "," ] "}" ;
type_decl      = "type" identifier [ generic_params ] "=" type ";" ;
mod_decl       = "mod" identifier ";" | "mod" identifier block_expr ;
use_decl       = "use" import_path [ "as" identifier ] ";" ;
trait_decl     = "trait" identifier [ generic_params ] [ where_clause ]
                 "{" trait_item* "}" ;
impl_decl      = "impl" [ generic_params ] type [ "for" type ]
                 [ where_clause ] "{" impl_item* "}" ;
agent_decl     = "agent" identifier "=" "Agent" "(" agent_config ")" ;
tool_decl      = "tool" identifier "(" [ param_list ] ")" [ "->" type ] block_expr ;
schema_decl    = "schema" identifier "{" field_list "}" ;

generic_params = "<" generic_param { "," generic_param } ">" ;
generic_param  = identifier [ ":" trait_bounds ] ;
trait_bounds   = trait_bound { "+" trait_bound } ;
trait_bound    = identifier [ "<" type { "," type } ">" ] ;
where_clause   = "where" where_bound { "," where_bound } ;
where_bound    = type ":" trait_bounds ;

param_list     = param { "," param } ;
param          = ["own" | "ref" | "mut"] identifier ":" type ;
field_list     = field { "," field } ;
field          = ["pub"] identifier ":" type ;
enum_variant   = identifier [ "(" type { "," type } ")"
                           | "{" field { "," field } "}" ] ;
```

### 12.6 Pattern Grammar

```ebnf
pattern        = literal_pattern | identifier_pattern | wildcard_pattern
               | tuple_pattern | struct_pattern | enum_pattern
               | range_pattern | or_pattern | ref_pattern
               | deref_pattern ;

literal_pattern   = integer_literal | float_literal | string_literal
                  | char_literal | bool_literal ;
identifier_pattern = identifier ;
wildcard_pattern  = "_" ;
tuple_pattern     = "(" [ pattern { "," pattern } ] ")" ;
struct_pattern    = identifier "{" [ field_pattern { "," field_pattern } ] "}" ;
field_pattern     = identifier [ ":" pattern ] ;
enum_pattern      = identifier ["::" identifier] [ "(" [ pattern { "," pattern } ] ")" ] ;
range_pattern     = literal ".." literal | literal "..=" literal ;
or_pattern        = pattern { "|" pattern } ;
ref_pattern       = "ref" identifier ;
deref_pattern     = "*" pattern ;
```

### 12.7 Program Grammar

```ebnf
program        = { declaration | statement | comment | whitespace } ;
```

---

## 13. Ownership and Borrowing Rules

Nexus enforces memory safety at compile time through an ownership system. Every value has exactly one owner. When the owner goes out of scope, the value is dropped (deallocated). Borrowing allows temporary access without taking ownership.

### 13.1 Ownership Rules

**Rule 1: Every value has exactly one owner.**

```
let s = String::from("hello");  // s owns the String
```

**Rule 2: When the owner goes out of scope, the value is dropped.**

```
{
    let s = String::from("hello");
    // s is valid here
}   // s goes out of scope; String is dropped (memory freed)
// s is no longer valid
```

**Rule 3: Assignment moves ownership (unless the type is `Copy`).**

```
let a = String::from("hello");
let b = a;          // ownership moves from a to b
// a is no longer valid
// b is valid
```

**Rule 4: Function arguments move ownership (unless borrowed).**

```
fn take(s: String) {
    // s owns the String
}   // s is dropped here

let name = String::from("Nexus");
take(name);         // ownership moves into take()
// name is no longer valid
```

**Rule 5: Function return values transfer ownership to the caller.**

```
fn create() -> String {
    let s = String::from("hello");
    s               // ownership moves to caller
}

let s = create();   // s owns the String
```

### 13.2 Copy Types

Some types are `Copy` — they are bitwise-copied on assignment instead of moved. A type is `Copy` if all its fields are `Copy`.

**Built-in Copy types:**
- All integer types: `u8`–`u128`, `i8`–`i128`
- Float types: `f32`, `f64`
- `bool`
- `char`
- Tuples of Copy types: `(i32, bool)` is `Copy`, `(i32, String)` is not
- Fixed-size arrays of Copy types: `[i32; 5]` is `Copy`
- References: `&T` and `&mut T` are `Copy` (the reference itself, not the referent)

**Non-Copy types:**
- `String`
- `Vector<T>` (unless T is `Copy`)
- `HashMap<K, V>`
- Any type containing a non-Copy field

```
let a: i32 = 5;
let b = a;          // a is copied; both a and b are valid
println!("{a} {b}"); // works

let x = String::from("hello");
let y = x;          // x is moved; only y is valid
// println!("{x}");  // ERROR: x is no longer valid
```

### 13.3 Move Semantics

Moves transfer ownership from one binding to another. After a move, the source binding is invalidated.

**Where moves happen:**
- Assignment: `let b = a;`
- Function argument: `func(a)`
- Function return: `return a;`
- Pattern matching: `let (x, y) = tuple;` (moves out of tuple)
- Field access: `let name = user.name;` (moves field out of struct)

**Move restrictions:**
- You cannot use a value after it is moved
- You cannot move a value that is borrowed
- You cannot move individual fields out of a struct that is borrowed
- You cannot move out of an index into a container (the container owns the elements)

```
let point = Point { x: 1.0, y: 2.0 };
let x = point.x;     // moves x field out of point
// point is partially moved; point.y is still accessible
let y = point.y;     // moves y field
// point is now fully moved
```

### 13.4 Borrowing Rules

Borrowing allows you to refer to a value without taking ownership. There are two kinds of borrows:

**Shared borrow (`&T`):**
- Multiple shared borrows can coexist
- No exclusive borrows can coexist with shared borrows
- The referent cannot be modified through a shared borrow

**Exclusive borrow (`&mut T`):**
- Only one exclusive borrow can exist at a time
- No shared borrows can coexist with an exclusive borrow
- The referent can be modified through an exclusive borrow

**Rule 1: At any given time, you can have EITHER one exclusive borrow OR any number of shared borrows.**

```
let mut s = String::from("hello");

// Multiple shared borrows — OK
let r1 = &s;
let r2 = &s;
println!("{r1} {r2}");

// Exclusive borrow — OK (no shared borrows active)
let r3 = &mut s;
r3.push_str(" world");
println!("{r3}");
```

**Rule 2: Borrows must not outlive the referent.**

```
let r;
{
    let s = String::from("hello");
    r = &s;         // ERROR: s does not live long enough
}   // s is dropped here
// r would be a dangling reference
```

**Rule 3: A borrow scope is from the borrow point to the last use.**

```
let mut s = String::from("hello");

let r1 = &s;
let r2 = &s;        // r1 and r2 coexist — OK
println!("{r1} {r2}");
// r1 and r2 are no longer used after this point

let r3 = &mut s;    // OK: r1 and r2 are no longer active
r3.push_str(" world");
println!("{r3}");
```

### 13.5 Borrowing in Practice

**Function parameters:**

```
fn inspect(s: &String) {        // shared borrow
    println!("{s}");
}

fn modify(s: &mut String) {     // exclusive borrow
    s.push_str(" world");
}

fn take(s: String) {            // ownership transfer
    // s is consumed
}

let mut name = String::from("Nexus");

inspect(&name);         // borrows name shared
modify(&mut name);      // borrows name exclusively
take(name);             // moves name
// name is no longer valid
```

**Method calls:**

```
let mut v = Vector::new();
v.push(1);              // borrows v exclusively (push takes &mut self)
let len = v.len();      // borrows v shared (len takes &self)
```

### 13.6 Dangling References

The compiler prevents dangling references — references that point to freed memory:

```
fn dangling() -> &String {      // ERROR: missing lifetime annotation
    let s = String::from("hello");
    &s      // s is dropped when function returns; reference would dangle
}
```

**Solution:** Return ownership, not a reference:

```
fn not_dangling() -> String {
    let s = String::from("hello");
    s       // ownership transfers to caller
}
```

### 13.7 Self-Referential Types

Self-referential types (a struct that contains a reference to its own field) are not permitted in safe Nexus. The ownership system prevents creating such types because the borrow checker cannot express the relationship between the two fields.

**Workaround:** Use indices instead of references:

```
// INVALID:
struct Bad {
    data: String,
    slice: &str,        // reference into data — NOT ALLOWED
}

// VALID:
struct Good {
    data: String,
    start: usize,
    end: usize,         // indices into data
}
```

### 13.8 Interior Mutability

Some types allow mutation through a shared reference. This is handled through special wrapper types (planned for future spec versions):

```
let cell = Cell::new(5);
let ref_cell = RefCell::new(String::from("hello"));

// Cell: copy-based interior mutability (no borrow checking at runtime)
cell.set(10);
let val = cell.get();

// RefCell: runtime borrow checking
{
    let mut borrowed = ref_cell.borrow_mut();
    borrowed.push_str(" world");
}   // mutable borrow dropped here
let borrowed = ref_cell.borrow();
println!("{borrowed}");
```

### 13.9 Drop Semantics

When a value is dropped (goes out of scope or is explicitly consumed), its `drop` method is called. Drop order:

1. Local variables are dropped in reverse declaration order
2. Struct fields are dropped in declaration order
3. Enum variant data is dropped
4. Heap-allocated memory is freed

```
struct Resource {
    name: String,
}

impl Drop for Resource {
    fn drop(&mut self) {
        println!("Dropping resource: {self.name}");
    }
}

{
    let a = Resource { name: "first".into() };
    let b = Resource { name: "second".into() };
}   // b is dropped first, then a
// Output:
// Dropping resource: second
// Dropping resource: first
```

### 13.10 Unsafe Escape Hatch

The `unsafe` block disables borrow checking for specific operations:

```
unsafe {
    let raw = &mut value as *mut i32;
    *raw = 42;          // write through raw pointer
}
```

**Operations that require `unsafe`:**
- Dereferencing raw pointers
- Calling unsafe functions
- Accessing mutable static variables
- Implementing unsafe traits
- Accessing fields of a union

**Rules:**
- `unsafe` does not disable all checks — only borrow checking
- Bounds checking, overflow checking, and type checking still apply
- `unsafe` blocks should be as small as possible
- All `unsafe` code should have a comment explaining why it is safe

---

## 14. Lifetime Inference

Lifetimes describe the scope for which a reference is valid. Nexus uses lifetime inference to minimize explicit annotations while preventing dangling references.

### 14.1 Lifetime Basics

A lifetime is a stretch of code for which a reference is guaranteed to be valid:

```
{
    let r;                 // ------+-- 'a: lifetime of r
    {                       //       |
        let x = 5;         // --+   |
        r = &x;            //   |   |  'b: lifetime of x
    }                       // --+   |  x goes out of scope
    // println!("{r}");     //       |  ERROR: x does not live long enough
}                           // ------+
```

### 14.2 Lifetime Annotations

Lifetime annotations are written as `'name`:

```
&'a T        // reference with lifetime 'a
&'a mut T    // mutable reference with lifetime 'a
```

**Lifetime annotations do not change how long references live.** They describe relationships between lifetimes of multiple references.

### 14.3 Lifetime Elision Rules

The compiler infers lifetimes in common patterns. Explicit annotations are only needed when the compiler cannot determine the relationship.

**Elision rule 1: Each reference parameter gets its own lifetime.**

```
// Written:
fn foo(x: &str, y: &str) -> &str { ... }

// Elided to:
fn foo<'a, 'b>(x: &'a str, y: &'b str) -> &str { ... }
```

**Elision rule 2: If there is exactly one input lifetime, it is assigned to all output lifetimes.**

```
// Written:
fn first(s: &str) -> &str { ... }

// Elided to:
fn first<'a>(s: &'a str) -> &'a str { ... }
```

**Elision rule 3: If one of the parameters is `&self` or `&mut self`, the lifetime of `self` is assigned to all output lifetimes.**

```
// Written:
impl Foo {
    fn bar(&self, x: &str) -> &str { ... }
}

// Eladed to:
impl Foo {
    fn bar<'a, 'b>(&'a self, x: &'b str) -> &'a str { ... }
}
```

**When elision fails:** The compiler requires explicit annotations:

```
// ERROR: cannot infer lifetime
fn longest(x: &str, y: &str) -> &str {
    if x.len() > y.len() { x } else { y }
}

// FIXED: output lifetime is min of input lifetimes
fn longest<'a>(x: &'a str, y: &'a str) -> &'a str {
    if x.len() > y.len() { x } else { y }
}
```

### 14.4 Lifetime Inference Algorithm

The compiler uses the following algorithm:

1. **Assign lifetimes to all reference parameters** (each gets a unique lifetime)
2. **Apply elision rules** to determine output lifetimes
3. **Unify lifetimes** based on constraints:
   - If `&'a T` is assigned to `&'b T`, then `'a >= 'b` (the reference must live at least as long as the binding)
   - If `&'a T` is returned, then `'a` must outlive the function call
4. **Solve the constraint system** to find the minimal lifetime assignments
5. **Report errors** if constraints cannot be satisfied

### 14.5 Lifetime Bounds

Lifetime bounds specify relationships between lifetimes:

```
T: 'a           // T must outlive lifetime 'a
T: 'static      // T must live for the entire program
&'a T: 'b       // the reference &'a T must outlive 'b
```

**Common patterns:**

```
// Bounded by a specific lifetime
fn parse<'input, 'alloc>(input: &'input str, alloc: &'alloc Allocator) -> AST<'input>
where
    'alloc: 'input,    // allocator outlives the input
{ ... }

// Static lifetime (lives forever)
let s: &'static str = "hello";     // string literal is static
```

### 14.6 Higher-Ranked Lifetimes

For closures and trait objects that need to work with any lifetime:

```
// For any lifetime 'a, this function takes &'a str
fn for_all<'a>(f: impl for<'a> Fn(&'a str) -> &'a str) { ... }

// Equivalent to:
// f must work with any lifetime 'a
```

### 14.7 Lifetime Subtyping

Lifetime `'a` is a subtype of `'b` (written `'a: 'b`) if `'a` is at least as long as `'b`. This means anything valid for `'b` is also valid for `'a`.

```
'static: 'a        // 'static outlives any 'a
'a: 'b             // if 'a outlives 'b, then &'a T can be used where &'b T is expected
```

**Covariance:** If `'a: 'b`, then `&'a T` can be used where `&'b T` is expected (for shared references).

**Invariance:** `&'a mut T` is invariant in its lifetime — you cannot substitute a longer lifetime for a shorter one (or vice versa) for mutable references.

### 14.8 Common Lifetime Patterns

**Pattern 1: Output outlives inputs**

```
fn get_ref(data: &Data) -> &Item {
    &data.items[0]     // returned reference lives as long as data
}
```

**Pattern 2: Multiple inputs, shared output lifetime**

```
fn merge<'a>(a: &'a str, b: &'a str) -> String {
    format!("{a}{b}")  // returns owned String, no lifetime issue
}
```

**Pattern 3: Struct with references**

```
struct Excerpt<'a> {
    text: &'a str,      // excerpt borrows from the original string
}

let novel = String::from("Call me Ishmael.");
let excerpt = Excerpt { text: &novel[..15] };
```

**Pattern 4: Closures with lifetimes**

```
fn apply<F>(data: &str, f: F) -> &str
where
    F: Fn(&str) -> &str,
{
    f(data)
}
```

### 14.9 Common Lifetime Errors

**Error 1: Reference does not live long enough**

```
fn get_ref() -> &str {
    let s = String::from("hello");
    &s      // ERROR: s is dropped when function returns
}
```

**Fix:** Return owned data, or take a parameter.

**Error 2: Cannot infer lifetime**

```
fn pick(a: &str, b: &str) -> &str {
    if condition { a } else { b }
}
```

**Fix:** Add lifetime annotation: `fn pick<'a>(a: &'a str, b: &'a str) -> &'a str`

**Error 3: Mutable reference outlives borrow**

```
let mut data = vec![1, 2, 3];
let first = &data[0];      // shared borrow
data.push(4);              // ERROR: cannot modify while borrowed
println!("{first}");
```

**Fix:** End the borrow before modifying:

```
let mut data = vec![1, 2, 3];
let first = data[0];       // copy the value (i32 is Copy)
data.push(4);              // OK: no active borrows
println!("{first}");
```

---

## 15. Semantics

This section defines the runtime behavior of Nexus programs.

### 15.1 Evaluation Order

Nexus defines a strict evaluation order for expressions:

**Function arguments:** Evaluated left to right.

```
foo(a(), b(), c())    // a() is called first, then b(), then c()
```

**Binary operators:** Left operand is evaluated before right operand.

```
a + b    // a is evaluated first, then b
```

**Short-circuit operators:** Right operand is not evaluated if the left determines the result.

```
x && y    // if x is false, y is not evaluated
x || y    // if x is true, y is not evaluated
```

**Match expressions:** Scrutinee is evaluated first, then patterns are tried top to bottom.

```
match compute() {       // compute() is called first
    Pattern1 => ...,    // Pattern1 is tried first
    Pattern2 => ...,    // Pattern2 is tried if Pattern1 fails
}
```

**If expressions:** Condition is evaluated first.

```
if condition { ... }    // condition is evaluated first
```

**Let bindings:** Right-hand side is evaluated first.

```
let x = compute();     // compute() is called, then x is bound
```

**Block expressions:** Statements are evaluated in order; the final expression is the value.

```
{
    a();                // evaluated first
    b();                // evaluated second
    c()                 // evaluated third; its value is the block's value
}
```

### 15.2 Drop Order

Values are dropped in reverse declaration order (stack-like):

```
{
    let a = Resource("first");
    let b = Resource("second");
    let c = Resource("third");
}   // drop order: c, b, a
```

**Struct fields** are dropped in declaration order:

```
struct Foo {
    a: Resource,
    b: Resource,
}
// drop order: a, then b
```

**Enum variants** are dropped when the enum value is dropped.

### 15.3 Error Handling Semantics

**`Result<T, E>` propagation:**

```
fn read_file(path: &str) -> Result<String> {
    let content = fs::read_to_string(path)?;  // if Err, returns Err immediately
    Ok(content)
}
```

The `?` operator desugars to:

```
match result {
    Ok(value) => value,
    Err(e) => return Err(From::from(e)),
}
```

**`?T` (nullable) propagation:**

```
fn get_name(id: u64) -> ?String {
    let user = find_user(id)?;    // if null, returns null
    Some(user.name)
}
```

The `?` operator on nullable types desugars to:

```
match option {
    Some(value) => value,
    null => return null,
}
```

### 15.4 Concurrency Semantics

**`parallel` blocks:**

```
parallel {
    let a = fetch(url_a);    // spawned on thread 1
    let b = fetch(url_b);    // spawned on thread 2
}   // join: waits for both to complete
// a and b are both available
```

- Each statement in a `parallel` block is executed concurrently
- The block acts as a join point — execution continues only after all statements complete
- If any statement panics or returns an error, all other statements are cancelled
- Variables bound inside `parallel` are accessible after the block

**`async`/`await`:**

```
async fn fetch_all(urls: Vec<&str>) -> Vec<Data> {
    let futures: Vec<_> = urls.iter().map(|url| fetch(url)).collect();
    let results = join_all(futures).await;
    results
}
```

- `async` functions return `Future` objects
- `.await` pauses execution until the future resolves
- The runtime polls futures and drives them to completion
- Multiple futures can be concurrent via `join!` or `join_all!`

### 15.5 Panic Semantics

When a panic occurs:

1. The current function is unwound (local variables are dropped in reverse order)
2. The panic propagates to the calling function
3. This continues until `main` is reached
4. The process exits with a non-zero status code

```
fn might_panic() {
    panic!("something went wrong");    // unwinds the stack
}

fn main() {
    might_panic();     // panic propagates to main
    // program exits
}
```

**Catch panics:**

```
match std::panic::catch_unwind(|| {
    might_panic();
}) {
    Ok(()) => println!("no panic"),
    Err(e) => println!("caught panic: {e:?}"),
}
```

### 15.6 Arithmetic Semantics

**Integer overflow:**
- Debug mode: panic on overflow
- Release mode: wrapping arithmetic (two's complement)
- Explicit checked/wrapping/saturating operations available

```
let x: u8 = 255;
let y = x + 1;         // debug: panic; release: wraps to 0
let z = x.wrapping_add(1);  // always wraps
let w = x.checked_add(1);   // returns ?u8: None on overflow
let v = x.saturating_add(1); // returns 255 (saturates at max)
```

**Division by zero:**
- Integer division by zero: runtime panic
- Float division by zero: produces infinity or NaN (IEEE 754)

---

## 16. Runtime Specification

This section defines the memory layout, calling conventions, and runtime behavior of Nexus programs.

### 16.1 Memory Layout

**Primitive types:**

| Type | Size (bytes) | Alignment |
|------|-------------|-----------|
| `bool` | 1 | 1 |
| `char` | 4 | 4 |
| `u8`–`u128` | 1–16 | same as size |
| `i8`–`i128` | 1–16 | same as size |
| `f32` | 4 | 4 |
| `f64` | 8 | 8 |
| `()` | 0 | 1 |
| `*const T` | pointer size | pointer size |
| `*mut T` | pointer size | pointer size |
| `&T` | pointer size | pointer size |
| `&mut T` | pointer size | pointer size |

**Compound types:**

| Type | Size | Alignment |
|------|------|-----------|
| `(A, B)` | sum of field sizes + padding | max field alignment |
| `[T; N]` | N × size_of(T) | alignment of T |
| `struct` | sum of field sizes + padding | max field alignment |
| `enum` | tag size + max variant size + padding | max field alignment |

**Struct layout:**

```
struct Point {     // size: 16 bytes, alignment: 8
    x: f64,        // offset: 0, size: 8
    y: f64,        // offset: 8, size: 8
}

struct Padded {    // size: 16 bytes, alignment: 8
    a: u8,         // offset: 0, size: 1
    b: u64,        // offset: 8, size: 8 (aligned to 8)
    c: u8,         // offset: 16, size: 1
    // padding: 7 bytes to reach alignment
}
```

**Enum layout:**

```
enum Shape {           // size: 24 bytes (tag + largest variant)
    Circle(f64),       // variant 0: 8 bytes
    Rectangle(f64, f64), // variant 1: 16 bytes
}
// tag: 4 bytes (i32)
// padding: 4 bytes (alignment)
// data: 16 bytes (largest variant)
```

### 16.2 Stack and Heap

**Stack:**
- Local variables are allocated on the stack
- Stack allocation is just a pointer bump (very fast)
- Stack-allocated values are automatically dropped when they go out of scope
- Stack size is typically 1–8 MB per thread

**Heap:**
- `String`, `Vector`, and other owned types allocate on the heap
- Heap allocation uses the system allocator (malloc/free) or a custom allocator
- Heap-allocated values are dropped when their owner goes out of scope
- Arena allocation allocates from a bulk pool and frees all at once

**Ownership determines drop location:**
- Stack values: dropped when the variable goes out of scope
- Heap values: the heap memory is freed when the owner is dropped
- Arena values: freed when the arena scope ends

### 16.3 Calling Conventions

Nexus uses the platform's default calling convention:

**System V AMD64 ABI (Linux, macOS):**
- First 6 integer/pointer arguments: `rdi`, `rsi`, `rdx`, `rcx`, `r8`, `r9`
- First 8 float arguments: `xmm0`–`xmm7`
- Return value: `rax` (integer) or `xmm0` (float)
- Stack alignment: 16 bytes before `call`

**Windows x64 ABI:**
- First 4 integer/pointer arguments: `rcx`, `rdx`, `r8`, `r9`
- First 4 float arguments: `xmm0`–`xmm3`
- Return value: `rax` (integer) or `xmm0` (float)
- Shadow space: 32 bytes reserved by caller

**Function call example:**

```
fn add(a: i32, b: i32) -> i32 {
    a + b
}

// Compiled (System V):
// a in edi, b in esi
// result returned in eax
// add(3, 5) → mov edi, 3; mov esi, 5; call add; result in eax
```

### 16.4 Error Propagation

**Result<T, E> representation:**

```
// Result<T, E> is represented as a tagged union:
struct Result<T, E> {
    tag: u8,        // 0 = Ok, 1 = Err
    data: union {
        ok: T,
        err: E,
    },
}
```

**Setjmp/longjmp for error propagation (C backend):**

When transpiling to C, the `?` operator uses `setjmp`/`longjmp` for non-local returns:

```
// Nexus:
fn read_file(path: &str) -> Result<String> {
    let content = fs::read_to_string(path)?;
    Ok(content)
}

// C transpilation:
Result read_file(const char* path) {
    jmp_buf jump_buffer;
    if (setjmp(jump_buffer) != 0) {
        return Err(/* captured error */);
    }
    String content = fs_read_to_string(path, &jump_buffer);
    return Ok(content);
}
```

### 16.5 Startup Sequence

1. **Runtime initialization:** Initialize the allocator, thread pool, and async runtime (if used)
2. **Static initialization:** Initialize static variables and constants
3. **Call `main`:** Execute the `main` function
4. **Exit:** Return the exit code from `main` (0 on success, non-zero on error)

### 16.6 Minimum Runtime Library

The Nexus runtime library is minimal (~1-2K lines of C):

```
runtime/
├── alloc.c         // memory allocation (malloc wrapper or custom allocator)
├── panic.c         // panic handling (print message, unwind stack)
├── io.c            // basic I/O (stdout, stderr)
├── string.c        // string operations (concat, format)
├── result.c        // Result type support (setjmp/longjmp for C backend)
└── runtime.h       // runtime API
```

**No garbage collector.** No reference counting. No hidden runtime calls. All memory management is explicit through ownership.

### 16.7 FFI Calling Convention

When calling C functions from Nexus:

```
extern "C" {
    fn printf(format: *const u8, ...) -> i32;
}

// Nexus passes arguments according to the C ABI
// The compiler generates appropriate marshaling code
```

**Rules:**
- Nexus types are mapped to C types:
  - `i32` → `int32_t`
  - `u64` → `uint64_t`
  - `bool` → `uint8_t` (0 or 1)
  - `&T` → `T*` (const pointer)
  - `&mut T` → `T*` (mutable pointer)
  - `String` → `{ char* ptr; size_t len; }`
  - `Vector<T>` → `{ T* ptr; size_t len; size_t cap; }`
- C functions are implicitly `unsafe`
- No closures or owned types across FFI boundary

---

*End of Language Specification*
