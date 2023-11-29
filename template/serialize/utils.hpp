/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-11-28 19:06:18
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-11-29 01:25:54
 */

#include <type_traits>

#ifndef SERIALIZE_DETAILS_UTILS_HPP_
#define SERIALIZE_DETAILS_UTILS_HPP_

namespace serialize {
namespace details {
template <class T>
static constexpr bool is_fundamental_v = std::is_fundamental<T>::value;

template <typename T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

[[noreturn]] inline void unreachable() {
  // Uses compiler specific extensions if possible.
  // Even if no extension is used, undefined behavior is still raised by
  // an empty function body and the noreturn attribute.
#ifdef __GNUC__  // GCC, Clang, ICC
  __builtin_unreachable();
#endif
}

template <typename T>
[[noreturn]] constexpr T declval() {
  unreachable();
}

}  // namespace details
}  // namespace serialize

#endif  // SERIALIZE_DETAILS_UTILS_HPP_