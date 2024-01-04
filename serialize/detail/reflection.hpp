/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-12-27 20:32:47
 * @LastEditors: chen, hua
 * @LastEditTime: 2024-01-04 22:09:30
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "for_each_macro.hpp"
#include "trivial_view.hpp"
#include "utils.hpp"

namespace serialize {
namespace detail {

template <typename... Args>
using get_args_type = remove_cvref_t<typename std::conditional<
    sizeof...(Args) == 1, std::tuple_element_t<0, std::tuple<Args...>>,
    std::tuple<Args...>>::type>;

// writer
template <typename T, typename = void>
struct writer_t_impl : std::false_type {};

template <typename T>
struct writer_t_impl<T, std::void_t<decltype(std::declval<T>().write(
                            (const char *)nullptr, std::size_t{}))>>
    : std::true_type {};

template <typename T>
constexpr bool writer_t = writer_t_impl<T>::value;

// reader
template <typename T, typename = void>
struct reader_t_impl : std::false_type {};

template <typename T>
struct reader_t_impl<
    T, std::void_t<decltype(std::declval<T>().read((char *)nullptr,
                                                   std::size_t{})),
                   decltype(std::declval<T>().ignore(std::size_t{})),
                   decltype(std::declval<T>().tellg())>> : std::true_type {};

template <typename T>
constexpr bool reader_t = reader_t_impl<T>::value;

template <typename T, typename = void>
struct view_reader_t_impl : std::false_type {};

template <typename T>
struct view_reader_t_impl<
    T, std::void_t<decltype(std::declval<T>().read_view(std::size_t{}))>>
    : std::true_type {};

template <typename T>
constexpr bool view_reader_t = reader_t<T> &&view_reader_t_impl<T>::value;

template <typename T, typename = void>
struct seek_reader_t_impl : std::false_type {};

template <typename T>
struct seek_reader_t_impl<
    T, std::void_t<decltype(std::declval<T>().seekg(std::size_t{}))>>
    : std::true_type {};

template <typename T>
constexpr bool seek_reader_t = reader_t<T> &&seek_reader_t_impl<T>::value;

template <typename T, typename = void>
struct deserialize_view_impl : std::false_type {};
template <typename T>
struct deserialize_view_impl<T, std::void_t<decltype(std::declval<T>().size()),
                                            decltype(std::declval<T>().data())>>
    : std::true_type {};

template <typename Type>
constexpr bool deserialize_view = deserialize_view_impl<Type>::value;

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
    std::is_same<Type, char32_t>::value;

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
constexpr bool string = string_impl<T>::value &&container<T>;

// string_view
template <typename T, typename = void>
struct string_view_impl : std::true_type {};

template <typename T>
struct string_view_impl<
    T, std::void_t<decltype(std::declval<T>().resize(std::size_t{}))>>
    : std::false_type {};

template <typename T>
constexpr bool string_view = string<T> &&string_view_impl<T>::value;

// span
template <typename T, typename = void>
struct span_impl : std::false_type {};

template <typename T>
struct span_impl<
    T, std::void_t<
           decltype(T{(typename T::value_type *)nullptr, std::size_t{}}),
           decltype(std::declval<T>().subspan(std::size_t{}, std::size_t{}))>>
    : std::true_type {};

template <typename T>
constexpr bool span = container<T> &&span_impl<T>::value;

// dynamic_span
template <typename T, typename = void>
struct dynamic_span_impl : std::false_type {};

template <typename T>
struct dynamic_span_impl<T,
                         std::void_t<std::enable_if_t<(T::extent == SIZE_MAX)>>>
    : std::true_type {};

template <typename T>
constexpr bool dynamic_span = span<T> &&dynamic_span_impl<T>::value;

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
struct has_mapped_type_impl : std::false_type {};

template <typename T>
struct has_mapped_type_impl<
    T, std::void_t<typename remove_cvref_t<T>::mapped_type>> : std::true_type {
};

template <typename T, typename = void>
struct has_key_type_impl : std::false_type {};

template <typename T>
struct has_key_type_impl<T, std::void_t<typename remove_cvref_t<T>::key_type>>
    : std::true_type {};

template <typename T>
constexpr bool map_container =
    container<T> &&has_key_type_impl<T>::value &&has_mapped_type_impl<T>::value;
// set
template <typename T>
constexpr bool set_container = container<T> &&has_key_type_impl<T>::value &&
                               !has_mapped_type_impl<T>::value;

// expect
template <typename T, typename = void>
struct has_error_type_impl : std::false_type {};

template <typename T>
struct has_error_type_impl<T,
                           std::void_t<typename remove_cvref_t<T>::error_type>>
    : std::true_type {};

template <typename T, typename = void>
struct has_unexpected_type_impl : std::false_type {};

template <typename T>
struct has_unexpected_type_impl<
    T, std::void_t<typename remove_cvref_t<T>::unexpected_type>>
    : std::true_type {};

template <typename T, typename = void>
struct expected_or_optional_impl : std::false_type {};

template <typename T>
struct expected_or_optional_impl<
    T, std::void_t<typename remove_cvref_t<T>::value_type,
                   decltype(std::declval<T>().has_value()),
                   decltype(std::declval<T>().value())>> : std::true_type {};
// TODO: check e.value()
template <typename T>
constexpr bool expected = expected_or_optional_impl<T>::value
    &&has_error_type_impl<T>::value &&has_unexpected_type_impl<T>::value;

// optional
template <typename T, typename = void>
struct optional_impl : std::false_type {};

template <typename T>
struct optional_impl<T, std::void_t<decltype(std::declval<T>().value()),
                                    decltype(std::declval<T>().has_value()),
                                    typename remove_cvref_t<T>::value_type>>
    : std::true_type {};

template <typename T>
constexpr bool optional =
    !has_error_type_impl<T>::value && !has_unexpected_type_impl<T>::value &&
    expected_or_optional_impl<T>::value;

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

// template <typename T,
//           std::enable_if_t<bitset_impl<T>::value && T{}.size() < 64, int> =
//           0>
// constexpr bool bitset_size_checker() {
//   return 8 == sizeof(T);
// }

// template <typename T,
//           std::enable_if_t<bitset_impl<T>::value && T{}.size() >= 64, int> =
//           0>
// constexpr bool bitset_size_checker() {
//   return (T{}.size() + 7) / 8 == sizeof(T);
// }

// template <typename T, std::enable_if_t<!bitset_impl<T>::value, int> = 0>
// constexpr bool bitset_size_checker() {
//   return false;
// }

template <typename T>
constexpr bool bitset = bitset_impl<T>::value;

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
    std::is_array<T>::value &&std::extent<remove_cvref_t<T>>::value > 0;

// tuple_size
template <typename T, typename = void>
struct tuple_size_impl : std::false_type {};

template <typename T>
struct tuple_size_impl<
    T, std::void_t<decltype(std::tuple_size<remove_cvref_t<T>>::value)>>
    : std::true_type {};

template <typename T>
constexpr bool tuple_size = tuple_size_impl<T>::value;

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

// TODO struct
// template <typename T, typename = void>
// struct struct_impl : std::false_type {};

// template <typename T>
// struct struct_impl<T, std::void_t<typename remove_cvref_t<T>::first_type,
//                                   typename remove_cvref_t<T>::second_type,
//                                   decltype(std::declval<T>().first),
//                                   decltype(std::declval<T>().second)>>
//     : std::true_type {};

template <typename T>
constexpr bool user_struct = !container<T> && !bitset<T> && !pair<T> &&
                             !tuple<T> && !array<T> && std::is_class<T>::value;

template <typename T, typename = void>
struct user_defined_refl_impl : std::false_type {};

template <typename T>
struct user_defined_refl_impl<
    T, std::void_t<std::enable_if_t<
           std::is_same<decltype(std::declval<T &>()), T &>::value>>>
    : std::true_type {};

template <typename T>
constexpr bool user_defined_refl = user_defined_refl_impl<T>::value;

// is trivail
template <typename T>
constexpr bool is_trivial_tuple = false;

template <typename Type>
constexpr static bool is_trivial_view_v = false;

template <typename Type>
constexpr static bool is_trivial_view_v<trivial_view<Type>> = true;

// members count
// members count
struct UniversalVectorType {
  template <typename T>
  operator std::vector<T>();
};

struct UniversalType {
  template <typename T>
  operator T();
};

struct UniversalIntegralType {
  template <typename T, typename = std::enable_if_t<std::is_integral<T>::value>>
  operator T();
};

struct UniversalNullptrType {
  operator std::nullptr_t();
};

struct UniversalOptionalType {
  template <typename U, typename = std::enable_if_t<optional<U>>>
  operator U();
};

template <typename T, typename construct_param_t, typename = void,
          typename... Args>
struct is_constructable_impl : std::false_type {};
template <typename T, typename construct_param_t, typename... Args>
struct is_constructable_impl<
    T, construct_param_t,
    std::void_t<decltype(T{{Args{}}..., {construct_param_t{}}})>, Args...>
    : std::true_type {};

template <typename T, typename construct_param_t, typename... Args>
constexpr bool is_constructable =
    is_constructable_impl<T, construct_param_t, void, Args...>::value;

template <typename T, typename... Args,
          std::enable_if_t<is_constructable<T, UniversalVectorType, Args...>,
                           int> = 0>
constexpr std::size_t members_count_impl() {
  return members_count_impl<T, Args..., UniversalVectorType>();
}

template <
    typename T, typename... Args,
    std::enable_if_t<is_constructable<T, UniversalType, Args...>, int> = 0>
constexpr std::size_t members_count_impl() {
  return members_count_impl<T, Args..., UniversalType>();
}

template <typename T, typename... Args,
          std::enable_if_t<is_constructable<T, UniversalOptionalType, Args...>,
                           int> = 0>
constexpr std::size_t members_count_impl() {
  return members_count_impl<T, Args..., UniversalOptionalType>();
}

template <typename T, typename... Args,
          std::enable_if_t<is_constructable<T, UniversalIntegralType, Args...>,
                           int> = 0>
constexpr std::size_t members_count_impl() {
  return members_count_impl<T, Args..., UniversalIntegralType>();
}

template <typename T, typename... Args,
          std::enable_if_t<is_constructable<T, UniversalNullptrType, Args...>,
                           int> = 0>
constexpr std::size_t members_count_impl() {
  return members_count_impl<T, Args..., UniversalNullptrType>();
}

template <
    typename T, typename... Args,
    std::enable_if_t<!(is_constructable<T, UniversalVectorType, Args...> &&
                       is_constructable<T, UniversalType, Args...> &&
                       is_constructable<T, UniversalOptionalType, Args...> &&
                       is_constructable<T, UniversalIntegralType, Args...> &&
                       is_constructable<T, UniversalNullptrType, Args...> &&
                       is_constructable<T, UniversalNullptrType, Args...>),
                     int> = 0>
constexpr std::size_t members_count_impl() {
  return sizeof...(Args);
}

template <typename T, typename type = remove_cvref_t<T>,
          std::enable_if_t<tuple_size<type>, int> = 0>
constexpr std::size_t members_count() {
  return std::tuple_size<type>::value;
}
template <typename T, typename type = remove_cvref_t<T>,
          std::enable_if_t<!tuple_size<type>, int> = 0>
constexpr std::size_t members_count() {
  return members_count_impl<type>();
}

// visit members
constexpr static auto MaxVisitMembers = 64;

template <std::size_t Count, typename Object, typename Visitor>
constexpr decltype(auto) inline visit_members_impl(
    Object &&object, Visitor &&visitor, std::enable_if_t<Count == 0, int> = 0) {
  using type = remove_cvref_t<decltype(object)>;
  if (Count == 0 && user_struct<type>) {
    static_assert(!sizeof(type),
                  "1. If the struct is empty, which is not allowed in "
                  "serialize type system.\n"
                  "2. If the strut is not empty, it means can't "
                  "calculate struct members' count. ");
  }
  static_assert(Count <= MaxVisitMembers, "exceed max visit members");
  return visitor();
}
template <std::size_t Count, typename Object, typename Visitor>
constexpr decltype(auto) inline visit_members_impl(
    Object &&object, Visitor &&visitor, std::enable_if_t<Count == 1, int> = 0) {
  return visitor(STRUCT_PACK_GET_0(object));
}
template <std::size_t Count, typename Object, typename Visitor>
constexpr decltype(auto) inline visit_members_impl(
    Object &&object, Visitor &&visitor, std::enable_if_t<Count == 2, int> = 0) {
  return visitor(STRUCT_PACK_GET_0(object), STRUCT_PACK_GET_1(object));
}
template <std::size_t Count, typename Object, typename Visitor>
constexpr decltype(auto) inline visit_members_impl(
    Object &&object, Visitor &&visitor, std::enable_if_t<Count == 3, int> = 0) {
  return visitor(STRUCT_PACK_GET_0(object), STRUCT_PACK_GET_1(object),
                 STRUCT_PACK_GET_2(object));
}

template <typename Object, typename Visitor>
constexpr decltype(auto) visit_members(Object &&object, Visitor &&visitor) {
  constexpr auto Count = decltype(STRUCT_PACK_FIELD_COUNT(object))::value;
  return visit_members_impl<Count>(object, visitor);
}

// template <typename Object, typename Visitor,
//           std::enable_if_t<!user_defined_refl<remove_cvref_t<Object>>, int> =
//           0>
// constexpr decltype(auto) visit_members(Object &&, Visitor &&) {
//   static_assert(user_struct<remove_cvref_t<Object>>,
//                 "user defined struct but not defined refl macro");
//   return;
// }

// get_types
template <
    typename U, typename T = remove_cvref_t<U>,
    std::enable_if_t<(std::is_fundamental<T>::value || std::is_enum<T>::value ||
                      string<T> || container<T> || optional<T> || expected<T> ||
                      array<T> || c_array<T> || bitset<T>
#if (__GNUC__ || __clang__)
                      || std::is_same<__int128, T>::value ||
                      std::is_same<unsigned __int128, T>::value
#endif
                      ) &&
                         !tuple<T>,
                     int> = 0>
constexpr auto get_types() {
  return declval<std::tuple<T>>();
}

template <
    typename U, typename T = remove_cvref_t<U>,
    std::enable_if_t<(tuple<T> || is_trivial_tuple<T>)&&!pair<T>, int> = 0>
constexpr auto get_types() {
  return declval<T>();
}

template <typename U, typename T = remove_cvref_t<U>,
          std::enable_if_t<pair<T>, int> = 0>
constexpr auto get_types() {
  return declval<std::tuple<typename T::first_type, typename T::second_type>>();
}

template <typename U, typename T = remove_cvref_t<U>,
          std::enable_if_t<std::is_class<T>::value && !container<T> &&
                               !string<T> && !tuple<T>,
                           int> = 0>
constexpr auto get_types() {
  return visit_members(declval<T>(), [](auto &&...args) {
    return declval<std::tuple<remove_cvref_t<decltype(args)>...>>();
  });
}

template <typename Ty>
struct is_trivial_serializable {
 private:
  template <typename U, std::size_t... I>
  static constexpr bool class_visit_helper(std::index_sequence<I...>) {
    bool ret = true;
    static_cast<void>(std::initializer_list<int>{
        (ret &= is_trivial_serializable<std::tuple_element_t<I, U>>::value,
         0)...});
    return ret;
  }

