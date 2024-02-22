

#pragma once

#include <cstdint>
#include <functional>
#include <limits>
#include <mutex>
#include <thread>

#include "listener/conditional_listener.hpp"
#include "listener/conditional_verialble_data.hpp"
#include "listener/enum_trigger_types.hpp"
#include "listener/notifier_callback.hpp"
#include "shm/loffli.hpp"
#include "types/expected.hpp"

namespace shm {
namespace details {

namespace internal {
class Event_t {
 public:
  ~Event_t() noexcept;

  bool isEqualTo(const void* const origin, const uint64_t eventType,
                 const uint64_t eventTypeHash) const noexcept;
  bool reset() noexcept;
  void init(const uint64_t eventId, void* const origin, void* const userType,
            const uint64_t eventType, const uint64_t eventTypeHash,
            internal::GenericCallbackRef_t callback,
            internal::TranslationCallbackRef_t translationCallback,
            const std::function<void(uint64_t)> invalidationCallback) noexcept;
  void executeCallback() noexcept;
  bool isInitialized() const noexcept;

 private:
  static constexpr uint64_t INVALID_ID = std::numeric_limits<uint64_t>::max();

  void* m_origin = nullptr;
  uint64_t m_eventType = INVALID_ID;
  uint64_t m_eventTypeHash = INVALID_ID;

  internal::GenericCallbackPtr_t m_callback = nullptr;
  internal::TranslationCallbackPtr_t m_translationCallback = nullptr;
  void* m_userType = nullptr;

  uint64_t m_eventId = INVALID_ID;
  std::function<void(uint64_t)> m_invalidationCallback = [](auto) {};
};
}  // namespace internal

enum class ListenerError {
  LISTENER_FULL,
  EVENT_ALREADY_ATTACHED,
  EMPTY_EVENT_CALLBACK,
};

/// @brief The Listener is a class which reacts to registered events by
///        executing a corresponding callback concurrently. This is achieved via
///        an encapsulated thread inside this class.
/// @note  The Listener is threadsafe and can be used without any restrictions
/// concurrently.
/// @attention Calling detachEvent for the same event from multiple threads is
/// supported but
///            can cause a race condition if you attach the same event again
///            concurrently from another thread. Example:
///             1. One calls detachEvent [1] from thread A, B and C
///             2. thread B wins and detaches event [1]
///             3. A new thread D spawns and would like to attach event [1]
///             again while thread A and C are
///                 still waiting to detach [1].
///             4. Thread A wins but cannot detach event [1] since it is not
///             attached.
///             5. Thread D wins and attaches event [1].
///             6. Finally thread C can continue and detaches event [1] again.
///
///            If thread D is executed last then the event is attached. So
///            depending on the operating system defined execution order the
///            event is either attached or detached.
///
///            Best practice: Detach a specific event only from one specific
///            thread and not
///                           from multiple contexts.
template <uint64_t Capacity>
class ListenerImpl {
 public:
  ListenerImpl() noexcept;
  ListenerImpl(const ListenerImpl&) = delete;
  ListenerImpl(ListenerImpl&&) = delete;
  ~ListenerImpl() noexcept;

  ListenerImpl& operator=(const ListenerImpl&) = delete;
  ListenerImpl& operator=(ListenerImpl&&) = delete;

  /// @brief Attaches an event. Hereby the event is defined as a class T, the
  /// eventOrigin, an enum which further
  ///        defines the event inside the class and the corresponding callback
  ///        which will be called when the event occurs.
  /// @note This method can be called from any thread concurrently without any
  /// restrictions!
  ///        Furthermore, attachEvent does not take ownership of callback in the
  ///        underlying eventCallback or the optional contextData. The user has
  ///        to ensure that both will live as long as the event is attached.
  /// @tparam[in] T type of the class which will signal the event
  /// @param[in] eventOrigin the object which will signal the event (the origin)
  /// @param[in] eventType enum required to specify the type of event inside of
  /// eventOrigin
  /// @param[in] eventCallback callback which will be executed concurrently when
  /// the event occurs. has to be created with
  /// iox::popo::createNotificationCallback
  /// @return If an error occurs the enum packed inside an expected which
  /// describes the error.
  template <typename T, typename EventType, typename ContextDataType,
            typename = std::enable_if_t<std::is_enum<EventType>::value>>
  tl::expected<void, ListenerError> attachEvent(
      T& eventOrigin, const EventType eventType,
      const NotificationCallback<T, ContextDataType>& eventCallback) noexcept;

