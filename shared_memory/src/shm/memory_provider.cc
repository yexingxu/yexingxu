

#include "shm/memory_provider.hpp"

#include <spdlog/spdlog.h>

#include <string>

#include "memory/bump_allocator.hpp"
#include "shm/memory.hpp"
#include "shm/memory_block.hpp"
#include "shm/relative_pointer.hpp"
#include "types/expected.hpp"

namespace shm {
namespace details {

MemoryProvider::~MemoryProvider() noexcept {
  // destroy has to be called manually from outside, since it calls a pure
  // virtual function
}

tl::expected<void, MemoryProviderError> MemoryProvider::addMemoryBlock(
    not_null<MemoryBlock*> memoryBlock) noexcept {
  if (isAvailable()) {
    return tl::make_unexpected(MemoryProviderError::MEMORY_ALREADY_CREATED);
  }

  if (m_memoryBlocks.size() >=
      MAX_NUMBER_OF_MEMORY_BLOCKS_PER_MEMORY_PROVIDER) {
    return tl::make_unexpected(MemoryProviderError::MEMORY_BLOCKS_EXHAUSTED);
  }

  m_memoryBlocks.push_back(memoryBlock);
  return {};
}

tl::expected<void, MemoryProviderError> MemoryProvider::create() noexcept {
  if (m_memoryBlocks.empty()) {
    return tl::make_unexpected(MemoryProviderError::NO_MEMORY_BLOCKS_PRESENT);
  }

  if (isAvailable()) {
    return tl::make_unexpected(MemoryProviderError::MEMORY_ALREADY_CREATED);
  }

  uint64_t totalSize = 0u;
  uint64_t maxAlignment = 1;
  for (auto* memoryBlock : m_memoryBlocks) {
    auto alignment = memoryBlock->alignment();
    if (alignment > maxAlignment) {
      maxAlignment = alignment;
    }

    // just in case the memory block doesn't calculate its size as multiple of
    // the alignment this shouldn't be necessary, but also doesn't harm
    auto size = align(memoryBlock->size(), alignment);
    totalSize = align(totalSize, alignment) + size;
  }

  auto memoryResult = createMemory(totalSize, maxAlignment);

  if (!memoryResult.has_value()) {
    return tl::make_unexpected(memoryResult.error());
  }

  m_memory = memoryResult.value();
  m_size = totalSize;
  auto maybeSegmentId = UntypedRelativePointer::registerPtr(m_memory, m_size);

  if (!maybeSegmentId.has_value()) {
    // TODO
    // errorHandler(PoshError::MEMORY_PROVIDER__INSUFFICIENT_SEGMENT_IDS);
  }
  m_segmentId = maybeSegmentId.value();

  spdlog::debug("Registered memory segment with size " +
                std::to_string(m_size) + " to id " +
                std::to_string(m_segmentId));

  memory::BumpAllocator allocator(m_memory, m_size);

  for (auto* memoryBlock : m_memoryBlocks) {
    auto allocationResult =
        allocator.allocate(memoryBlock->size(), memoryBlock->alignment());
    if (!allocationResult.has_value()) {
      return tl::make_unexpected(MemoryProviderError::MEMORY_ALLOCATION_FAILED);
    }
    memoryBlock->m_memory = allocationResult.value();
  }

  return {};
}

tl::expected<void, MemoryProviderError> MemoryProvider::destroy() noexcept {
  if (!isAvailable()) {
    return tl::make_unexpected(MemoryProviderError::MEMORY_NOT_AVAILABLE);
  }

  for (auto* memoryBlock : m_memoryBlocks) {
    memoryBlock->destroy();
  }

  auto destructionResult = destroyMemory();

  if (destructionResult.has_value()) {
    UntypedRelativePointer::unregisterPtr(segment_id_t{m_segmentId});
    m_memory = nullptr;
    m_size = 0U;
  }

  return destructionResult;
}

tl::optional<void*> MemoryProvider::baseAddress() const noexcept {
  return isAvailable() ? tl::make_optional<void*>(m_memory) : tl::nullopt;
}

uint64_t MemoryProvider::size() const noexcept { return m_size; }

tl::optional<uint64_t> MemoryProvider::segmentId() const noexcept {
  return isAvailable() ? tl::make_optional<uint64_t>(m_segmentId) : tl::nullopt;
}

void MemoryProvider::announceMemoryAvailable() noexcept {
  if (!m_memoryAvailableAnnounced) {
    for (auto memoryBlock : m_memoryBlocks) {
      memoryBlock->onMemoryAvailable(memoryBlock->m_memory);
    }

    m_memoryAvailableAnnounced = true;
  }
}

bool MemoryProvider::isAvailable() const noexcept {
  return m_memory != nullptr;
}

bool MemoryProvider::isAvailableAnnounced() const noexcept {
  return m_memoryAvailableAnnounced;
}

const char* MemoryProvider::getErrorString(
    const MemoryProviderError error) noexcept {
  switch (error) {
    case MemoryProviderError::MEMORY_BLOCKS_EXHAUSTED:
      return "MEMORY_BLOCKS_EXHAUSTED";
    case MemoryProviderError::NO_MEMORY_BLOCKS_PRESENT:
      return "NO_MEMORY_BLOCKS_PRESENT";
    case MemoryProviderError::MEMORY_ALREADY_CREATED:
      return "MEMORY_ALREADY_CREATED";
    case MemoryProviderError::MEMORY_CREATION_FAILED:
      return "MEMORY_CREATION_FAILED";
    case MemoryProviderError::MEMORY_ALIGNMENT_EXCEEDS_PAGE_SIZE:
      return "MEMORY_ALIGNMENT_EXCEEDS_PAGE_SIZE";
    case MemoryProviderError::MEMORY_ALLOCATION_FAILED:
      return "MEMORY_ALLOCATION_FAILED";
    case MemoryProviderError::MEMORY_MAPPING_FAILED:
      return "MEMORY_MAPPING_FAILED";
    case MemoryProviderError::MEMORY_NOT_AVAILABLE:
      return "MEMORY_NOT_AVAILABLE";
    case MemoryProviderError::MEMORY_DESTRUCTION_FAILED:
      return "MEMORY_DESTRUCTION_FAILED";
    case MemoryProviderError::MEMORY_DEALLOCATION_FAILED:
      return "MEMORY_DEALLOCATION_FAILED";
    case MemoryProviderError::MEMORY_UNMAPPING_FAILED:
      return "MEMORY_UNMAPPING_FAILED";
    case MemoryProviderError::SIGACTION_CALL_FAILED:
      return "SIGACTION_CALL_FAILED";
    default:
      return "UNDEFINED";
  }

  // this will actually never be reached, but the compiler issues a warning
  return "UNDEFINED";
}

}  // namespace details
}  // namespace shm