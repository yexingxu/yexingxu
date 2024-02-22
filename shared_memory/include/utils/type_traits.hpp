

#pragma once

#include <type_traits>
namespace shm {

///
/// @brief Conditionally add const to type T if C has the const qualifier
/// @tparam T is the type to conditionally add the const qualifier
/// @tparam Condition is the type which determines if the const qualifier needs
/// to be added to T
///
template <typename T, typename C>
struct add_const_conditionally {
  using type = T;
};
template <typename T, typename C>
struct add_const_conditionally<T, const C> {
  using type = const T;
};
///
/// @brief Helper type for add_const_conditionally which adds const to type T if
/// C has the const qualifier
///
template <typename T, typename C>
using add_const_conditionally_t = typename add_const_conditionally<T, C>::type;

///
/// @brief Helper value to bind a static_assert to a type
/// @code
/// static_assert(always_false_v<Foo>, "Not implemented for the given type!");
/// @endcode
///
template <typename>
constexpr bool always_false_v{false};

///
/// @brief Verifies whether the passed Callable type is in fact invocable with
/// the given arguments
///
template <typename Callable, typename... ArgTypes>
struct is_invocable {
  // This variant is chosen when Callable(ArgTypes) successfully resolves to a
  // valid type, i.e. is invocable.
  template <typename C, typename... As>
  static constexpr std::true_type test(
      typename std::__invoke_result<C, As...>::type*) noexcept {
    return {};
  }

  // AXIVION Next Construct AutosarC++19_03-A8.4.1 : we require a SFINEA failure
  // case where all parameter types (non invokable ones) are allowed, this can
  // be achieved with variadic arguments This is chosen if Callable(ArgTypes)
  // does not resolve to a valid type.
  template <typename C, typename... As>
  /// @NOLINTNEXTLINE(cert-dcl50-cpp)
  static constexpr std::false_type test(...) noexcept {
    return {};
  }

  // Test with nullptr as this can stand in for a pointer to any type.
  static constexpr bool value{
      decltype(test<Callable, ArgTypes...>(nullptr))::value};
};

///
/// @brief Verifies whether the passed Callable type is in fact invocable with
/// the given arguments
///        and the result of the invocation is convertible to ReturnType.
///
/// @note This is an implementation of std::is_invokable_r (C++17).
///
template <typename ReturnType, typename Callable, typename... ArgTypes>
struct is_invocable_r {
  template <typename C, typename... As>
  static constexpr std::true_type test(
      std::enable_if_t<
          std::is_convertible<typename std::__invoke_result<C, As...>::type,
                              ReturnType>::value>*) noexcept {
    return {};
  }
  // AXIVION Next Construct AutosarC++19_03-A8.4.1 : we require a SFINEA failure
  // case where all parameter types (non invokable ones) are allowed, this can
  // be achieved with variadic arguments
  template <typename C, typename... As>
  /// @NOLINTNEXTLINE(cert-dcl50-cpp)
  static constexpr std::false_type test(...) noexcept {
    return {};
  }

  // Test with nullptr as this can stand in for a pointer to any type.
  static constexpr bool value{
      decltype(test<Callable, ArgTypes...>(nullptr))::value};
};

}  // namespace shm