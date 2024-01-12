/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2024-01-06 00:28:00
 * @LastEditors: chen, hua
 * @LastEditTime: 2024-01-06 07:22:24
 */

#include "detail/alignment.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <string>

#include "detail/reflection.hpp"
#include "serialize.hpp"
#include "user_defined_struct.h"

namespace {
// constexpr std::size_t kAlignment1 = 1u;
constexpr std::size_t kAlignment2 = 2u;
constexpr std::size_t kAlignment4 = 4u;
constexpr std::size_t kAlignment8 = 8u;
}  // namespace

TEST(SerializerTest, AlignmentTest) {
  using namespace serialize::detail;

  //   members_count<std::string>();
  //   members_count<AA>();

  auto res12 = calculate_padding_size<AA>(kAlignment8);
  for (auto res : res12) {
    std::cout << res << " ";
  }
  std::cout << "size: " << res12.size() << "\n";

  auto res2 = calculate_padding_size<A>(kAlignment2);
  for (auto res : res2) {
    std::cout << res << " ";
  }
  std::cout << "\n";
  auto res4 = calculate_padding_size<A>(kAlignment4);
  for (auto res : res4) {
    std::cout << res << " ";
  }
  std::cout << "\n";
  auto res8 = calculate_padding_size<A>(kAlignment8);
  for (auto res : res8) {
    std::cout << res << " ";
  }
  std::cout << "\n";
}