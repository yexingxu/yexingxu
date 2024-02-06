

#include "types/unique_port_id.hpp"

#include <cstdlib>

namespace shm {
namespace types {

std::atomic<uint16_t> UniquePortId::uniqueRouDiId{0U};

// start with 1 to prevent accidentally generating an invalid ID when unique
// roudi ID is 0
std::atomic<UniquePortId::value_type> UniquePortId::globalIDCounter{1U};

UniquePortId::UniquePortId() noexcept
    : ThisType(ProtectedConstructor,
               (static_cast<UniquePortId::value_type>(getUniqueRouDiId())
                << UNIQUE_ID_BIT_LENGTH) +
                   ((globalIDCounter.fetch_add(1u, std::memory_order_relaxed)
                     << ROUDI_ID_BIT_LENGTH) >>
                    ROUDI_ID_BIT_LENGTH)) {
  UniquePortId::finalizeSetUniqueRouDiId();

  if (globalIDCounter.load() >=
      (static_cast<UniquePortId::value_type>(1u) << UNIQUE_ID_BIT_LENGTH)) {
    // TODO
    // errorHandler(PoshError::POPO__TYPED_UNIQUE_ID_OVERFLOW,
    // ErrorLevel::FATAL);
    std::exit(EXIT_FAILURE);
  }
}

UniquePortId::UniquePortId(InvalidPortId_t) noexcept
    /// we have to cast INVALID_UNIQUE_ID with static_cast<value_type> otherwise
    /// it will not link with gcc-7.x - gcc-10.x. Who knows why?!
    : ThisType(ProtectedConstructor,
               static_cast<UniquePortId::value_type>(INVALID_UNIQUE_ID)) {
  // finalizeSetUniqueRouDiId intentionally not called since the InvalidPortId
  // does not have a unique RouDi ID anyway
}

bool UniquePortId::isValid() const noexcept {
  return UniquePortId(InvalidPortId) != *this;
}

void UniquePortId::setUniqueRouDiId(const uint16_t id) noexcept {
  if (finalizeSetUniqueRouDiId()) {
    // errorHandler(
    //     PoshError::
    //         POPO__TYPED_UNIQUE_ID_ROUDI_HAS_ALREADY_DEFINED_CUSTOM_UNIQUE_ID,
    //     ErrorLevel::SEVERE);
    std::exit(EXIT_FAILURE);
  }
  uniqueRouDiId.store(id, std::memory_order_relaxed);
}

void UniquePortId::rouDiEnvOverrideUniqueRouDiId(const uint16_t id) noexcept {
  uniqueRouDiId.store(id, std::memory_order_relaxed);
}

bool UniquePortId::finalizeSetUniqueRouDiId() noexcept {
  static bool finalized{false};
  auto oldFinalized = finalized;
  finalized = true;
  return oldFinalized;
}

uint16_t UniquePortId::getUniqueRouDiId() noexcept {
  return uniqueRouDiId.load(std::memory_order_relaxed);
}

}  // namespace types
}  // namespace shm