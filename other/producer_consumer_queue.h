/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-11-07 13:34:41
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-11-08 11:17:04
 */

#ifndef PRODUCER_CONSUMER_QUEUE_H_
#define PRODUCER_CONSUMER_QUEUE_H_

#include <atomic>
#include <cassert>
#include <cstdint>
#include <memory>
#include <new>
#include <type_traits>

namespace util {

template <class T>
class ProducerConsumerQueue {
#ifdef __cpp_lib_hardware_interference_size
  using std::hardware_constructive_interference_size;
  using std::hardware_destructive_interference_size;
#else
  // 64 bytes on x86-64 │ L1_CACHE_BYTES │ L1_CACHE_SHIFT │ __cacheline_aligned
  // │ ...
  static constexpr std::size_t hardware_constructive_interference_size = 64;
  static constexpr std::size_t hardware_destructive_interference_size = 64;
#endif
 public:
  using value_type = T;
  //因为这种多核通信组件的拷贝非常难，因为拷贝过程中并不能阻止生产者和消费者去读写。
  ProducerConsumerQueue(const ProducerConsumerQueue&) = delete;
  ProducerConsumerQueue& operator=(const ProducerConsumerQueue&) = delete;
  //根据writeIndex_和readIndex_的定义，writeIndex_和readIndex_在非空时不能指向同一个位置，而writeIndex_指向的是下一个可写位置，没有存储元素，所以capacity是size_
  //-
  // 1，这样size_也必须大于等于2，不然capacity是0，没有意义了。初始化的时候就根据T的大小和size_申请了相应大小的内存，但没有初始化。
  // std::malloc 只分配内存而不进行初始化
  explicit ProducerConsumerQueue(std::uint32_t size)
      : size_(size),
        records_(static_cast<T*>(std::malloc(sizeof(T) * size))),
        readIndex_(0),
        writeIndex_(0) {
    assert(size_ >= 2);
    if (!records_) {
      throw std::bad_alloc();
    }
  }
  //无论是堆上存储队列，生产者和消费者各自持有队列的shared_ptr，还是栈上存储队列，常规情况下当队列析构时，生产者和消费者也都肯定已经析构过了，所以析构函数并不需要处理多线程问题。如果T是trivially_destructible的，直接free
  // records_回收这块内存即可，否则还需要对这些对象一个一个地执行析构。records_的用法是环形队列，所以当index碰到边界的时候要拐弯到开头。
  ~ProducerConsumerQueue() {
    if (!std::is_trivially_destructible<T>::value) {
      size_t readIndex = readIndex_;
      size_t endIndex = writeIndex_;
      while (readIndex != endIndex) {
        records_[readIndex].~T();
        if (++readIndex == size_) {
          readIndex = 0;
        }
      }
    }
    std::free(records_);
  }

  template <class... Args>
  bool Write(Args&&... args) {
    auto const currentIndex = writeIndex_.load(std::memory_order_relaxed);
    auto nextRecord = currentIndex + 1;
    if (nextRecord == size_) {
      nextRecord = 0;
    }
    if (nextRecord != readIndex_.load(std::memory_order_acquire)) {
      &records_[currentIndex] = new T(std::forward<Args>(args)...);
      writeIndex_.store(nextRecord, std::memory_order_release);
      return true;
    }
    // is full
    return false;
  }

  bool read(T& record) {
    auto const currentIndex = readIndex_.load(std::memory_order_relaxed);
    if (currentIndex == writeIndex_.load(std::memory_order_acquire)) {
      // is empty
      return false;
    }
    auto nextRecord = currentIndex + 1;
    if (nextRecord == size_) {
      nextRecord = 0;
    }
    record = std::move(records_[currentIndex]);
    records_[currentIndex].~T();
    readIndex_.store(nextRecord, std::memory_order_release);
    return true;
  }

 private:
  // hardware_destructive_interference_size在这里我理解就是CacheLineSize的含义，将多个变量放在一个cache
  // line上会引起false sharing现象，极大降低性能。将他们都对齐到cache
  // line边界，这样每个变量独占一个cache line，就避免了false
  // sharing。records_指向一个数组，size_是这个数组的大小，这个队列的实现是bounded
  // queue，大小就是由size_限制的，不会扩容，多提一句，把无锁队列实现成bounded其实会复杂一些，但是生产环境中为了防止内存泄漏，这个特性又是很重要的。readIndex_指向下一个可读元素的位置，writeIndex_指向下一个可写元素。
  using AtomicIndex = std::atomic<unsigned int>;
  char pad0_[hardware_destructive_interference_size];
  const std::uint32_t size_;
  T* const records_;
  alignas(hardware_destructive_interference_size) AtomicIndex readIndex_;
  alignas(hardware_destructive_interference_size) AtomicIndex writeIndex_;
  char pad1_[hardware_destructive_interference_size - sizeof(AtomicIndex)];
};
}  // namespace util

#endif