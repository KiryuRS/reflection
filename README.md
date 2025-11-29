# Reflection

Lightweight compile time reflection library that provides the bare minimum reflection code.  
Inspiration from C++26's `std::meta` feature, though with limited capabilities because we do not have `for template` syntax in C++ (pre C++26).  
  
## Implementation

### Compiler Version

* Compiled with GCC 15.1.0 with C++23 enabled
    * _not required for this version, as long as its C++20 compilant that supports `concepts`, `consteval` and lambda as template arguments_
* cmake version 4.1.2
* conan version 2.21.0
    * _not required for conan, but ease the pain of finding Boost / GTest_

### Under The Hood

* Uses macro (alongside with Boost' preprocessor helper macros) to build the reflection codes
* Main bulk of the logic is creating an unique descriptor for each member variable of a class
* Designed for compile time reflection, trade-off would be a potential increase in compilation time
    * Depending on how many types you need to reflect for _(did not stress test with the number of types to reflect, but should not be a problem)_

The meta information is embedded into each of the class. For example:

```c++
struct foo
{
    int i;
    float f;

    GENERATE_META_INFO(foo, (i, f));
};
```

will generate the following information:

* Descriptor for each reflected member variables specified in the macros. Each descriptor has an unique name. In the above example, there will be 2 descriptor classes created
* A static function that returns a meta id that points to a static array (hidden in the function).
    * This array consists of meta ids pointing to each descriptors

Note that, it is required to use the helper functions in `reflect` class to retrieve the underlying descriptor from the meta id.  
It is by design for end users to not be able to use the meta id and use the helper functions.  
  
Adding reflection to a class will not increase the class size nor change the type of the class. E.g. if its an `aggregated` class, it remains as `aggregated` class _(see `test_class_traits_should_remain_same`)_

## Examples

Minimal example with `GENERATE_META_INFO`:

```c++
struct position_info
{
    double bod_position;
    double position;
    double buy_quantity;
    double sell_quantity;

    GENERATE_META_INFO(position_info, (bod_position, position, buy_quantity, sell_quantity));
};

int main(void)
{
    constexpr position_info pos{.bod_position = 1.0, .position = 3.0, .buy_quantity = 4.0, .sell_quantity = 2.0};

    // Using only template parameters
    reflect::for_each<position_info>([&pos] <typename Descriptor> () {
        // access each member variable explicitly
        std::cout << std::format("position_info for member {} has value of '{}'", Descriptor::name, reflect::get_member_variable<Descriptor>(pos)) << '\n';
    });

    // OR using both template parameter and normal parameters
    reflect::for_each<position_info>([&pos] <typename Descriptor> (Descriptor desc) {
        // access each member variable explicitly
        std::cout << std::format("position_info for member {} has value of '{}'", Descriptor::name, reflect::get_member_variable(pos, desc)) << '\n';
    });

    // OR using only normal parameters
    reflect::for_each<position_info>([&pos] (auto desc) {
        // access each member variable explicitly
        std::cout << std::format("position_info for member {} has value of '{}'", desc.name, reflect::get_member_variable(pos, desc)) << '\n';
    });
}
```

With `REFLECT`:

```c++
struct position_info
{
    double bod_position;
    double position;
    double buy_quantity;
    double sell_quantity;

    REFLECT(position_info, (bod_position, position, buy_quantity, sell_quantity));
};

int main(void)
{
    constexpr position_info pos{.bod_position = 1.0, .position = 3.0, .buy_quantity = 4.0, .sell_quantity = 2.0};
    
    std::cout << to_string(pos) << '\n';
    // OR
    std::cout << pos << '\n';
    // OR
    std::cout << std::format("{}\n", pos);
}
```
