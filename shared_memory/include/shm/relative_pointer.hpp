
#pragma once

#include <cstdint>
#include <limits>
#include <type_traits>

#include "shm/algorithm.hpp"
#include "shm/new_type.hpp"
#include "shm/pointer_repository.hpp"
#include "types/optional.hpp"

namespace shm {
namespace details {

struct segment_id_t
    : public NewType<segment_id_t, uint64_t, DefaultConstructable,
                     CopyConstructable, Convertable, ConstructByValueCopy,
                     MoveConstructable> {
  using ThisType::ThisType;
};

using segment_id_underlying_t = typename segment_id_t::value_type;

/// @brief pointer class to use when pointer and pointee are located in
/// different shared memory segments We can have the following scenario: Pointer
/// p is stored in segment S1 and points to object X of type T in segment S2.
///
/// Shared Memory   S1:  p              S2:  X
///                      |___________________^
/// App1            a1   b1             c1   d1
/// App2            a2   b2             c2   d2
///
/// Now it is no longer true in general that both segments will be offset by the
/// same difference in App2 and therefore relocatable pointers are no longer
/// sufficient. Relative pointers solve this problem by incorporating the
/// information from where they need to measure differences (i.e. relative to
/// the given address). This requires an additional registration mechanism to be
/// used by all applications where the start addresses and the size of all
/// segments to be used are registered. Since these start address may differ
/// between applications, each segment is identified by a unique id, which can
/// be provided upon registration by the first application. In the figure, this
/// means that the starting addresses of both segments(a1, a2 and c1, c2) would
/// have to be registered in both applications. Once this registration is done,
/// relative pointers can be constructed from raw pointers similar to
/// relocatable pointers.
/// @note It should be noted that relocating a memory segment will invalidate
/// relative pointers, i.e. relative pointers are NOT relocatable. This is
/// because the registration mechanism cannot be automatically informed about
/// the copy of a whole segment, such a segment would have to be registered on
/// its own (and the original segment deregistered).
template <typename T>
class RelativePointer final {
 public:
  using ptr_t = T*;
  using offset_t = std::uintptr_t;

  /// @brief Default constructs a RelativePointer as a logical nullptr
  RelativePointer() noexcept = default;

  ~RelativePointer() noexcept = default;

  /// @brief Constructs a RelativePointer pointing to the same pointee as ptr in
  /// a segment identified by id
  /// @param[in] ptr The pointer whose pointee shall be the same for this
  /// @param[in] id Is the unique id of the segment
  RelativePointer(ptr_t const ptr, const segment_id_t id) noexcept;

  /// @brief Constructs a RelativePointer from a given offset and segment id
  /// @param[in] offset Is the offset
  /// @param[in] id Is the unique id of the segment
  RelativePointer(const offset_t offset, const segment_id_t id) noexcept;

  /// @brief Constructs a RelativePointer pointing to the same pointee as ptr
  /// @param[in] ptr The pointer whose pointee shall be the same for this
  explicit RelativePointer(ptr_t const ptr) noexcept;

  RelativePointer(const RelativePointer& other) noexcept = default;
  RelativePointer(RelativePointer&& other) noexcept;
  RelativePointer& operator=(const RelativePointer& other) noexcept;
  // AXIVION Next Line AutosarC++19_03-A3.1.6 : False positive, this is not a
  // trivial accessor/ mutator function but the move c'tor
  RelativePointer& operator=(RelativePointer&& other) noexcept;

  /// @brief Assigns the RelativePointer to point to the same pointee as ptr
  /// @param[in] ptr The pointer whose pointee shall be the same for this
  /// @return Reference to self
  RelativePointer& operator=(ptr_t const ptr) noexcept;

  /// @brief Dereferencing operator which returns a reference to the underlying
  /// object
  /// @tparam U a template parameter to enable the dereferencing operator only
  /// for non-void T
  /// @return A reference to the underlying object
  template <typename U = T>
  typename std::enable_if<!std::is_void<U>::value, const U&>::type operator*()
      const noexcept;

  /// @brief Access to the underlying object. If the RelativePointer does not
  /// point to anything the application terminates.
  /// @return A pointer to the underlying object
  T* operator->() const noexcept;

  /// @brief Access the underlying object.
  /// @return A pointer to the underlying object
  T* get() const noexcept;

  /// @brief Converts the RelativePointer to bool
  /// @return Bool which contains true if the RelativePointer contains a pointer
  // AXIVION Next Line AutosarC++19_03-A13.5.3 : Explicitly wanted to mimic the
  // behaviour of raw pointers
  explicit operator bool() const noexcept;

  /// @brief Returns the id which identifies the segment
  /// @return The id which identifies the segment
  segment_id_underlying_t getId() const noexcept;

  /// @brief Returns the offset
  /// @return The offset of the RelativePointer object
  offset_t getOffset() const noexcept;

