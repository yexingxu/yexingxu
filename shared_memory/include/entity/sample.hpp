#pragma once

#include <spdlog/spdlog.h>

#include "entity/smart_chunk.hpp"

namespace shm {
namespace entity {

template <typename T, typename H>
class PublisherInterface;

/// @brief The Sample class is a mutable abstraction over types which are
/// written to loaned shared memory. These samples are publishable to the
/// iceoryx system.
template <typename T,
          typename H = add_const_conditionally_t<memory::NoUserHeader, T>>
class Sample : public SmartChunk<PublisherInterface<T, H>, T, H> {
  using BaseType = SmartChunk<PublisherInterface<T, H>, T, H>;

 public:
  template <typename T1, typename T2>
  using ForPublisherOnly = typename BaseType::template ForProducerOnly<T1, T2>;

  /// @brief Constructor for a Sample used by the publisher/subscriber
  /// @tparam S is a dummy template parameter to enable the constructor only for
  /// non-const T
  /// @param smartChunkUniquePtr is a 'rvalue' to a 'iox::unique_ptr<T>' with to
  /// the data of the encapsulated type
  /// T
  /// @param producer (for publisher only) is a reference to the publisher to be
  /// able to use publisher specific methods
  using BaseType::BaseType;

  /// @brief Retrieve the user-header of the underlying memory chunk loaned to
  /// the sample.
  /// @return The user-header of the underlying memory chunk.
  using BaseType::getUserHeader;

  /// @brief Publish the sample via the publisher from which it was loaned and
  /// automatically release ownership to it.
  /// @details Only available for non-const type T.
  template <typename S = T, typename = ForPublisherOnly<S, T>>
  void publish() noexcept;

 private:
  template <typename, typename, typename>
  friend class PublisherImpl;

  /// @note used by the publisher to release the chunk ownership from the
  /// 'Sample' after publishing the chunk and therefore preventing the
  /// invocation of the custom deleter
  using BaseType::release;

  using BaseType::m_members;
};

template <typename T, typename H>
template <typename S, typename>
void Sample<T, H>::publish() noexcept {
  if (BaseType::m_members.smartChunkUniquePtr) {
    BaseType::m_members.producerRef.get().publish(std::move(*(this)));
  } else {
    spdlog::error(
        "Tried to publish empty Sample! Might be an already published or "
        "moved Sample!");
  }
}

}  // namespace entity
}  // namespace shm