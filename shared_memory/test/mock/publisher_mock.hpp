#pragma once

#include "../test.hpp"
#include "entity/publisher_port_data.hpp"
#include "memory/chunk_sender.hpp"

using namespace ::testing;

class MockPublisherPortUser {
 public:
  using MemberType_t = shm::entity::PublisherPortData;
  MockPublisherPortUser() = default;
  MockPublisherPortUser(std::nullptr_t) {}
  MockPublisherPortUser(MemberType_t*){};

  MockPublisherPortUser(const MockPublisherPortUser& rhs [[maybe_unused]]){};
  MockPublisherPortUser(MockPublisherPortUser&& rhs [[maybe_unused]]){};
  MockPublisherPortUser& operator=(const MockPublisherPortUser& rhs
                                   [[maybe_unused]]) {
    return *this;
  };
  MockPublisherPortUser& operator=(MockPublisherPortUser&& rhs
                                   [[maybe_unused]]) {
    return *this;
  };
  shm::entity::ServiceDescription getCaProServiceDescription() const noexcept {
    return getServiceDescription();
  }
  MOCK_CONST_METHOD0(getServiceDescription, shm::entity::ServiceDescription());
  MOCK_METHOD4(
      tryAllocateChunk,
      tl::expected<shm::memory::ChunkHeader*, shm::memory::AllocationError>(
          const uint32_t, const uint32_t, const uint32_t, const uint32_t));
  MOCK_METHOD1(releaseChunk, void(shm::memory::ChunkHeader* const));
  MOCK_METHOD1(sendChunk, void(shm::memory::ChunkHeader* const));
  MOCK_METHOD0(tryGetPreviousChunk, tl::optional<shm::memory::ChunkHeader*>());
  MOCK_METHOD0(offer, void());
  MOCK_METHOD0(stopOffer, void());
  MOCK_CONST_METHOD0(isOffered, bool());
  MOCK_CONST_METHOD0(hasSubscribers, bool());

  operator bool() const { return true; }

  MOCK_CONST_METHOD0(getUniqueID, shm::types::UniquePortId());
  MOCK_METHOD0(destroy, void());
};

template <typename T>
class MockBasePublisher {
 public:
  using PortType = MockPublisherPortUser;

  MockBasePublisher(const shm::entity::ServiceDescription&,
                    const shm::entity::PublisherOptions&){};
  MOCK_CONST_METHOD0(getUid, uid_t());
  MOCK_CONST_METHOD0(getServiceDescription, shm::entity::ServiceDescription());
  MOCK_METHOD0(offer, void(void));
  MOCK_METHOD0(stopOffer, void(void));
  MOCK_CONST_METHOD0(isOffered, bool(void));
  MOCK_CONST_METHOD0(hasSubscribers, bool(void));

  const MockPublisherPortUser& port() const noexcept { return m_port; }

  MockPublisherPortUser& port() noexcept { return m_port; }

  // for testing
  MockPublisherPortUser& mockPort() noexcept { return port(); }

  MockPublisherPortUser m_port;
};