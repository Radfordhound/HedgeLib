#pragma once
#include <type_traits>

namespace HedgeLib
{
    // Adapted from "Possible implementation" section of this page:
    // https://en.cppreference.com/w/cpp/experimental/nonesuch
    // TODO: Remove these once nonesuch is implemented in the C++ standard

    struct nonesuch
    {
        ~nonesuch() = delete;
        nonesuch(nonesuch const&) = delete;
        void operator=(nonesuch const&) = delete;
    };

    // Adapted from "Possible implementation" section of this page:
    // https://en.cppreference.com/w/cpp/experimental/is_detected
    // TODO: Remove these once is_detected is implemented in the C++ standard

    template <class Default, class AlwaysVoid,
        template<class...> class Op, class... Args>
    struct detector
    {
        using value_t = std::false_type;
        using type = Default;
    };

    template <class Default, template<class...> class Op, class... Args>
    struct detector<Default, std::void_t<Op<Args...>>, Op, Args...>
    {
        using value_t = std::true_type;
        using type = Op<Args...>;
    };

    template <template<class...> class Op, class... Args>
    using is_detected = typename detector<nonesuch, void, Op, Args...>::value_t;

    template<template<class...> class Op, class... Args>
    constexpr bool is_detected_v = is_detected<Op, Args...>::value;
}
