/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-12-08 05:19:20
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-12-08 05:22:05
 */

#ifndef SERIALIZE_TRIVIAL_VIEW_HPP_
#define SERIALIZE_TRIVIAL_VIEW_HPP_

namespace serialize {
template <typename T>
struct trivial_view {
 private:
  const T* ref;

 public:
  trivial_view(const T* t) : ref(t){};
  trivial_view(const T& t) : ref(&t){};
  trivial_view(const trivial_view&) = default;
  trivial_view(trivial_view&&) = default;
  trivial_view() : ref(nullptr){};

  trivial_view& operator=(const trivial_view&) = default;
  trivial_view& operator=(trivial_view&&) = default;

  using value_type = T;

  const T& get() const {
    assert(ref != nullptr);
    return *ref;
  }
  const T* operator->() const {
    assert(ref != nullptr);
    return ref;
  }
};
}  // namespace serialize

#endif