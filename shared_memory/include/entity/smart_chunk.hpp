#pragma once

#include <functional>
#include <iostream>
#include <memory>

#include "memory/chunk_header.hpp"
#include "plog/Log.h"
#include "types/optional.hpp"
#include "utils/type_traits.hpp"

namespace shm {
namespace entity {

namespace internal {
/// @brief helper struct for smartChunk
template <typename TransmissionInterface, typename T, typename H>
struct SmartChunkPrivateData {
  using DeleteFunction = std::function<void(T*)>;

  SmartChunkPrivateData(
      std::unique_ptr<T, DeleteFunction>&& smartChunkUniquePtr,
      TransmissionInterface& producer) noexcept;

  SmartChunkPrivateData(SmartChunkPrivateData&& rhs) noexcept = default;
  SmartChunkPrivateData& operator=(SmartChunkPrivateData&& rhs) noexcept =
      default;

  SmartChunkPrivateData(const SmartChunkPrivateData&) = delete;
  SmartChunkPrivateData& operator=(const SmartChunkPrivateData&) = delete;
  ~SmartChunkPrivateData() = default;

  tl::optional<std::unique_ptr<T, DeleteFunction>> smartChunkUniquePtr;
  std::reference_wrapper<TransmissionInterface> producerRef;
};

/// @brief specialization of helper struct for smartChunk for const T
template <typename TransmissionInterface, typename T, typename H>
struct SmartChunkPrivateData<TransmissionInterface, const T, H> {
  using DeleteFunction = std::function<void(T*)>;

  explicit SmartChunkPrivateData(
      std::unique_ptr<const T, DeleteFunction>&& smartChunkUniquePtr) noexcept;

  SmartChunkPrivateData(SmartChunkPrivateData&& rhs) noexcept = default;
  SmartChunkPrivateData& operator=(SmartChunkPrivateData&& rhs) noexcept =
      default;

  SmartChunkPrivateData(const SmartChunkPrivateData&) = delete;
  SmartChunkPrivateData& operator=(const SmartChunkPrivateData&) = delete;
  ~SmartChunkPrivateData() = default;

  tl::optional<std::unique_ptr<const T, DeleteFunction>> smartChunkUniquePtr;
};
}  // namespace internal

template <typename TransmissionInterface, typename T,
          typename H = add_const_conditionally_t<memory::NoUserHeader, T>>
class SmartChunk {
  using DeleteFunction = std::function<void(T*)>;

 protected:
  static_assert(
      std::is_const<T>::value == std::is_const<H>::value,
      "The type 'T' and the user-header 'H' must be equal in their const "
      "qualifier to ensure the same "
      "access restrictions for the user-header as for the smartChunk data!");

  /// @brief Helper type to enable the constructor for the producer, i.e. when T
  /// has no const qualifier
  template <typename S, typename TT>
  using ForProducerOnly =
      std::enable_if_t<std::is_same<S, TT>::value && !std::is_const<TT>::value,
                       S>;

  /// @brief Helper type to enable the constructor for the consumer, i.e. when T
  /// has a const qualifier
  template <typename S, typename TT>
  using ForConsumerOnly =
      std::enable_if_t<std::is_same<S, TT>::value && std::is_const<TT>::value,
                       S>;

  /// @brief Helper type to enable some methods only if a user-header is used
  template <typename R, typename HH>
  using HasUserHeader =
      std::enable_if_t<std::is_same<R, HH>::value &&
                           !std::is_same<R, memory::NoUserHeader>::value,
                       R>;

 public:
  /// @brief Constructor for a SmartChunk used by the Producer
  /// @tparam S is a dummy template parameter to enable the constructor only for
  /// non-const T
  /// @param smartChunkUniquePtr is a 'rvalue' to a 'iox::unique_ptr<T>' with to
  /// the data of the encapsulated type
  /// T
  /// @param producer is a reference to the producer to be able to use producer
  /// specific methods
  template <typename S = T, typename = ForProducerOnly<S, T>>
  SmartChunk(std::unique_ptr<T, DeleteFunction>&& smartChunkUniquePtr,
             TransmissionInterface& producer) noexcept;

