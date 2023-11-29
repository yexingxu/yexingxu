/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-11-28 18:30:38
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-11-28 19:10:49
 */
#include "serialize/type_id.hpp"

#include <gtest/gtest.h>

TEST(SerializerTest, TypeIdTest) {
  EXPECT_EQ(serialize::details::get_type_id<int>(),
            serialize::details::type_id::int32_t);
  std::cout << "test start..." << std::endl;
}