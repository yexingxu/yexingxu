/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-11-06 14:12:47
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-11-29 01:44:26
 */

#include <gtest/gtest.h>

#include <iostream>
#include <type_traits>

#include "serialize/serializer.hpp"
#include "shared_ptr.h"
// #include "template/type_id_test.h"

// struct A {
//   int a;
//   double b;
//   std::string c;
// };

// TEST(SharedPtrTest, OperatorTest) {
//   util::SharedPtr<A> test_a(new A{2, 3.0, "cc"});
//   EXPECT_EQ(test_a->a, 2);
//   EXPECT_EQ(test_a->b, 3.0);
//   EXPECT_EQ(test_a->c, "cc");
//   test_a->a = 1;
//   test_a->b = 2.0;
//   test_a->c = "ss";
//   EXPECT_EQ(test_a.use_count(), 1);
//   auto test_b = test_a;
//   EXPECT_EQ(test_a.use_count(), 2);
//   EXPECT_EQ(test_b.use_count(), 2);
//   EXPECT_EQ(test_b->a, 1);
//   EXPECT_EQ(test_b->b, 2.0);
//   EXPECT_EQ(test_b->c, "ss");
// }

int main() {
  std::cout << "test start..." << std::endl;
  testing::InitGoogleTest();

  // 1行代码序列化
  std::vector<char> result = serialize::details::Serializer(person1);

  return RUN_ALL_TESTS();
}
