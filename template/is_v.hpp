/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-11-28 16:08:19
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-11-28 16:08:48
 */

#include <type_traits>

namespace serialize {
namespace details {
template <class T>
constexpr bool is_pointer_v = std::is_pointer<T>::value;
}
}  // namespace serialize