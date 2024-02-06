

#pragma once

#include "memory/bump_allocator.hpp"
#include "memory/config.hpp"
#include "memory/memory_manager.hpp"
#include "memory/segment.hpp"
#include "shm/memory_info.hpp"
#include "shm/user.hpp"

namespace shm {
namespace memory {

template <typename SegmentType = Segment<>>
class SegmentManager {
  using ShmName_t = std::string;

 public:
  SegmentManager(const Config& segmentConfig,
                 BumpAllocator* managementAllocator) noexcept;
  ~SegmentManager() noexcept = default;

  SegmentManager(const SegmentManager& rhs) = delete;
  SegmentManager(SegmentManager&& rhs) = delete;

  SegmentManager& operator=(const SegmentManager& rhs) = delete;
  SegmentManager& operator=(SegmentManager&& rhs) = delete;

  struct SegmentMapping {
   public:
    SegmentMapping(const ShmName_t& sharedMemoryName, uint64_t size,
                   bool isWritable, uint64_t segmentId,
                   const MemoryInfo& memoryInfo = MemoryInfo()) noexcept
        : m_sharedMemoryName(sharedMemoryName),
          m_size(size),
          m_isWritable(isWritable),
          m_segmentId(segmentId),
          m_memoryInfo(memoryInfo)

    {}

    ShmName_t m_sharedMemoryName{""};
    uint64_t m_size{0};
    bool m_isWritable{false};
    uint64_t m_segmentId{0};
    MemoryInfo m_memoryInfo;  // we can specify additional info
                              // about a segments memory here
  };

  struct SegmentUserInformation {
    tl::optional<std::reference_wrapper<MemoryManager>> m_memoryManager;
    uint64_t m_segmentID;
  };

  using SegmentMappingContainer = std::vector<SegmentMapping>;

  SegmentMappingContainer getSegmentMappings(const User& user) noexcept;
  SegmentUserInformation getSegmentInformationWithWriteAccessForUser(
      const User& user) noexcept;

  static uint64_t requiredManagementMemorySize(const Config& config) noexcept;
  static uint64_t requiredChunkMemorySize(const Config& config) noexcept;
  static uint64_t requiredFullMemorySize(const Config& config) noexcept;

 private:
  void createSegment(const Config::Entry& segmentEntry) noexcept;

 private:
  //   template <typename MemoryManger, typename SegmentManager,
  //             typename PublisherPort>
  //   friend class roudi::MemPoolIntrospection;

  BumpAllocator* m_managementAllocator;
  std::vector<SegmentType> m_segmentContainer;
  bool m_createInterfaceEnabled{true};
};

}  // namespace memory
}  // namespace shm