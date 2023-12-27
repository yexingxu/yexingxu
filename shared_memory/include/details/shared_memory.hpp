

#pragma once

#include <cstdint>
#include <string>

#include "details/expected.hpp"
#include "details/file_access.hpp"
#include "details/file_management_interface.hpp"
namespace shm {

/// @brief Shared memory file descriptor type
using shm_handle_t = int;

namespace details {
enum class SharedMemoryError {
  EMPTY_NAME,
  INVALID_FILE_NAME,
  INSUFFICIENT_PERMISSIONS,
  DOES_EXIST,
  PROCESS_LIMIT_OF_OPEN_FILES_REACHED,
  SYSTEM_LIMIT_OF_OPEN_FILES_REACHED,
  DOES_NOT_EXIST,
  NOT_ENOUGH_MEMORY_AVAILABLE,
  REQUESTED_MEMORY_EXCEEDS_MAXIMUM_FILE_SIZE,
  PATH_IS_A_DIRECTORY,
  TOO_MANY_SYMBOLIC_LINKS,
  NO_FILE_RESIZE_SUPPORT,
  NO_RESIZE_SUPPORT,
  INVALID_FILEDESCRIPTOR,
  INCOMPATIBLE_OPEN_AND_ACCESS_MODE,
  UNKNOWN_ERROR
};

class SharedMemory : public FileManagementInterface<SharedMemory> {
 public:
  //   static constexpr uint64_t NAME_SIZE = platform::IOX_MAX_SHM_NAME_LENGTH;
  static constexpr int INVALID_HANDLE = -1;
  using Name_t = std::string;

  SharedMemory(const SharedMemory&) = delete;
  SharedMemory& operator=(const SharedMemory&) = delete;
  SharedMemory(SharedMemory&&) noexcept;
  SharedMemory& operator=(SharedMemory&&) noexcept;
  ~SharedMemory() noexcept;

  /// @brief returns the file handle of the shared memory
  shm_handle_t getHandle() const noexcept;

  /// @brief this class has the ownership of the shared memory when the shared
  ///        memory was created by this class. This is the case when this class
  ///        was successful created with EXCLUSIVE_CREATE, PURGE_AND_CREATE or
  ///        OPEN_OR_CREATE and the shared memory was created. If an already
  ///        available shared memory is opened then this class does not have the
  ///        ownership.
  bool hasOwnership() const noexcept;

  /// @brief removes shared memory with a given name from the system
  /// @param[in] name name of the shared memory
  /// @return true if the shared memory was removed, false if the shared memory
  /// did not exist and
  ///         SharedMemoryError when the underlying shm_unlink call failed.
  static tl::expected<bool, SharedMemoryError> unlinkIfExist(
      const Name_t& name) noexcept;

  friend class SharedMemoryBuilder;

 private:
  SharedMemory(const Name_t& name, const shm_handle_t handle,
               const bool hasOwnership) noexcept;

  bool unlink() noexcept;
  bool close() noexcept;
  void destroy() noexcept;
  void reset() noexcept;

  static SharedMemoryError errnoToEnum(const int32_t errnum) noexcept;
  friend struct FileManagementInterface<SharedMemory>;
  shm_handle_t get_file_handle() const noexcept;

  Name_t m_name;
  shm_handle_t m_handle{INVALID_HANDLE};
  bool m_hasOwnership{false};
};

class SharedMemoryBuilder {
  /// @brief A valid file name for the shared memory with the restriction that
  ///        no leading dot is allowed since it is not compatible with every
  ///        file system
  SharedMemory::Name_t m_name;

  /// @brief Defines if the memory should be mapped read only or with write
  /// access.
  ///        A read only memory section will cause a segmentation fault when
  ///        written to.
  AccessMode m_accessMode = AccessMode::READ_ONLY;

  /// @brief Defines how the shared memory is acquired
  OpenMode m_openMode = OpenMode::OPEN_EXISTING;

  /// @brief Defines the access permissions of the shared memory
  access_rights m_filePermissions = perms::none;

  /// @brief Defines the size of the shared memory
  std::uint64_t m_size;

 public:
  /// @brief creates a valid SharedMemory object. If the construction failed
  /// the expected
  ///        contains an enum value describing the error.
  /// @return expected containing SharedMemory on success otherwise
  /// SharedMemoryError
  explicit SharedMemoryBuilder(SharedMemory::Name_t& name,
                               AccessMode accessMode, OpenMode openMode,
                               access_rights& ar, std::uint64_t size)
      : m_name(name),
        m_accessMode(accessMode),
        m_openMode(openMode),
        m_filePermissions(ar),
        m_size(size) {}
  tl::expected<SharedMemory, SharedMemoryError> create() noexcept;
};

}  // namespace details
}  // namespace shm