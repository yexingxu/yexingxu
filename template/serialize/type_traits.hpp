/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-11-29 01:58:32
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-11-29 02:22:16
 */

#include <type_traits>

#include "reflection.hpp"

namespace serialize {
namespace details {
template <typename T>
constexpr bool serialize_byte =
    std::is_same<char, T>::value || std::is_same<unsigned char, T>::value ||
    std::is_same<signed char, T>::value;

template <typename T>
constexpr bool serialize_buffer =
    trivially_copyable_container<T>&& serialize_byte<typename T::value_type>;

template <typename Reader>
struct MD5_reader_wrapper;

template <typename T>
static constexpr bool is_MD5_reader_wrapper = false;

template <typename T>
static constexpr bool is_MD5_reader_wrapper<MD5_reader_wrapper<T>> = true;
}  // namespace details
}  // namespace serialize