  template <typename T = Ty,
            std::enable_if_t<std::is_abstract<T>::value, int> = 0>
  static constexpr bool solve() {
    return false;
  }
  template <typename T = Ty, std::enable_if_t<is_trivial_view_v<T>, int> = 0>
  static constexpr bool solve() {
    return true;
  }
  template <typename T = Ty,
            std::enable_if_t<std::is_enum<T>::value ||
                                 std::is_fundamental<T>::value || bitset<T>,
                             int> = 0>
  static constexpr bool solve() {
    return true;
  }

  template <typename T = Ty, std::enable_if_t<array<T>, int> = 0>
  static constexpr bool solve() {
    return is_trivial_serializable<typename T::value_type>::value;
  }
  template <typename T = Ty, std::enable_if_t<c_array<T>, int> = 0>
  static constexpr bool solve() {
    return is_trivial_serializable<
        typename std::remove_all_extents<T>::type>::value;
  }
  template <
      typename T = Ty,
      std::enable_if_t<
          !pair<T> && tuple<T> && !is_trivial_tuple<T> && !array<T>, int> = 0>
  static constexpr bool solve() {
    return false;
  }

  // map set string vector list deque queue optional expected
  template <typename T = Ty,
            std::enable_if_t<(container<T> && !array<T>) || optional<T> ||
                                 expected<T> || container_adapter<T>,
                             int> = 0>
  static constexpr bool solve() {
    return false;
  }

