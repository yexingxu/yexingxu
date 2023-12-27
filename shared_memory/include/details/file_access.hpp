/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-12-14 17:22:08
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-12-15 15:15:42
 */

#pragma once

#include <cstdint>
#include <ostream>

namespace shm {

enum class AccessMode : uint64_t {
  READ_ONLY = 0U,
  READ_WRITE = 1U,
  WRITE_ONLY = 2U
};

/// @brief describes how the shared memory is opened or created
enum class OpenMode : uint64_t {
  /// @brief creates the shared memory, if it exists already the construction
  /// will fail
  EXCLUSIVE_CREATE = 0U,
  /// @brief creates the shared memory, if it exists it will be deleted and
  /// recreated
  PURGE_AND_CREATE = 1U,
  /// @brief creates the shared memory, if it does not exist otherwise it opens
  /// it
  OPEN_OR_CREATE = 2U,
  /// @brief opens the shared memory, if it does not exist it will fail
  OPEN_EXISTING = 3U
};

class access_rights final {
 public:
  using value_type = uint16_t;

  access_rights() noexcept = default;
  access_rights(const access_rights&) noexcept = default;
  access_rights(access_rights&&) noexcept = default;

  access_rights& operator=(const access_rights&) noexcept = default;
  access_rights& operator=(access_rights&&) noexcept = default;

  ~access_rights() noexcept = default;

  /// @brief Creates an 'access_rights' object from a sanitized value, ensuring
  /// that only the bits defined in 'iox::perms::mask' are set and all other
  /// bits are reset
  /// @param[in] value for the 'access_rights'
  /// @return the 'access_rights' object
  static constexpr access_rights from_value_sanitized(
      const value_type value) noexcept {
    // the code cannot be moved to the '*.inl' file since the function is used
    // in the 'perms' namespace to define the 'access_rights' constants
    return access_rights{static_cast<value_type>(value & details::MASK)};
  }

  constexpr value_type value() const noexcept { return m_value; }

  struct details {
    // AXIVION DISABLE STYLE AutosarC++19_03-M2.13.2 : Filesystem permissions
    // are defined in octal representation

    static constexpr value_type NONE{0};

    static constexpr value_type OWNER_READ{0400};
    static constexpr value_type OWNER_WRITE{0200};
    static constexpr value_type OWNER_EXEC{0100};
    static constexpr value_type OWNER_ALL{0700};

    static constexpr value_type GROUP_READ{040};
    static constexpr value_type GROUP_WRITE{020};
    static constexpr value_type GROUP_EXEC{010};
    static constexpr value_type GROUP_ALL{070};

    static constexpr value_type OTHERS_READ{04};
    static constexpr value_type OTHERS_WRITE{02};
    static constexpr value_type OTHERS_EXEC{01};
    static constexpr value_type OTHERS_ALL{07};

    static constexpr value_type ALL{0777};

    static constexpr value_type SET_UID{04000};
    static constexpr value_type SET_GID{02000};
    static constexpr value_type STICKY_BIT{01000};

    static constexpr value_type MASK{07777};

    static constexpr access_rights unknown() noexcept {
      // Intentionally different from 'std::filesystem::perms::unknown' (which
      // is 0xFFFF) to prevent unexpected results. Combining a permission set to
      // 'std::filesystem::perms::unknown' with other permission flags using
      // bitwise AND may have an unexpected result, e.g.
      // 'std::filesystem::perms::unknown & std::filesystem::perms::mask'
      // results in having all permission flags set to 1. By using '0x8000' only
      // the MSB is 1 and all permission bits are set to 0 and a bitwise AND
      // will therefore also always result in a 0.
      constexpr value_type UNKNOWN{0x8000U};
      return access_rights{UNKNOWN};
    }

    // AXIVION ENABLE STYLE AutosarC++19_03-M2.13.2
  };

  friend constexpr bool operator==(const access_rights lhs,
                                   const access_rights rhs) noexcept {
    return lhs.value() == rhs.value();
  }
  friend constexpr bool operator!=(const access_rights lhs,
                                   const access_rights rhs) noexcept {
    return !(lhs == rhs);
  }

