/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-11-07 11:43:16
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-11-10 11:30:07
 */

#ifndef SPSC_QUEUE_H_
#define SPSC_QUEUE_H_

#include <atomic>
#include <cstddef>
#include <memory>
namespace util {

#ifdef __cpp_lib_hardware_interference_size
using std::hardware_constructive_interference_size;
using std::hardware_destructive_interference_size;
#else
static constexpr std::size_t hardware_destructive_interference_size = 64;
static constexpr std::size_t hardware_constructive_interference_size = 64;
#endif

template <class T, size_t N, typename Allocator = std::allocator<T> >
class SPSCQueue {
  using AtomicIndex = std::atomic<unsigned int>;

 public:
 private:
  const T* record_;
  const size_t capacity_;
  alignas(hardware_constructive_interference_size) AtomicIndex readIndex_;
  alignas(hardware_constructive_interference_size) AtomicIndex WriteIndex_;
  Allocator allocator_;
};
}  // namespace util

#endif
