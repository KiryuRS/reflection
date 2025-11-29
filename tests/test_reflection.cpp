#include "../include/reflect.hpp"

#include <string_view>
#include <string>
#include <format>
#include <iostream>

struct foo
{
    int x;
    float y;
    long double z;
    std::string_view str_view = "Hello World";
    std::string str;

    REFLECT(foo, (x, y, z, str_view, str));
};

struct bar
{
    uint64_t id;
    uint32_t book_id;
    double price;
    double multiplier;
    double position;
};

int main(void)
{
    foo f;
    f.x = 100;
    f.y = 3.14f;
    f.z = 6.9;
    f.str = "goodbye my friend";

    reflect::for_each<foo>([&f] <typename Descriptor> () {
        std::cout << std::format("foo member: {} of type {} has value {}",
                                 Descriptor::name,
                                 Descriptor::mem_type_str,
                                 reflect::get_member_variable<Descriptor>(f))
                  << '\n';
    });

    reflect::for_each<bar>([] <typename Descriptor> () {});
}
