

#include "shm/algorithm.hpp"

namespace shm {
namespace details {

void Require(const bool condition, const char* file, const int line,
             const char* function, const char* conditionString) noexcept {
  if (!condition) {
    spdlog::error("Condition: " + std::string(conditionString) + " in " +
                  function + " is violated: " + ". (" + file + ":" +
                  std::to_string(line) + ")");
    std::exit(EXIT_FAILURE);
  }
}

}  // namespace details
}  // namespace shm