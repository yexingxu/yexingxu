#pragma once

#include <sys/acl.h>

#include <cstdint>
#include <limits>
#include <memory>
#include <vector>

#include "types/expected.hpp"

namespace shm {
namespace details {

class Acl {
 public:
  enum class Error : uint8_t {
    COULD_NOT_ALLOCATE_NEW_ACL,
  };

  /// @brief maximum number of permission entries the 'PosixAcl' can store
  static constexpr int32_t MaxNumOfPermissions = 20;

  /// @brief identifier for a permission entry (user, group, others, ...)
  enum class Category : acl_tag_t {
    USER = ACL_USER_OBJ,
    /// a specific user must be identified by a name
    SPECIFIC_USER = ACL_USER,
    GROUP = ACL_GROUP_OBJ,
    /// a specific group must be identified by a name
    SPECIFIC_GROUP = ACL_GROUP,
    OTHERS = ACL_OTHER,
  };

  /// @brief access right for a permission entry
  enum class Permission : acl_perm_t {
    READ = ACL_READ,
    WRITE = ACL_WRITE,
    READWRITE = Permission::READ | Permission::WRITE,
    NONE = 0
  };

  /// @brief Value for an invalid user or group id
  static constexpr uint32_t INVALID_ID = std::numeric_limits<uint32_t>::max();

  /// @brief define and store a specific permission entry to be used by
  /// writePermissionsToFile.
  /// @param[id] id of the user or group. For Category::SPECIFIC_USER or
  /// Category::SPECIFIC_GROUP the id is required. Otherwise writing the
  /// permission entry to a file will fail. For the default user/group/others
  /// categories the id is ignored and can therefore be left empty. Do not
  /// forget to add permissions of the standard user/group/others categories
  /// before writing to a file.
  /// @param[permission] Permissions which should be applied to the category.
  /// @param[id] The group or user id - depending on the category. For
  /// Category::USER, Category::GROUP and
  ///   Category::OTHER the id is not required for everything else a valid group
  ///   or user id is mandatory.
  bool addPermissionEntry(const Category category, const Permission permission,
                          const uint32_t id = INVALID_ID) noexcept;

  /// @brief See addPermissionEntry, but one provides the user name instead of
  /// user id
  /// @param[in] permissions The permissions which should be applied to the
  /// category.
  /// @param[in] name the user name to which the permissions should be applied
  /// @return true when the permissions are applied, on failure false
  bool addUserPermission(const Permission permission,
                         const std::string& name) noexcept;

  /// @brief See addPermissionEntry, but one provides the user group instead of
  /// group id
  /// @param[in] permissions The permissions which should be applied to the
  /// category.
  /// @param[in] name the group name to which the permissions should be applied
  /// @return true when the permissions are applied, on failure false
  bool addGroupPermission(const Permission permission,
                          const std::string& name) noexcept;

  /// @brief Write permission entries stored by the 'PosixAcl' to a file
  /// identified by a file descriptor.
  /// @param[fileDescriptor] identifier for a file (can be regular file, shared
  /// memory file, message queue file... everything is a file).
  /// @return true if succesful. If false, you can assume that the file has not
  /// been touched at all.
  bool writePermissionsToFile(const int32_t fileDescriptor) const noexcept;

 private:
  using smartAclPointer_t = std::unique_ptr<std::remove_pointer<acl_t>::type>;

  struct PermissionEntry {
    unsigned int m_category;
    Permission m_permission;
    unsigned int m_id;
  };

  std::vector<PermissionEntry> m_permissions;

  static tl::expected<smartAclPointer_t, Error> createACL(
      const int32_t numEntries) noexcept;
  static bool createACLEntry(const acl_t ACL,
                             const PermissionEntry& entry) noexcept;
  static bool addAclPermission(acl_permset_t permset, acl_perm_t perm) noexcept;

  bool m_useACLMask{false};
};

}  // namespace details
}  // namespace shm