  /// @brief Get the base pointer associated with this' id
  /// @return The registered base pointer of the RelativePointer object
  T* getBasePtr() const noexcept;

  /// @brief Tries to registers a memory segment at ptr with size to a new id
  /// @param[in] ptr Starting address of the segment to be registered
  /// @param[in] size Is the size of the segment, defaults to size 0 if argument
  /// is not provided
  /// @return segment_id to which the pointer was registered, wrapped in an
  /// optional
  static tl::optional<segment_id_underlying_t> registerPtr(
      ptr_t const ptr, const uint64_t size = 0U) noexcept;

  /// @brief Tries to register a memory segment with a given size starting at
  /// ptr to a given id
  /// @param[in] id Is the id of the segment
  /// @param[in] ptr Starting address of the segment to be registered
  /// @param[in] size Is the size of the segment
  /// @return True if successful (id not occupied), false otherwise
  static bool registerPtrWithId(const segment_id_t id, ptr_t const ptr,
                                const uint64_t size = 0U) noexcept;

  /// @brief Unregisters ptr with given id
  /// @param[in] id Is the id of the segment
  /// @return True if successful (ptr was registered with this id before), false
  /// otherwise
  static bool unregisterPtr(const segment_id_t id) noexcept;

  /// @brief Get the base ptr associated with the given id
  /// @param[in] id Is the id of the segment
  /// @return The pointer registered at the given id, nullptr if none was
  /// registered
  static T* getBasePtr(const segment_id_t id) noexcept;

  /// @brief Unregisters all ptr id pairs leading to initial state. This affects
  /// all pointer both typed and untyped.
  static void unregisterAll() noexcept;

  /// @brief Get the offset from id and ptr
  /// @param[in] id Is the id of the segment and is used to get the base pointer
  /// @param[in] ptr Is the pointer whose offset should be calculated
  /// @return The offset of the passed pointer
  static offset_t getOffset(const segment_id_t id, ptr_t const ptr) noexcept;

  /// @brief Get the pointer from id and offset ("inverse" to getOffset)
  /// @param[in] id Is the id of the segment and is used to get the base pointer
  /// @param[in] offset Is the offset for which the pointer should be calculated
  /// @return The pointer from id and offset
  static T* getPtr(const segment_id_t id, const offset_t offset) noexcept;

  /// @brief Get the id for a given ptr
  /// @param[in] ptr The pointer whose corresponding id is searched for
  /// @return segment_id to which the pointer was registered to
  static segment_id_underlying_t searchId(ptr_t const ptr) noexcept;

  /// @brief Get the offset from the start address of the segment and ptr
  /// @param[in] ptr Is the pointer whose offset should be calculated
  /// @return The offset of the passed pointer
  offset_t computeOffset(ptr_t const ptr) const noexcept;

  /// @brief Get the pointer from stored id and offset
  /// @return The pointer for stored id and offset
  T* computeRawPtr() const noexcept;

  static constexpr segment_id_underlying_t NULL_POINTER_ID{
      std::numeric_limits<segment_id_underlying_t>::max()};
  static constexpr offset_t NULL_POINTER_OFFSET{
      std::numeric_limits<offset_t>::max()};

