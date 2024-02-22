#pragma once

#include <type_traits>
namespace shm {
namespace entity {

/// @brief This type trait ensures that the template parameter for Publisher,
/// Subscriber, Client and Server fulfill specific constrains.
/// @code
/// template <typename Data>
/// class Producer
/// {
///     using DataTypeAssert = typename TypedPortApiTrait<Data>::Assert;
///   public:
///     // ...
/// }
/// @endcode
/// @note 'typename TypedPortApiTrait<T>::Assert' has to be used otherwise the
/// compiler ignores the static_assert's
template <typename T>
struct TypedPortApiTrait {
  static_assert(!std::is_void<T>::value,
                "Must not be void. Use the untyped API for void types");
  static_assert(!std::is_const<T>::value, "Must not be const");
  static_assert(!std::is_reference<T>::value, "Must not be a reference");
  static_assert(!std::is_pointer<T>::value, "Must not be a pointer");
  using Assert = void;
};

}  // namespace entity
}  // namespace shm