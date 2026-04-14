# Module: include/reflect — Core Reflection Engine

## Files

| File | Purpose |
|---|---|
| `reflect.hpp` | Public API + macros (`REFLECT`, `REFLECT_PRINTABLE`, `GENERATE_DESCRIPTOR`) |
| `preprocessor.hpp` | Macro utilities: `PP_FOR_EACH`, `PP_GET_NTH_ARG`, token-paste helpers |
| `concepts.hpp` | C++20 concepts constraining descriptors, reflectable types, functors |
| `utility.hpp` | `get_name<T>()`, `get_short_name<T>()`, `hash_dj2ba()`, `concat_arrays()` |
| `typelist.hpp` | `typelist<Ts...>` and associated utilities (`same_as_typelist`, `typelist_size_v`, `typelist_element_t`) |
| `enum.hpp` | `ENUM_PRINTABLE` macro for enum ↔ string conversion |

---

## Technique 1 — Preprocessor `PP_FOR_EACH` (`preprocessor.hpp`)

`PP_FOR_EACH(fn, arg0, x, y, z)` counts its variadic args using `PP_GET_NTH_ARG` — which shifts all args left so the count lands at position 128 — then dispatches to `PP_FUNC_CALL_N`. Each `PP_FUNC_CALL_N` calls `fn(arg0, head)` then recursively expands `PP_FUNC_CALL_{N-1}` on the tail. Supports up to 128 members.

`PP_FOR_EACH_IN_TUPLE` wraps this to accept a parenthesised tuple `(x, y)` and skips entirely on `()` via `__VA_OPT__`. This is how the empty-bases case is handled without generating any code.

---

## Technique 2 — `introspection<T>` + Descriptor structs (`reflect.hpp`)

`detail::introspection<T>` is a primary template with two specializations that decode a pointer-to-member type:

```cpp
// Member variable: Member Class::*
template <typename Class, typename Member>
struct introspection<Member Class::*> {
    using member_type         = Member;
    using member_pointer_type = Member (Class::*);
    static constexpr std::string_view mem_type_str = get_name<member_type>();
};

// Member function: ReturnType (Class::*)(Args...)
template <typename Class, typename ReturnType, typename... Args>
struct introspection<ReturnType (Class::*)(Args...)> {
    using member_type         = ReturnType(Args...);
    using member_pointer_type = ReturnType (Class::*)(Args...);
    using return_type         = ReturnType;
    using arguments_type      = typelist<Args...>;   // from typelist.hpp
    static constexpr std::string_view mem_type_str = "class member function";
};
```

`GENERATE_DESCRIPTOR(Class, Member)` emits one struct per member, routing all type information through `introspection`:

```cpp
struct descriptor_Class_Member {
    using introspection_type  = reflect::detail::introspection<decltype(&Class::Member)>;
    using class_type          = Class;
    using member_type         = typename introspection_type::member_type;
    using member_pointer_type = typename introspection_type::member_pointer_type;

    static constexpr std::string_view name         = "Member";
    static constexpr std::string_view mem_type_str = introspection_type::mem_type_str;
    static constexpr member_pointer_type mem_ptr   = &Class::Member;
};
```

Everything is `static constexpr` — zero runtime storage. `mem_ptr` is a pointer-to-member constant used by `get_member_variable` as `obj.*Descriptor::mem_ptr`.

Member functions are distinguished at compile time via `std::is_function_v<typename Descriptor::member_type>` (function descriptors have `member_type = ReturnType(Args...)`, a function type).

`get_member_variable` handles both cases via `if constexpr`:
- **Member variable** — returns `obj.*mem_ptr` (the value)
- **Member function** — returns `std::bind_front(mem_ptr, std::forward<T>(obj))`, a `constexpr`-compatible callable bound to the object

**Restriction:** member functions can only be reflected with `REFLECT`, not `REFLECT_PRINTABLE`. `to_string` uses `OSTREAM_PRINT` which directly streams `object.member` by name, and `print_meta` streams the result of `get_member_variable` — both are ill-formed for function types.

---

## Technique 3 — `meta_id` / injected friend (`reflect.hpp:15–50`)

**Problem:** associate a heterogeneous list of descriptor types with a class `T` at compile time, without a runtime map.

**Solution:** stateful compile-time association via ADL-injected `consteval` friend.

```cpp
template <auto>
struct meta_id {
    friend consteval auto get_meta_info(meta_id);  // declared but not defined here
};

template <typename T, auto V>
struct meta_type {
    using value_type = T;
    static constexpr auto value = V;   // V = the descriptor array

    static void id() {}  // unique void(*)() per <T,V> instantiation

    friend consteval auto get_meta_info(meta_id<id>) {  // definition injected into ADL
        return meta_type{};
    }
};

template <typename T, auto V>
static constexpr auto meta_type_info = meta_type<T, V>::id;  // forces instantiation
```

