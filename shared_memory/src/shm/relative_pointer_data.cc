#include "shm/relative_pointer_data.hpp"

#include <type_traits>

namespace shm {
namespace details {

// This is necessary if an supervising application needs to do a cleanup of
// resources hold by a crashed application. If the size is larger than 8 bytes
// on a 64 bit system, torn writes happens and if the application crashes at the
// wrong time, the supervisor reads corrupt data.
static_assert(
    sizeof(RelativePointerData) <=
        RelativePointerData::MAX_ALLOWED_SIZE_OF_RELATIVE_POINTER_DATA,
    "The RelativePointerData size must not exceed 64 bit!");
// This ensures that the address of the RelativePointerData object is
// appropriately aligned to be accessed within one CPU cycle, i.e. if the size
// is 8 and the alignment is 4 it could be placed at an address with modulo 4
// which would also result in torn writes.
static_assert((sizeof(RelativePointerData)) == (alignof(RelativePointerData)),
              "A RelativePointerData must be placed on an address which does "
              "not cross the native alignment!");
// This is important for the use in the SOFI where under some conditions the
// copy operation could work on partially obsolet data and therefore non-trivial
// copy ctor/assignment operator or dtor would work on corrupted data.
static_assert(std::is_trivially_copyable<RelativePointerData>::value,
              "The RelativePointerData must be trivially copyable!");

constexpr RelativePointerData::identifier_t RelativePointerData::ID_RANGE;
constexpr RelativePointerData::identifier_t
    RelativePointerData::NULL_POINTER_ID;
constexpr RelativePointerData::identifier_t RelativePointerData::MAX_VALID_ID;
constexpr RelativePointerData::offset_t RelativePointerData::OFFSET_RANGE;
constexpr RelativePointerData::offset_t
    RelativePointerData::NULL_POINTER_OFFSET;
constexpr RelativePointerData::offset_t RelativePointerData::MAX_VALID_OFFSET;
constexpr uint64_t
    RelativePointerData::MAX_ALLOWED_SIZE_OF_RELATIVE_POINTER_DATA;
constexpr uint64_t RelativePointerData::ID_BIT_SIZE;
constexpr RelativePointerData::offset_t RelativePointerData::LOGICAL_NULLPTR;

RelativePointerData::identifier_t RelativePointerData::id() const noexcept {
  return static_cast<identifier_t>(m_idAndOffset & ID_RANGE);
}

// AXIVION Next Construct AutosarC++19_03-A12.1.2 : NSDMI with null value is
// more explicit
// constexpr RelativePointerData::RelativePointerData(identifier_t id,
//                                                    offset_t offset) noexcept
//     : m_idAndOffset(static_cast<uint64_t>(id) | (offset << ID_BIT_SIZE)) {
//   if ((id > MAX_VALID_ID) || (offset > MAX_VALID_OFFSET)) {
//     m_idAndOffset = LOGICAL_NULLPTR;
//   }
// }

RelativePointerData::offset_t RelativePointerData::offset() const noexcept {
  // extract offset by removing id (first 16 bits)
  return (m_idAndOffset >> ID_BIT_SIZE) & OFFSET_RANGE;
}

void RelativePointerData::reset() noexcept {
  this->m_idAndOffset = LOGICAL_NULLPTR;
}

bool RelativePointerData::isLogicalNullptr() const noexcept {
  return m_idAndOffset == LOGICAL_NULLPTR;
}

}  // namespace details
}  // namespace shm