  /// @brief Attaches an event. Hereby the event is defined as a class T, the
  /// eventOrigin and
  ///        the corresponding callback which will be called when the event
  ///        occurs.
  /// @note This method can be called from any thread concurrently without any
  /// restrictions!
  ///        Furthermore, attachEvent does not take ownership of callback in the
  ///        underlying eventCallback or the optional contextData. The user has
  ///        to ensure that both will live as long as the event is attached.
  /// @tparam[in] T type of the class which will signal the event
  /// @param[in] eventOrigin the object which will signal the event (the origin)
  /// @param[in] eventCallback callback which will be executed concurrently when
  /// the event occurs. Has to be created with
  /// iox::popo::createNotificationCallback
  /// @return If an error occurs the enum packed inside an expected which
  /// describes the error.
  template <typename T, typename ContextDataType>
  tl::expected<void, ListenerError> attachEvent(
      T& eventOrigin,
      const NotificationCallback<T, ContextDataType>& eventCallback) noexcept;

  /// @brief Detaches an event. Hereby, the event is defined as a class T, the
  /// eventOrigin and
  ///        the eventType with further specifies the event inside of
  ///        eventOrigin
  /// @note This method can be called from any thread concurrently without any
  /// restrictions!
  /// @tparam[in] T type of the class which will signal the event
  /// @param[in] eventOrigin the object which will signal the event (the origin)
  /// @param[in] eventType enum required to specify the type of event inside of
  /// eventOrigin
  template <typename T, typename EventType,
            typename = std::enable_if_t<std::is_enum<EventType>::value>>
  void detachEvent(T& eventOrigin, const EventType eventType) noexcept;

  /// @brief Detaches an event. Hereby, the event is defined as a class T, the
  /// eventOrigin.
  /// @note This method can be called from any thread concurrently without any
  /// restrictions!
  /// @tparam[in] T type of the class which will signal the event
  template <typename T>
  void detachEvent(T& eventOrigin) noexcept;

  /// @brief Returns the capacity of the Listener
  /// @return capacity of the Listener
  static constexpr uint64_t capacity() noexcept;

  /// @brief Returns the size of the Listener
  /// @return size of the Listener
  uint64_t size() const noexcept;

 protected:
  ListenerImpl(ConditionVariableData& conditionVariableData) noexcept;

 private:
  class Event_t;

  void threadLoop() noexcept;
  tl::expected<uint32_t, ListenerError> addEvent(
      void* const origin, void* const userType, const uint64_t eventType,
      const uint64_t eventTypeHash, internal::GenericCallbackRef_t callback,
      internal::TranslationCallbackRef_t translationCallback,
      const std::function<void(uint64_t)> invalidationCallback) noexcept;

  void removeTrigger(const uint64_t index) noexcept;

 private:
  enum class NoEnumUsed : EventEnumIdentifier { PLACEHOLDER = 0 };

  class IndexManager_t {
   public:
    IndexManager_t() noexcept;
    bool pop(uint32_t& index) noexcept;
    void push(const uint32_t index) noexcept;
    uint64_t indicesInUse() const noexcept;

    std::array<LoFFLi::Index_t,
               LoFFLi::requiredIndexMemorySize(Capacity) / sizeof(uint32_t)>
        m_loffliStorage;
    LoFFLi m_loffli;
    std::atomic<uint64_t> m_indicesInUse{0U};
  } m_indexManager;

  std::thread m_thread;
  //   concurrent::smart_lock<internal::Event_t, std::recursive_mutex>
  //       m_events[Capacity];
  std::mutex m_addEventMutex;

  std::atomic_bool m_wasDtorCalled{false};
  ConditionVariableData* m_conditionVariableData = nullptr;
  ConditionListener m_conditionListener;
};

static constexpr const std::size_t MAX_NUMBER_OF_EVENTS_PER_LISTENER = 256U;

class Listener : public ListenerImpl<MAX_NUMBER_OF_EVENTS_PER_LISTENER> {
 public:
  using Parent = ListenerImpl<MAX_NUMBER_OF_EVENTS_PER_LISTENER>;
  Listener() noexcept;

 protected:
  Listener(ConditionVariableData& conditionVariableData) noexcept;
};

}  // namespace details
}  // namespace shm