- `meta_type<DescArray>::id` is a unique `void(*)()` function pointer that acts as a compile-time key.
- Instantiating `meta_type<T, V>` injects a `get_meta_info` overload for `meta_id<id>` via ADL.
- The `REFLECT` macro evaluates `meta_type_info<Class, metas>` inside `meta_info_array_as_id()`, triggering the instantiation (and thus the injection).
- Later, `generate_meta_info<T>()` calls `get_meta_info(meta_id<T::meta_info_array_as_id()>{})` and returns `::value` — the descriptor array — with no runtime cost.

`meta_type_underlying_type<MetaTypeInfo>` is the alias that, given a `void(*)()` key, resolves back to the descriptor struct type via `decltype(get_meta_info(...))::value_type`.

---

## Technique 4 — `index_sequence` + fold expression (`reflect.hpp:78–93`)

Pre-C++26 range-for cannot iterate a `constexpr` array with heterogeneous element types. `for_each` uses the index-sequence unroll:

```cpp
static constexpr auto META_ARRAY_INFO = generate_meta_info<T>();

const auto on_each_visit = [&func] <size_t I>() {
    using descriptor_t = meta_type_underlying_type<META_ARRAY_INFO[I]>;
    // dispatch to one of three functor calling conventions via if constexpr
};

const auto visitor = [&on_each_visit] <size_t... Is>(std::index_sequence<Is...>) {
    (on_each_visit.template operator()<Is>(), ...);  // fold — fully unrolled at compile time
};

visitor(std::make_index_sequence<META_ARRAY_INFO.size()>{});
```

Each `I` resolves `META_ARRAY_INFO[I]` to a different `void(*)()` key, which in turn resolves to a different concrete descriptor type. The fold `(f<0>(), f<1>(), ...)` is fully unrolled — no loop, no branching at runtime.

---

## `utility.hpp` internals

- **`get_name<T>()`** — extracts a type name as `string_view` from `std::source_location::current().function_name()`. Finds the `"void"` anchor in the `<void>` specialisation to calculate the prefix length, then applies it to `<T>`. Compiler branches: MSVC uses `rfind(">(")`, GCC/Clang use `find(']')`.
- **`get_short_name<T>()`** — strips leading namespace qualifiers, tracking angle-bracket nesting depth to avoid splitting `foo<a::b>::c` incorrectly.
- **`hash_dj2ba(str)`** — djb2a hash over a `string_view`, used by `ENUM_PRINTABLE` to dispatch string-to-enum via a `switch` on hash values.
- **`concat_arrays(...)`** — `consteval` variadic array merge. Used inside `meta_info_array_as_id()` to combine base-class and own descriptor arrays into a single flat array before storing in `meta_type`.

---

## `enum.hpp` internals

`ENUM_PRINTABLE(Enum, (V1, V2, ...))` generates (outside the enum definition):

- `enum_to_string(Enum)` — `switch` on value, returns `string_view` of the enumerator name.
- `string_to_enum(Enum, str)` — `switch` on `hash_dj2ba(str)`, returns the matching enumerator; falls back to `Enum::NONE`.
- `operator<<` — writes `enum_to_string(v)` to the stream.
- `std::formatter<T>` specialisation — constrained by the `enumerable` concept (which requires `T::NONE` to exist).

Enums do **not** use the descriptor/meta_id mechanism — the simpler hash-switch approach is sufficient and avoids forcing enum authors to write descriptors.

---

## `typelist.hpp`

A minimal compile-time type list:

```cpp
template <typename... Ts>
struct typelist {};
```

Utilities:

| Utility | What it does |
|---|---|
| `same_as_typelist<T>` concept | True if `T` is a `typelist<...>` instantiation |
| `typelist_size_v<Typelist>` | `sizeof...(Ts)` as a `constexpr std::size_t` |
| `typelist_element_t<I, Typelist>` | The `I`-th type in the list (recursive specialization) |

Used by `introspection` to expose function argument types as `arguments_type = typelist<Args...>`.

---

## Concepts (`concepts.hpp`)

| Concept | What it checks |
|---|---|
| `descriptor_like<T>` | Has `introspection_type`, `class_type`, `member_type`, `member_pointer_type`; `name`, `mem_type_str` are `const string_view`; `mem_ptr` is the correct pointer-to-member type |
| `reflectable<T>` | `T::meta_info_array_as_id()` returns `void(*)()` |
| `reflect_and_printable<T>` | `reflectable` + `to_string(T)` + `print_meta(T)` are well-formed |
| `enumerable<T>` | `std::is_enum_v<T>` and `T::NONE` exists |
| `template_only_invocable<F,T>` | `f.template operator()<T>()` is valid |
| `template_invocable<F,T>` | `f.template operator()<T>(T{})` is valid |
| `any_invocable<F,T>` | Any of the three calling conventions is valid |
