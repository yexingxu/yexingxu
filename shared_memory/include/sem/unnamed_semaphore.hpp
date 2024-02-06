#pragma once

#include "sem/semaphore_interface.hpp"
#include "types/optional.hpp"

namespace shm {
namespace details {

class UnnamedSemaphore final : public SemaphoreInterface<UnnamedSemaphore> {
 public:
  UnnamedSemaphore(const UnnamedSemaphore&) noexcept = delete;
  UnnamedSemaphore(UnnamedSemaphore&&) noexcept = delete;
  UnnamedSemaphore& operator=(const UnnamedSemaphore&) noexcept = delete;
  UnnamedSemaphore& operator=(UnnamedSemaphore&&) noexcept = delete;
  ~UnnamedSemaphore() noexcept;

 private:
  friend class UnnamedSemaphoreBuilder;
  friend class tl::optional<UnnamedSemaphore>;
  friend class SemaphoreInterface<UnnamedSemaphore>;

  UnnamedSemaphore() noexcept = default;
  sem_t* getHandle() noexcept;

  sem_t m_handle;
  bool m_destroyHandle = true;
};

class UnnamedSemaphoreBuilder {
  /// @brief Set the initial value of the unnamed posix semaphore
  uint32_t m_initialValue = 0U;

  /// @brief Set if the unnamed semaphore can be stored in the shared memory
  ///        for inter process usage
  bool m_isInterProcessCapable = true;

 public:
  /// @brief create an unnamed semaphore
  /// @param[in] uninitializedSemaphore since the semaphore is not movable the
  /// user has to provide
  ///            memory to store the semaphore into - packed in an optional
  /// @return an error describing the failure or success
  tl::expected<void, SemaphoreError> create(
      tl::optional<UnnamedSemaphore>& uninitializedSemaphore) const noexcept;
};

}  // namespace details
}  // namespace shm