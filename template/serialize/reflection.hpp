/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-11-28 16:41:13
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-11-29 03:30:20
 */
#include <cstdint>
#include <type_traits>
#include <vector>

#include "compatible.hpp"
#include "utils.hpp"

#ifndef SERIALIZE_DETAILS_REFLECTION_HPP_
#define SERIALIZE_DETAILS_REFLECTION_HPP_
namespace serialize {

enum sp_config {
  DEFAULT = 0,
  DISABLE_TYPE_INFO = 0b1,
  ENABLE_TYPE_INFO = 0b10,
  DISABLE_ALL_META_INFO = 0b11
};

namespace details {

template <typename... Args>
using get_args_type = remove_cvref_t<typename std::conditional<
    sizeof...(Args) == 1, std::tuple_element_t<0, std::tuple<Args...>>,
    std::tuple<Args...>>::type>;

template <typename U>
constexpr auto get_types();

template <typename T, typename = void>
struct has_user_defined_id_impl : std::false_type {};

template <typename T>
struct has_user_defined_id_impl<
    T, std::void_t<std::integral_constant<std::size_t, T::struct_pack_id>>>
    : std::true_type {};

template <typename T>
constexpr bool has_user_defined_id = has_user_defined_id_impl<T>::value;

template <std::size_t sz>
struct constant_checker {};

template <typename T, typename = void>
struct has_user_defined_id_ADL_impl : std::false_type {};

template <typename T>
struct has_user_defined_id_ADL_impl<
    T, std::void_t<constant_checker<struct_pack_id((T*)nullptr)>>>
    : std::true_type {};

template <typename T>
constexpr bool has_user_defined_id_ADL = has_user_defined_id_ADL_impl<T>::value;

// is_base_class
// template <typename T, typename = void>
// struct is_base_class_impl : std::false_type {};

// template <typename T>
// struct is_base_class_impl<
//     T, std::void_t<
//            std::enable_if<std::is_same<
//                decltype(((T*)nullptr)->get_struct_pack_id()),
//                uint32_t>::value>,
//            typename struct_pack::detail::derived_class_set_t<T>>>
//     : std::true_type {};
// template <typename T>
// constexpr bool is_base_class = is_base_class_impl<T>::value;

// container_adapter
template <typename T, typename = void>
struct container_adapter_impl : std::false_type {};

template <typename T>
struct container_adapter_impl<
    T, std::void_t<typename remove_cvref_t<T>::value_type,
                   decltype(std::declval<T>().size()),
                   decltype(std::declval<T>().pop())>> : std::true_type {};

template <typename T>
constexpr bool container_adapter = container_adapter_impl<T>::value;

// container
template <typename T, typename = void>
struct container_impl : std::false_type {};

template <typename T>
struct container_impl<T, std::void_t<typename remove_cvref_t<T>::value_type,
                                     decltype(std::declval<T>().size()),
                                     decltype(std::declval<T>().begin()),
                                     decltype(std::declval<T>().end())>>
    : std::true_type {};

template <typename T>
constexpr bool container = container_impl<T>::value;

// is_char_t
template <typename Type>
constexpr bool is_char_t =
    std::is_same<Type, signed char>::value || std::is_same<Type, char>::value ||
    std::is_same<Type, unsigned char>::value ||
    std::is_same<Type, wchar_t>::value || std::is_same<Type, char16_t>::value ||
    std::is_same<Type, char32_t>::value
#ifdef __cpp_lib_char8_t
    || std::is_same<Type, char8_t>::value
#endif
    ;

// string
template <typename T, typename = void>
struct string_impl : std::false_type {};

template <typename T>
struct string_impl<
    T, std::void_t<
           std::enable_if_t<is_char_t<typename remove_cvref_t<T>::value_type>>,
           decltype(std::declval<T>().length()),
           decltype(std::declval<T>().data())>> : std::true_type {};

template <typename T>
constexpr bool string = string_impl<T>::value&& container<T>;

// string_view
template <typename T, typename = void>
struct string_view_impl : std::true_type {};

template <typename T>
struct string_view_impl<
    T, std::void_t<decltype(std::declval<T>().resize(std::size_t{}))>>
    : std::false_type {};

template <typename T>
constexpr bool string_view = string<T>&& string_view_impl<T>::value;

// span
template <typename T, typename = void>
struct span_impl : std::false_type {};

template <typename T>
struct span_impl<
    T, std::void_t<decltype(T{(typename T::value_type*)nullptr, std::size_t{}}),
                   decltype(std::declval<T>().subspan(
                       std::size_t{}, std::size_t{}))>> : std::true_type {};

template <typename T>
constexpr bool span = container<T>&& span_impl<T>::value;

// dynamic_span
template <typename T, typename = void>
struct dynamic_span_impl : std::false_type {};

template <typename T>
struct dynamic_span_impl<T,
                         std::void_t<std::enable_if_t<(T::extent == SIZE_MAX)>>>
    : std::true_type {};

template <typename T>
constexpr bool dynamic_span = span<T>&& dynamic_span_impl<T>::value;

// static_span
template <typename Type>
constexpr bool static_span = span<Type> && !dynamic_span<Type>;

//
template <typename Type>
static constexpr bool is_std_basic_string_v = false;

template <typename... args>
static constexpr bool is_std_basic_string_v<std::basic_string<args...>> = true;

template <typename Type>
static constexpr bool is_std_vector_v = false;

template <typename... args>
static constexpr bool is_std_vector_v<std::vector<args...>> = true;

template <typename Type>
static constexpr bool continuous_container =
    string<Type> ||
    (container<Type> && (is_std_vector_v<Type> || is_std_basic_string_v<Type>));

// map
template <typename T, typename = void>
struct map_container_impl : std::false_type {};

template <typename T>
struct map_container_impl<T,
                          std::void_t<typename remove_cvref_t<T>::mapped_type>>
    : std::true_type {};

template <typename T>
constexpr bool map_container = container<T>&& map_container_impl<T>::value;

// set
template <typename T, typename = void>
struct set_container_impl : std::false_type {};

template <typename T>
struct set_container_impl<T, std::void_t<typename remove_cvref_t<T>::key_type>>
    : std::true_type {};

template <typename T>
constexpr bool set_container = container<T>&& set_container_impl<T>::value;

// bitset
template <typename T, typename = void>
struct bitset_impl : std::false_type {};

template <typename T>
struct bitset_impl<T, std::void_t<decltype(std::declval<T>().flip()),
                                  decltype(std::declval<T>().set()),
                                  decltype(std::declval<T>().reset()),
                                  decltype(std::declval<T>().count()),
                                  decltype(std::declval<T>().size())>>
    : std::true_type {};

template <typename T, std::enable_if_t<bitset_impl<T>::value, int> = 0>
constexpr bool bitset_size_checker() {
  return (T{}.size() + 7) / 8 == sizeof(T);
}

template <typename T, std::enable_if_t<!bitset_impl<T>::value, int> = 0>
constexpr bool bitset_size_checker() {
  return false;
}

template <typename T>
constexpr bool bitset = bitset_impl<T>::value&& bitset_size_checker<T>();

// tuple
template <typename T, typename = void>
struct tuple_impl : std::false_type {};

template <typename T>
struct tuple_impl<
    T, std::void_t<decltype(std::get<0>(std::declval<T>())),
                   decltype(sizeof(std::tuple_size<remove_cvref_t<T>>::value))>>
    : std::true_type {};

template <typename T>
constexpr bool tuple = tuple_impl<T>::value;

//
template <typename T, typename = void>
struct user_defined_refl_impl : std::false_type {};

template <typename T>
struct user_defined_refl_impl<T, std::void_t<std::enable_if_t<std::is_same<
                                     decltype(std::declval<T&>()), T&>::value>>>
    : std::true_type {};

template <typename T>
constexpr bool user_defined_refl = user_defined_refl_impl<T>::value;

// tuple_size
template <typename T, typename = void>
struct tuple_size_impl : std::false_type {};

template <typename T>
struct tuple_size_impl<
    T, std::void_t<decltype(std::tuple_size<remove_cvref_t<T>>::value)>>
    : std::true_type {};

template <typename T>
constexpr bool tuple_size = tuple_size_impl<T>::value;
// array
template <typename T, typename = void>
struct array_impl : std::false_type {};

template <typename T>
struct array_impl<T,
                  std::void_t<decltype(std::declval<T>().size()),
                              decltype(std::tuple_size<remove_cvref_t<T>>{})>>
    : std::true_type {};

template <typename T>
constexpr bool array = array_impl<T>::value;

template <class T>
constexpr bool c_array =
    std::is_array<T>::value&& std::extent<remove_cvref_t<T>>::value > 0;

// pair
template <typename T, typename = void>
struct pair_impl : std::false_type {};

template <typename T>
struct pair_impl<T, std::void_t<typename remove_cvref_t<T>::first_type,
                                typename remove_cvref_t<T>::second_type,
                                decltype(std::declval<T>().first),
                                decltype(std::declval<T>().second)>>
    : std::true_type {};

template <typename T>
constexpr bool pair = pair_impl<T>::value;

// expect
template <typename T, typename = void>
struct expected_impl : std::false_type {};

template <typename T>
struct expected_impl<T, std::void_t<typename remove_cvref_t<T>::value_type,
                                    typename remove_cvref_t<T>::error_type,
                                    typename remove_cvref_t<T>::unexpected_type,
                                    decltype(std::declval<T>().has_value()),
                                    decltype(std::declval<T>().error())>>
    : std::true_type {};
// TODO: check e.value()
template <typename T>
constexpr bool expected = expected_impl<T>::value;

// unique_ptr
template <typename T, typename = void>
struct unique_ptr_impl : std::false_type {};

template <typename T>
struct unique_ptr_impl<T, std::void_t<typename remove_cvref_t<T>::element_type,
                                      decltype(std::declval<T>().operator*())>>
    : std::true_type {};

template <typename T>
constexpr bool unique_ptr = unique_ptr_impl<T>::value;

// optional
template <typename T, typename = void>
struct optional_impl : std::false_type {};

template <typename T>
struct optional_impl<T, std::void_t<decltype(std::declval<T>().value()),
                                    decltype(std::declval<T>().has_value()),
                                    decltype(std::declval<T>().operator*()),
                                    typename remove_cvref_t<T>::value_type>>
    : std::true_type {};

template <typename T>
constexpr bool optional = !expected<T> && optional_impl<T>::value;

template <typename Type>
static constexpr bool is_compatible_v = false;

template <typename Type, uint64_t version>
static constexpr bool is_compatible_v<types::compatible<Type, version>> = true;

template <typename Type>
static constexpr bool is_variant_v = false;

// template <typename... args>
// static constexpr bool is_variant_v<std::variant<args...>> = true;

template <typename T>
static constexpr bool is_trivial_tuple = false;

template <typename T>
class varint;

template <typename T>
class sint;

template <typename T>
constexpr bool varintable_t = std::is_same<T, varint<int32_t>>::value ||
                              std::is_same<T, varint<int64_t>>::value ||
                              std::is_same<T, varint<uint32_t>>::value ||
                              std::is_same<T, varint<uint64_t>>::value;
template <typename T>
constexpr bool sintable_t = std::is_same<T, sint<int32_t>>::value ||
                            std::is_same<T, sint<int64_t>>::value;

template <typename T>
constexpr bool varint_t = varintable_t<T> || sintable_t<T>;

template <typename Type>
static constexpr bool is_trivial_view_v = false;

template <typename T, bool ignore_compatible_field = false>
struct is_trivial_serializable {
 private:
  template <typename U, std::size_t... I>
  static constexpr bool class_visit_helper(std::index_sequence<I...>) {
    std::initializer_list<bool> list{
        (0, is_trivial_serializable<std::tuple_element_t<I, U>,
                                    ignore_compatible_field>::value)...};
    bool ret = true;
    for (auto i = list.begin(); i != list.end(); ++i) {
      ret = ret & *i;
    }
    return ret;
  }

