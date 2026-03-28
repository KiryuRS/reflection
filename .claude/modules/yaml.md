# Module: include/yaml — yaml-cpp Integration

## Files

| File | Purpose |
|---|---|
| `convert.hpp` | Specialises `YAML::convert<T>` for any `reflectable` type |
| `concepts.hpp` | `container_like`, `same_as_optional` concepts used by the converter |
| `parser.hpp` | Higher-level YAML parsing helpers |

## How it works (`convert.hpp`)

A single `YAML::convert<T>` partial specialisation constrained to `reflect::concepts::reflectable`:

```cpp
template <::reflect::concepts::reflectable T>
struct YAML::convert<T> {
    static Node encode(const T& obj);
    static bool decode(const Node& node, T& obj);
};
```

Both `encode` and `decode` call `reflect::for_each<T>` to iterate descriptors. Per-member dispatch:

| Member type | encode behaviour | decode behaviour |
|---|---|---|
| `container_like` | skip if empty | skip if YAML key absent |
| `same_as_optional` | skip if `nullopt` | decode inner type if key present |
| everything else | always encode | always decode (throws if key missing) |

YAML key = `Descriptor::name` (the C++ member variable name as a `string_view`).

## Usage

```cpp
#include "yaml/convert.hpp"

struct Config {
    std::string host;
    int port;
    std::optional<int> timeout;
    REFLECT(Config, (), (host, port, timeout));
};

Config cfg = YAML::Node{node}.as<Config>();             // decode
YAML::Node out = YAML::convert<Config>::encode(cfg);    // encode
```

## Constraints / gotchas

- YAML keys must match C++ member names exactly — renaming a field is a breaking schema change.
- No key validation on decode (TODO in source). A missing non-optional key causes yaml-cpp to throw at `node[key].as<T>()`.
- Nested reflected types work automatically — yaml-cpp recurses into `YAML::convert<MemberType>` if that type is also reflected.
