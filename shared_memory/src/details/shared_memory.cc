/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-12-14 18:10:45
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-12-15 15:22:28
 */
#include "details/shared_memory.hpp"

#include <spdlog/spdlog.h>
#include <sys/mman.h>

#include "details/scope_guard.hpp"
#include "details/utils.h"

namespace shm {
namespace details {

tl::expected<SharedMemory, SharedMemoryError>
SharedMemoryBuilder::create() noexcept {
  auto printError = [this] {
    spdlog::error(
        "Unable to create shared memory with the following properties [ name "
        "= " +
        m_name + ", access mode = " + asStringLiteral(m_accessMode) +
        ", open mode = " + asStringLiteral(m_openMode) +
        ", mode = " + std::to_string(m_filePermissions.value()) +
        ", sizeInBytes = " + std::to_string(m_size) + " ]");
  };

  // on qnx the current working directory will be added to the /dev/shmem path
  // if the leading slash is missing
  if (m_name.empty()) {
    spdlog::error("No shared memory name specified!");
    return tl::make_unexpected(SharedMemoryError::EMPTY_NAME);
  }

  if (!isValidFileName(m_name)) {
    spdlog::error(
        "Shared memory requires a valid file name (not path) as name and \"" +
        m_name + "\" is not a valid file name");
    return tl::make_unexpected(SharedMemoryError::INVALID_FILE_NAME);
  }

  auto nameWithLeadingSlash = addLeadingSlash(m_name);

  bool hasOwnership = (m_openMode == OpenMode::EXCLUSIVE_CREATE ||
                       m_openMode == OpenMode::PURGE_AND_CREATE ||
                       m_openMode == OpenMode::OPEN_OR_CREATE);

  if (hasOwnership && (m_accessMode == AccessMode::READ_ONLY)) {
    spdlog::error("Cannot create shared-memory file \"" + m_name +
                  "\" in read-only mode. " +
                  "Initializing a new file requires write access");
    return tl::make_unexpected(
        SharedMemoryError::INCOMPATIBLE_OPEN_AND_ACCESS_MODE);
  }

  // the mask will be applied to the permissions, therefore we need to set it to
  // 0
  shm_handle_t sharedMemoryFileHandle = SharedMemory::INVALID_HANDLE;
  mode_t umaskSaved = umask(0U);
  {
    ScopeGuard umaskGuard([&] { umask(umaskSaved); });

    if (m_openMode == OpenMode::PURGE_AND_CREATE) {
      auto ret = shm_unlink(nameWithLeadingSlash.c_str());
      if (ret == SharedMemory::INVALID_HANDLE) {
        spdlog::warn("unlink " + nameWithLeadingSlash + " failed");
      }
    }
    auto open_res = shm_open(
        nameWithLeadingSlash.c_str(),
        convertToOflags(m_accessMode, (m_openMode == OpenMode::OPEN_OR_CREATE)
                                          ? OpenMode::EXCLUSIVE_CREATE
                                          : m_openMode),
        m_filePermissions.value());
    if (open_res == SharedMemory::INVALID_HANDLE) {
      // if it was not possible to create the shm exclusively someone else has
      // the ownership and we just try to open it
      spdlog::warn("open " + nameWithLeadingSlash + " failed");
      if (m_openMode == OpenMode::OPEN_OR_CREATE && errno == EEXIST) {
        hasOwnership = false;
        open_res =
            shm_open(nameWithLeadingSlash.c_str(),
                     convertToOflags(m_accessMode, OpenMode::OPEN_EXISTING),
                     m_filePermissions.value());
        if (open_res == SharedMemory::INVALID_HANDLE) {
          printError();
          return tl::make_unexpected(SharedMemory::errnoToEnum(errno));
        }
      }
    }

    sharedMemoryFileHandle = open_res;
  }

  if (hasOwnership) {
    auto res = ftruncate(sharedMemoryFileHandle, static_cast<int64_t>(m_size));
    if (res == SharedMemory::INVALID_HANDLE) {
      printError();
      auto closed = close(sharedMemoryFileHandle);
      if (closed == SharedMemory::INVALID_HANDLE) {
        spdlog::error(
            "Unable to close filedescriptor (close failed) for SharedMemory "
            "\"" +
            m_name + "\"");
      }
      auto ret = shm_unlink(nameWithLeadingSlash.c_str());
      if (ret == SharedMemory::INVALID_HANDLE) {
        spdlog::error("Unable to remove previously created SharedMemory \"" +
                      m_name + "\". This may be a SharedMemory leak.");
      }
      return tl::make_unexpected(SharedMemory::errnoToEnum(errno));
    }
  }

  return SharedMemory(m_name, sharedMemoryFileHandle, hasOwnership);
}

SharedMemory::SharedMemory(const Name_t& name, const shm_handle_t handle,
                           const bool hasOwnership) noexcept
    : m_name{name}, m_handle{handle}, m_hasOwnership{hasOwnership} {}

SharedMemory::~SharedMemory() noexcept { destroy(); }

void SharedMemory::destroy() noexcept {
  close();
  unlink();
}

void SharedMemory::reset() noexcept {
  m_hasOwnership = false;
  m_name = Name_t();
  m_handle = INVALID_HANDLE;
}

SharedMemory::SharedMemory(SharedMemory&& rhs) noexcept {
  *this = std::move(rhs);
}

SharedMemory& SharedMemory::operator=(SharedMemory&& rhs) noexcept {
  if (this != &rhs) {
    destroy();

    m_name = rhs.m_name;
    m_hasOwnership = rhs.m_hasOwnership;
    m_handle = rhs.m_handle;

    rhs.reset();
  }
  return *this;
}

shm_handle_t SharedMemory::getHandle() const noexcept { return m_handle; }

shm_handle_t SharedMemory::get_file_handle() const noexcept { return m_handle; }

bool SharedMemory::hasOwnership() const noexcept { return m_hasOwnership; }

tl::expected<bool, SharedMemoryError> SharedMemory::unlinkIfExist(
    const Name_t& name) noexcept {
  auto nameWithLeadingSlash = addLeadingSlash(name);
  auto res = shm_unlink(nameWithLeadingSlash.c_str());
  if (res == INVALID_HANDLE) {
    return tl::make_unexpected(errnoToEnum(ENOENT));
  }

  return true;
}

bool SharedMemory::unlink() noexcept {
  if (m_hasOwnership) {
    auto unlinkResult = unlinkIfExist(m_name);
    if (!unlinkResult.has_value() || !unlinkResult.value()) {
      spdlog::error("Unable to unlink SharedMemory (shm_unlink failed).");
      return false;
    }
    m_hasOwnership = false;
  }

  reset();
  return true;
}

bool SharedMemory::close() noexcept {
  if (m_handle != INVALID_HANDLE) {
    auto res = ::close(m_handle);
    if (res == INVALID_HANDLE) {
      spdlog::error(
          "Unable to close SharedMemory filedescriptor (close failed)");
    }
    m_handle = INVALID_HANDLE;
    return false;
  }
  return true;
}

// NOLINTJUSTIFICATION the function size and cognitive complexity results from
// the error handling and the expanded log macro
// NOLINTNEXTLINE(readability-function-size,readability-function-cognitive-complexity)
SharedMemoryError SharedMemory::errnoToEnum(const int32_t errnum) noexcept {
  switch (errnum) {
    case EACCES:
      spdlog::error(
          "No permission to modify, truncate or access the shared memory!");
      return SharedMemoryError::INSUFFICIENT_PERMISSIONS;
    case EPERM:
      spdlog::error(
          "Resizing a file beyond its current size is not supported by the "
          "filesystem!");
      return SharedMemoryError::NO_RESIZE_SUPPORT;
    case EFBIG:
      spdlog::error(
          "Requested Shared Memory is larger then the maximum file size.");
      return SharedMemoryError::REQUESTED_MEMORY_EXCEEDS_MAXIMUM_FILE_SIZE;
    case EINVAL:
      spdlog::error(
          "Requested Shared Memory is larger then the maximum file size or "
          "the filedescriptor does not "
          "belong to a regular file.");
      return SharedMemoryError::REQUESTED_MEMORY_EXCEEDS_MAXIMUM_FILE_SIZE;
    case EBADF:
      spdlog::error("Provided filedescriptor is not a valid filedescriptor.");
      return SharedMemoryError::INVALID_FILEDESCRIPTOR;
    case EEXIST:
      spdlog::error("A Shared Memory with the given name already exists.");
      return SharedMemoryError::DOES_EXIST;
    case EISDIR:
      spdlog::error("The requested Shared Memory file is a directory.");
      return SharedMemoryError::PATH_IS_A_DIRECTORY;
    case ELOOP:
      spdlog::error(
          "Too many symbolic links encountered while traversing the path.");
      return SharedMemoryError::TOO_MANY_SYMBOLIC_LINKS;
    case EMFILE:
      spdlog::error("Process limit of maximum open files reached.");
      return SharedMemoryError::PROCESS_LIMIT_OF_OPEN_FILES_REACHED;
    case ENFILE:
      spdlog::error("System limit of maximum open files reached.");
      return SharedMemoryError::SYSTEM_LIMIT_OF_OPEN_FILES_REACHED;
    case ENOENT:
      spdlog::error("Shared Memory does not exist.");
      return SharedMemoryError::DOES_NOT_EXIST;
    case ENOMEM:
      spdlog::error("Not enough memory available to create shared memory.");
      return SharedMemoryError::NOT_ENOUGH_MEMORY_AVAILABLE;
    default:
      spdlog::error("This should never happen! An unknown error occurred!");
      return SharedMemoryError::UNKNOWN_ERROR;
  }
}

}  // namespace details
}  // namespace shm