  template <typename Type = T>
  static constexpr std::enable_if_t<std::is_abstract<Type>::value> solve() {
    return false;
  }
  template <typename Type = T>
  static constexpr std::enable_if_t<is_compatible_v<Type> ||
                                    is_trivial_view_v<Type>>
  solve() {
    return ignore_compatible_field;
  }
  template <typename Type = T>
  static constexpr std::enable_if_t<
      std::is_enum<Type>::value || std::is_fundamental<Type>::value ||
      bitset<Type>
#if (__GNUC__ || __clang__)
      || std::is_same<__int128, Type>::value ||
      std::is_same<unsigned __int128, Type>::value>
#endif
  solve() {
    return true;
  }

  template <typename Type = T>
  static constexpr std::enable_if_t<array<Type>> solve() {
    return is_trivial_serializable<typename Type::value_type,
                                   ignore_compatible_field>::value;
  }
  template <typename Type = T>
  static constexpr std::enable_if_t<c_array<Type>> solve() {
    return is_trivial_serializable<typename std::remove_all_extents<Type>::type,
                                   ignore_compatible_field>::value;
  }

  template <typename Type = T>
  static constexpr std::enable_if_t<!pair<Type> && tuple<Type> &&
                                    !is_trivial_tuple<Type>>
  solve() {
    return false;
  }
  template <typename Type = T>
  static constexpr std::enable_if_t<user_defined_refl<Type>> solve() {
    return false;
  }
  template <typename Type = T>
  static constexpr std::enable_if_t<container<Type> || optional<Type> ||
                                    is_variant_v<Type> || unique_ptr<Type> ||
                                    expected<Type> || container_adapter<Type> ||
                                    varint_t<Type>>
  solve() {
    return false;
  }

