/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-11-21 09:53:04
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-11-28 22:53:12
 */

#include "conditional.h"
// #include "if_constexpr.hpp"
#include <utility>

#include "template-template-parameter.h"
enum { value = 1 };

int main() {
  template_template_parameter_test();
  conditional_test();

  // int a = 10;
  // decrement_kindof<int>(a);
  // std::cout << a << std::endl;
  // switch_test<int>();
  return 0;
}
