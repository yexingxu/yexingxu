

#include "shm/memory_map.hpp"

#include <spdlog/spdlog.h>
#include <sys/mman.h>

#include <bitset>
#include <string>

#include "shm/shared_memory.hpp"
#include "shm/system_call.hpp"
#include "shm/utils.h"

namespace shm {
namespace details {

tl::expected<MemoryMap, MemoryMapError> MemoryMapBuilder::create() noexcept {
  // AXIVION Next Construct AutosarC++19_03-A5.2.3, CertC++-EXP55 :
  // Incompatibility with POSIX definition of mmap
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast) low-level memory
  // management
  auto result = SYSTEM_CALL(mmap)(const_cast<void*>(m_baseAddressHint),
                                  m_length, convertToProtFlags(m_accessMode),
                                  static_cast<int32_t>(m_flags),
                                  m_fileDescriptor, m_offset)

                    // NOLINTJUSTIFICATION cast required, type of error
                    // MAP_FAILED defined by POSIX to be void*
                    // NOLINTBEGIN(cppcoreguidelines-pro-type-cstyle-cast,
                    // performance-no-int-to-ptr)
                    .failureReturnValue(MAP_FAILED)
                    // NOLINTEND(cppcoreguidelines-pro-type-cstyle-cast,
                    // performance-no-int-to-ptr)
                    .evaluate();

  if (result) {
    return MemoryMap(result.value().value, m_length);
  }
  constexpr uint64_t FLAGS_BIT_SIZE = 32U;
  spdlog::error(
      "Unable to map memory with the following properties [ baseAddressHint "
      "= " +
      std::string(static_cast<const char*>(m_baseAddressHint)) +
      ", length = " + std::to_string(m_length) +
      ", fileDescriptor = " + std::to_string(m_fileDescriptor) +
      ", access mode = " + asStringLiteral(m_accessMode) + ", flags = " +
      std::bitset<FLAGS_BIT_SIZE>(static_cast<uint32_t>(m_flags)).to_string() +
      ", offset = " + std::to_string(m_offset) + " ]");
  return tl::make_unexpected(MemoryMap::errnoToEnum(errno));
}

MemoryMap::MemoryMap(void* const baseAddress, const uint64_t length) noexcept
    : m_baseAddress(baseAddress), m_length(length) {}

// NOLINTJUSTIFICATION the function size results from the error handling and the
// expanded log macro NOLINTNEXTLINE(readability-function-size)
MemoryMapError MemoryMap::errnoToEnum(const int32_t errnum) noexcept {
  switch (errnum) {
    case EACCES:
      spdlog::error(
          "One or more of the following failures happened:\n"
          "  1. The file descriptor belongs to a non-regular file.\n"
          "  2. The file descriptor is not opened for reading.\n"
          "  3. MAP_SHARED is requested and PROT_WRITE is set but "
          "the file descriptor is not opened for "
          "writing.\n"
          "  4. PROT_WRITE is set but the file descriptor is set to "
          "append-only.");
      return MemoryMapError::ACCESS_FAILED;
    case EAGAIN:
      spdlog::error(
          "Either too much memory has been locked or the file is already "
          "locked.");
      return MemoryMapError::UNABLE_TO_LOCK;
    case EBADF:
      spdlog::error("Invalid file descriptor provided.");
      return MemoryMapError::INVALID_FILE_DESCRIPTOR;
    case EEXIST:
      spdlog::error(
          "The mapped range that is requested is overlapping with an "
          "already mapped memory range.");
      return MemoryMapError::MAP_OVERLAP;
    case EINVAL:
      spdlog::error(
          "One or more of the following failures happened:\n"
          "  1. The address, length or the offset is not "
          "aligned on a page boundary.\n"
          "  2. The provided length is 0.\n"
          "  3. One of the flags of MAP_PRIVATE, MAP_SHARED "
          "or MAP_SHARED_VALIDATE is missing.");
      return MemoryMapError::INVALID_PARAMETERS;
    case ENFILE:
      spdlog::error("System limit of maximum open files reached");
      return MemoryMapError::OPEN_FILES_SYSTEM_LIMIT_EXCEEDED;
    case ENODEV:
      spdlog::error(
          "Memory mappings are not supported by the underlying filesystem.");
      return MemoryMapError::FILESYSTEM_DOES_NOT_SUPPORT_MEMORY_MAPPING;
    case ENOMEM:
      spdlog::error(
          "One or more of the following failures happened:\n"
          "  1. Not enough memory available.\n"
          "  2. The maximum supported number of mappings is exceeded.\n"
          "  3. Partial unmapping of an already mapped memory region "
          "dividing it into two parts.\n"
          "  4. The processes maximum size of data segments is "
          "exceeded.\n"
          "  5. The sum of the number of pages used for length and the "
          "pages used for offset would overflow "
          "and unsigned long. (only 32-bit architecture)");
      return MemoryMapError::NOT_ENOUGH_MEMORY_AVAILABLE;
    case EOVERFLOW:
      spdlog::error(
          "The sum of the number of pages and offset are overflowing. "
          "(only 32-bit architecture)");
      return MemoryMapError::OVERFLOWING_PARAMETERS;
    case EPERM:
      spdlog::error(
          "One or more of the following failures happened:\n"
          "  1. Mapping a memory region with PROT_EXEC which "
          "belongs to a filesystem that has no-exec.\n"
          "  2. The corresponding file is sealed.");
      return MemoryMapError::PERMISSION_FAILURE;
    case ETXTBSY:
      spdlog::error(
          "The memory region was set up with MAP_DENYWRITE but write "
          "access was requested.");
      return MemoryMapError::NO_WRITE_PERMISSION;
    default:
      spdlog::error("This should never happened. An unknown error occurred!\n");
      return MemoryMapError::UNKNOWN_ERROR;
  };
}

MemoryMap::MemoryMap(MemoryMap&& rhs) noexcept { *this = std::move(rhs); }

MemoryMap& MemoryMap::operator=(MemoryMap&& rhs) noexcept {
  if (this != &rhs) {
    if (!destroy()) {
      spdlog::error("move assignment failed to unmap mapped memory");
    }

    m_baseAddress = rhs.m_baseAddress;
    m_length = rhs.m_length;

    rhs.m_baseAddress = nullptr;
    rhs.m_length = 0U;
  }
  return *this;
}

MemoryMap::~MemoryMap() noexcept {
  if (!destroy()) {
    spdlog::error("destructor failed to unmap mapped memory");
  }
}

const void* MemoryMap::getBaseAddress() const noexcept { return m_baseAddress; }

void* MemoryMap::getBaseAddress() noexcept { return m_baseAddress; }

bool MemoryMap::destroy() noexcept {
  if (m_baseAddress != nullptr) {
    auto unmapResult = SYSTEM_CALL(munmap)(m_baseAddress, m_length)
                           .failureReturnValue(-1)
                           .evaluate();
    m_baseAddress = nullptr;
    m_length = 0U;

    if (!unmapResult.has_value()) {
      errnoToEnum(unmapResult.error().errnum);
      spdlog::error("unable to unmap mapped memory [ address = " +
                    std::string(static_cast<char*>(m_baseAddress)) +
                    ", size = " + std::to_string(m_length) + " ]");
      return false;
    }
  }

  return true;
}
}  // namespace details
}  // namespace shm