# Module: include/argparse — CLI Argument Parser

## Files

| File | Purpose |
|---|---|
| `argparse.hpp` | `argparse::parse_args<T>(argc, argv)` — main entry point |
| `convertors.hpp` | `convertors::parse_value<T>(string_view)` — string-to-type conversion |
| `concepts.hpp` | `same_as_optional`, `same_as_vector` concepts |

## How it works (`argparse.hpp`)

`parse_args<T>` requires `T` to satisfy `reflect::concepts::reflectable`. It:

1. Throws immediately if `argc <= 1`.
2. Copies `argv` into a `vector<string_view>`.
3. Allocates a fixed `std::array<arg_supplied_info, N>` (size = number of reflected members — computed at compile time via `generate_meta_info<T>().size()`).
4. Calls `reflect::for_each<T>` — for each member:
   - Searches `argv` for `--<member_name>` (exact, case-sensitive).
   - Takes the next token as value if it exists and doesn't start with `--`.
   - Calls `convertors::parse_value<MemberType>(value)` — returns `std::optional`; throws on parse failure.
   - Marks the member as supplied in `args_supplied`.
5. Calls `validate_args` — throws `std::invalid_argument` listing all unsupplied required members.

**What counts as "has a default" (i.e., not required on CLI):**
- `std::optional<T>` — required only if it has no pre-set value.
- `std::vector<T>` — always optional; empty vector is valid.
- Everything else — required.

## Parsing rules

1. Only `--name value` syntax. No short form (`-n`).
2. Key and value separated by whitespace.
3. Value must not start with `--` (otherwise treated as missing).
4. Vectors: comma-separated, e.g. `--items a,b,c`.
5. Booleans: `"true"` / `"false"` (case handling is in the convertor).
6. Enum members: uses `string_to_enum` (requires `ENUM_PRINTABLE` applied to the enum).

## Usage

```cpp
#include "argparse/argparse.hpp"

struct Args {
    std::string host;
    int port;
    std::optional<int> timeout;    // not required
    std::vector<std::string> tags; // not required, defaults to empty
    REFLECT(Args, (), (host, port, timeout, tags));
};

int main(int argc, const char* argv[]) {
    // ./app --host localhost --port 8080 --tags a,b
    auto args = argparse::parse_args<Args>(argc, argv);
}
```

## Known TODOs / limitations

- No short-form flag support (`-x` as alias for `--x`).
- No boolean presence flags — booleans require an explicit value (`--verbose true`).
- No sub-commands or nested argument groups.
- No `--help` generation.
