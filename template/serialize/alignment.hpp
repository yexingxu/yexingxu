#include <algorithm>
#include <cstddef>
#include <type_traits>

#include "reflection.hpp"

#ifndef SERIALIZE_ALIGN_ALIGNMENT_HPP_
#define SERIALIZE_ALIGN_ALIGNMENT_HPP_

namespace serialize {
namespace details {

namespace align {

template <typename T>
constexpr std::size_t alignment_impl();

template <typename T>
static constexpr std::size_t alignment_v = alignment_impl<T>();

template <typename type, std::size_t... I>
constexpr std::size_t default_alignment_helper(std::index_sequence<I...>) {
  return (std::max)({(
      is_compatible_v<remove_cvref_t<std::tuple_element_t<I, type>>>
          ? std::size_t{0}
          : align::alignment_v<
                remove_cvref_t<std::tuple_element_t<I, type>>>)...});
}

template <typename T, std::enable_if_t<is_trivial_view_v<T>, int> = 0>
constexpr std::size_t default_alignment() {
  return std::alignment_of<typename T::value_type>::value;
}

template <typename T,
          std::enable_if_t<is_trivial_serializable<T>::value, int> = 0>
constexpr std::size_t default_alignment() {
  return std::alignment_of<T>::value;
}

template <typename T, std::enable_if_t<!is_trivial_serializable<T>::value &&
                                           !is_trivial_view_v<T>,
                                       int> = 0>
constexpr std::size_t default_alignment() {
  using type = decltype(get_types<T>());
  return default_alignment_helper<type>(
      std::make_index_sequence<std::tuple_size<type>::value>());
}

template <typename T>
constexpr std::size_t default_alignment_v = default_alignment<T>();

template <typename T>
constexpr std::size_t alignment_impl();

template <typename type, std::size_t... I>
constexpr std::size_t pack_alignment_impl_helper(std::index_sequence<I...>) {
  return (std::max)({(
      is_compatible_v<remove_cvref_t<std::tuple_element_t<I, type>>>
          ? std::size_t{0}
          : align::alignment_v<
                remove_cvref_t<std::tuple_element_t<I, type>>>)...});
}

template <typename T, std::size_t ret = serialize::pack_alignment_v<T>,
          std::enable_if_t<ret == 0, int> = 0>
constexpr std::size_t pack_alignment_impl() {
  static_assert(std::is_class<T>::value, "");
  static_assert(!is_trivial_view_v<T>, "");
  static_assert(
      ret == 0 || ret == 1 || ret == 2 || ret == 4 || ret == 8 || ret == 16,
      "");
  using type = decltype(get_types<T>());
  return pack_alignment_impl_helper<type>(
      std::make_index_sequence<std::tuple_size<type>::value>());
}

template <typename T, std::size_t ret = serialize::pack_alignment_v<T>,
          std::enable_if_t<ret != 0, int> = 0>
constexpr std::size_t pack_alignment_impl() {
  static_assert(std::is_class<T>::value, "");
  static_assert(!is_trivial_view_v<T>, "");
  static_assert(
      ret == 0 || ret == 1 || ret == 2 || ret == 4 || ret == 8 || ret == 16,
      "");
  return ret;
}

template <typename T>
constexpr std::size_t pack_alignment_v = pack_alignment_impl<T>();

template <typename T, std::enable_if_t<serialize::alignment_v<T> == 0 &&
                                           serialize::pack_alignment_v<T> == 0,
                                       int> = 0>
constexpr std::size_t alignment_impl() {
  return default_alignment_v<T>;
}

template <typename T, std::enable_if_t<serialize::alignment_v<T> != 0, int> = 0>
constexpr std::size_t alignment_impl() {
  constexpr auto ret = serialize::alignment_v<T>;
  static_assert(
      [](std::size_t align) {
        while (align % 2 == 0) {
          align /= 2;
        }
        return align == 1;
      }(ret),
      "alignment should be power of 2");
  return ret;
}

template <typename T, std::enable_if_t<serialize::pack_alignment_v<T> != 0 &&
                                           is_trivial_serializable<T>::value,
                                       int> = 0>
constexpr std::size_t alignment_impl() {
  return default_alignment_v<T>;
}

template <typename T, std::enable_if_t<serialize::pack_alignment_v<T> != 0 &&
                                           !is_trivial_serializable<T>::value,
                                       int> = 0>
constexpr std::size_t alignment_impl() {
  return pack_alignment_v<T>;
}

template <typename P, typename T, std::size_t I>
struct calculate_trival_obj_size {
  constexpr void operator()(std::size_t &total);
};

template <typename P, typename T, std::size_t I>
class calculate_padding_size_impl {
 private:
  void helper(std::size_t &offset,
              std::enable_if_t<is_trivial_serializable<T>::value, int> = 0) {
    offset += sizeof(T);
  }
  void helper(std::size_t &offset,
              std::enable_if_t<!is_trivial_serializable<T>::value, int> = 0) {
    for_each<T, calculate_trival_obj_size>(offset);
    static_assert(is_trivial_serializable<T, true>::value, "");
  }

