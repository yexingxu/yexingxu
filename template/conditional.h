/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-11-21 09:42:34
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-11-21 10:00:22
 */

#include <iostream>
#include <type_traits>
#include <typeinfo>

constexpr int sign = 10;
inline void TestCond() {
  std::conditional<
      (sign > 100), double,
      std::conditional<(sign > 80), float,
                       std::conditional<(sign > 40), int, char>::type>::type>::
      type var;
  std::cout << "cur type is:" << typeid(decltype(var)).name() << std::endl;
}

///
const bool isOK = false;
class A {
 public:
  A() { std::cout << "call A" << std::endl; }
};
class B {
 public:
  B() { std::cout << "call B" << std::endl; }
};
class MyTest : public std::conditional<isOK, A, B>::type {
 public:
  MyTest() { std::cout << "call MyTest！" << std::endl; }
};

inline void conditional_test() {
  typedef std::conditional<true, int, double>::type Type1;
  typedef std::conditional<false, int, double>::type Type2;
  typedef std::conditional<sizeof(int) >= sizeof(double), int, double>::type
      Type3;

  std::cout << typeid(Type1).name() << '\n';
  std::cout << typeid(Type2).name() << '\n';
  std::cout << typeid(Type3).name() << '\n';

  //
  TestCond();
  //
  MyTest mt;
}