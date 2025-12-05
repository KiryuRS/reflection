
# Compile Time Reflection 

Lightweight compile time reflection library that provides the bare metal for reflecting each member variable of a user defined type.  
Inspiration mainly from C++26's `std::meta` feature and `BOOST_DESCRIBE_STRUCT` / `BOOST_DESCRIBE_CLASS`.  
The library mainly focuses on public _(attribute)_ only member variables reflection, traversing through each reflected element and logging capabilities.

# Overview

This version adhere of the following principles for building the system:

* _something_ to explain or describe the attributes of a member variable _(e.g. type, name of the type, name of variable, etc.)_, we call this a `Descriptor`
* _some_ container to store all of the descriptors
* _some ways_ to traverse through all of the descriptors from the container
* this container should _ideally_ be able to traverse the descriptors at compile time, using `template for` (C++ 26 feature)

## Principles

### Descriptor

A descriptor can be seen as a struct that captures all information required at zero-abstraction cost.  
This means everything must be using `static constexpr` variables and `using` type alias keywords. For example, `std::string str` for type `foo`:

```c++
struct some_descriptor
{
    using class_type = foo;
    using member_type = std::basic_string<char>;
    using member_pointer_type = std::basic_string<char> foo::*;

    static constexpr std::string_view name = "str";
    static constexpr std::string member_type_name = "std::basic_string<char>";
    static constexpr member_pointer_type member_pointer = &foo::str;
};
```

### Descriptors Container

Depending on implementation, the container to hold all of the descriptors may vary. If the descriptor is based off templates,  
this means each instantiation is a different type. Hence one possible container to hold them is `std::tuple`.  
  
But this version also implements a meta id mapping. Means each descriptor maps to an ID which has the same type throughout.  
Which will be possible to store them into an `std::array`

### Traversing the Descriptors Container

With the container implemented, we can traverse the container to obtain each descriptor and use it for the application logic.  
  
Unfortunately there is no way to traverse the container using the regular for-loop or ranged based for-loop,  
hence there will be a need to implement a visitor on the container, similar to `std::for_each`. For instance:

```c++
reflect::for_each<foo>([] <typename Descriptor>() {
    // do something with the Descriptor template parameter
});
```

## Benefits of Reflection

Adding reflection to any user defined type will not:

* Change the behavior of the type _(e.g. aggregated type will remain as aggregated type)_
* Size of the type will not change
* All reflection logic are contained within the type itself _(with the exception of enums)_
* Similar to `virtual`, you only activate `reflection` when you need it. It is disabled by default.

# Examples

There are two version of reflection in this library. `REFLECT` and `REFLECT_PRINTABLE`.

## REFLECT

`REFLECT` provides the bare metal of reflection. When used, the type gains the ability to use the following functions:

* `reflect::generate_meta_info<T>`
* `reflect::for_each<T>(...)`

The first one returns you an array of meta ids. Each meta id maps to a descriptor.  
However, it is by design that the meta id is not useable without any helper functions.  
  
Minimal example:

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

## REFLECT and PRINTABLE

`REFLECT_PRINTABLE` is an extension of `REFLECT`, but provides the printable version. Printable version consists of:

* `to_string` function which converts the class and reflected member variables into a string format. Note that this is contained in the type
* `std::ostream::operator<<` overload with the type
* `std::formatter` template specialization

_It is possible to add on to `REFLECT_PRINTABLE`, depending on the use case_

Minimal Example:

```c++
struct position_info
{
    double bod_position;
    double position;
    double buy_quantity;
    double sell_quantity;

    REFLECT_PRINTABLE(position_info, (bod_position, position, buy_quantity, sell_quantity));
};

int main(void)
{
    constexpr position_info pos{.bod_position = 1.0, .position = 3.0, .buy_quantity = 4.0, .sell_quantity = 2.0};
    
    std::cout << to_string(pos) << '\n'; // should not be used
    // OR
    std::cout << pos << '\n';
    // OR
    std::cout << std::format("{}\n", pos);
}
```
