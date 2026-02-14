#pragma once

#include "../../include/reflect/reflect.hpp"
#include "concepts.hpp"

#include <yaml-cpp/yaml.h>

namespace YAML {

template <::reflect::concepts::reflectable T>
struct convert<T>
{
    static Node encode(const T& obj)
    {
        Node node{};
        ::reflect::for_each<T>([&node, &obj] <typename Descriptor> () {
            using member_type = typename Descriptor::member_type;
            const auto& member = ::reflect::get_member_variable<Descriptor>(obj);

            if constexpr (::yaml::concepts::container_like<member_type>)
            {
                if (member.empty())
                    return;
                node[Descriptor::name] = member;
            }
            else if constexpr (::yaml::concepts::same_as_optional<member_type>)
            {
                if (!member)
                    return;
                node[Descriptor::name] = *member;
            }
            else
            {
                node[Descriptor::name] = member;
            }
        });
        return node;
    }

    static bool decode(const Node& node, T& obj)
    {
        // TODO: yaml key validation?

        ::reflect::for_each<T>([&node, &obj] <typename Descriptor> () {
            using member_type = typename Descriptor::member_type;
            auto& member = ::reflect::get_member_variable<Descriptor>(obj);

            if constexpr (::yaml::concepts::container_like<member_type>)
            {
                if (!node[Descriptor::name])
                    return;
                member = node[Descriptor::name].template as<member_type>();
            }
            else if constexpr (::yaml::concepts::same_as_optional<member_type>)
            {
                if (!node[Descriptor::name])
                    return;
                member = node[Descriptor::name].template as<typename member_type::value_type>();
            }
            else
            {
                member = node[Descriptor::name].template as<member_type>();
            }
        });
        return true;
    }
};

} // namespace YAML
