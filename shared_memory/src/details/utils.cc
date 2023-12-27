/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-12-15 10:22:22
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-12-15 14:59:42
 */

#include "details/utils.h"

#include <spdlog/spdlog.h>

namespace shm {
namespace details {

std::string addLeadingSlash(const std::string& name) noexcept {
  std::string nameWithLeadingSlash = "/";
  nameWithLeadingSlash.append(name);
  return nameWithLeadingSlash;
}

int convertToOflags(const AccessMode accessMode) noexcept {
  switch (accessMode) {
    case AccessMode::READ_ONLY:
      return O_RDONLY;
    case AccessMode::READ_WRITE:
      return O_RDWR;
    case AccessMode::WRITE_ONLY:
      return O_WRONLY;
    default:
      spdlog::error(
          "Unable to convert to O_ flag since an undefined iox::AccessMode was "
          "provided");
      return 0;
  }
}

int convertToOflags(const OpenMode openMode) noexcept {
  switch (openMode) {
    case OpenMode::OPEN_EXISTING:
      return 0;
    case OpenMode::OPEN_OR_CREATE:
      return O_CREAT;
    case OpenMode::EXCLUSIVE_CREATE:
    case OpenMode::PURGE_AND_CREATE:
      // wrapped inside function so that the user does not have to use bitwise
      // operations; operands have positive values and result is within integer
      // range NOLINTNEXTLINE(hicpp-signed-bitwise)
      return O_CREAT | O_EXCL;
    default:
      spdlog::error(
          "Unable to convert to O_ flag since an undefined iox::OpenMode was "
          "provided");
      return 0;
  }
}

int convertToProtFlags(const AccessMode accessMode) noexcept {
  switch (accessMode) {
    case AccessMode::READ_ONLY:
      return PROT_READ;
    case AccessMode::READ_WRITE:
      // NOLINTNEXTLINE(hicpp-signed-bitwise) enum type is defined by POSIX, no
      // logical fault
      return PROT_READ | PROT_WRITE;
    case AccessMode::WRITE_ONLY:
      return PROT_WRITE;
    default:
      spdlog::error(
          "Unable to convert to PROT_ flag since an undefined iox::AccessMode "
          "was provided");
      return PROT_NONE;
  }
}

int convertToOflags(const AccessMode accessMode,
                    const OpenMode openMode) noexcept {
  // wrapped inside function so that the user does not have to use bitwise
  // operations; operands have positive values and result is within integer
  // range NOLINTNEXTLINE(hicpp-signed-bitwise)
  return convertToOflags(accessMode) | convertToOflags((openMode));
}

bool isValidPathEntry(
    const std::string& name,
    const RelativePathComponents relativePathComponents) noexcept {
  const std::string currentDirectory{"."};
  const std::string parentDirectory{".."};

  if ((name == currentDirectory) || (name == parentDirectory)) {
    return relativePathComponents == RelativePathComponents::ACCEPT;
  }

  const auto nameSize = name.size();

  for (uint64_t i{0}; i < nameSize; ++i) {
    // AXIVION Next Construct AutosarC++19_03-A3.9.1: Not used as an integer but
    // as actual character
    const char c{name[i]};

    // AXIVION DISABLE STYLE FaultDetection-UnusedAssignments : False positive,
    // variable IS used AXIVION DISABLE STYLE AutosarC++19_03-A0.1.1 : False
    // positive, variable IS used AXIVION DISABLE STYLE AutosarC++19_03-M4.5.3 :
    // We are explicitly checking for ASCII characters which have defined
    // consecutive values
    const bool isSmallLetter{(internal::ASCII_A <= c) &&
                             (c <= internal::ASCII_Z)};
    const bool isCapitalLetter{(internal::ASCII_CAPITAL_A <= c) &&
                               (c <= internal::ASCII_CAPITAL_Z)};
    const bool isNumber{(internal::ASCII_0 <= c) && (c <= internal::ASCII_9)};
    const bool isSpecialCharacter{
        ((c == internal::ASCII_DASH) || (c == internal::ASCII_DOT)) ||
        ((c == internal::ASCII_COLON) || (c == internal::ASCII_UNDERSCORE))};
    // AXIVION ENABLE STYLE AutosarC++19_03-M4.5.3
    // AXIVION ENABLE STYLE AutosarC++19_03-A0.1.1
    // AXIVION ENABLE STYLE FaultDetection-UnusedAssignments

    if ((!isSmallLetter && !isCapitalLetter) &&
        (!isNumber && !isSpecialCharacter)) {
      return false;
    }
  }

  if (nameSize == 0) {
    return true;
  }

  // dot at the end is invalid to be compatible with windows api
  return !(name[nameSize - 1] == '.');
}

bool isValidFileName(const std::string& name) noexcept {
  if (name.empty()) {
    return false;
  }

  // check if the file contains only valid characters
  return isValidPathEntry(name, RelativePathComponents::REJECT);
}

}  // namespace details
}  // namespace shm