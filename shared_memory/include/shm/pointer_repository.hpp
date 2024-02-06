
#pragma once

#include <cstdint>
#include <vector>

#include "types/optional.hpp"
namespace shm {
namespace details {

constexpr uint64_t MAX_POINTER_REPO_CAPACITY{10000U};

/// @brief Allows registration of memory segments with their start pointers and
/// size. This class is used to resolve relative pointers in the corresponding
/// address space of the application. Up to CAPACITY segments can be registered
/// with MIN_ID = 1 to MAX_ID = CAPACITY - 1 id 0 is reserved and allows
/// relative pointers to behave like normal pointers (which is equivalent to
/// measure the offset relative to 0).
template <typename id_t, typename ptr_t,
          uint64_t CAPACITY = MAX_POINTER_REPO_CAPACITY>
class PointerRepository final {
 private:
  struct Info {
    ptr_t basePtr{nullptr};
    ptr_t endPtr{nullptr};
  };

  static constexpr id_t MIN_ID{1U};
  static constexpr id_t MAX_ID{CAPACITY - 1U};

  static_assert(MAX_ID >= MIN_ID,
                "MAX_ID must be greater or equal than MIN_ID!");
  static_assert(CAPACITY >= 2, "CAPACITY must be at least 2!");

 public:
  /// @note 0 is a special purpose id and reserved
  /// id 0 is reserved to interpret the offset just as a raw pointer,
  /// i.e. its corresponding base ptr is 0
  static constexpr id_t RAW_POINTER_BEHAVIOUR_ID{0};

  /// @brief default constructor
  PointerRepository() noexcept;
  ~PointerRepository() noexcept = default;

  PointerRepository(const PointerRepository&) = delete;
  PointerRepository(PointerRepository&&) = delete;
  PointerRepository& operator=(const PointerRepository&) = delete;
  PointerRepository& operator=(PointerRepository&&) = delete;

  /// @brief registers the start pointer of the segment in another application
  /// with a specific id
  /// @param[in] id identifies the segment that the pointer should be added to
  /// @param[in] ptr is the start pointer of the segment
  /// @param[in] size is the size of the segment
  /// @return true if the registration was successful, otherwise false
  bool registerPtrWithId(const id_t id, const ptr_t ptr,
                         const uint64_t size) noexcept;

  /// @brief registers the start pointer of a segment with a specific size
  /// @param[in] ptr is the start pointer of the segment
  /// @param[in] size is the size of the segment
  /// @return the segment id to which the pointer was added wrapped in an
  /// optional, nullopt if pointer was not added
  tl::optional<id_t> registerPtr(const ptr_t ptr,
                                 const uint64_t size = 0U) noexcept;

  /// @brief unregisters the id
  /// @param[in] id is the id to be unregistered
  /// @return true if successful, otherwise false
  /// @attention the relative pointers corresponding to this id become unsafe to
  /// use
  bool unregisterPtr(const id_t id) noexcept;

  /// @brief unregisters all ids
  /// @attention the relative pointers corresponding to this id become unsafe to
  /// use
  void unregisterAll() noexcept;

  /// @brief gets the base pointer, i.e. the starting address, associated with
  /// id
  /// @param[in] id is the segment id
  /// @return the base pointer associated with the id
  ptr_t getBasePtr(const id_t id) const noexcept;

  /// @brief returns the id for a given pointer ptr
  /// @param[in] ptr is the pointer whose corresponding id is searched for
  /// @return the id the pointer was registered to
  id_t searchId(const ptr_t ptr) const noexcept;

 private:
  /// @todo iox-#1701 if required protect vector against concurrent modification
  /// whether this is required depends on the use case, we currently do not need
  /// it we control the ids, so if they are consecutive we only need a
  /// vector/array to get the address this variable exists once per application
  /// using relative pointers, and each needs to initialize it via register
  /// calls above

  std::vector<Info> m_info;
  uint64_t m_maxRegistered{0U};

