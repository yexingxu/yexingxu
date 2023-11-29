/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-11-28 16:48:43
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-11-29 02:19:24
 */

#include <cstdint>

#include "optional.hpp"

#ifndef TYPES_COMPATIBLE_HPP_
#define TYPES_COMPATIBLE_HPP_

namespace types {

template <typename T, uint64_t version>
struct compatible : public Optional<T> {
  constexpr compatible() = default;
  constexpr compatible(const compatible &other) = default;
  constexpr compatible(compatible &&other) = default;
  constexpr compatible(Optional<T> &&other) : Optional<T>(std::move(other)){};
  constexpr compatible(const Optional<T> &other) : Optional<T>(other){};
  constexpr compatible &operator=(const compatible &other) = default;
  constexpr compatible &operator=(compatible &&other) = default;
  using base = Optional<T>;
  using base::base;
  friend bool operator==(const compatible<T, version> &self,
                         const compatible<T, version> &other) {
    return static_cast<bool>(self) == static_cast<bool>(other) &&
           (!self || *self == *other);
  }
  static constexpr uint64_t version_number = version;
};
}  // namespace types

#endif