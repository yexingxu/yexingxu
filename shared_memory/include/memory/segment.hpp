

#pragma once

#include <spdlog/spdlog.h>

#include <cstdlib>
#include <string>

#include "memory/bump_allocator.hpp"
#include "memory/config.hpp"
#include "memory/memory_manager.hpp"
#include "shm/group.hpp"
#include "shm/memory_info.hpp"
#include "shm/shared_memory_object.hpp"
#include "types/optional.hpp"
#include "utils/acl.hpp"

namespace shm {
namespace memory {

template <typename SharedMemoryObjectType = SharedMemoryObject,
          typename MemoryManagerType = MemoryManager>
class Segment {
 public:
  Segment(const Config& mempoolConfig, BumpAllocator& managementAllocator,
          const Group& readerGroup, const Group& writerGroup,
          const MemoryInfo& memoryInfo = MemoryInfo()) noexcept;

  Group getWriterGroup() const noexcept;
  Group getReaderGroup() const noexcept;

  MemoryManagerType& getMemoryManager() noexcept;

  uint64_t getSegmentId() const noexcept;

  uint64_t getSegmentSize() const noexcept;

 protected:
  tl::optional<SharedMemoryObjectType> createSharedMemoryObject(
      const Config& mempoolConfig, const Group& writerGroup) noexcept;

 protected:
  Group m_readerGroup;
  Group m_writerGroup;
  uint64_t m_segmentId{0};
  uint64_t m_segmentSize{0};
  MemoryInfo m_memoryInfo;
  tl::optional<SharedMemoryObjectType> m_sharedMemoryObject;
  MemoryManagerType m_memoryManager;

  static constexpr access_rights SEGMENT_PERMISSIONS =
      perms::owner_read | perms::owner_write | perms::group_read |
      perms::group_write;
};

template <typename SharedMemoryObjectType, typename MemoryManagerType>
constexpr access_rights
    Segment<SharedMemoryObjectType, MemoryManagerType>::SEGMENT_PERMISSIONS;

template <typename SharedMemoryObjectType, typename MemoryManagerType>
inline Segment<SharedMemoryObjectType, MemoryManagerType>::Segment(
    const Config& mempoolConfig, BumpAllocator& managementAllocator,
    const Group& readerGroup, const Group& writerGroup,
    const MemoryInfo& memoryInfo) noexcept
    : m_readerGroup(readerGroup),
      m_writerGroup(writerGroup),
      m_memoryInfo(memoryInfo),
      m_sharedMemoryObject(
          createSharedMemoryObject(mempoolConfig, writerGroup)) {
  using namespace details;
  Acl acl;
  if (!(readerGroup == writerGroup)) {
    acl.addGroupPermission(Acl::Permission::READ, readerGroup.getName());
  }
  acl.addGroupPermission(Acl::Permission::READWRITE, writerGroup.getName());
  acl.addPermissionEntry(Acl::Category::USER, Acl::Permission::READWRITE);
  acl.addPermissionEntry(Acl::Category::GROUP, Acl::Permission::READWRITE);
  acl.addPermissionEntry(Acl::Category::OTHERS, Acl::Permission::NONE);
  if (m_sharedMemoryObject.has_value()) {
    if (!acl.writePermissionsToFile(
            m_sharedMemoryObject.value().getFileHandle())) {
      // TODO
      // errorHandler(
      //     PoshError::
      //         MEPOO__SEGMENT_COULD_NOT_APPLY_POSIX_RIGHTS_TO_SHARED_MEMORY);
    }
    auto resSize = m_sharedMemoryObject.value().get_size();
    if (resSize.has_value()) {
      BumpAllocator allocator(m_sharedMemoryObject.value().getBaseAddress(),
                              resSize.value());
      m_memoryManager.configureMemoryManager(mempoolConfig, managementAllocator,
                                             allocator);
    }
  }
}

template <typename SharedMemoryObjectType, typename MemoryManagerType>
inline tl::optional<SharedMemoryObjectType>
Segment<SharedMemoryObjectType, MemoryManagerType>::createSharedMemoryObject(
    const Config& mempoolConfig, const Group& writerGroup) noexcept {
  auto res = typename SharedMemoryObjectType::Builder(
                 writerGroup.getName(),
                 MemoryManager::requiredChunkMemorySize(mempoolConfig),
                 AccessMode::READ_WRITE, OpenMode::PURGE_AND_CREATE,
                 SEGMENT_PERMISSIONS)
                 .create();
  if (res.has_value()) {
    auto& sharedMemoryObject = res.value();
    auto resSize = sharedMemoryObject.get_size();
    if (!resSize.has_value()) {
      return tl::nullopt;
    }
    auto maybeSegmentId = UntypedRelativePointer::registerPtr(
        sharedMemoryObject.getBaseAddress(), resSize.value());
    if (!maybeSegmentId.has_value()) {
      // TODO
      //   errorHandler(PoshError::MEPOO__SEGMENT_INSUFFICIENT_SEGMENT_IDS);
    }
    this->m_segmentId = static_cast<uint64_t>(maybeSegmentId.value());
    if (resSize.has_value()) {
      this->m_segmentSize = resSize.value();
    }
    spdlog ::debug("Registered payload data segment with size " +
                   std::to_string(m_segmentSize) + " to id " +
                   std::to_string(m_segmentId));
    return std::move(sharedMemoryObject);
  } else {
    return tl::nullopt;
  }
}

template <typename SharedMemoryObjectType, typename MemoryManagerType>
inline Group Segment<SharedMemoryObjectType,
                     MemoryManagerType>::getWriterGroup() const noexcept {
  return m_writerGroup;
}

template <typename SharedMemoryObjectType, typename MemoryManagerType>
inline Group Segment<SharedMemoryObjectType,
                     MemoryManagerType>::getReaderGroup() const noexcept {
  return m_readerGroup;
}

template <typename SharedMemoryObjectType, typename MemoryManagerType>
inline MemoryManagerType& Segment<
    SharedMemoryObjectType, MemoryManagerType>::getMemoryManager() noexcept {
  return m_memoryManager;
}

template <typename SharedMemoryObjectType, typename MemoryManagerType>
inline uint64_t Segment<SharedMemoryObjectType,
                        MemoryManagerType>::getSegmentId() const noexcept {
  return m_segmentId;
}

template <typename SharedMemoryObjectType, typename MemoryManagerType>
inline uint64_t Segment<SharedMemoryObjectType,
                        MemoryManagerType>::getSegmentSize() const noexcept {
  return m_segmentSize;
}

}  // namespace memory
}  // namespace shm