  bool addPointerIfIdIsFree(const id_t id, const ptr_t ptr,
                            const uint64_t size) noexcept;
};

template <typename id_t, typename ptr_t, uint64_t CAPACITY>
inline PointerRepository<id_t, ptr_t, CAPACITY>::PointerRepository() noexcept
    : m_info(CAPACITY) {}

template <typename id_t, typename ptr_t, uint64_t CAPACITY>
inline bool PointerRepository<id_t, ptr_t, CAPACITY>::registerPtrWithId(
    const id_t id, const ptr_t ptr, const uint64_t size) noexcept {
  if (id > MAX_ID) {
    return false;
  }
  return addPointerIfIdIsFree(id, ptr, size);
}

template <typename id_t, typename ptr_t, uint64_t CAPACITY>
inline tl::optional<id_t> PointerRepository<id_t, ptr_t, CAPACITY>::registerPtr(
    const ptr_t ptr, const uint64_t size) noexcept {
  for (id_t id{1U}; id <= MAX_ID; ++id) {
    if (addPointerIfIdIsFree(id, ptr, size)) {
      return id;
    }
  }

  return tl::nullopt;
}

template <typename id_t, typename ptr_t, uint64_t CAPACITY>
inline bool PointerRepository<id_t, ptr_t, CAPACITY>::unregisterPtr(
    const id_t id) noexcept {
  if ((id <= MAX_ID) && (id >= MIN_ID)) {
    if (m_info[id].basePtr != nullptr) {
      m_info[id].basePtr = nullptr;

      /// @note do not search for next lower registered index but we could do it
      /// here
      return true;
    }
  }

  return false;
}

template <typename id_t, typename ptr_t, uint64_t CAPACITY>
inline void PointerRepository<id_t, ptr_t, CAPACITY>::unregisterAll() noexcept {
  for (auto& info : m_info) {
    info.basePtr = nullptr;
  }
  m_maxRegistered = 0U;
}

template <typename id_t, typename ptr_t, uint64_t CAPACITY>
inline ptr_t PointerRepository<id_t, ptr_t, CAPACITY>::getBasePtr(
    const id_t id) const noexcept {
  if ((id <= MAX_ID) && (id >= MIN_ID)) {
    return m_info[id].basePtr;
  }

  /// @note for id 0 nullptr is returned, meaning we will later interpret a
  /// relative pointer by casting the offset into a pointer (i.e. we measure
  /// relative to 0)

  /// @note we cannot distinguish between not registered and nullptr registered,
  /// but we do not need to
  return nullptr;
}

template <typename id_t, typename ptr_t, uint64_t CAPACITY>
inline id_t PointerRepository<id_t, ptr_t, CAPACITY>::searchId(
    const ptr_t ptr) const noexcept {
  for (id_t id{1U}; id <= m_maxRegistered; ++id) {
    // return first id where the ptr is in the corresponding interval
    // AXIVION Next Construct AutosarC++19_03-M5.14.1 : False positive.
    // vector::operator[](index) has no side-effect when index is less than
    // vector size which is guaranteed by PointerRepository design
    if ((ptr >= m_info[id].basePtr) && (ptr <= m_info[id].endPtr)) {
      return id;
    }
  }
  /// @note treat the pointer as a regular pointer if not found
  /// by setting id to RAW_POINTER_BEHAVIOUR_ID
  return RAW_POINTER_BEHAVIOUR_ID;
}
template <typename id_t, typename ptr_t, uint64_t CAPACITY>
inline bool PointerRepository<id_t, ptr_t, CAPACITY>::addPointerIfIdIsFree(
    const id_t id, const ptr_t ptr, const uint64_t size) noexcept {
  if (m_info[id].basePtr == nullptr) {
    m_info[id].basePtr = ptr;
    // AXIVION Next Construct AutosarC++19_03-M5.2.9 : Used for pointer
    // arithmetic with void pointer, uintptr_t is capable of holding a void ptr
    // AXIVION Next Construct AutosarC++19_03-A5.2.4 : Cast is needed for
    // pointer arithmetic and casted back to the original type
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    m_info[id].endPtr =
        reinterpret_cast<ptr_t>(reinterpret_cast<uintptr_t>(ptr) + (size - 1U));

    if (id > m_maxRegistered) {
      m_maxRegistered = id;
    }
    return true;
  }
  return false;
}

}  // namespace details
}  // namespace shm