  friend constexpr access_rights operator|(const access_rights lhs,
                                           const access_rights rhs) noexcept {
    return access_rights(lhs.value() | rhs.value());
  }
  friend constexpr access_rights operator&(const access_rights lhs,
                                           const access_rights rhs) noexcept {
    return access_rights(lhs.value() & rhs.value());
  }
  friend constexpr access_rights operator^(const access_rights lhs,
                                           const access_rights rhs) noexcept {
    return access_rights(lhs.value() ^ rhs.value());
  }
  friend constexpr access_rights operator~(const access_rights value) noexcept {
    return access_rights(
        static_cast<access_rights::value_type>(~value.value()));
  }
  friend constexpr access_rights operator|=(const access_rights lhs,
                                            const access_rights rhs) noexcept {
    return operator|(lhs, rhs);
  }
  friend constexpr access_rights operator&=(const access_rights lhs,
                                            const access_rights rhs) noexcept {
    return operator&(lhs, rhs);
  }
  friend constexpr access_rights operator^=(const access_rights lhs,
                                            const access_rights rhs) noexcept {
    return operator^(lhs, rhs);
  }

 private:
  explicit constexpr access_rights(value_type value) noexcept
      : m_value(value) {}

 private:
  value_type m_value{0};
};

namespace perms {
// AXIVION DISABLE STYLE AutosarC++19_03-A2.10.5 : Name reuse is intentional
// since they refer to the same value. Additionally, different namespaces are
// used.

/// @brief Deny everything
/// @note the underlying value is '0'
static constexpr auto none{
    access_rights::from_value_sanitized(access_rights::details::NONE)};

/// @brief owner has read permission
/// @note the underlying value is '0o400'
static constexpr auto owner_read{
    access_rights::from_value_sanitized(access_rights::details::OWNER_READ)};
/// @brief owner has write permission
/// @note the underlying value is '0o200'
static constexpr auto owner_write{
    access_rights::from_value_sanitized(access_rights::details::OWNER_WRITE)};
/// @brief owner has execution permission
/// @note the underlying value is '0o100'
static constexpr auto owner_exec{
    access_rights::from_value_sanitized(access_rights::details::OWNER_EXEC)};
/// @brief owner has all permissions
/// @note the underlying value is '0o700'
static constexpr auto owner_all{
    access_rights::from_value_sanitized(access_rights::details::OWNER_ALL)};

/// @brief group has read permission
/// @note the underlying value is '0o040'
static constexpr auto group_read{
    access_rights::from_value_sanitized(access_rights::details::GROUP_READ)};
/// @brief group has write permission
/// @note the underlying value is '0o020'
static constexpr auto group_write{
    access_rights::from_value_sanitized(access_rights::details::GROUP_WRITE)};
/// @brief group has execution permission
/// @note the underlying value is '0o010'
static constexpr auto group_exec{
    access_rights::from_value_sanitized(access_rights::details::GROUP_EXEC)};
/// @brief group has all permissions
/// @note the underlying value is '0o070'
static constexpr auto group_all{
    access_rights::from_value_sanitized(access_rights::details::GROUP_ALL)};

/// @brief others have read permission
/// @note the underlying value is '0o004'
static constexpr auto others_read{
    access_rights::from_value_sanitized(access_rights::details::OTHERS_READ)};
/// @brief others have write permission
/// @note the underlying value is '0o002'
static constexpr auto others_write{
    access_rights::from_value_sanitized(access_rights::details::OTHERS_WRITE)};
/// @brief others have execution permission
/// @note the underlying value is '0o001'
static constexpr auto others_exec{
    access_rights::from_value_sanitized(access_rights::details::OTHERS_EXEC)};
/// @brief others have all permissions
/// @note the underlying value is '0o007'
static constexpr auto others_all{
    access_rights::from_value_sanitized(access_rights::details::OTHERS_ALL)};

/// @brief all permissions for everyone
/// @note the underlying value is '0o777'
static constexpr auto all{
    access_rights::from_value_sanitized(access_rights::details::ALL)};

/// @brief set uid bit
/// @note the underlying value is '0o4000'; introduction into setgit/setuid:
/// https://en.wikipedia.org/wiki/Setuid
// AXIVION Next Construct AutosarC++19_03-M2.10.1: The constant is in a
// namespace and mimics the C++17 STL equivalent
static constexpr auto set_uid{
    access_rights::from_value_sanitized(access_rights::details::SET_UID)};
/// @brief set gid bit
/// @note the underlying value is '0o2000'; introduction into setgit/setuid:
/// https://en.wikipedia.org/wiki/Setuid
// AXIVION Next Construct AutosarC++19_03-M2.10.1: The constant is in a
// namespace and mimics the C++17 STL equivalent
static constexpr auto set_gid{
    access_rights::from_value_sanitized(access_rights::details::SET_GID)};
/// @brief set sticky bit
/// @note the underlying value is '0o1000'; sticky bit introduction:
/// https://en.wikipedia.org/wiki/Sticky_bit
static constexpr auto sticky_bit{
    access_rights::from_value_sanitized(access_rights::details::STICKY_BIT)};

/// @brief all permissions for everyone as well as uid, gid and sticky bit
/// @note the underlying value is '0o7777'
static constexpr auto mask{
    access_rights::from_value_sanitized(access_rights::details::MASK)};

/// @brief unknown permissions
/// @note the underlying value is '0x8000'
static constexpr auto unknown{access_rights::details::unknown()};

// AXIVION ENABLE STYLE AutosarC++19_03-A2.10.5
}  // namespace perms

/// @brief Implements the equal operator
/// @param[in] lhs left hand side of the operation
/// @param[in] rhs right hand side of the operation
/// @return lhs == rhs
constexpr bool operator==(const access_rights lhs,
                          const access_rights rhs) noexcept;

/// @brief Implements the not equal operator
/// @param[in] lhs left hand side of the operation
/// @param[in] rhs right hand side of the operation
/// @return lhs != rhs
constexpr bool operator!=(const access_rights lhs,
                          const access_rights rhs) noexcept;

/// @brief Implements the binary or operation
/// @param[in] lhs left hand side of the operation
/// @param[in] rhs right hand side of the operation
/// @return lhs | rhs
constexpr access_rights operator|(const access_rights lhs,
                                  const access_rights rhs) noexcept;

/// @brief Implements the binary and operation
/// @param[in] lhs left hand side of the operation
/// @param[in] rhs right hand side of the operation
/// @return lhs & rhs
constexpr access_rights operator&(const access_rights lhs,
                                  const access_rights rhs) noexcept;

/// @brief Implements the binary exclusive or operation
/// @param[in] lhs left hand side of the operation
/// @param[in] rhs right hand side of the operation
/// @return lhs ^ rhs
constexpr access_rights operator^(const access_rights lhs,
                                  const access_rights rhs) noexcept;

/// @brief Implements the binary complement operation
/// @param[in] value the value used for the operation
/// @return ~value
constexpr access_rights operator~(const access_rights value) noexcept;

/// @brief Implements the binary or assignment operation
/// @param[in] lhs left hand side of the operation
/// @param[in] rhs right hand side of the operation
/// @return lhs = lhs | rhs
constexpr access_rights operator|=(const access_rights lhs,
                                   const access_rights rhs) noexcept;

/// @brief Implements the binary and assignment operation
/// @param[in] lhs left hand side of the operation
/// @param[in] rhs right hand side of the operation
/// @return lhs = lhs & rhs
constexpr access_rights operator&=(const access_rights lhs,
                                   const access_rights rhs) noexcept;

/// @brief Implements the binary exclusive or assignment operation
/// @param[in] lhs left hand side of the operation
/// @param[in] rhs right hand side of the operation
/// @return lhs = lhs ^ rhs
constexpr access_rights operator^=(const access_rights lhs,
                                   const access_rights rhs) noexcept;

/// @brief The 'ostream' operator for the 'access_rights' class. It handles the
/// class as if
///        it was a bitset and always lists the values for owner, group, others,
///        special bits
/// @param[in] stream reference to the 'ostream'
/// @param[in] value the file permission
/// @return the reference to the stream
std::ostream& operator<<(std::ostream& stream,
                         const access_rights value) noexcept;

}  // namespace shm