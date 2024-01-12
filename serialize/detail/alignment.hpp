/*
 * @Description:e
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2024-01-05 23:06:19
 * @LastEditors: chen, hua
 * @LastEditTime: 2024-01-11 14:15:40
 */

#pragma once

#include <cstddef>
#include <type_traits>

#include "detail/reflection.hpp"
#include "ser_config.hpp"

namespace serialize {
namespace detail {

template <alignment Align>
constexpr std::size_t inline calculate_one_padding_size(std::size_t &offset) {
  return offset & (static_cast<std::size_t>(Align) - 1);
}

template <typename P, typename T, std::size_t I>
struct calculate_trival_obj_size;

template <typename P, typename T, std::size_t I>
struct calculate_padding_size_impl {
  template <typename Type = T>
  constexpr void operator()(
      std::size_t align, std::size_t &offset,
      std::array<std::size_t, members_count<P>() + 1> &padding_size,
      std::enable_if_t<is_trivial_view_v<Type>, int> = 0) {
    calculate_padding_size_impl<P, typename Type::value_type, I>{}(
        align, offset, padding_size);
  }
  template <typename Type = T>
  constexpr void operator()(
      std::size_t align, std::size_t &offset,
      std::array<std::size_t, members_count<P>() + 1> &padding_size,
      std::enable_if_t<!is_trivial_view_v<Type> &&
                           is_trivial_serializable<Type>::value,
                       int> = 0) {
    if (offset % align) {
      padding_size[I] = align - offset % align;
    } else {
      padding_size[I] = 0;
    }
    offset += padding_size[I];
    offset += sizeof(Type);
  }
  template <typename Type = T>
  constexpr void operator()(
      std::size_t align, std::size_t &offset,
      std::array<std::size_t, members_count<P>() + 1> &padding_size,
      std::enable_if_t<!is_trivial_view_v<Type> &&
                           !is_trivial_serializable<Type>::value,
                       int> = 0) {
    if (offset % align) {
      padding_size[I] = align - offset % align;
    } else {
      padding_size[I] = 0;
    }
    offset += padding_size[I];
    // TODO
    for_each<T, calculate_trival_obj_size>(align, offset);
  }
};

template <typename T>
constexpr auto calculate_padding_size(std::size_t align) {
  std::array<std::size_t, members_count<T>() + 1> padding_size{};
  std::size_t offset = 0;
  for_each<T, calculate_padding_size_impl>(align, offset, padding_size);

  if (offset % align) {
    padding_size[members_count<T>()] = align - offset % align;
  } else {
    padding_size[members_count<T>()] = 0;
  }
  return padding_size;
}

template <typename T>
constexpr std::size_t get_total_padding_size(std::size_t align) {
  std::size_t sum = 0;
  auto padding_size = calculate_padding_size<T>(align);
  for (auto &e : padding_size) {
    sum += e;
  }
  return sum;
};

template <typename P, typename T, std::size_t I>
using calculate_trival_obj_size_wrapper = calculate_trival_obj_size<P, T, I>;

template <typename P, typename Ty, std::size_t I>
struct calculate_trival_obj_size {
  template <typename T = Ty, std::enable_if_t<is_trivial_view_v<T>, int> = 0>
  constexpr void operator()(std::size_t align, std::size_t &total,
                            std::enable_if_t<is_trivial_view_v<T>, int> = 0) {
    helper(align, total);
    total += sizeof(typename T::value_type);
  }
  template <typename T = Ty,
            std::enable_if_t<is_trivial_serializable<T>::value, int> = 0>
  constexpr void operator()(std::size_t align, std::size_t &total) {
    helper(align, total);

    total += sizeof(T);
  }
  template <typename T = Ty,
            std::enable_if_t<!is_trivial_serializable<T>::value, int> = 0>
  constexpr void operator()(std::size_t, std::size_t &) {}

 private:
  template <std::size_t In = I, typename std::enable_if_t<In == 0, int> = 0>
  constexpr void helper(std::size_t align, std::size_t &total) {
    total += get_total_padding_size<P>(align);
  }
  template <std::size_t In = I, typename std::enable_if_t<In != 0, int> = 0>
  constexpr void helper(std::size_t, std::size_t &) {
    return;
  }
};

}  // namespace detail
}  // namespace serialize