  template <typename Type = T>
  static constexpr std::enable_if_t<pair<Type>> solve() {
    return is_trivial_serializable<typename T::first_type,
                                   ignore_compatible_field>::value &&
           is_trivial_serializable<typename T::second_type,
                                   ignore_compatible_field>::value;
  }

  template <typename Type = T>
  static constexpr std::enable_if_t<is_trivial_tuple<Type>> solve() {
    return class_visit_helper<Type>(
        std::make_index_sequence<std::tuple_size<Type>::value>{});
  }

  template <typename Type = T>
  static constexpr std::enable_if_t<std::is_class<Type>::value> solve() {
    using U = decltype(get_types<T>());
    return class_visit_helper<U>(
        std::make_index_sequence<std::tuple_size<U>::value>{});
  }

 public:
  static constexpr bool value = is_trivial_serializable::solve();
};

// trivially_copyable_container
template <typename T, typename = void>
struct trivially_copyable_container_impl : std::false_type {};

template <typename T>
struct trivially_copyable_container_impl<
    T, std::void_t<std::enable_if_t<
           is_trivial_serializable<typename T::value_type>::value>>>
    : std::true_type {};

template <typename Type>
constexpr bool trivially_copyable_container =
    continuous_container<Type>&& trivially_copyable_container_impl<Type>::value;

}  // namespace details
}  // namespace serialize

#endif