 public:
  constexpr void operator()(
      std::size_t &,
      std::array<std::size_t, serialize::members_count<P> + 1> &padding_size,
      std::enable_if_t<is_compatible_v<T>, int> = 0) {
    padding_size[I] = 0;
  }

  constexpr void operator()(
      std::size_t &offset,
      std::array<std::size_t, serialize::members_count<P> + 1> &padding_size,
      std::enable_if_t<is_trivial_view_v<T>, int> = 0) {
    calculate_padding_size_impl<P, typename T::value_type, I>{}(offset,
                                                                padding_size);
  }

  constexpr void operator()(
      std::size_t &offset,
      std::array<std::size_t, serialize::members_count<P> + 1> &padding_size,
      std::enable_if_t<!is_trivial_view_v<T> && !is_compatible_v<T>, int> = 0) {
    if (offset % align::alignment_v<T>) {
      padding_size[I] =
          (std::min)(align::pack_alignment_v<P> - 1,
                     align::alignment_v<T> - offset % align::alignment_v<T>);
    } else {
      padding_size[I] = 0;
    }
    offset += padding_size[I];

    helper(offset);
  }
};

template <typename T>
constexpr auto calculate_padding_size() {
  std::array<std::size_t, serialize::members_count<T> + 1> padding_size{};
  std::size_t offset = 0;
  for_each<T, calculate_padding_size_impl>(offset, padding_size);
  if (offset % align::alignment_v<T>) {
    padding_size[serialize::members_count<T>] =
        align::alignment_v<T> - offset % align::alignment_v<T>;
  } else {
    padding_size[serialize::members_count<T>] = 0;
  }
  return padding_size;
}

template <typename T>
constexpr std::array<std::size_t, serialize::members_count<T> + 1>
    padding_size = calculate_padding_size<T>();

template <typename T>
constexpr std::size_t get_total_padding_size() {
  std::size_t sum = 0;
  for (auto &e : padding_size<T>) {
    sum += e;
  }
  return sum;
};

template <typename T>
constexpr std::size_t total_padding_size = get_total_padding_size<T>();

template <typename P, typename T, std::size_t I>
using calculate_trival_obj_size_wrapper = calculate_trival_obj_size<P, T, I>;

template <typename P, typename T, std::size_t I>
constexpr void calculate_trival_obj_size<P, T, I>::operator()(
    std::size_t &total) {
  if (I == 0) {
    total += total_padding_size<P>;
  }
  if (!is_compatible_v<T>) {
    if (is_trivial_serializable<T>::value) {
      total += sizeof(T);
    } else if (is_trivial_view_v<T>) {
      total += sizeof(typename T::value_type);
    } else {
      //   static_assert(is_trivial_serializable<T, true>::value);
      std::size_t offset = 0;
      for_each<T, calculate_trival_obj_size_wrapper>(offset);
      total += offset;
    }
  }
}

}  // namespace align
}  // namespace details
}  // namespace serialize

#endif