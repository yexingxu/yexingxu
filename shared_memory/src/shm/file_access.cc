

#include "shm/file_access.hpp"

namespace shm {
std::ostream& operator<<(std::ostream& stream,
                         const access_rights value) noexcept {
  printAccessControl(stream, value);
  return stream;
}
}  // namespace shm