 private:
  segment_id_underlying_t m_id{NULL_POINTER_ID};
  offset_t m_offset{NULL_POINTER_OFFSET};
};

using UntypedRelativePointer = RelativePointer<void>;

PointerRepository<segment_id_underlying_t, UntypedRelativePointer::ptr_t>&
getRepository() noexcept;

template <typename T>
// NOLINTJUSTIFICATION NewType size is comparable to an integer, hence pass by
// value is preferred NOLINTNEXTLINE(performance-unnecessary-value-param)
inline RelativePointer<T>::RelativePointer(ptr_t const ptr,
                                           const segment_id_t id) noexcept
    : RelativePointer(getOffset(id, ptr), id) {}

template <typename T>
// AXIVION Next Construct AutosarC++19_03-A12.1.5 : This is the main c'tor which
// the other c'tors use NOLINTJUSTIFICATION NewType size is comparable to an
// integer, hence pass by value is preferred
// NOLINTNEXTLINE(performance-unnecessary-value-param)
inline RelativePointer<T>::RelativePointer(const offset_t offset,
                                           const segment_id_t id) noexcept
    : m_id(id), m_offset(offset) {}

template <typename T>
inline RelativePointer<T>::RelativePointer(ptr_t const ptr) noexcept
    : RelativePointer([this, ptr]() noexcept -> RelativePointer {
        const segment_id_t id{this->searchId(ptr)};
        const offset_t offset{this->getOffset(id, ptr)};
        return RelativePointer{offset, id};
      }()) {}

template <typename T>
inline RelativePointer<T>& RelativePointer<T>::operator=(
    const RelativePointer& other) noexcept {
  if (this != &other) {
    m_id = other.m_id;
    m_offset = other.m_offset;
  }
  return *this;
}

template <typename T>
RelativePointer<T>::RelativePointer(RelativePointer&& other) noexcept
    : m_id(std::move(other.m_id)), m_offset(std::move(other.m_offset)) {
  other.m_id = NULL_POINTER_ID;
  other.m_offset = NULL_POINTER_OFFSET;
}

template <typename T>
RelativePointer<T>& RelativePointer<T>::operator=(
    RelativePointer&& other) noexcept {
  if (this != &other) {
    m_id = other.m_id;
    m_offset = other.m_offset;
    other.m_id = NULL_POINTER_ID;
    other.m_offset = NULL_POINTER_OFFSET;
  }
  return *this;
}

template <typename T>
inline RelativePointer<T>& RelativePointer<T>::operator=(
    ptr_t const ptr) noexcept {
  m_id = searchId(ptr);
  m_offset = computeOffset(ptr);

  return *this;
}

template <typename T>
template <typename U>
inline typename std::enable_if<!std::is_void<U>::value, const U&>::type
RelativePointer<T>::operator*() const noexcept {
  return *get();
}

template <typename T>
inline T* RelativePointer<T>::operator->() const noexcept {
  auto* const ptr{get()};
  ENSURES(ptr != nullptr);
  return ptr;
}

template <typename T>
inline T* RelativePointer<T>::get() const noexcept {
  return static_cast<ptr_t>(computeRawPtr());
}

template <typename T>
inline RelativePointer<T>::operator bool() const noexcept {
  return computeRawPtr() != nullptr;
}

template <typename T>
inline segment_id_underlying_t RelativePointer<T>::getId() const noexcept {
  return m_id;
}

template <typename T>
inline typename RelativePointer<T>::offset_t RelativePointer<T>::getOffset()
    const noexcept {
  return m_offset;
}

template <typename T>
inline T* RelativePointer<T>::getBasePtr() const noexcept {
  return getBasePtr(segment_id_t{m_id});
}

template <typename T>
inline tl::optional<segment_id_underlying_t> RelativePointer<T>::registerPtr(
    ptr_t const ptr, const uint64_t size) noexcept {
  return getRepository().registerPtr(ptr, size);
}

template <typename T>
// NOLINTJUSTIFICATION NewType size is comparable to an integer, hence pass by
// value is preferred NOLINTNEXTLINE(performance-unnecessary-value-param)
inline bool RelativePointer<T>::registerPtrWithId(
    const segment_id_t id, ptr_t const ptr, const uint64_t size) noexcept {
  return getRepository().registerPtrWithId(
      static_cast<segment_id_underlying_t>(id), ptr, size);
}

template <typename T>
// NOLINTJUSTIFICATION NewType size is comparable to an integer, hence pass by
// value is preferred NOLINTNEXTLINE(performance-unnecessary-value-param)
inline bool RelativePointer<T>::unregisterPtr(const segment_id_t id) noexcept {
  return getRepository().unregisterPtr(
      static_cast<segment_id_underlying_t>(id));
}

template <typename T>
// NOLINTJUSTIFICATION NewType size is comparable to an integer, hence pass by
// value is preferred NOLINTNEXTLINE(performance-unnecessary-value-param)
inline T* RelativePointer<T>::getBasePtr(const segment_id_t id) noexcept {
  // AXIVION Next Construct AutosarC++19_03-M5.2.8 : Cast to the underyling
  // pointer type is safe as this is encapsulated in the RelativePointer class
  // and type safety is ensured by using templates
  return static_cast<ptr_t>(
      getRepository().getBasePtr(static_cast<segment_id_underlying_t>(id)));
}

template <typename T>
inline void RelativePointer<T>::unregisterAll() noexcept {
  getRepository().unregisterAll();
}

template <typename T>
// NOLINTJUSTIFICATION NewType size is comparable to an integer, hence pass by
// value is preferred NOLINTNEXTLINE(performance-unnecessary-value-param)
inline typename RelativePointer<T>::offset_t RelativePointer<T>::getOffset(
    const segment_id_t id, ptr_t const ptr) noexcept {
  if (static_cast<segment_id_underlying_t>(id) == NULL_POINTER_ID) {
    return NULL_POINTER_OFFSET;
  }
  const auto* const basePtr = getBasePtr(id);
  // AXIVION Next Construct AutosarC++19_03-A5.2.4, AutosarC++19_03-M5.2.9 :
  // Cast needed for pointer arithmetic
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  return reinterpret_cast<offset_t>(ptr) - reinterpret_cast<offset_t>(basePtr);
}

template <typename T>
// NOLINTJUSTIFICATION NewType size is comparable to an integer, hence pass by
// value is preferred NOLINTNEXTLINE(performance-unnecessary-value-param)
inline T* RelativePointer<T>::getPtr(const segment_id_t id,
                                     const offset_t offset) noexcept {
  if (offset == NULL_POINTER_OFFSET) {
    return nullptr;
  }
  const auto* const basePtr = getBasePtr(id);
  // AXIVION DISABLE STYLE AutosarC++19_03-A5.2.4 : Cast needed for pointer
  // arithmetic AXIVION DISABLE STYLE AutosarC++19_03-M5.2.8 : Cast needed for
  // pointer arithmetic AXIVION DISABLE STYLE AutosarC++19_03-M5.2.9 : Cast
  // needed for pointer arithmetic
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast,
  // performance-no-int-to-ptr)
  return reinterpret_cast<ptr_t>(offset + reinterpret_cast<offset_t>(basePtr));
  // AXIVION ENABLE STYLE AutosarC++19_03-M5.2.9
  // AXIVION ENABLE STYLE AutosarC++19_03-M5.2.8
  // AXIVION ENABLE STYLE AutosarC++19_03-A5.2.4
}

template <typename T>
inline segment_id_underlying_t RelativePointer<T>::searchId(
    ptr_t const ptr) noexcept {
  if (ptr == nullptr) {
    return NULL_POINTER_ID;
  }
  return getRepository().searchId(ptr);
}

template <typename T>
inline typename RelativePointer<T>::offset_t RelativePointer<T>::computeOffset(
    ptr_t const ptr) const noexcept {
  return getOffset(segment_id_t{m_id}, ptr);
}

template <typename T>
inline T* RelativePointer<T>::computeRawPtr() const noexcept {
  return getPtr(segment_id_t{m_id}, m_offset);
}

// AXIVION Next Construct AutosarC++19_03-A15.5.3, AutosarC++19_03-A15.4.2,
// FaultDetection-NoexceptViolations : False positive, std::terminate is not
// called in the c'tor of PointerRepository and noexcept-specification is not
// violated
inline PointerRepository<segment_id_underlying_t,
                         UntypedRelativePointer::ptr_t>&
getRepository() noexcept {
  // AXIVION Next Construct AutosarC++19_03-A3.3.2 : PointerRepository can't be
  // constexpr, usage of the static object is encapsulated in the
  // RelativePointer class
  static PointerRepository<segment_id_underlying_t,
                           UntypedRelativePointer::ptr_t>
      repository;
  return repository;
}

template <typename T>
// AXIVION Next Line AutosarC++19_03-A13.5.5 : The RelativePointer shall
// explicitly be comparable to raw pointers
inline bool operator==(const RelativePointer<T> lhs,
                       const T* const rhs) noexcept {
  return lhs.get() == rhs;
}

template <typename T>
// AXIVION Next Line AutosarC++19_03-A13.5.5 : The RelativePointer shall
// explicitly be comparable to raw pointers
inline bool operator==(const T* const lhs,
                       const RelativePointer<T> rhs) noexcept {
  return rhs == lhs;
}

template <typename T>
// AXIVION Next Line AutosarC++19_03-A13.5.5 : The RelativePointer shall
// explicitly be comparable to nullptrs
inline bool operator==(std::nullptr_t, const RelativePointer<T> rhs) noexcept {
  return rhs.get() == nullptr;
}

template <typename T>
// AXIVION Next Line AutosarC++19_03-A13.5.5 : The RelativePointer shall
// explicitly be comparable to nullptrs
inline bool operator==(const RelativePointer<T> lhs, std::nullptr_t) noexcept {
  return lhs.get() == nullptr;
}

template <typename T>
// AXIVION Next Line AutosarC++19_03-A13.5.5 : The RelativePointer shall
// explicitly be comparable to raw pointers
inline bool operator!=(const RelativePointer<T> lhs,
                       const T* const rhs) noexcept {
  return !(lhs == rhs);
}

template <typename T>
// AXIVION Next Line AutosarC++19_03-A13.5.5 : The RelativePointer shall
// explicitly be comparable to raw pointers
inline bool operator!=(const T* const lhs,
                       const RelativePointer<T> rhs) noexcept {
  return rhs != lhs;
}

template <typename T>
// AXIVION Next Line AutosarC++19_03-A13.5.5 : The RelativePointer shall
// explicitly be comparable to nullptrs
inline bool operator!=(std::nullptr_t, const RelativePointer<T> rhs) noexcept {
  return rhs.get() != nullptr;
}

template <typename T>
// AXIVION Next Line AutosarC++19_03-A13.5.5 : The RelativePointer shall
// explicitly be comparable to nullptrs
inline bool operator!=(const RelativePointer<T> lhs, std::nullptr_t) noexcept {
  return lhs.get() != nullptr;
}

}  // namespace details
}  // namespace shm