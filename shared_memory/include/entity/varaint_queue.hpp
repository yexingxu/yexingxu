#pragma once

#include <cstdint>

#include "entity/fifo.hpp"
#include "entity/resizeable_lockfree_queue.hpp"
#include "entity/sofi.hpp"
#include "shm/algorithm.hpp"
#include "types/optional.hpp"
#include "types/variant.hpp"
namespace shm {
namespace entity {

/// @brief list of the supported underlying queue types
/// @note  if a new queue type is added the following steps have to be
///         performed:
///         1. add queue type here
///         2. add queue type in m_fifo data member variant type
///         3. increase numberOfQueueTypes in test_cxx_variant_queue test
enum class VariantQueueTypes : uint64_t {
  FiFo_SingleProducerSingleConsumer = 0,
  SoFi_SingleProducerSingleConsumer = 1,
  FiFo_MultiProducerSingleConsumer = 2,
  SoFi_MultiProducerSingleConsumer = 3
};

// remark: we need to consider to support the non-resizable queue as well
//         since it should have performance benefits if resize is not actually
//         needed for now we just use the most general variant, which allows
//         resizing

/// @brief wrapper of multiple fifo's
/// @param[in] ValueType type which should be stored
/// @param[in] Capacity capacity of the underlying fifo
/// @code
///     cxx::VariantQueue<int, 5>
///     nonOverflowingQueue(cxx::VariantQueueTypes::FiFo_SingleProducerSingleConsumer);
///     cxx::VariantQueue<int, 5>
///     overflowingQueue(cxx::VariantQueueTypes::SoFi_SingleProducerSingleConsumer);
///
///     // overflow case
///     auto status = nonOverflowingQueue.push(123);
///     if ( !status ) {
///         IOX_LOG(INFO, "queue is full");
///     }
///
///     auto overriddenElement = overflowingQueue.push(123);
///     if ( overriddenElement->has_value() ) {
///         IOX_LOG(INFO, "element " << overriddenElement->value() << " was
///         overridden");
///     }
/// @endcode
template <typename ValueType, uint64_t Capacity>
class VariantQueue {
 public:
  using fifo_t =
      mpark::variant<FiFo<ValueType, Capacity>, SoFi<ValueType, Capacity>,
                     ResizeableLockFreeQueue<ValueType, Capacity>,
                     ResizeableLockFreeQueue<ValueType, Capacity>>;

  /// @brief Constructor of a VariantQueue
  /// @param[in] type type of the underlying queue
  explicit VariantQueue(const VariantQueueTypes type) noexcept;

  /// @brief pushs an element into the fifo
  /// @param[in] value value which should be added in the fifo
  /// @return if the underlying queue has an overflow the optional will contain
  ///         the value which was overridden (SOFI) or which was dropped (FIFO)
  ///         otherwise the optional contains nullopt_t
  tl::optional<ValueType> push(const ValueType& value) noexcept;

  /// @brief pops an element from the fifo
  /// @return if the fifo did contain an element it is returned inside the
  /// optional
  ///         otherwise the optional contains nullopt_t
  tl::optional<ValueType> pop() noexcept;

  /// @brief returns true if empty otherwise true
  bool empty() const noexcept;

  /// @brief get the current size of the queue. Caution, another thread can have
  /// changed the size just after reading it
  /// @return queue size
  uint64_t size() noexcept;

  /// @brief set the capacity of the queue
  /// @param[in] newCapacity valid values are 0 < newCapacity <
  /// MAX_SUBSCRIBER_QUEUE_CAPACITY
  /// @return true if setting the new capacity succeeded, false otherwise
  /// @pre it is important that no pop or push calls occur during
  ///         this call
  /// @note depending on the internal queue used, concurrent pushes and pops are
  /// possible
  ///       (for FiFo_MultiProducerSingleConsumer and
  ///       SoFi_MultiProducerSingleConsumer)
  /// @concurrent not thread safe
  bool setCapacity(const uint64_t newCapacity) noexcept;

  /// @brief get the capacity of the queue.
  /// @return queue size
  uint64_t capacity() const noexcept;

