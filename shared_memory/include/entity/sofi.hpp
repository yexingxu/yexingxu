#pragma once

#include <array>
#include <atomic>
#include <cstdint>
#include <cstring>
#include <exception>
#include <type_traits>
namespace shm {
namespace entity {

/// @brief
/// Thread safe producer and consumer queue with a safe overflowing behavior.
/// SoFi is designed in a FIFO Manner but prevents data loss when pushing into
/// a full SoFi. When SoFi is full and a Sender tries to push, the data at the
/// current read position will be returned. SoFi is a Thread safe without using
/// locks. When the buffer is filled, new data is written starting at the
/// beginning of the buffer and overwriting the old.The SoFi is especially
/// designed to provide fixed capacity storage. When its capacity is exhausted,
/// newly inserted elements will cause elements either at the beginning
/// to be overwritten.The SoFi only allocates memory when
/// created , capacity can be is adjusted explicitly.
///
/// @param[in] ValueType        DataType to be stored, must be trivially
/// copyable
/// @param[in] CapacityValue    Capacity of the SoFi
template <class ValueType, uint64_t CapacityValue>
class SoFi {
  static_assert(std::is_trivially_copyable<ValueType>::value,
                "SoFi can handle only trivially copyable data types");
  /// @brief Check if Atomic integer is lockfree on platform
  /// ATOMIC_INT_LOCK_FREE = 2 - is always lockfree
  /// ATOMIC_INT_LOCK_FREE = 1 - is sometimes lockfree
  /// ATOMIC_INT_LOCK_FREE = 0 - is never lockfree
  static_assert(2 <= ATOMIC_INT_LOCK_FREE,
                "SoFi is not able to run lock free on this data type");

  /// @brief Internal size needs to be bigger than the size desirred by the user
  /// This is because of buffer empty detection and overflow handling
  static constexpr uint32_t INTERNAL_SIZE_ADD_ON = 1;

  /// @brief This is the resulting internal size on creation
  static constexpr uint32_t INTERNAL_SOFI_SIZE =
      CapacityValue + INTERNAL_SIZE_ADD_ON;

 public:
  /// @brief default constructor which constructs an empty sofi
  SoFi() noexcept = default;

  /// @brief pushs an element into sofi. if sofi is full the oldest data will be
  ///         returned and the pushed element is stored in its place instead.
  /// @param[in] valueIn value which should be stored
  /// @param[out] valueOut if sofi is overflowing  the value of the overridden
  /// value
  ///                      is stored here
  /// @concurrent restricted thread safe: single pop, single push no
  ///             push calls from multiple contexts
  /// @return return true if push was sucessfull else false.
  /// @code
  /// (initial situation, SOFI is FULL)
  ///     Start|-----A-------|
  ///                        |-----B-------|
  ///                                      |-----C-------|
  ///                                                    |-----D-------|
  ///
  ///
  /// (calling push with data ’E’)
  ///     Start|-----E-------|
  ///                        |-----A-------|
  ///                                      |-----B-------|
  ///                                                    |-----C-------|
  ///                                     (’D’ is returned as valueOut)
  ///
  /// ###################################################################
  ///
  /// (if SOFI is not FULL , calling push() add new data)
  ///     Start|-------------|
  ///                        |-------------|  ( Initial SOFI )
  ///  (push() Called two times)
  ///
  ///                                      |-------------|
  ///                                      (New Data)
  ///                                                     |-------------|
  ///                                                      (New Data)
  /// @endcode
  bool push(const ValueType& valueIn, ValueType& valueOut) noexcept;

  /// @brief pop the oldest element
  /// @param[out] valueOut storage of the pop'ed value
  /// @concurrent restricted thread safe: single pop, single push no
  ///             pop or popIf calls from multiple contexts
  /// @return false if sofi is empty, otherwise true
  bool pop(ValueType& valueOut) noexcept;

