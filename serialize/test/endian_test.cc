/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2024-01-05 16:27:42
 * @LastEditors: chen, hua
 * @LastEditTime: 2024-01-05 19:37:55
 */

#include <gtest/gtest.h>

#include <cstdint>

#include "detail/endian_wrapper.hpp"
#include "detail/memory_writer.hpp"
#include "serialize.hpp"
#include "user_defined_struct.h"

struct serialize_props_little {
  using LengthFieldType = std::uint8_t;
  using AlignmentType = serialize::alignment;
  using ByteOrderType = serialize::byte_order;
  static constexpr AlignmentType kAlignment = serialize::alignment::kAlignment4;
  static constexpr ByteOrderType kByteOrder =
      serialize::byte_order::kLittleEndian;
  static constexpr LengthFieldType kStringLengthField = 2u;
  static constexpr LengthFieldType kArrayLengthField = 4u;
  static constexpr LengthFieldType kUnionLengthField = 1u;
  static constexpr LengthFieldType kStructLengthField = 8u;
};

struct serialize_props_big {
  using LengthFieldType = std::uint8_t;
  using AlignmentType = serialize::alignment;
  using ByteOrderType = serialize::byte_order;
  static constexpr AlignmentType kAlignment = serialize::alignment::kAlignment4;
  static constexpr ByteOrderType kByteOrder = serialize::byte_order::kBigEndian;
  static constexpr LengthFieldType kStringLengthField = 2u;
  static constexpr LengthFieldType kArrayLengthField = 4u;
  static constexpr LengthFieldType kUnionLengthField = 1u;
  static constexpr LengthFieldType kStructLengthField = 8u;
};

TEST(SerializerTest, EndianTest) {
  using namespace serialize::detail;
  std::cout << std::hex << '\n';
  std::vector<char> writer_little;
  uint32_t b = 0x12345678;
  writer_little.resize(sizeof(b));
  auto real_writer_little = memory_writer{(char *)writer_little.data()};
  write_wrapper<true, sizeof(b)>(real_writer_little, (char *)&b);

  auto buffer1 = serialize::serialize<serialize_props_little>(b);
  EXPECT_EQ(writer_little, buffer1);

  // writer_little.clear();
  person3 p3{{2, 2.2, 2}, {1, 1.2, {1, 2}}};
  auto buffer2 = serialize::serialize<serialize_props_little>(p3);
  auto des_res =
      serialize::deserialize<serialize_props_little, person3>(buffer2);
  if (des_res.has_value()) {
    EXPECT_EQ(p3, des_res.value());
  } else {
    std::cout << "error " << __LINE__ << std::endl;
  }

  auto buffer3 = serialize::serialize<serialize_props_big>(p3);
  auto des_res1 = serialize::deserialize<serialize_props_big, person3>(buffer3);
  if (des_res1.has_value()) {
    EXPECT_EQ(p3, des_res1.value());
  } else {
    std::cout << "error " << __LINE__ << std::endl;
  }
  // EXPECT_EQ(buffer2, buffer3);
};