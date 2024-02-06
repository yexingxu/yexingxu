

#include "shared_memory_user.hpp"

#include <spdlog/spdlog.h>

#include <cstdint>
#include <string>

#include "memory/segment_manager.hpp"
#include "shm/new_type.hpp"
#include "shm/relative_pointer.hpp"
#include "shm/user.hpp"

namespace {
constexpr const std::uint32_t MAX_SHM_SEGMENTS = 10U;
}

namespace shm {

SharedMemoryUser::SharedMemoryUser(
    const size_t dataSize, const uint64_t segmentId,
    const details::UntypedRelativePointer::offset_t
        segmentManagerAddressOffset) noexcept {
  auto res = details::SharedMemoryObjectBuilder(
                 "chh", dataSize, AccessMode::READ_WRITE,
                 OpenMode::OPEN_EXISTING, SHM_SEGMENT_PERMISSIONS)
                 .create();

  if (res.has_value()) {
    bool registeredSuccessfully = false;
    auto& sharedMemoryObject = res.value();
    if (sharedMemoryObject.get_size().has_value()) {
      registeredSuccessfully =
          details::UntypedRelativePointer::registerPtrWithId(
              details::segment_id_t{segmentId},
              sharedMemoryObject.getBaseAddress(),
              sharedMemoryObject.get_size().value());
    }

    if (!registeredSuccessfully) {
      // TODO error handler
      spdlog::error("Application registered management segment " +
                    std::to_string(*reinterpret_cast<const uint64_t*>(
                        sharedMemoryObject.getBaseAddress())) +
                    " with size " +
                    std::to_string(sharedMemoryObject.get_size().value()) +
                    "Failed to acquire SHM size." + " to id " +
                    std::to_string(segmentId));
    }

    this->OpenDataSegments(segmentId, segmentManagerAddressOffset);

    shmObject_.emplace(std::move(sharedMemoryObject));
  } else {
    spdlog::error("Application registered management segment ");
  }
}

void SharedMemoryUser::OpenDataSegments(
    const uint64_t segmentId, const details::UntypedRelativePointer::offset_t
                                  segmentManagerAddressOffset) noexcept {
  auto* ptr = details::UntypedRelativePointer::getPtr(
      details::segment_id_t{segmentId}, segmentManagerAddressOffset);
  auto* segmentManager = static_cast<details::SegmentManager<>*>(ptr);

  auto segmentMapping = segmentManager->getSegmentMappings(
      details::User::getUserOfCurrentProcess());
  for (const auto& segment : segmentMapping) {
    auto accessMode =
        segment.m_isWritable ? AccessMode::READ_WRITE : AccessMode::READ_ONLY;
    auto res = details::SharedMemoryObjectBuilder(
                   segment.m_sharedMemoryName, segment.m_size, accessMode,
                   OpenMode::OPEN_EXISTING, SHM_SEGMENT_PERMISSIONS)
                   .create();

    if (res.has_value()) {
      auto& sharedMemoryObject = res.value();
      if (static_cast<uint32_t>(dataShmObjects_.size()) >= MAX_SHM_SEGMENTS) {
        // errorHandler(PoshError::POSH__SHM_APP_SEGMENT_COUNT_OVERFLOW);
        spdlog::error("SHM_APP_SEGMENT_COUNT_OVERFLOW");
        return;
      }

      auto registeredSuccessfully =
          details::UntypedRelativePointer::registerPtrWithId(
              details::segment_id_t{segment.m_segmentId},
              sharedMemoryObject.getBaseAddress(),
              sharedMemoryObject.get_size().value());

      if (!registeredSuccessfully) {
        spdlog::error("Application registered management segment " +
                      std::to_string(*reinterpret_cast<const uint64_t*>(
                          sharedMemoryObject.getBaseAddress())) +
                      " with size " +
                      std::to_string(sharedMemoryObject.get_size().value()) +
                      "Failed to acquire SHM size." + " to id " +
                      std::to_string(segmentId));
      }
      dataShmObjects_.emplace_back(std::move(sharedMemoryObject));
    } else {
      spdlog::error("Application registered management segment ");
    }
  }
}

}  // namespace shm