  /// @brief Constructor for a SmartChunk used by the Consumer
  /// @tparam S is a dummy template parameter to enable the constructor only for
  /// const T
  /// @param smartChunkUniquePtr is a 'rvalue' to a 'iox::unique_ptr<T>' with to
  /// the data of the encapsulated type
  /// T
  template <typename S = T, typename = ForConsumerOnly<S, T>>
  explicit SmartChunk(
      std::unique_ptr<T, DeleteFunction>&& smartChunkUniquePtr) noexcept;

  ~SmartChunk() noexcept = default;

  SmartChunk& operator=(SmartChunk&& rhs) noexcept = default;
  SmartChunk(SmartChunk&& rhs) noexcept = default;

  SmartChunk(const SmartChunk&) = delete;
  SmartChunk& operator=(const SmartChunk&) = delete;

  ///
  /// @brief Transparent access to the encapsulated type.
  /// @return a pointer to the encapsulated type.
  ///
  T* operator->() noexcept;

  ///
  /// @brief Transparent read-only access to the encapsulated type.
  /// @return a const pointer to the encapsulated type.
  ///
  const T* operator->() const noexcept;

  ///
  /// @brief Provides a reference to the encapsulated type.
  /// @return A T& to the encapsulated type.
  ///
  T& operator*() noexcept;

  ///
  /// @brief Provides a const reference to the encapsulated type.
  /// @return A const T& to the encapsulated type.
  ///
  const T& operator*() const noexcept;

  ///
  /// @brief Indicates whether the smartChunk is valid, i.e. refers to allocated
  /// memory.
  /// @return true if the smartChunk is valid, false otherwise.
  ///
  explicit operator bool() const noexcept;

  ///
  /// @brief Mutable access to the encapsulated type loaned to the smartChunk.
  /// @return a pointer to the encapsulated type.
  ///
  T* get() noexcept;

  ///
  /// @brief Read-only access to the encapsulated type loaned to the smartChunk.
  /// @return a const pointer to the encapsulated type.
  ///
  const T* get() const noexcept;

  ///
  /// @brief Retrieve the ChunkHeader of the underlying memory chunk loaned to
  /// the smartChunk.
  /// @return The ChunkHeader of the underlying memory chunk.
  ///
  add_const_conditionally_t<memory::ChunkHeader, T>* getChunkHeader() noexcept;

  ///
  /// @brief Retrieve the ChunkHeader of the underlying memory chunk loaned to
  /// the smartChunk.
  /// @return The const ChunkHeader of the underlying memory chunk.
  ///
  const memory::ChunkHeader* getChunkHeader() const noexcept;

 protected:
  /// @brief Retrieve the user-header of the underlying memory chunk loaned to
  /// the SmartChunk.
  /// @return The user-header of the underlying memory chunk.
  ///
  template <typename R = H, typename = HasUserHeader<R, H>>
  add_const_conditionally_t<R, T>& getUserHeader() noexcept;

  ///
  /// @brief Retrieve the user-header of the underlying memory chunk loaned to
  /// the SmartChunk.
  /// @return The user-header of the underlying memory chunk.
  ///
  template <typename R = H, typename = HasUserHeader<R, H>>
  const R& getUserHeader() const noexcept;

  /// @note used by the producer to release the chunk ownership from the
  /// 'SmartChunk' after publishing the chunk and therefore preventing the
  /// invocation of the custom deleter
  T* release() noexcept;

