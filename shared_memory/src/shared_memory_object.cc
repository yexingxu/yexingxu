

#include "shared_memory_object.hpp"

#include <spdlog/spdlog.h>

#include <sstream>
#include <string>

#include "details/signal_handle.hpp"
#include "details/utils.h"

namespace shm {

namespace {
constexpr uint64_t MAX_SHM_NAME_LENGTH = 4096U;
constexpr uint64_t SIGBUS_ERROR_MESSAGE_LENGTH = 1024U + MAX_SHM_NAME_LENGTH;

/// NOLINTJUSTIFICATION global variables are only accessible from within this
/// compilation unit
/// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
///
/// NOLINTJUSTIFICATION c array required to print a signal safe error message in
/// memsetSigbusHandler
/// NOLINTNEXTLINE(hicpp-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
static char sigbusErrorMessage[SIGBUS_ERROR_MESSAGE_LENGTH];
constexpr bool SHM_WRITE_ZEROS_ON_CREATION = true;
static std::mutex sigbusHandlerMutex;
static void memsetSigbusHandler(int) noexcept {
  auto result =
      write(STDERR_FILENO, &sigbusErrorMessage[0],
            strnlen(&sigbusErrorMessage[0], SIGBUS_ERROR_MESSAGE_LENGTH));
  if (!result) {
    spdlog::error("memsetSigbusHandler failed");
  }

  _exit(EXIT_FAILURE);
}
}  // namespace

tl::expected<SharedMemoryObject, SharedMemoryObjectError>
SharedMemoryObjectBuilder::create() noexcept {
  auto printErrorDetails = [this] {
    auto logBaseAddressHint =
        [this](std::stringstream& stream) noexcept -> std::stringstream& {
      if (this->m_baseAddressHint) {
        stream << std::hex << m_baseAddressHint.value();
      } else {
        stream << " (no hint set)";
      }
      return stream;
    };
    std::stringstream stream;
    spdlog::error(
        "Unable to create a shared memory object with the following "
        "properties [ name = " +
        m_name + ", sizeInBytes = " + std::to_string(m_memorySizeInBytes) +
        ", access mode = " + details::asStringLiteral(m_accessMode) +
        ", open mode = " + details::asStringLiteral(m_openMode) +
        ", baseAddressHint = " + logBaseAddressHint(stream).str() +
        ", permissions = " + std::to_string(m_permissions.value()) + " ]");
  };

  auto sharedMemory =
      details::SharedMemoryBuilder(m_name, m_accessMode, m_openMode,
                                   m_permissions, m_memorySizeInBytes)
          .create();

  if (!sharedMemory) {
    printErrorDetails();
    spdlog::error(
        "Unable to create SharedMemoryObject since we could not acquire a "
        "SharedMemory resource");
    return tl::make_unexpected(
        SharedMemoryObjectError::SHARED_MEMORY_CREATION_FAILED);
  }

  const auto realSizeResult = sharedMemory->get_size();
  if (!realSizeResult) {
    printErrorDetails();
    spdlog::error(
        "Unable to create SharedMemoryObject since we could not acquire "
        "the memory size of the "
        "underlying object.");
    return tl::make_unexpected(
        SharedMemoryObjectError::UNABLE_TO_VERIFY_MEMORY_SIZE);
  }

  const auto realSize = *realSizeResult;
  if (realSize < m_memorySizeInBytes) {
    printErrorDetails();
    spdlog::error("Unable to create SharedMemoryObject since a size of " +
                  std::to_string(m_memorySizeInBytes) +
                  " was requested but the object has only a size of " +
                  std::to_string(realSize));
    return tl::make_unexpected(
        SharedMemoryObjectError::REQUESTED_SIZE_EXCEEDS_ACTUAL_SIZE);
  }

  auto hint = m_baseAddressHint ? m_baseAddressHint.value() : nullptr;

  auto memoryMap = details::MemoryMapBuilder(
                       hint, realSize, sharedMemory->getHandle(), m_accessMode,
                       details::MemoryMapFlags::SHARE_CHANGES, 0)
                       .create();

  if (!memoryMap) {
    printErrorDetails();
    spdlog::error("Failed to map created shared memory into process!");
    return tl::make_unexpected(
        SharedMemoryObjectError::MAPPING_SHARED_MEMORY_FAILED);
  }

  if (sharedMemory->hasOwnership()) {
    spdlog::debug("Trying to reserve " + std::to_string(m_memorySizeInBytes) +
                  " bytes in the shared memory [" + m_name + "]");
    if (SHM_WRITE_ZEROS_ON_CREATION) {
      // this lock is required for the case that multiple threads are creating
      // multiple shared memory objects concurrently
      std::lock_guard<std::mutex> lock(sigbusHandlerMutex);
      auto memsetSigbusGuard =
          registerSignalHandler(details::Signal::BUS, memsetSigbusHandler);
      if (!memsetSigbusGuard.has_value()) {
        printErrorDetails();
        spdlog::error(
            "Failed to temporarily override SIGBUS to safely zero the "
            "shared memory");
        return tl::make_unexpected(
            SharedMemoryObjectError::INTERNAL_LOGIC_FAILURE);
      }

      // NOLINTJUSTIFICATION snprintf required to populate char array so that it
      // can be used signal safe in
      //                     a possible signal call
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg,hicpp-vararg)
      snprintf(
          &sigbusErrorMessage[0], SIGBUS_ERROR_MESSAGE_LENGTH,
          "While setting the acquired shared memory to zero a fatal SIGBUS "
          "signal appeared caused by memset. The "
          "shared memory object with the following properties [ name = %s, "
          "sizeInBytes = %llu, access mode = %s, "
          "open mode = %s, baseAddressHint = %p, permissions = %u ] maybe "
          "requires more memory than it is "
          "currently available in the system.\n",
          m_name.c_str(), static_cast<unsigned long long>(m_memorySizeInBytes),
          details::asStringLiteral(m_accessMode),
          details::asStringLiteral(m_openMode),
          (m_baseAddressHint) ? *m_baseAddressHint : nullptr,
          m_permissions.value());

      memset(memoryMap->getBaseAddress(), 0, m_memorySizeInBytes);
    }
    spdlog::debug("Acquired " + std::to_string(m_memorySizeInBytes) +
                  " bytes successfully in the shared memory [" + m_name + "]");
  }

  return SharedMemoryObject(std::move(*sharedMemory), std::move(*memoryMap));
}

SharedMemoryObject::SharedMemoryObject(details::SharedMemory&& sharedMemory,
                                       details::MemoryMap&& memoryMap) noexcept
    : m_sharedMemory(std::move(sharedMemory)),
      m_memoryMap(std::move(memoryMap)) {}

const void* SharedMemoryObject::getBaseAddress() const noexcept {
  return m_memoryMap.getBaseAddress();
}

void* SharedMemoryObject::getBaseAddress() noexcept {
  return m_memoryMap.getBaseAddress();
}

shm_handle_t SharedMemoryObject::get_file_handle() const noexcept {
  return m_sharedMemory.getHandle();
}

shm_handle_t SharedMemoryObject::getFileHandle() const noexcept {
  return m_sharedMemory.getHandle();
}

bool SharedMemoryObject::hasOwnership() const noexcept {
  return m_sharedMemory.hasOwnership();
}

}  // namespace shm