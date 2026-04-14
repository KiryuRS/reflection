#pragma once

#include "concepts.hpp"
#include "typelist.hpp"
#include "utility.hpp"
#include "preprocessor.hpp"

#include <iomanip>
#include <sstream>
#include <utility>

namespace reflect {

namespace detail {

template <typename>
struct introspection;

template <typename Class, typename Member>
struct introspection<Member Class::*>
{
    using member_type = Member;
    using member_pointer_type = Member (Class::*);

    static constexpr std::string_view mem_type_str = ::reflect::utility::get_name<member_type>();
};

template <typename Class, typename ReturnType, typename ... Args>
struct introspection<ReturnType (Class::*)(Args...)>
{
    using member_type = ReturnType(Args...);
    using member_pointer_type = ReturnType (Class::*)(Args...);
    using return_type = ReturnType;
    using arguments_type = typelist<Args...>;

    // dont extend this for introspection. lazy way of differentiating from member variable
    static constexpr std::string_view mem_type_str = "class member function";
};

template <auto>
struct meta_id
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-template-friend"
    friend consteval auto get_meta_info(meta_id);
#pragma GCC diagnostic pop
};

template <typename T, auto V = int{}>
struct meta_type
{
    using value_type = T;
    static constexpr auto value = V;

    static void id() {} // unique type

    friend consteval auto get_meta_info(meta_id<id>)
    {
        return meta_type{};
    }
};

template <typename T, auto V = int{}>
static constexpr auto meta_type_info = meta_type<T, V>::id;

template <auto MetaTypeInfo>
using meta_type_underlying_type = typename decltype(get_meta_info(meta_id<MetaTypeInfo>{}))::value_type;

} // namespace detail

template <concepts::reflectable T>
consteval auto generate_meta_info()
{
    return decltype(get_meta_info(detail::meta_id<T::meta_info_array_as_id()>{}))::value;
}

template <concepts::descriptor_like Descriptor, concepts::reflectable T>
constexpr decltype(auto) get_member_variable(T&& obj)
{
    return std::forward<T>(obj).*Descriptor::mem_ptr;
}

template <concepts::descriptor_like Descriptor, concepts::reflectable T>
constexpr decltype(auto) get_member_variable(T&& obj, Descriptor descriptor)
{
    return std::forward<T>(obj).*(descriptor.mem_ptr);
}

template <concepts::reflectable T, typename Functor>
constexpr void for_each(Functor&& func)
{
    // unfortunately the existing range-based for loop (pre C++26) does not have constexpr support,
    // so we use the traditional lambda + variadic to loop through all of the meta types.
    // ideally what we want is:
    // template for (auto meta : generate_meta_info<T>())
    // {
    //     constexpr auto descriptor = get_descriptor<meta>();
    //     ... // do something with the descriptor
    // }
    static constexpr auto META_ARRAY_INFO = generate_meta_info<T>();
    static_assert(concepts::any_invocable<Functor, detail::meta_type_underlying_type<META_ARRAY_INFO[0]>>, "Functor is not invocable!");

    const auto on_each_visit = [&func] <size_t I> () {
        using descriptor_t = detail::meta_type_underlying_type<META_ARRAY_INFO[I]>;

        if constexpr (concepts::template_only_invocable<Functor, descriptor_t>)
            func.template operator()<descriptor_t>();
        else if constexpr (concepts::template_invocable<Functor, descriptor_t>)
            func.template operator()<descriptor_t>(descriptor_t{});
        else if constexpr (std::invocable<Functor, descriptor_t>)
            func(descriptor_t{});
    };

    const auto visitor = [&on_each_visit] <size_t ... Is> (std::index_sequence<Is...>) {
        (on_each_visit.template operator()<Is>(), ...);
    };

    visitor(std::make_index_sequence<META_ARRAY_INFO.size()>{});
}