  /// @brief conditional pop call to provide an alternative for a peek
  ///         and pop approach. If the verificator returns true the
  ///         peeked element is returned.
  /// @param[out] valueOut storage of the pop'ed value
  /// @param[in] verificator callable of type bool(const ValueType& peekValue)
  ///             which takes the value which would be pop'ed as argument and
  ///             returns true if it should be pop'ed, otherwise false
  /// @code
  ///     int limit = 7128;
  ///     mysofi.popIf(value, [=](const ValueType & peek)
  ///         {
  ///             return peek < limit; // pop only when peek is smaller than
  ///             limit
  ///         }
  ///     ); // pop's a value only if it is smaller than 9012
  /// @endcode
  /// @concurrent restricted thread safe: single pop, single push no
  ///             pop or popIf calls from multiple contexts
  /// @return false if sofi is empty or when verificator returns false,
  /// otherwise true
  template <typename Verificator_T>
  bool popIf(ValueType& valueOut, const Verificator_T& verificator) noexcept;

  /// @brief returns true if sofi is empty, otherwise false
  /// @note the use of this function is limited in the concurrency case. if you
  ///         call this and in another thread pop is called the result can be
  ///         out of date as soon as you require it
  /// @concurrent unrestricted thread safe
  bool empty() const noexcept;

  /// @brief resizes sofi
  /// @param[in] newSize valid values are 0 < newSize < CapacityValue
  /// @pre it is important that no pop or push calls occur during
  ///         this call
  /// @concurrent not thread safe
  bool setCapacity(const uint64_t newSize) noexcept;

  /// @brief returns the capacity of sofi
  /// @concurrent unrestricted thread safe
  uint64_t capacity() const noexcept;

  /// @brief returns the current size of sofi
  /// @concurrent unrestricted thread safe
  uint64_t size() const noexcept;

 private:
  std::array<ValueType, INTERNAL_SOFI_SIZE> m_data;
  uint64_t m_size = INTERNAL_SOFI_SIZE;

