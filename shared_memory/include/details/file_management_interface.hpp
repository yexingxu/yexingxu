

#pragma once

#include <spdlog/spdlog.h>

#include <limits>
#include <string>

#include "details/expected.hpp"
#include "details/file_access.hpp"
#include "details/optional.hpp"

namespace shm {

using UserName = std::string;
using GroupName = std::string;

using shm_file_stat = struct stat;

enum class FileStatError {
  IoFailure,
  FileTooLarge,
  BadFileDescriptor,
  UnknownError,
};

/// @brief Describes failures when setting the owner of a file
enum class FileSetOwnerError {
  IoFailure,
  Interrupt,
  PermissionDenied,
  ReadOnlyFilesystem,
  InvalidUidOrGid,
  BadFileDescriptor,
  UnknownError,
};

/// @brief Describes failures when setting the permissions of a file
enum class FileSetPermissionError {
  PermissionDenied,
  ReadOnlyFilesystem,
  BadFileDescriptor,
  UnknownError,
};

namespace details {
tl::expected<shm_file_stat, FileStatError> get_file_status(
    const int fildes) noexcept;
tl::expected<void, FileSetOwnerError> set_owner(const int fildes,
                                                const uid_t uid,
                                                const gid_t gid) noexcept;
tl::expected<void, FileSetPermissionError> set_permissions(
    const int fildes, const access_rights perms) noexcept;
}  // namespace details

/// @brief Represents the POSIX owners (user and group) of a file.
class Ownership {
 public:
  /// @brief Acquire the user id
  /// @returns uid
  uid_t uid() const noexcept { return m_uid; }

  /// @brief Acquire the group id
  /// @returns gid
  gid_t gid() const noexcept { return m_gid; }

  /// @brief Constructs a ownership object from a uid and a gid.
  /// @returns If the user or group does not exist it returns 'cxx::nullopt'
  /// otherwise an Ownership object
  ///             with existing user and group
  static tl::optional<Ownership> from_user_and_group(const uid_t uid,
                                                     const gid_t gid) noexcept;

  /// @brief Constructs a ownership object from a user name and a group name.
  /// @returns If the user or group does not exist it returns 'cxx::nullopt'
  /// otherwise an Ownership object
  ///             with existing user and group
  static tl::optional<Ownership> from_user_and_group(
      const UserName& user_name, const GroupName& group_name) noexcept;

  /// @brief Returns the user and group owner of the current process.
  static Ownership from_process() noexcept;

 private:
  template <typename>
  friend struct FileManagementInterface;
  Ownership(const uid_t uid, const gid_t gid) noexcept;

 private:
  uid_t m_uid{std::numeric_limits<uid_t>::max()};
  gid_t m_gid{std::numeric_limits<gid_t>::max()};
};

/// @brief Abstract implementation to manage things common to all file
/// descriptor
///        based constructs like ownership and permissions.
/// @note Can be used by every class which provides the method 'get_file_handle'
///       via inheritance.
/// @code
///   class MyResourceBasedOnFileDescriptor: public
///   FileManagementInterface<MyResourceBasedOnFileDescriptor> {
///     public:
///       // must be implemented
///       int get_file_handle() const noexcept;
///   };
/// @endcode
template <typename Derived>
struct FileManagementInterface {
  /// @brief Returns the owners of the underlying file descriptor.
  /// @return On failure a 'FileStatError' describing the error otherwise
  /// 'Ownership'.
  tl::expected<Ownership, FileStatError> get_ownership() const noexcept;

  /// @brief Sets the owners of the underlying file descriptor.
  /// @param[in] owner the new owners of the file descriptor
  /// @return On failure a 'FileSetOwnerError' describing the error.
  tl::expected<void, FileSetOwnerError> set_ownership(
      const Ownership owner) noexcept;

  /// @brief Returns the permissions of the underlying file descriptor.
  /// @return On failure a 'FileStatError' describing the error otherwise
  /// 'access_rights'.
  tl::expected<access_rights, FileStatError> get_permissions() const noexcept;

  /// @brief Sets the permissions of the underlying file descriptor.
  /// @param[in] permissions the new permissions of the file descriptor
  /// @return On failure a 'FileSetPermissionError' describing the error.
  tl::expected<void, FileSetPermissionError> set_permissions(
      const access_rights permissions) noexcept;

  /// @brief Returns the size of the corresponding file.
  /// @return On failure a 'FileStatError' describing the error otherwise the
  /// size.
  tl::expected<uint64_t, FileStatError> get_size() const noexcept {
    const auto& derived_this = *static_cast<const Derived*>(this);
    auto result = details::get_file_status(derived_this.get_file_handle());
    if (!result.has_value()) {
      return tl::make_unexpected(result.error());
    }

    return static_cast<uint64_t>(result->st_size);
  }
};
}  // namespace shm