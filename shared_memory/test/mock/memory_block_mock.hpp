

#pragma once

#include "../test.hpp"
#include "shm/algorithm.hpp"
#include "shm/memory_block.hpp"

class MemoryBlockMock final : public shm::details::MemoryBlock {
 public:
  MOCK_METHOD(uint64_t, size, (), (const, noexcept, override));
  MOCK_METHOD(uint64_t, alignment, (), (const, noexcept, override));
  MOCK_METHOD(void, onMemoryAvailable, (shm::details::not_null<void*>),
              (noexcept, override));
  MOCK_METHOD(void, destroy, (), (noexcept, override));
};