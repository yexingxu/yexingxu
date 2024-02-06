

#include "shm/relative_pointer.hpp"
#include "shm/shared_memory_object.hpp"

namespace shm {

class SharedMemoryUser {
 public:
  /// @brief Constructor
  /// @param[in] topicSize size of the shared memory management segment
  /// @param[in] segmentManagerAddr adress of the segment manager that does the
  /// final mapping of memory in the process
  /// @param[in] segmentId of the relocatable shared memory segment
  /// address space
  SharedMemoryUser(const size_t dataSize, const uint64_t segmentId,
                   const details::UntypedRelativePointer::offset_t
                       segmentManagerAddressOffset) noexcept;

 private:
  void OpenDataSegments(const uint64_t segmentId,
                        const details::UntypedRelativePointer::offset_t
                            segmentManagerAddressOffset) noexcept;

 private:
  tl::optional<details::SharedMemoryObject> shmObject_;
  std::vector<details::SharedMemoryObject> dataShmObjects_;
  static constexpr access_rights SHM_SEGMENT_PERMISSIONS =
      perms::owner_read | perms::owner_write | perms::group_read |
      perms::group_write;
};

}  // namespace shm