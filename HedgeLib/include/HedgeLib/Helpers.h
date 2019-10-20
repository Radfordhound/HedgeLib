#pragma once
#ifdef __cplusplus
#include <type_traits>

// Adapted from "Possible implementation" section of this page:
// https://en.cppreference.com/w/cpp/experimental/nonesuch

// TODO: Add an ifdef that checks if std::nonesuch is present, and if so, aliases
// it instead of using this once it's added to the C++ standard
struct hl_INoneSuch
{
    ~hl_INoneSuch() = delete;
    hl_INoneSuch(hl_INoneSuch const&) = delete;
    void operator=(hl_INoneSuch const&) = delete;
};

// Adapted from "Possible implementation" section of this page:
// https://en.cppreference.com/w/cpp/experimental/is_detected

// TODO: Add an ifdef that checks if std::is_detected is present, and if so, aliases
// it instead of using this once it's added to the C++ standard

template <class Default, class AlwaysVoid,
    template<class...> class Op, class... Args>
struct hl_IDetector
{
    using value_t = std::false_type;
    using type = Default;
};

template <class... _Types>
using hl_IVoid_t =
#ifdef __cpp_lib_void_t
std::void_t<_Types...>;
#else
void;
#endif

template <class Default, template<class...> class Op, class... Args>
struct hl_IDetector<Default, hl_IVoid_t<Op<Args...>>, Op, Args...>
{
    using value_t = std::true_type;
    using type = Op<Args...>;
};

template <template<class...> class Op, class... Args>
using hl_IIsDetected = typename hl_IDetector<hl_INoneSuch, void, Op, Args...>::value_t;

template<template<class...> class Op, class... Args>
constexpr bool hl_IIsDetected_v = hl_IIsDetected<Op, Args...>::value;
#endif