 private:
  const VariantQueueTypes m_type;
  fifo_t m_fifo;
};

template <typename ValueType, uint64_t Capacity>
inline VariantQueue<ValueType, Capacity>::VariantQueue(
    const VariantQueueTypes type) noexcept
    : m_type(type) {
  switch (m_type) {
    case VariantQueueTypes::FiFo_SingleProducerSingleConsumer: {
      m_fifo.template emplace<FiFo<ValueType, Capacity>>();
      break;
    }
    case VariantQueueTypes::SoFi_SingleProducerSingleConsumer: {
      m_fifo.template emplace<SoFi<ValueType, Capacity>>();
      break;
    }
    case VariantQueueTypes::FiFo_MultiProducerSingleConsumer:
      [[fallthrough]];
    case VariantQueueTypes::SoFi_MultiProducerSingleConsumer: {
      m_fifo.template emplace<ResizeableLockFreeQueue<ValueType, Capacity>>();
      break;
    }
  }
}

template <typename ValueType, uint64_t Capacity>
tl::optional<ValueType> VariantQueue<ValueType, Capacity>::push(
    const ValueType& value) noexcept {
  switch (m_type) {
    case VariantQueueTypes::FiFo_SingleProducerSingleConsumer: {
      // SAFETY: 'm_type' ist 'const' and does not change after construction
      auto& queue = mpark::get<static_cast<uint64_t>(
          VariantQueueTypes::FiFo_SingleProducerSingleConsumer)>(m_fifo);
      auto hadSpace = queue.push(value);

      return (hadSpace) ? tl::nullopt : tl::make_optional<ValueType>(value);
    }
    case VariantQueueTypes::SoFi_SingleProducerSingleConsumer: {
      ValueType overriddenValue;
      // SAFETY: 'm_type' ist 'const' and does not change after construction
      auto& queue = mpark::get<static_cast<uint64_t>(
          VariantQueueTypes::SoFi_SingleProducerSingleConsumer)>(m_fifo);
      auto hadSpace = queue.push(value, overriddenValue);

      return (hadSpace) ? tl::nullopt
                        : tl::make_optional<ValueType>(overriddenValue);
    }
    case VariantQueueTypes::FiFo_MultiProducerSingleConsumer: {
      // SAFETY: 'm_type' ist 'const' and does not change after construction
      auto& queue = mpark::get<static_cast<uint64_t>(
          VariantQueueTypes::FiFo_MultiProducerSingleConsumer)>(m_fifo);
      auto hadSpace = queue.tryPush(value);

      return (hadSpace) ? tl::nullopt : tl::make_optional<ValueType>(value);
    }
    case VariantQueueTypes::SoFi_MultiProducerSingleConsumer: {
      // SAFETY: 'm_type' ist 'const' and does not change after construction
      auto& queue = mpark::get<static_cast<uint64_t>(
          VariantQueueTypes::FiFo_MultiProducerSingleConsumer)>(m_fifo);
      return queue.push(value);
    }
    default:
      return tl::nullopt;
  }

  return tl::nullopt;
}

template <typename ValueType, uint64_t Capacity>
inline tl::optional<ValueType>
VariantQueue<ValueType, Capacity>::pop() noexcept {
  switch (m_type) {
    case VariantQueueTypes::FiFo_SingleProducerSingleConsumer: {
      // SAFETY: 'm_type' ist 'const' and does not change after construction
      auto& queue = mpark::get<static_cast<uint64_t>(
          VariantQueueTypes::FiFo_SingleProducerSingleConsumer)>(m_fifo);
      return queue.pop();
    }
    case VariantQueueTypes::SoFi_SingleProducerSingleConsumer: {
      ValueType returnType;
      // SAFETY: 'm_type' ist 'const' and does not change after construction
      auto& queue = mpark::get<static_cast<uint64_t>(
          VariantQueueTypes::SoFi_SingleProducerSingleConsumer)>(m_fifo);
      auto hasReturnType = queue.pop(returnType);

      return (hasReturnType) ? tl::make_optional<ValueType>(returnType)
                             : tl::nullopt;
    }
    case VariantQueueTypes::FiFo_MultiProducerSingleConsumer:
    case VariantQueueTypes::SoFi_MultiProducerSingleConsumer: {
      // SAFETY: 'm_type' ist 'const' and does not change after construction
      auto& queue = mpark::get<static_cast<uint64_t>(
          VariantQueueTypes::FiFo_MultiProducerSingleConsumer)>(m_fifo);
      return queue.pop();
    }
  }

  return tl::nullopt;
}

template <typename ValueType, uint64_t Capacity>
inline bool VariantQueue<ValueType, Capacity>::empty() const noexcept {
  switch (m_type) {
    case VariantQueueTypes::FiFo_SingleProducerSingleConsumer: {
      // SAFETY: 'm_type' ist 'const' and does not change after construction
      auto& queue = mpark::get<static_cast<uint64_t>(
          VariantQueueTypes::FiFo_SingleProducerSingleConsumer)>(m_fifo);
      return queue.empty();
    }
    case VariantQueueTypes::SoFi_SingleProducerSingleConsumer: {
      // SAFETY: 'm_type' ist 'const' and does not change after construction
      auto& queue = mpark::get<static_cast<uint64_t>(
          VariantQueueTypes::SoFi_SingleProducerSingleConsumer)>(m_fifo);
      return queue.empty();
    }
    case VariantQueueTypes::FiFo_MultiProducerSingleConsumer:
    case VariantQueueTypes::SoFi_MultiProducerSingleConsumer: {
      // SAFETY: 'm_type' ist 'const' and does not change after construction
      auto& queue = mpark::get<static_cast<uint64_t>(
          VariantQueueTypes::FiFo_MultiProducerSingleConsumer)>(m_fifo);
      return queue.empty();
    }
  }

  return true;
}

template <typename ValueType, uint64_t Capacity>
inline uint64_t VariantQueue<ValueType, Capacity>::size() noexcept {
  switch (m_type) {
    case VariantQueueTypes::FiFo_SingleProducerSingleConsumer: {
      // SAFETY: 'm_type' ist 'const' and does not change after construction
      auto& queue = mpark::get<static_cast<uint64_t>(
          VariantQueueTypes::FiFo_SingleProducerSingleConsumer)>(m_fifo);
      return queue.size();
    }
    case VariantQueueTypes::SoFi_SingleProducerSingleConsumer: {
      // SAFETY: 'm_type' ist 'const' and does not change after construction
      auto& queue = mpark::get<static_cast<uint64_t>(
          VariantQueueTypes::SoFi_SingleProducerSingleConsumer)>(m_fifo);
      return queue.size();
    }
    case VariantQueueTypes::FiFo_MultiProducerSingleConsumer:
    case VariantQueueTypes::SoFi_MultiProducerSingleConsumer: {
      // SAFETY: 'm_type' ist 'const' and does not change after construction
      auto& queue = mpark::get<static_cast<uint64_t>(
          VariantQueueTypes::FiFo_MultiProducerSingleConsumer)>(m_fifo);
      return queue.size();
    }
  }

  return 0U;
}

template <typename ValueType, uint64_t Capacity>
inline bool VariantQueue<ValueType, Capacity>::setCapacity(
    const uint64_t newCapacity) noexcept {
  switch (m_type) {
    case VariantQueueTypes::FiFo_SingleProducerSingleConsumer: {
      /// @todo iox-#1147 must be implemented for FiFo
      EXPECTS(false);
      return false;
    }
    case VariantQueueTypes::SoFi_SingleProducerSingleConsumer: {
      // SAFETY: 'm_type' ist 'const' and does not change after construction
      auto& queue = mpark::get<static_cast<uint64_t>(
          VariantQueueTypes::SoFi_SingleProducerSingleConsumer)>(m_fifo);
      queue.setCapacity(newCapacity);
      return true;
    }
    case VariantQueueTypes::FiFo_MultiProducerSingleConsumer:
    case VariantQueueTypes::SoFi_MultiProducerSingleConsumer: {
      // SAFETY: 'm_type' ist 'const' and does not change after construction
      auto& queue = mpark::get<static_cast<uint64_t>(
          VariantQueueTypes::FiFo_MultiProducerSingleConsumer)>(m_fifo);
      // we may discard elements in the queue if the size is reduced and the
      // fifo contains too many elements
      return queue.setCapacity(newCapacity);
    }
  }
  return false;
}

template <typename ValueType, uint64_t Capacity>
inline uint64_t VariantQueue<ValueType, Capacity>::capacity() const noexcept {
  switch (m_type) {
    case VariantQueueTypes::FiFo_SingleProducerSingleConsumer: {
      // SAFETY: 'm_type' ist 'const' and does not change after construction
      auto& queue = mpark::get<static_cast<uint64_t>(
          VariantQueueTypes::FiFo_SingleProducerSingleConsumer)>(m_fifo);
      return queue.capacity();
    }
    case VariantQueueTypes::SoFi_SingleProducerSingleConsumer: {
      // SAFETY: 'm_type' ist 'const' and does not change after construction
      auto& queue = mpark::get<static_cast<uint64_t>(
          VariantQueueTypes::SoFi_SingleProducerSingleConsumer)>(m_fifo);
      return queue.capacity();
    }
    case VariantQueueTypes::FiFo_MultiProducerSingleConsumer:
    case VariantQueueTypes::SoFi_MultiProducerSingleConsumer: {
      // SAFETY: 'm_type' ist 'const' and does not change after construction
      auto& queue = mpark::get<static_cast<uint64_t>(
          VariantQueueTypes::FiFo_MultiProducerSingleConsumer)>(m_fifo);
      return queue.capacity();
    }
  }

  return 0U;
}

}  // namespace entity
}  // namespace shm