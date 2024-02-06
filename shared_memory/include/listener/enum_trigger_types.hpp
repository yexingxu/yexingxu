#pragma once

#include <cstdint>
#include <type_traits>
namespace shm {
namespace details {

/// @brief used as underlying type it identifies an enum as a state based enum
using StateEnumIdentifier = uint64_t;
/// @brief used as underlying type it identifies an enum as an event based enum
using EventEnumIdentifier = int64_t;

/// @brief contains true when T is an event based enum, otherwise false
template <typename T>
constexpr bool IS_EVENT_ENUM = std::is_enum<T>::value&&
    std::is_same<std::underlying_type_t<T>, EventEnumIdentifier>::value;

/// @brief contains true when T is a state based enum, otherwise false
template <typename T>
constexpr bool IS_STATE_ENUM = std::is_enum<T>::value&&
    std::is_same<std::underlying_type_t<T>, StateEnumIdentifier>::value;

}  // namespace details
}  // namespace shm