  // pair
  template <typename T = Ty, std::enable_if_t<pair<T>, int> = 0>
  static constexpr bool solve() {
    return is_trivial_serializable<typename T::first_type>::value &&
           is_trivial_serializable<typename T::second_type>::value;
  }

  // tuple
  template <typename T = Ty, std::enable_if_t<is_trivial_tuple<T>, int> = 0>
  static constexpr bool solve() {
    return class_visit_helper<T>(
        std::make_index_sequence<std::tuple_size<T>::value>{});
  }

  // user defined struct
  template <typename T = Ty, std::enable_if_t<user_struct<T>, int> = 0>
  static constexpr bool solve() {
    return false;
  }

 public:
  static constexpr bool value = is_trivial_serializable::solve();
};

template <typename T, typename = void>
struct trivially_copyable_container_impl : std::false_type {};

template <typename T>
struct trivially_copyable_container_impl<
    T, std::void_t<std::enable_if_t<
           is_trivial_serializable<typename T::value_type>::value>>>
    : std::true_type {};

template <typename Type>
constexpr bool trivially_copyable_container =
    continuous_container<Type> &&trivially_copyable_container_impl<Type>::value;

template <typename T>
constexpr bool serialize_byte =
    std::is_same<char, T>::value || std::is_same<unsigned char, T>::value ||
    std::is_same<signed char, T>::value;

template <typename T>
constexpr bool serialize_buffer =
    trivially_copyable_container<T> &&serialize_byte<typename T::value_type>;

}  // namespace detail
}  // namespace serialize

