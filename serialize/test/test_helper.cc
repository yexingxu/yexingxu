/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-11-06 14:12:47
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-12-27 22:54:46
 */

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <string>
#include <type_traits>
#include <vector>

int main() {
  std::cout << "test start..." << std::endl;
  testing::InitGoogleTest();

  return RUN_ALL_TESTS();
  // return 0;
}
