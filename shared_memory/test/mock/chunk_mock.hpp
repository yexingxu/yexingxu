#include "../test.hpp"
#include "memory/chunk_header.hpp"
#include "shm/memory.hpp"

template <typename Topic, typename UserHeader = shm::memory::NoUserHeader>
class ChunkMock {
 public:
  ChunkMock() {
    const uint32_t userPayloadSize = sizeof(Topic);
    const uint32_t userPayloadAlignment = alignof(Topic);
    const uint32_t userHeaderSize =
        std::is_same<UserHeader, shm::memory::NoUserHeader>::value
            ? 0U
            : sizeof(UserHeader);
    const uint32_t userHeaderAlignment = alignof(UserHeader);

    auto chunkSettingsResult = shm::memory::ChunkSettings::create(
        userPayloadSize, userPayloadAlignment, userHeaderSize,
        userHeaderAlignment);

    ENSURES(chunkSettingsResult.has_value());
    auto& chunkSettings = chunkSettingsResult.value();
    auto chunkSize = chunkSettings.requiredChunkSize();

    m_rawMemory = static_cast<uint8_t*>(shm::details::alignedAlloc(
        alignof(shm::memory::ChunkHeader), chunkSize));
    assert(m_rawMemory != nullptr && "Could not get aligned memory");
    memset(m_rawMemory, 0xFF, chunkSize);

    m_chunkHeader =
        new (m_rawMemory) shm::memory::ChunkHeader(chunkSize, chunkSettings);
    m_topic = static_cast<Topic*>(m_chunkHeader->userPayload());
  }
  ~ChunkMock() {
    if (m_chunkHeader != nullptr) {
      m_chunkHeader->~ChunkHeader();
    }
    if (m_rawMemory != nullptr) {
      shm::details::alignedFree(m_rawMemory);
      m_rawMemory = nullptr;
    }
  }

  shm::memory::ChunkHeader* chunkHeader() { return m_chunkHeader; }

  const shm::memory::ChunkHeader* chunkHeader() const { return m_chunkHeader; }

  UserHeader* userHeader() {
    return static_cast<UserHeader*>(m_chunkHeader->userHeader());
  }

  const UserHeader* userHeader() const {
    return const_cast<ChunkMock*>(this)->userHeader();
  }

  Topic* sample() { return m_topic; }

  const Topic* sample() const { return m_topic; }

  ChunkMock(const ChunkMock&) = delete;
  ChunkMock(ChunkMock&&) = delete;
  ChunkMock& operator=(const ChunkMock&) = delete;
  ChunkMock& operator=(ChunkMock&&) = delete;

 private:
  uint8_t* m_rawMemory{nullptr};
  shm::memory::ChunkHeader* m_chunkHeader = nullptr;
  Topic* m_topic = nullptr;
};