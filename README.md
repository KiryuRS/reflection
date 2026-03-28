
# Compile-Time Reflection

A lightweight, header-only C++23 library for compile-time reflection of user-defined types.
Reflect public member variables, iterate over them, and build powerful integrations — all with **zero runtime overhead**.

---

## Overview

### Motivation

C++ has long lacked native reflection. Runtime approaches (type-erasure, virtual dispatch, external code generators) all impose a cost — either in performance, complexity, or build system friction.

This library takes a different approach: use the compiler itself to generate all reflection metadata at compile time. The result is a reflection system that:

- Adds **no overhead** to object size or construction
- Preserves **aggregate type properties** (`std::is_aggregate`, trivial construction, etc.)
- Requires only a single macro inside the class definition
- Composes naturally with C++20 concepts, `std::format`, and template metaprogramming

The design is inspired by [C++26 `std::meta`](https://wg21.link/P2996) and [`BOOST_DESCRIBE_STRUCT`](https://www.boost.org/doc/libs/release/libs/describe/doc/html/describe.html), with a focus on being self-contained and zero-dependency.

### Limitations

- **Public member variables only.** Member functions and static members are not reflected.
- **Manual opt-in.** Each type must be annotated with a macro. There is no automatic or non-intrusive reflection.
- **Maximum 128 members per class.** The preprocessor loop is unrolled up to 128 entries.
- **Enums use a separate macro** (`ENUM_PRINTABLE`) and a different underlying mechanism from structs.
- **Compile times increase** proportionally with the number of reflected types, as the compiler must instantiate templates for all descriptors.

---

## Examples

> Try it live on Compiler Explorer (GCC 15.2, `-std=c++23 -O3`): [godbolt link](https://godbolt.org/z/P9PbWs45h)

### `REFLECT` — bare-metal reflection

Use `REFLECT` when you only need to iterate or access member variables programmatically, without printing support.

```cpp
#include "reflect/reflect.hpp"

struct position_info
{
    double bod_position;
    double position;
    double buy_quantity;
    double sell_quantity;

    REFLECT(position_info, (), (bod_position, position, buy_quantity, sell_quantity));
};
```

The second argument `()` means no base classes. The third argument is the member list.

Once reflected, three APIs become available:

```cpp
constexpr position_info pos{.bod_position = 1.0, .position = 3.0,
                             .buy_quantity = 4.0, .sell_quantity = 2.0};

// 1. Template-only — Descriptor available as a compile-time type parameter
reflect::for_each<position_info>([&pos] <typename Descriptor>() {
    std::cout << Descriptor::name << ": "
              << reflect::get_member_variable<Descriptor>(pos) << "\n";
});

// 2. Template + instance — when you also need a descriptor object at runtime
reflect::for_each<position_info>([&pos] <typename Descriptor>(Descriptor desc) {
    std::cout << desc.name << ": "
              << reflect::get_member_variable(pos, desc) << "\n";
});

// 3. Runtime only — when only the instance is needed
reflect::for_each<position_info>([&pos](auto desc) {
    std::cout << desc.name << ": "
              << reflect::get_member_variable(pos, desc) << "\n";
});
```

> **Note:** `REFLECT` does **not** change the object's layout, size, or aggregate properties.
> `static_assert(sizeof(position_info_with_reflect) == sizeof(position_info_without_reflect))` holds.

---

### `REFLECT_PRINTABLE` — reflection with printing

`REFLECT_PRINTABLE` is a superset of `REFLECT`. In addition to all reflection APIs, it generates:

- `to_string(obj)` — returns a formatted `std::string`
- `operator<<` overload for `std::ostream`
- `std::formatter<T>` specialisation for use with `std::format`
- `print_meta(obj)` — prints each member's type name, variable name, and current value

```cpp
struct position_info
{
    double bod_position;
    double position;
    double buy_quantity;
    double sell_quantity;

    REFLECT_PRINTABLE(position_info, (), (bod_position, position, buy_quantity, sell_quantity));
};

int main()
{
    constexpr position_info pos{.bod_position = 1.0, .position = 3.0,
                                .buy_quantity = 4.0, .sell_quantity = 2.0};

    std::cout << pos << "\n";
    // {position_info: {'bod_position': 1.000, 'position': 3.000, 'buy_quantity': 4.000, 'sell_quantity': 2.000} }

    std::cout << std::format("{}\n", pos);
    // same output via std::format

    std::cout << print_meta(pos) << "\n";
    // struct position_info reflected variables:
    //   double bod_position = 1.000
    //   double position = 3.000
    //   ...
}
```

---

### Reflecting Inherited Classes

Both `REFLECT` and `REFLECT_PRINTABLE` accept a **base-class tuple** as their second argument. When provided, the derived class's reflection automatically includes all base class members, in order from outermost base to the derived class's own members.

> Base classes must be reflected **before** the derived class.

```cpp
struct base
{
    std::string str;
    double d;

    REFLECT_PRINTABLE(base, (), (str, d));
};

struct base_2
{
    char c;

    REFLECT_PRINTABLE(base_2, (), (c));
};

struct derived_more : base, base_2
{
    int x;

    REFLECT_PRINTABLE(derived_more, (base, base_2), (x));
};
```

`reflect::for_each<derived_more>` iterates: `str`, `d` (from `base`), `c` (from `base_2`), then `x`.

```cpp
int main()
{
    derived_more dm{};
    dm.str = "Hello World";
    dm.d   = 3.14;
    dm.c   = 'A';
    dm.x   = 100;

    std::cout << dm << "\n";
    // {derived_more: {{base: {'str': Hello World, 'd': 3.140}}, {base_2: {'c': A}}, 'x': 100} }

    std::cout << print_meta(dm) << "\n";
    // struct derived_more reflected variables:
    //   std::string str = Hello World
    //   double d = 3.140
    //   char c = A
    //   int x = 100
}
```

When no base classes are involved, pass an empty tuple `()` as the second argument.

---

## Built-in Extensions

### YAML Integration (`include/yaml/`)

The YAML integration bridges compile-time reflection with [yaml-cpp](https://github.com/jbeder/yaml-cpp). By including `yaml/convert.hpp`, any reflected type automatically gains `YAML::convert<T>` encode and decode support — no manual mapping code needed.

```cpp
#include "yaml/convert.hpp"

struct server_config
{
    std::string host;
    int port;
    std::optional<int> timeout_ms;
    std::vector<std::string> allowed_origins;

    REFLECT(server_config, (), (host, port, timeout_ms, allowed_origins));
};

// Decode from YAML
YAML::Node node = YAML::Load(yaml_string);
server_config cfg = node.as<server_config>();

// Encode to YAML
YAML::Node out = YAML::convert<server_config>::encode(cfg);
```

YAML keys are derived directly from C++ member variable names.

#### Supported member types

| Type | Encode | Decode |
|---|---|---|
| Scalars (`int`, `double`, `bool`, `std::string`, etc.) | Always | Always (throws if key missing) |
| `std::optional<T>` | Skipped if `nullopt` | Skipped if key absent |
| Containers (`std::vector<T>`, etc.) | Skipped if empty | Skipped if key absent |
| Nested reflected types | Recursively encoded | Recursively decoded |

#### Limitations

- YAML key names must match C++ member names exactly. Renaming a field is a breaking schema change.
- No key validation on decode — missing required (non-optional) keys cause yaml-cpp to throw at runtime.
- `std::string` and `std::string_view` are treated as scalars, not containers, even though they satisfy range concepts.

---

### Argument Parsing (`include/argparse/`)

The argument parser is inspired by Python's `argparse` module — but driven entirely by compile-time reflection rather than runtime registration. Declare a config struct, reflect it, and pass `argc`/`argv` directly.

```cpp
#include "argparse/argparse.hpp"

struct program_args
{
    std::string config_path;
    int worker_count;
    bool verbose;
    std::optional<int> timeout_ms;     // not required on the command line
    std::vector<std::string> plugins;  // not required; defaults to empty

    REFLECT(program_args, (), (config_path, worker_count, verbose, timeout_ms, plugins));
};

int main(int argc, const char* argv[])
{
    // ./app --config_path ./cfg.yaml --worker_count 4 --verbose true --plugins a,b,c
    auto args = argparse::parse_args<program_args>(argc, argv);
}
```

#### Argument syntax rules

| Rule | Detail |
|---|---|
| Prefix | `--name value` only. No short form (`-n`). |
| Separator | Whitespace between key and value. |
| Lists | Comma-separated: `--plugins a,b,c` → `std::vector{"a", "b", "c"}` |
| Booleans | `"true"` or `"false"` (case-insensitive) |
| Enums | String name of the enumerator (requires `ENUM_PRINTABLE` on the enum type) |

#### Required vs optional arguments

| Member type | Required on CLI? |
|---|---|
| Any scalar type | Yes — throws if not provided |
| `std::optional<T>` | No — skipped if absent |
| `std::vector<T>` | No — defaults to empty |

#### Limitations

- No short-form flags (`-v` as alias for `--verbose`).
- No boolean presence flags — booleans require an explicit value (`--verbose true`).
- No sub-commands or nested argument groups.
- No `--help` generation.

---

## Building

Dependencies are managed via [Conan](https://conan.io/):

```bash
./conan_build.sh   # installs deps, builds with CMake, runs test suite
```

Or using Docker:

```bash
docker build -t reflection-library .
docker run --rm reflection-library
```

**Requirements:** C++23 compiler (GCC ≥ 13, Clang ≥ 16, MSVC ≥ 19.34), CMake ≥ 4.1.0.
