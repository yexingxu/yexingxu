/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-11-21 09:09:23
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-11-21 09:57:13
 */
#include <iostream>

template <typename T>
struct MyTemplate {
  T value;

  MyTemplate(T val) : value(val) {}

  void print() { std::cout << value << std::endl; }
};

// 使用模板模板参数
template <template <typename> class Container>
void printValue(Container<int>& container) {
  container.print();
}

inline void template_template_parameter_test() {
  MyTemplate<int> myObject(42);

  printValue(myObject);

  return;
}