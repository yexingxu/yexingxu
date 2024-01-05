/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-12-28 00:06:13
 * @LastEditors: chen, hua
 * @LastEditTime: 2024-01-05 05:25:20
 */

#pragma once

#include <cstdint>
namespace serialize {

enum ser_config {
  DEFAULT = 0,
  DISABLE_TYPE_INFO = 0b1,
  ENABLE_TYPE_INFO = 0b10,
  DISABLE_ALL_META_INFO = 0b11
};

enum class alignment : std::uint8_t {
  kAlignment2 = 2,
  kAlignment4 = 4,
  kAlignment8 = 8,
  kAlignment16 = 16,
};

enum class byte_order : std::uint8_t {
  kBigEndian = 0,
  kLittleEndian = 1,
};

enum class length_field_size : std::uint8_t {
  k1 = 1,
  k2 = 2,
  k4 = 4,
  k8 = 8,
  k16 = 16,
};

struct serialize_props {
  static constexpr alignment kAlignment = alignment::kAlignment4;
  static constexpr byte_order kByteOrder = byte_order::kLittleEndian;
  static constexpr std::uint8_t kStringLengthField = 2u;
  static constexpr std::uint8_t kArrayLengthField = 4u;
  static constexpr std::uint8_t kUnionLengthField = 1u;
  static constexpr std::uint8_t kStructLengthField = 8u;
};

}  // namespace serialize
