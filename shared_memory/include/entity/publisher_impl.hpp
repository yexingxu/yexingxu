#pragma once

#include <functional>
#include <iostream>

#include "entity/base_publisher.hpp"
#include "entity/publisher_interface.hpp"
#include "entity/typed_port_traits.hpp"
#include "types/expected.hpp"

namespace shm {
namespace entity {

/// @brief The PublisherImpl class implements the typed publisher API
/// @note Not intended for public usage! Use the 'Publisher' instead!
template <typename T, typename H = memory::NoUserHeader,
          typename BasePublisherType = BasePublisher<>>
class PublisherImpl : public BasePublisherType,
                      private PublisherInterface<T, H> {
  using DataTypeAssert = typename TypedPortApiTrait<T>::Assert;
  using HeaderTypeAssert = typename TypedPortApiTrait<H>::Assert;

 public:
  explicit PublisherImpl(
      const ServiceDescription& service,
      const PublisherOptions& publisherOptions = PublisherOptions());
  PublisherImpl(const PublisherImpl& other) = delete;
  PublisherImpl& operator=(const PublisherImpl&) = delete;
  PublisherImpl(PublisherImpl&& rhs) = delete;
  PublisherImpl& operator=(PublisherImpl&& rhs) = delete;
  virtual ~PublisherImpl() = default;

  ///
  /// @brief loan Get a sample from loaned shared memory and consctruct the data
  /// with the given arguments.
  /// @param args Arguments used to construct the data.
  /// @return An instance of the sample that resides in shared memory or an
  /// error if unable ot allocate memory to loan.
  /// @details The loaned sample is automatically released when it goes out of
  /// scope.
  ///
  template <typename... Args>
  tl::expected<Sample<T, H>, memory::AllocationError> loan(
      Args&&... args) noexcept;

  ///
  /// @brief publish Publishes the given sample and then releases its loan.
  /// @param sample The sample to publish.
  ///
  void publish(Sample<T, H>&& sample) noexcept override;

  ///
  /// @brief publishCopyOf Copy the provided value into a loaned shared memory
  /// chunk and publish it.
  /// @param val Value to copy.
  /// @return Error if unable to allocate memory to loan.
  ///
  tl::expected<void, memory::AllocationError> publishCopyOf(
      const T& val) noexcept;
  ///
  /// @brief publishResultOf Loan a sample from memory, execute the provided
  /// callable to write to it, then publish it.
  /// @param c Callable with the signature void(T*, ArgTypes...) that write's
  /// it's result to T*.
  /// @param args The arguments of the callable.
  /// @return Error if unable to allocate memory to loan.
  ///
  template <typename Callable, typename... ArgTypes>
  tl::expected<void, memory::AllocationError> publishResultOf(
      Callable c, ArgTypes... args) noexcept;

 protected:
  using BasePublisherType::port;

 private:
  Sample<T, H> convertChunkHeaderToSample(
      memory::ChunkHeader* const header) noexcept;

  tl::expected<Sample<T, H>, memory::AllocationError> loanSample() noexcept;
};

template <typename T, typename H, typename BasePublisherType>
inline PublisherImpl<T, H, BasePublisherType>::PublisherImpl(
    const ServiceDescription& service, const PublisherOptions& publisherOptions)
    : BasePublisherType(service, publisherOptions) {}

template <typename T, typename H, typename BasePublisherType>
template <typename... Args>
inline tl::expected<Sample<T, H>, memory::AllocationError>
PublisherImpl<T, H, BasePublisherType>::loan(Args&&... args) noexcept {
  return std::move(loanSample().and_then(
      [&](auto&& sample)
          -> tl::expected<Sample<T, H>, memory::AllocationError> {
        std::cout << __LINE__ << std::endl;
        new (sample.get()) T(std::forward<Args>(args)...);
        std::cout << __LINE__ << std::endl;
      }));
}

template <typename T, typename H, typename BasePublisherType>
template <typename Callable, typename... ArgTypes>
inline tl::expected<void, memory::AllocationError>
PublisherImpl<T, H, BasePublisherType>::publishResultOf(
    Callable c, ArgTypes... args) noexcept {
  static_assert(is_invocable<Callable, T*, ArgTypes...>::value,
                "Publisher<T>::publishResultOf expects a valid callable with a "
                "specific signature as the "
                "first argument");
  static_assert(is_invocable_r<void, Callable, T*, ArgTypes...>::value,
                "callable provided to Publisher<T>::publishResultOf must have "
                "signature void(T*, ArgsTypes...)");
  auto res = loanSample();
  if (res.has_value()) {
    c(new (res.value().get()) T, std::forward<ArgTypes>(args)...);
    res.value().publish();
  }
  return {};
}

template <typename T, typename H, typename BasePublisherType>
inline tl::expected<void, memory::AllocationError>
PublisherImpl<T, H, BasePublisherType>::publishCopyOf(const T& val) noexcept {
  auto res = loanSample();
  if (res.has_value()) {
    new (res.value().get())
        T(val);  // Placement new copy-construction of sample, avoid
                 // copy-assigment because there might not be an existing
                 // instance of T in the sample memory
    res.value().publish();
  }
  return {};
}

template <typename T, typename H, typename BasePublisherType>
inline tl::expected<Sample<T, H>, memory::AllocationError>
PublisherImpl<T, H, BasePublisherType>::loanSample() noexcept {
  static constexpr uint32_t USER_HEADER_SIZE{
      std::is_same<H, memory::NoUserHeader>::value ? 0U : sizeof(H)};
  std::cout << __LINE__ << std::endl;

  auto result = port().tryAllocateChunk(sizeof(T), alignof(T), USER_HEADER_SIZE,
                                        alignof(H));
  std::cout << __LINE__ << std::endl;

  if (!result.has_value()) {
    return tl::make_unexpected(result.error());
  } else {
    return convertChunkHeaderToSample(result.value());
  }
}

template <typename T, typename H, typename BasePublisherType>
inline void PublisherImpl<T, H, BasePublisherType>::publish(
    Sample<T, H>&& sample) noexcept {
  auto userPayload = sample.release();  // release the Samples ownership of the
                                        // chunk before publishing
  auto chunkHeader = memory::ChunkHeader::fromUserPayload(userPayload);
  port().sendChunk(chunkHeader);
}

template <typename T, typename H, typename BasePublisherType>
inline Sample<T, H>
PublisherImpl<T, H, BasePublisherType>::convertChunkHeaderToSample(
    memory::ChunkHeader* const header) noexcept {
  return Sample<T, H>(
      std::unique_ptr<T, std::function<void(T*)>>(
          reinterpret_cast<T*>(header->userPayload()),
          [this](T* userPayload) {
            auto* chunkHeader =
                memory::ChunkHeader::fromUserPayload(userPayload);
            this->port().releaseChunk(chunkHeader);
          }),
      *this);
}

}  // namespace entity
}  // namespace shm