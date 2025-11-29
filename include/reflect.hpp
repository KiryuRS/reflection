#pragma once

#include "concepts.hpp"
#include "utility.hpp"

#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/tuple/to_seq.hpp>

namespace reflect {

namespace detail {

template <auto>
struct meta_id
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-template-friend"
    friend consteval auto get_meta_info(meta_id);
#pragma GCC diagnostic pop
};

template <typename, auto>
struct descriptor;

template <typename Class, auto MemberPtr>
struct meta_type
{
    using value_type = descriptor<Class, MemberPtr>;
    static void id() {} // unique type

    friend consteval auto get_meta_info(meta_id<id>)
    {
        return meta_type{};
    }
};

template <typename Class, auto MemberPtr>
static constexpr auto meta_info_1 = detail::meta_type<Class, MemberPtr>::id;

template <typename>
struct is_descriptor : std::false_type
{
};

template <typename T, auto MemPtr>
struct is_descriptor<descriptor<T, MemPtr>> : std::true_type
{
};

template <typename T, typename RawT = std::remove_cvref_t<T>>
concept same_as_descriptor = is_descriptor<T>::value;

} // namespace detail

template <typename T>
consteval auto generate_meta_info();

template <auto MetaInfo1>
using type_from_meta_info = typename decltype(get_meta_info(detail::meta_id<MetaInfo1>{}))::value_type;

template <typename T, typename RawT = std::remove_cvref_t<T>>
concept reflectable = requires {
    { generate_meta_info<RawT>() } -> concepts::same_as_array_type;
    requires detail::same_as_descriptor<type_from_meta_info<generate_meta_info<RawT>()[0]>>; // did you forget to use GENERATE_META_INFO macro?
};

template <detail::same_as_descriptor Descriptor, reflectable T>
constexpr decltype(auto) get_member_variable(T&& obj)
{
    return std::forward<T>(obj).*Descriptor::mem_ptr;
}

template <detail::same_as_descriptor Descriptor, reflectable T>
constexpr decltype(auto) get_member_variable(T&& obj, Descriptor descriptor)
{
    return std::forward<T>(obj).*(descriptor.mem_ptr);
}

template <reflectable T, typename Functor>
constexpr void for_each(Functor&& func)
{
    static constexpr auto META_ARRAY_INFO = generate_meta_info<T>();
    static_assert(concepts::any_invocable<Functor, type_from_meta_info<META_ARRAY_INFO[0]>>, "Functor is not invocable!");

    const auto on_each_visit = [&func] <size_t I> () {
        using descriptor_t = type_from_meta_info<META_ARRAY_INFO[I]>;

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

#define DEFINE_SPECIALIZED_DESCRIPTOR(r, Class, Index, Member) \
    namespace detail { \
        template <> \
        struct descriptor<Class, &Class::Member> \
        { \
            using class_type = Class; \
            using member_type = decltype(Class::Member); \
            using member_pointer_type = member_type Class::*; \
            \
            static constexpr std::string_view name = BOOST_PP_STRINGIZE(Member); \
            static constexpr std::string_view mem_type_str = utility::get_name<member_type>(); \
            static constexpr member_pointer_type mem_ptr = &Class::Member; \
        }; \
    } /* namespace detail */

#define GENERATE_META_INFO_1(r, Class, Member) detail::meta_info_1<Class, &Class::Member>,

#define GENERATE_META_INFO(Class, ...) \
    namespace reflect { \
    BOOST_PP_SEQ_FOR_EACH_I(DEFINE_SPECIALIZED_DESCRIPTOR, Class, BOOST_PP_TUPLE_TO_SEQ(__VA_ARGS__)) \
    template <> \
    consteval auto generate_meta_info<Class>() \
    { \
        return std::array{BOOST_PP_SEQ_FOR_EACH(GENERATE_META_INFO_1, Class, BOOST_PP_TUPLE_TO_SEQ(__VA_ARGS__))}; \
    } \
    } /* namespace reflect */

#define REFLECT(Class, ...) \
    GENERATE_META_INFO(Class, __VA_ARGS__) \
    /* TODO: Other kinds of reflection. operator<< overload? std::formatter? */

} // namespace reflect
