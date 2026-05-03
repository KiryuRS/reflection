# reflect

> Compile-time struct reflection for C++23. Zero overhead. One macro.

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)]()
[![Header only](https://img.shields.io/badge/header--only-yes-brightgreen.svg)]()

Annotate a struct with a single macro. Get full compile-time introspection — iterate members, access them by descriptor, serialise to YAML, parse CLI arguments — all resolved at compile time with **zero runtime cost**.

Try it live → [Compiler Explorer (GCC 15, -std=c++23 -O3)](https://godbolt.org/z/P9PbWs45h)

---

```cpp
struct position_info
{
    double bod_position;
    double position;
    double buy_quantity;
    double sell_quantity;

    REFLECT_PRINTABLE(position_info, (), (bod_position, position, buy_quantity, sell_quantity));
};

constexpr position_info pos{1.0, 3.0, 4.0, 2.0};

// iterate all members at compile time
reflect::for_each<position_info>([&pos] <typename D>() {
    std::cout << D::name << ": " << reflect::get_member_variable<D>(pos) << "\n";
});

std::cout << pos;               // operator<< out of the box
std::format("{}", pos);         // std::formatter out of the box
```

---

## Why reflect?

- **Zero overhead.** All metadata is `static constexpr`. No runtime tables, no virtual dispatch, no heap.
- **Non-intrusive.** The macro adds only static member functions — `sizeof(T)`, aggregate initialisation, and trivial construction are all preserved.
- **Works with anything.** Mixed member types, nested structs, enums, containers, `std::optional`, member functions — all reflect cleanly.
- **Composes naturally.** Works with C++20 concepts, `std::format`, and template metaprogramming out of the box.
- **Batteries included.** Built-in YAML serialisation and CLI argument parsing, both driven by the same reflection metadata.

---

## Installation

Header-only. Copy `include/` into your project and `#include "reflect/reflect.hpp"`.

No build step required. Dependencies (`yaml-cpp`) are only needed for the optional YAML integration.

---

## Core API

### `REFLECT` and `REFLECT_PRINTABLE`

```
REFLECT(ClassName, (BaseClasses...), (members...))
REFLECT_PRINTABLE(ClassName, (BaseClasses...), (members...))
```

`REFLECT_PRINTABLE` is a superset of `REFLECT` — use it when you want printing for free.

| Feature | `REFLECT` | `REFLECT_PRINTABLE` |
|---|:---:|:---:|
| `reflect::for_each<T>` | ✓ | ✓ |
| `reflect::get_member_variable<D>(obj)` | ✓ | ✓ |
| `reflect::descriptor_for<T, &T::member>` | ✓ | ✓ |
| `to_string(obj)` | | ✓ |
| `operator<<` | | ✓ |
| `std::formatter<T>` | | ✓ |
| `print_meta(obj)` | | ✓ |

### `reflect::for_each<T>`

Iterates all reflected members at compile time. Each visit receives the descriptor as a template type argument:

```cpp
reflect::for_each<position_info>([&pos] <typename D>() {
    std::cout << D::name << ": " << reflect::get_member_variable<D>(pos) << "\n";
});
```

### `reflect::descriptor_for<T, MemberPtr>`

Reverse-lookup a descriptor from a member pointer — fully resolved at compile time:

```cpp
using D = reflect::descriptor_for<position_info, &position_info::position>;

static_assert(D::name == "position");
static_assert(std::same_as<D::member_type, double>);
static_assert(D::mem_ptr == &position_info::position);
```

### Concepts

| Concept | Passes when |
|---|---|
| `reflect::concepts::reflectable<T>` | `T` has `REFLECT` or `REFLECT_PRINTABLE` |
| `reflect::concepts::reflect_and_printable<T>` | `T` has `REFLECT_PRINTABLE` |
| `reflect::concepts::descriptor_like<D>` | `D` is a valid descriptor type |

---

## Inheritance

Pass base classes in the second argument. `for_each` walks base members first, then the derived class's own members.

> Base classes must be reflected before the derived class.

```cpp
struct base
{
    std::string name;
    double score;
    REFLECT_PRINTABLE(base, (), (name, score));
};

struct tag
{
    bool active;
    REFLECT_PRINTABLE(tag, (), (active));
};

struct record : base, tag
{
    int id;
    REFLECT_PRINTABLE(record, (base, tag), (id));
};

// for_each visits: name, score (base), active (tag), id (record)
std::cout << std::format("{}\n", record{"Alice", 9.5, true, 42});
```

---

## Member Functions

Member functions can be reflected alongside member variables. Use `REFLECT` (not `REFLECT_PRINTABLE` — streaming a function is ill-formed).

```cpp
struct entity
{
    int id;
    void update(float dt, int flags);

    REFLECT(entity, (), (id, update));
};

reflect::for_each<entity>([&obj] <typename D>() {
    if constexpr (std::is_function_v<typename D::member_type>)
    {
        using ret  = typename D::introspection_type::return_type;
        using args = typename D::introspection_type::arguments_type; // typelist<float, int>

        auto fn = reflect::get_member_variable<D>(obj); // callable bound to obj by reference
        fn(0.016f, 0);
    }
});
```

---

## YAML Integration

Include `yaml/convert.hpp`. Any reflected type automatically gets `YAML::convert<T>` encode/decode — no manual mapping.

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

server_config cfg = YAML::Load(yaml_string).as<server_config>();
YAML::Node out    = YAML::convert<server_config>::encode(cfg);
```

`std::optional` fields are skipped on encode if empty, and skipped on decode if the key is absent. Containers behave the same way.

---

## CLI Argument Parsing

Include `argparse/argparse.hpp`. Reflect a config struct and hand `argc`/`argv` directly to `parse_args`.

```cpp
#include "argparse/argparse.hpp"

struct program_args
{
    std::string config_path;
    int worker_count;
    std::optional<int> timeout_ms;
    std::vector<std::string> plugins;

    REFLECT(program_args, (), (config_path, worker_count, timeout_ms, plugins));
};

// ./app --config_path ./cfg.yaml --worker_count 4 --plugins a,b,c
auto args = argparse::parse_args<program_args>(argc, argv);
```

Scalar fields are required. `std::optional` and `std::vector` fields are optional (default to empty).

---

## Building & Testing

```bash
./conan_build.sh   # install deps via Conan, build with CMake, run test suite
```

Or with Docker:

```bash
docker build -t reflect . && docker run --rm reflect
```

**Requirements:** C++23 (GCC ≥ 13, Clang ≥ 16, MSVC ≥ 19.34), CMake ≥ 4.1.0.

---

## Limitations

- **Manual opt-in** — each type must be annotated. No automatic or non-intrusive reflection.
- **128-member limit** — the preprocessor loop is unrolled to 128 entries per class.
- **Member functions require `REFLECT`** — `REFLECT_PRINTABLE` cannot stream function members.
- **No short CLI flags** — argparse accepts `--name value` only, no `-n` aliases.
- **Compile times** scale with the number of reflected types.

---

## License

MIT © KiryuRS

---

<sub>Documentation and unit tests written with the assistance of [Claude](https://claude.ai) (Anthropic).</sub>
