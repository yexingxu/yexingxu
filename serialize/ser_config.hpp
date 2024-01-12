/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-12-28 00:06:13
 * @LastEditors: chen, hua
 * @LastEditTime: 2024-01-06 06:29:50
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
  kAlignment1 = 1,
  kAlignment2 = 2,
  kAlignment4 = 4,
  kAlignment8 = 8,
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

struct serialize_props_default {
  using LengthFieldType = std::uint8_t;
  using AlignmentType = alignment;
  using ByteOrderType = byte_order;
  static constexpr AlignmentType kAlignment = alignment::kAlignment4;
  static constexpr ByteOrderType kByteOrder = byte_order::kLittleEndian;
  static constexpr LengthFieldType kStringLengthField = 4u;
  static constexpr LengthFieldType kArrayLengthField = 4u;
  static constexpr LengthFieldType kUnionLengthField = 4u;
  static constexpr LengthFieldType kStructLengthField = 0u;
};

}  // namespace serialize