#define GENERATE_DESCRIPTOR(Class, Member)                                                              \
    struct PP_CREATE_CLASS_NAME(descriptor, Class, Member)                                              \
    {                                                                                                   \
        using introspection_type = ::reflect::detail::introspection<decltype(&Class::Member)>;          \
        using class_type = struct Class;                                                                \
        using member_type = typename introspection_type::member_type;                                   \
        using member_pointer_type = typename introspection_type::member_pointer_type;                   \
                                                                                                        \
        static constexpr std::string_view name = PP_STRINGIZE(Member);                                  \
        static constexpr std::string_view mem_type_str = introspection_type::mem_type_str;              \
        static constexpr member_pointer_type mem_ptr = &Class::Member;                                  \
    };

#define GENERATE_MEMBER_META_INFO(Class, Member) ::reflect::detail::meta_type_info<PP_CREATE_CLASS_NAME(descriptor, Class, Member)>,
#define GET_META_INFO_ARRAY(_, Base) Base::meta_info_array(),

#define REFLECT(Class, Bases, Members)                                                                                      \
    PP_FOR_EACH(GENERATE_DESCRIPTOR, Class, PP_EXPAND_STRIP(Members))                                                       \
    static consteval auto meta_info_array()                                                                                 \
    {                                                                                                                       \
        static constexpr std::array meta{PP_FOR_EACH(GENERATE_MEMBER_META_INFO, Class, PP_EXPAND_STRIP(Members))};          \
        return meta;                                                                                                        \
    }                                                                                                                       \
                                                                                                                            \
    static consteval auto meta_info_array_as_id()                                                                           \
    {                                                                                                                       \
        static constexpr std::array meta{PP_FOR_EACH(GENERATE_MEMBER_META_INFO, Class, PP_EXPAND_STRIP(Members))};          \
        constexpr auto metas = ::reflect::utility::concat_arrays(PP_FOR_EACH_IN_TUPLE(GET_META_INFO_ARRAY, _, Bases) meta); \
        return ::reflect::detail::meta_type_info<struct Class, metas>;                                                      \
    }

/* To be used within REFLECT_PRINTABLE macro */
#define OSTREAM_PRINT(_, value)                                                             \
    oss << std::fixed << std::setprecision(3) << std::exchange(delimiter, ", ") << "'"      \
        << PP_STRINGIZE(value) << "': " << object.value;

/* To be used within REFLECT_PRINTABLE macro */
#define OSTREAM_PRINT_BASE(_, Base) oss << to_string(static_cast<Base>(object)) << ", ";

#define REFLECT_PRINTABLE(Class, Bases, Members)                                                    \
    REFLECT(Class, Bases, Members)                                                                  \
    /* Other useful reflection helper functions. e.g. operator<< overload, to_string */             \
    friend std::string print_meta(const struct Class& object)                                       \
    {                                                                                               \
        std::ostringstream oss;                                                                     \
        const char* delimiter = "";                                                                 \
        oss << "struct " << PP_STRINGIZE(Class) << " has the following reflected variables:\n";     \
        ::reflect::for_each<struct Class>([&oss, &object, &delimiter] <typename Descriptor> () {    \
            oss << std::exchange(delimiter, "\n") << "  "                                           \
                << Descriptor::mem_type_str << " " << Descriptor::name << " = "                     \
                << ::reflect::get_member_variable<Descriptor>(object);                              \
        });                                                                                         \
        return oss.str();                                                                           \
    }                                                                                               \
                                                                                                    \
    friend std::string to_string(const struct Class& object)                                        \
    {                                                                                               \
        std::ostringstream oss;                                                                     \
        const char* delimiter = "";                                                                 \
        oss << '{' << PP_STRINGIZE(Class) << ": {";                                                 \
        PP_FOR_EACH_IN_TUPLE(OSTREAM_PRINT_BASE, _, Bases)                                          \
        PP_FOR_EACH(OSTREAM_PRINT, _, PP_EXPAND_STRIP(Members))                                     \
        oss << "} }";                                                                               \
        return oss.str();                                                                           \
    }                                                                                               \
                                                                                                    \
    friend std::ostream& operator<<(std::ostream& os, const struct Class& object)                   \
    {                                                                                               \
        return os << to_string(object);                                                             \
    }

} // namespace reflect

namespace std {

template <reflect::concepts::reflect_and_printable T>
struct formatter<T> : formatter<string>
{
    auto format(const T& object, format_context& ctx) const
    {
        return formatter<string>::format(to_string(object), ctx);
    }
};

} // namespace std