  /// @brief the write/read pointers are "atomic pointers" so that they are not
  /// reordered (read or written too late)
  std::atomic<uint64_t> m_readPosition{0};
  std::atomic<uint64_t> m_writePosition{0};
};

template <class ValueType, uint64_t CapacityValue>
uint64_t SoFi<ValueType, CapacityValue>::capacity() const noexcept {
  return m_size - INTERNAL_SIZE_ADD_ON;
}

template <class ValueType, uint64_t CapacityValue>
uint64_t SoFi<ValueType, CapacityValue>::size() const noexcept {
  uint64_t readPosition{0};
  uint64_t writePosition{0};
  do {
    readPosition = m_readPosition.load(std::memory_order_relaxed);
    writePosition = m_writePosition.load(std::memory_order_relaxed);
  } while (m_writePosition.load(std::memory_order_relaxed) != writePosition ||
           m_readPosition.load(std::memory_order_relaxed) != readPosition);

  return writePosition - readPosition;
}

template <class ValueType, uint64_t CapacityValue>
bool SoFi<ValueType, CapacityValue>::setCapacity(
    const uint64_t newSize) noexcept {
  uint64_t newInternalSize = newSize + INTERNAL_SIZE_ADD_ON;
  if (empty() && (newInternalSize <= INTERNAL_SOFI_SIZE)) {
    m_size = newInternalSize;

    m_readPosition.store(0, std::memory_order_release);
    m_writePosition.store(0, std::memory_order_release);

    return true;
  }

  return false;
}

template <class ValueType, uint64_t CapacityValue>
bool SoFi<ValueType, CapacityValue>::empty() const noexcept {
  uint64_t currentReadPosition{0};
  bool isEmpty{false};

  do {
    /// @todo iox-#1695 read before write since the writer increments the aba
    /// counter!!!
    /// @todo iox-#1695 write doc with example!!!
    currentReadPosition = m_readPosition.load(std::memory_order_acquire);
    uint64_t currentWritePosition =
        m_writePosition.load(std::memory_order_acquire);

    isEmpty = (currentWritePosition == currentReadPosition);
    // we need compare without exchange
  } while (
      !(currentReadPosition == m_readPosition.load(std::memory_order_acquire)));

  return isEmpty;
}

template <class ValueType, uint64_t CapacityValue>
bool SoFi<ValueType, CapacityValue>::pop(ValueType& valueOut) noexcept {
  return popIf(valueOut, [](ValueType) { return true; });
}

template <class ValueType, uint64_t CapacityValue>
template <typename Verificator_T>
inline bool SoFi<ValueType, CapacityValue>::popIf(
    ValueType& valueOut, const Verificator_T& verificator) noexcept {
  uint64_t currentReadPosition = m_readPosition.load(std::memory_order_acquire);
  uint64_t nextReadPosition{0};

  bool popWasSuccessful{true};
  do {
    if (currentReadPosition ==
        m_writePosition.load(std::memory_order_acquire)) {
      nextReadPosition = currentReadPosition;
      popWasSuccessful = false;
    } else {
      // we use memcpy here, since the copy assignment is not thread safe in
      // general (we might have an overflow in the push thread and invalidates
      // the object while the copy is running and therefore works on an invalid
      // object); memcpy is also not thread safe, but we discard the object
      // anyway and read it again if its overwritten in between; this is only
      // relevant for types larger than pointer size assign the user data
      std::memcpy(&valueOut, &m_data[currentReadPosition % m_size],
                  sizeof(ValueType));

      /// @brief first we need to peak valueOut if it is fitting the condition
      /// and then we have to verify
      ///        if valueOut is not am invalid object, this could be the case if
      ///        the read position has changed
      if (m_readPosition.load(std::memory_order_relaxed) ==
              currentReadPosition &&
          !verificator(valueOut)) {
        popWasSuccessful = false;
        nextReadPosition = currentReadPosition;
      } else {
        nextReadPosition = currentReadPosition + 1U;
        popWasSuccessful = true;
      }
    }

    // compare and swap
    // if(m_readPosition == currentReadPosition)
    //     m_readPosition = l_next_aba_read_pos
    // else
    //     currentReadPosition = m_readPosition
    // Assign m_aba_read_p to next readable location
  } while (!m_readPosition.compare_exchange_weak(
      currentReadPosition, nextReadPosition, std::memory_order_acq_rel,
      std::memory_order_acquire));

  return popWasSuccessful;
}

template <class ValueType, uint64_t CapacityValue>
bool SoFi<ValueType, CapacityValue>::push(const ValueType& valueIn,
                                          ValueType& valueOut) noexcept {
  constexpr bool SOFI_OVERFLOW{false};

  uint64_t currentWritePosition =
      m_writePosition.load(std::memory_order_relaxed);
  uint64_t nextWritePosition = currentWritePosition + 1U;

  m_data[currentWritePosition % m_size] = valueIn;
  m_writePosition.store(nextWritePosition, std::memory_order_release);

  uint64_t currentReadPosition = m_readPosition.load(std::memory_order_acquire);

  // check if there is a free position for the next push
  if (nextWritePosition < currentReadPosition + m_size) {
    return !SOFI_OVERFLOW;
  }

  // this is an overflow situation, which means that the next push has no free
  // position, therefore the oldest value needs to be passed back to the caller

  uint64_t nextReadPosition = currentReadPosition + 1U;

  // we need to update the read position
  // a) it works, then we need to pass the overflow value back
  // b) it doesn't work, which means that the pop thread already took the value
  // in the meantime an no further action is required memory order success is
  // memory_order_acq_rel
  //   - this is to prevent the reordering of m_writePosition.store(...) after
  //   the increment of the m_readPosition
  //     - in case of an overflow, this might result in the pop thread getting
  //     one element less than the capacity of
  //       the SoFi if the push thread is suspended in between this two
  //       statements
  //     - it's still possible to get more elements than the capacity, but this
  //     is an inherent issue with concurrent
  //       queues and cannot be prevented since there can always be a push
  //       during a pop operation
  //   - another issue might be that two consecutive pushes (not concurrent)
  //   happen on different CPU cores without
  //     synchronization, then the memory also needs to be synchronized for the
  //     overflow case
  // memory order failure is memory_order_relaxed since there is no further
  // synchronization needed if there is no overflow
  if (m_readPosition.compare_exchange_strong(
          currentReadPosition, nextReadPosition, std::memory_order_acq_rel,
          std::memory_order_relaxed)) {
    std::memcpy(&valueOut, &m_data[currentReadPosition % m_size],
                sizeof(ValueType));
    return SOFI_OVERFLOW;
  }

  return !SOFI_OVERFLOW;
}

}  // namespace entity
}  // namespace shm