// clang-format off

// template<std::size_t I,std::size_t Idx,typename Type,std::enable_if_t<I==Idx, int> = 0>
// inline decltype(auto) return_element(Type& c) {
//   return c.X;
// }

// template<std::size_t I,std::size_t Idx,typename Type,std::enable_if_t<!(I==Idx), int> = 0>
// inline decltype(auto) return_element(Type& c) {
//   static_assert(true,"");
// }

// template<std::size_t I,std::enable_if_t<I==0, int> = 0> auto& STRUCT_PACK_GET(Type& c) {
//   return c.X;
// }


// // #define STRUCT_PACK_RETURN_ELEMENT(Idx, X)

#define Func(Idx, Type, X) \
template<std::size_t I, std::enable_if_t<I==Idx, int> = 0> \
constexpr auto& FuncImpl(Type& c){return c.X;} \

#define Func_CONST(Idx, Type, X) \
template<std::size_t I, std::enable_if_t<I==Idx, int> = 0> \
constexpr const auto& FuncImpl(const Type& c){return c.X;} \

#define STRUCT_PACK_GET_INDEX(Idx, Type,X) \
inline auto& STRUCT_PACK_GET_##Idx(Type& c) {\
    return STRUCT_PACK_GET<STRUCT_PACK_FIELD_COUNT_IMPL<Type>()-1-Idx,Idx>(c);\
}\

