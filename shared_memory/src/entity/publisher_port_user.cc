#include "entity/publisher_port_user.hpp"

#include <iostream>

namespace shm {
namespace entity {

PublisherPortUser::PublisherPortUser(
    details::not_null<MemberType_t* const> publisherPortDataPtr) noexcept
    : BasePort(publisherPortDataPtr),
      m_chunkSender(&getMembers()->m_chunkSenderData)

{}

const PublisherPortUser::MemberType_t* PublisherPortUser::getMembers()
    const noexcept {
  return reinterpret_cast<const MemberType_t*>(BasePort::getMembers());
}

PublisherPortUser::MemberType_t* PublisherPortUser::getMembers() noexcept {
  return reinterpret_cast<MemberType_t*>(BasePort::getMembers());
}

tl::expected<memory::ChunkHeader*, memory::AllocationError>
PublisherPortUser::tryAllocateChunk(
    const uint32_t userPayloadSize, const uint32_t userPayloadAlignment,
    const uint32_t userHeaderSize,
    const uint32_t userHeaderAlignment) noexcept {
  return m_chunkSender.tryAllocate(getUniqueID(), userPayloadSize,
                                   userPayloadAlignment, userHeaderSize,
                                   userHeaderAlignment);
}

void PublisherPortUser::releaseChunk(
    memory::ChunkHeader* const chunkHeader) noexcept {
  std::cout << __LINE__ << std::endl;
  m_chunkSender.release(chunkHeader);
  std::cout << __LINE__ << std::endl;
}

void PublisherPortUser::sendChunk(
    memory::ChunkHeader* const chunkHeader) noexcept {
  const auto offerRequested =
      getMembers()->m_offeringRequested.load(std::memory_order_relaxed);

  if (offerRequested) {
    m_chunkSender.send(chunkHeader);
  } else {
    // if the publisher port is not offered, we do not send the chunk but we put
    // them in the history this is needed e.g. for AUTOSAR Adaptive fields just
    // always calling send and relying that there are no subscribers if not
    // offered does not work, as the list of subscribers is updated
    // asynchronously by RouDi (only RouDi has write access to the list of
    // subscribers)
    m_chunkSender.pushToHistory(chunkHeader);
  }
}

tl::optional<const memory::ChunkHeader*>
PublisherPortUser::tryGetPreviousChunk() const noexcept {
  return m_chunkSender.tryGetPreviousChunk();
}

void PublisherPortUser::offer() noexcept {
  if (!getMembers()->m_offeringRequested.load(std::memory_order_relaxed)) {
    getMembers()->m_offeringRequested.store(true, std::memory_order_relaxed);
  }
}

void PublisherPortUser::stopOffer() noexcept {
  if (getMembers()->m_offeringRequested.load(std::memory_order_relaxed)) {
    getMembers()->m_offeringRequested.store(false, std::memory_order_relaxed);
  }
}

bool PublisherPortUser::isOffered() const noexcept {
  return getMembers()->m_offeringRequested.load(std::memory_order_relaxed);
}

bool PublisherPortUser::hasSubscribers() const noexcept {
  return m_chunkSender.hasStoredQueues();
}

}  // namespace entity
}  // namespace shm