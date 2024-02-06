

#pragma once

namespace shm {
namespace details {
namespace internal {
using GenericCallbackPtr_t = void (*)();
using GenericCallbackRef_t = void (&)();

using TranslationCallbackRef_t = void (&)(void* const, void* const,
                                          GenericCallbackPtr_t const);
using TranslationCallbackPtr_t = void (*)(void* const, void* const,
                                          GenericCallbackPtr_t const);

}  // namespace internal

///@brief the struct describes a callback with a user defined type which can
///         be attached to a WaitSet or a Listener
template <typename OriginType, typename ContextDataType>
struct NotificationCallback {
  using Ref_t = void (&)(OriginType* const, ContextDataType* const);
  using Ptr_t = void (*)(OriginType* const, ContextDataType* const);

  Ptr_t m_callback = nullptr;
  ContextDataType* m_contextData = nullptr;
};

}  // namespace details
}  // namespace shm