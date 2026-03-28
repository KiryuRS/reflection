# Reflection Library — Agent Context

## What this repo is

A **C++23 header-only compile-time reflection library**. Annotate structs with a macro and iterate their member variables, access them by descriptor, serialize them, and parse CLI arguments — all with zero runtime overhead.

Inspired by C++26 `std::meta` and `BOOST_DESCRIBE_STRUCT`.

## Build & Test

```bash
./conan_build.sh          # install deps via Conan, build with CMake, run GTest suite
docker build -t reflection-library . && docker run --rm reflection-library
```

Dependencies: `gtest/1.17.0`, `yaml-cpp/0.8.0`. Requires CMake >= 4.1.0.

## Repo layout

```
include/
  reflect/      # core reflection engine
  yaml/         # yaml-cpp integration
  argparse/     # CLI argument parser
tests/
  test_reflection.cpp
  test_yaml_conversion.cpp
  test_argparse.cpp
.claude/
  CLAUDE.md            # this file
  INSTRUCTIONS.md      # design principles and conventions to follow
  modules/
    reflect.md         # deep dive: techniques, every file explained
    yaml.md            # yaml-cpp integration details
    argparse.md        # CLI parser details
```

## Quick reference

```cpp
struct Foo {
    int x;
    double y;
    REFLECT_PRINTABLE(Foo, (), (x, y));
    // arg2 () = no base classes; arg3 = member list
};

reflect::for_each<Foo>([&foo] <typename D>() {
    std::cout << D::name << ": " << reflect::get_member_variable<D>(foo) << "\n";
});

std::cout << foo;        // operator<< via REFLECT_PRINTABLE
std::format("{}", foo);  // std::formatter specialization
```

Inheritance — pass base classes in the second argument:
```cpp
struct Derived : Base {
    int z;
    REFLECT_PRINTABLE(Derived, (Base), (z));
    // reflect::for_each<Derived> walks Base members then z
};
```

Enums — separate macro, lives outside the class, requires a `NONE` sentinel:
```cpp
enum class Color { NONE, Red, Green };
ENUM_PRINTABLE(Color, (NONE, Red, Green));
// enum_to_string(v), string_to_enum(Color{}, str), operator<<, std::formatter
```

## Further reading

- `.claude/INSTRUCTIONS.md` — conventions, design constraints, things to avoid
- `.claude/modules/reflect.md` — all four compile-time techniques in depth
- `.claude/modules/yaml.md` — yaml-cpp integration
- `.claude/modules/argparse.md` — CLI parser
