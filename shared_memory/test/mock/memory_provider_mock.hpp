#pragma once

#include "../test.hpp"
#include "shm/memory.hpp"
#include "shm/memory_provider.hpp"

class MemoryProviderTestImpl : public shm::details::MemoryProvider {
 public:
  ~MemoryProviderTestImpl() {
    if (isAvailable()) {
      EXPECT_FALSE(!destroy().has_value());
    }
  }

  tl::expected<void*, shm::details::MemoryProviderError> createMemory(
      const uint64_t size, const uint64_t alignment) noexcept override {
    if (m_mockCallsEnabled) {
      createMemoryMock(size, alignment);
    }

    dummyMemory = shm::details::alignedAlloc(alignment, size);
    return dummyMemory;
  }
  MOCK_METHOD(void, createMemoryMock, (uint64_t, uint64_t), (noexcept));

  tl::expected<void, shm::details::MemoryProviderError> destroyMemory() noexcept
      override {
    if (m_mockCallsEnabled) {
      destroyMemoryMock();
    }

    shm::details::alignedFree(dummyMemory);
    dummyMemory = nullptr;

    return {};
  }
  MOCK_METHOD(void, destroyMemoryMock, (), (noexcept));

  void* dummyMemory{nullptr};

 protected:
  bool m_mockCallsEnabled{false};
};

class MemoryProviderMock final : public MemoryProviderTestImpl {
 public:
  MemoryProviderMock() : MemoryProviderTestImpl() { m_mockCallsEnabled = true; }
};