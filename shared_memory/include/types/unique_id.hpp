#pragma once

#include <unistd.h>

#include "shm/new_type.hpp"

namespace shm {
namespace types {

using namespace details;

/// @brief Unique IDs within a process starting with 1. Monotonic increasing IDs
/// are created with each call to the constructor. The IDs are copy/move
/// constructible/assignable, comparable, sortable and convertable to the
/// underlying value type.
class UniqueId
    : public NewType<UniqueId, uint64_t, ProtectedConstructByValueCopy,
                     Comparable, Sortable, Convertable, CopyConstructable,
                     MoveConstructable, CopyAssignable, MoveAssignable> {
 public:
  using ThisType::ThisType;

  /// @brief the constructor creates an ID which is greater than the previous
  /// created ID
  UniqueId() noexcept;

 private:
  /// @NOLINTJUSTIFICATION only accessible by this class. the global variable is
  /// required to
  ///                      generate a unique id from it incrementing value
  /// @NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
  static std::atomic<value_type>
      m_IdCounter;  // initialized in corresponding cpp file
};

}  // namespace types
}  // namespace shm