 protected:
  internal::SmartChunkPrivateData<TransmissionInterface, T, H> m_members;
};

namespace internal {
template <typename TransmissionInterface, typename T, typename H>
inline SmartChunkPrivateData<TransmissionInterface, T, H>::
    SmartChunkPrivateData(
        std::unique_ptr<T, DeleteFunction>&& smartChunkUniquePtr,
        TransmissionInterface& producer) noexcept
    : smartChunkUniquePtr(std::move(smartChunkUniquePtr)),
      producerRef(producer) {}

template <typename TransmissionInterface, typename T, typename H>
inline SmartChunkPrivateData<TransmissionInterface, const T, H>::
    SmartChunkPrivateData(
        std::unique_ptr<const T, DeleteFunction>&& smartChunkUniquePtr) noexcept
    : smartChunkUniquePtr(std::move(smartChunkUniquePtr)) {}
}  // namespace internal

template <typename TransmissionInterface, typename T, typename H>
template <typename S, typename>
inline SmartChunk<TransmissionInterface, T, H>::SmartChunk(
    std::unique_ptr<T, DeleteFunction>&& smartChunkUniquePtr,
    TransmissionInterface& producer) noexcept
    : m_members({std::move(smartChunkUniquePtr), producer}) {
  std::cout << __LINE__ << std::endl;
  LOG_DEBUG() << __LINE__;
}

template <typename TransmissionInterface, typename T, typename H>
template <typename S, typename>
inline SmartChunk<TransmissionInterface, T, H>::SmartChunk(
    std::unique_ptr<T, DeleteFunction>&& smartChunkUniquePtr) noexcept
    : m_members(std::move(smartChunkUniquePtr)) {}

template <typename TransmissionInterface, typename T, typename H>
inline T* SmartChunk<TransmissionInterface, T, H>::operator->() noexcept {
  return get();
}

template <typename TransmissionInterface, typename T, typename H>
inline const T* SmartChunk<TransmissionInterface, T, H>::operator->()
    const noexcept {
  return get();
}

template <typename TransmissionInterface, typename T, typename H>
inline T& SmartChunk<TransmissionInterface, T, H>::operator*() noexcept {
  return *get();
}

template <typename TransmissionInterface, typename T, typename H>
inline const T& SmartChunk<TransmissionInterface, T, H>::operator*()
    const noexcept {
  return *get();
}

template <typename TransmissionInterface, typename T, typename H>
inline SmartChunk<TransmissionInterface, T, H>::operator bool() const noexcept {
  return m_members.smartChunkUniquePtr.operator bool();
}

template <typename TransmissionInterface, typename T, typename H>
inline T* SmartChunk<TransmissionInterface, T, H>::get() noexcept {
  return m_members.smartChunkUniquePtr->get();
}

template <typename TransmissionInterface, typename T, typename H>
inline const T* SmartChunk<TransmissionInterface, T, H>::get() const noexcept {
  return m_members.smartChunkUniquePtr->get();
}

template <typename TransmissionInterface, typename T, typename H>
inline add_const_conditionally_t<memory::ChunkHeader, T>*
SmartChunk<TransmissionInterface, T, H>::getChunkHeader() noexcept {
  return memory::ChunkHeader::fromUserPayload(
      m_members.smartChunkUniquePtr->get());
}

template <typename TransmissionInterface, typename T, typename H>
inline const memory::ChunkHeader*
SmartChunk<TransmissionInterface, T, H>::getChunkHeader() const noexcept {
  return memory::ChunkHeader::fromUserPayload(
      m_members.smartChunkUniquePtr->get());
}

template <typename TransmissionInterface, typename T, typename H>
template <typename R, typename>
inline add_const_conditionally_t<R, T>&
SmartChunk<TransmissionInterface, T, H>::getUserHeader() noexcept {
  return *static_cast<R*>(
      memory::ChunkHeader::fromUserPayload(m_members.smartChunkUniquePtr->get())
          ->userHeader());
}

template <typename TransmissionInterface, typename T, typename H>
template <typename R, typename>
inline const R& SmartChunk<TransmissionInterface, T, H>::getUserHeader()
    const noexcept {
  return const_cast<SmartChunk<TransmissionInterface, T, H>*>(this)
      ->getUserHeader();
}

template <typename TransmissionInterface, typename T, typename H>
inline T* SmartChunk<TransmissionInterface, T, H>::release() noexcept {
  //   auto ptr =
  //       std::unique_ptr<T>::release(std::move(*m_members.smartChunkUniquePtr));
  //   m_members.smartChunkUniquePtr.reset();
  if ((*m_members.smartChunkUniquePtr)) {
    return (*m_members.smartChunkUniquePtr).release();
  }
  return nullptr;
}

}  // namespace entity
}  // namespace shm