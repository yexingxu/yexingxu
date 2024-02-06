#pragma once

#include "shm/new_type.hpp"

namespace shm {
namespace types {

using namespace details;
/// @brief Struct to signal the constructor to create an invalid id
struct InvalidPortId_t {};
constexpr InvalidPortId_t InvalidPortId = InvalidPortId_t();

/// @brief A counter which is monotonically advancing with each newly created
/// instance of UniquePortId. Additionally
///        it contains a unique RouDi id to be able to differentiate the sample
///        sources in a multi publisher multi subscriber pattern where samples
///        are exchanged over network via a third party middleware. The unique
///        RouDi id must be set manually when RouDi is started and it must be
///        ensured to be unique for a given instance for this feature to be used
///        to its full extend.
class UniquePortId
    : public NewType<UniquePortId, uint64_t, ProtectedConstructByValueCopy,
                     Comparable, Sortable, Convertable, CopyConstructable,
                     MoveConstructable, CopyAssignable, MoveAssignable> {
 public:
  using ThisType::ThisType;

  /// @brief The constructor creates an id which is greater than the
  ///         previous created id
  UniquePortId() noexcept;

  /// @brief Constructor which creates an invalid id
  UniquePortId(InvalidPortId_t) noexcept;

  /// @brief Indicates whether the object contains an invalid port id
  /// @return true if a valid id is present, false otherwise
  bool isValid() const noexcept;

  /// @brief Has to be set on RouDi startup so that a unique RouDi id is set
  ///        for all newly generated unique ids. If you call it when a unique
  ///        id is already set, an error is generated in the errorHandler.
  /// @param[in] id the unique id which you would like to set
  static void setUniqueRouDiId(const uint16_t id) noexcept;

  /// @brief Getter for the unique roudi id
  /// @return value of the unique roudi id
  static uint16_t getUniqueRouDiId() noexcept;

 private:
  // since the RouDiEnv gets restarted multiple times within a process, this
  // helps to reset the unique RouDi id during tests
  static void rouDiEnvOverrideUniqueRouDiId(const uint16_t id) noexcept;

  // returns true if setUniqueRouDiId was already called or a non-invalid
  // UniquePortId was created, otherwise false
  static bool finalizeSetUniqueRouDiId() noexcept;

 private:
  static constexpr ThisType::value_type INVALID_UNIQUE_ID = 0u;
  static constexpr ThisType::value_type ROUDI_ID_BIT_LENGTH = 16u;
  static constexpr ThisType::value_type UNIQUE_ID_BIT_LENGTH = 48u;
  static std::atomic<ThisType::value_type>
      globalIDCounter;                         // initialized in cpp file
  static std::atomic<uint16_t> uniqueRouDiId;  // initialized in cpp file
};

}  // namespace types
}  // namespace shm