#define STRUCT_PACK_GET_INDEX_CONST(Idx, Type,X) \
inline const auto& STRUCT_PACK_GET_##Idx(const Type& c) {\
    return STRUCT_PACK_GET<STRUCT_PACK_FIELD_COUNT_IMPL<Type>()-1-Idx,Idx>(c);\
}\

#define STRUCT_PACK_REFL(Type,...) \
inline Type& STRUCT_PACK_REFL_FLAG(Type& t) {return t;} \
template<typename T> \
constexpr std::size_t STRUCT_PACK_FIELD_COUNT_IMPL(); \
template<> \
constexpr std::size_t STRUCT_PACK_FIELD_COUNT_IMPL<Type>() {return STRUCT_PACK_ARG_COUNT(__VA_ARGS__);} \
inline decltype(auto) STRUCT_PACK_FIELD_COUNT(const Type &){ \
  return std::integral_constant<std::size_t,STRUCT_PACK_ARG_COUNT(__VA_ARGS__)>{}; \
} \
STRUCT_PACK_EXPAND_EACH(,Func, Type,__VA_ARGS__) \
STRUCT_PACK_EXPAND_EACH(,Func_CONST,Type, __VA_ARGS__) \
template<std::size_t I, std::size_t Idx> auto& STRUCT_PACK_GET(Type& c) { \
  return FuncImpl<I>(c);\
} \
template<std::size_t I, std::size_t Idx> const auto& STRUCT_PACK_GET(const Type& c) { \
  return FuncImpl<I>(c);\
} \
STRUCT_PACK_EXPAND_EACH(,STRUCT_PACK_GET_INDEX,Type,__VA_ARGS__) \
STRUCT_PACK_EXPAND_EACH(,STRUCT_PACK_GET_INDEX_CONST,Type,__VA_ARGS__) \

#define STRUCT_PACK_FRIEND_DECL(Type) \
template <std::size_t I> \
friend auto& STRUCT_PACK_GET(Type& c); \
template <std::size_t I> \
friend const auto& STRUCT_PACK_GET(const Type& c);


