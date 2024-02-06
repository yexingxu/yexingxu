#pragma once

#include <memory>

#include "memory/chunk_header.hpp"
#include "types/optional.hpp"
#include "utils/type_traits.hpp"

namespace shm {
namespace entity {

namespace internal {
/// @brief helper struct for smartChunk
template <typename TransmissionInterface, typename T, typename H>
struct SmartChunkPrivateData {
  SmartChunkPrivateData(std::unique_ptr<T>&& smartChunkUniquePtr,
                        TransmissionInterface& producer) noexcept;

  SmartChunkPrivateData(SmartChunkPrivateData&& rhs) noexcept = default;
  SmartChunkPrivateData& operator=(SmartChunkPrivateData&& rhs) noexcept =
      default;

  SmartChunkPrivateData(const SmartChunkPrivateData&) = delete;
  SmartChunkPrivateData& operator=(const SmartChunkPrivateData&) = delete;
  ~SmartChunkPrivateData() = default;

  tl::optional<std::unique_ptr<T>> smartChunkUniquePtr;
  std::reference_wrapper<TransmissionInterface> producerRef;
};

/// @brief specialization of helper struct for smartChunk for const T
template <typename TransmissionInterface, typename T, typename H>
struct SmartChunkPrivateData<TransmissionInterface, const T, H> {
  explicit SmartChunkPrivateData(
      std::unique_ptr<const T>&& smartChunkUniquePtr) noexcept;

  SmartChunkPrivateData(SmartChunkPrivateData&& rhs) noexcept = default;
  SmartChunkPrivateData& operator=(SmartChunkPrivateData&& rhs) noexcept =
      default;

  SmartChunkPrivateData(const SmartChunkPrivateData&) = delete;
  SmartChunkPrivateData& operator=(const SmartChunkPrivateData&) = delete;
  ~SmartChunkPrivateData() = default;

  tl::optional<std::unique_ptr<const T>> smartChunkUniquePtr;
};
}  // namespace internal

template <typename TransmissionInterface, typename T,
          typename H = add_const_conditionally_t<memory::NoUserHeader, T>>
class SmartChunk {
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
  SmartChunk(std::unique_ptr<T>&& smartChunkUniquePtr,
             TransmissionInterface& producer) noexcept;

  /// @brief Constructor for a SmartChunk used by the Consumer
  /// @tparam S is a dummy template parameter to enable the constructor only for
  /// const T
  /// @param smartChunkUniquePtr is a 'rvalue' to a 'iox::unique_ptr<T>' with to
  /// the data of the encapsulated type
  /// T
  template <typename S = T, typename = ForConsumerOnly<S, T>>
  explicit SmartChunk(std::unique_ptr<T>&& smartChunkUniquePtr) noexcept;

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

}  // namespace entity
}  // namespace shm