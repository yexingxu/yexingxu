
#pragma once

#include <fcntl.h>
#include <sys/mman.h>

#include <string>

#include "file_access.hpp"

namespace shm {

namespace details {

namespace internal {
// AXIVION DISABLE STYLE AutosarC++19_03-A3.9.1: Not used as an integer but as
// actual character.
constexpr char ASCII_A{'a'};
constexpr char ASCII_Z{'z'};
constexpr char ASCII_CAPITAL_A{'A'};
constexpr char ASCII_CAPITAL_Z{'Z'};
constexpr char ASCII_0{'0'};
constexpr char ASCII_9{'9'};
constexpr char ASCII_DASH{'-'};
constexpr char ASCII_DOT{'.'};
constexpr char ASCII_COLON{':'};
constexpr char ASCII_UNDERSCORE{'_'};
}  // namespace internal

enum class RelativePathComponents : std::uint32_t { REJECT, ACCEPT };

int convertToOflags(const AccessMode accessMode) noexcept;
int convertToOflags(const OpenMode openMode) noexcept;
int convertToProtFlags(const AccessMode accessMode) noexcept;
int convertToOflags(const AccessMode accessMode,
                    const OpenMode openMode) noexcept;

std::string addLeadingSlash(const std::string& name) noexcept;

bool isValidPathEntry(
    const std::string& name,
    const RelativePathComponents relativePathComponents) noexcept;
bool isValidFileName(const std::string& name) noexcept;

inline constexpr const char* asStringLiteral(const OpenMode mode) noexcept {
  switch (mode) {
    case OpenMode::EXCLUSIVE_CREATE:
      return "OpenMode::EXCLUSIVE_CREATE";
    case OpenMode::PURGE_AND_CREATE:
      return "OpenMode::PURGE_AND_CREATE";
    case OpenMode::OPEN_OR_CREATE:
      return "OpenMode::OPEN_OR_CREATE";
    case OpenMode::OPEN_EXISTING:
      return "OpenMode::OPEN_EXISTING";
    default:
      return "OpenMode::UNDEFINED_VALUE";
  }

  // return "OpenMode::UNDEFINED_VALUE";
}

inline constexpr const char* asStringLiteral(const AccessMode mode) noexcept {
  switch (mode) {
    case AccessMode::READ_ONLY:
      return "AccessMode::READ_ONLY";
    case AccessMode::READ_WRITE:
      return "AccessMode::READ_WRITE";
    case AccessMode::WRITE_ONLY:
      return "AccessMode::WRITE_ONLY";
    default:
      return "AccessMode::UNDEFINED_VALUE";
  }

  // return "AccessMode::UNDEFINED_VALUE";
}

}  // namespace details
}  // namespace shm