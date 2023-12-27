

#include "details/file_management_interface.hpp"

namespace shm {
namespace details {
tl::expected<shm_file_stat, FileStatError> get_file_status(
    const int fildes) noexcept {
  shm_file_stat file_status = {};
  auto res = fstat(fildes, &file_status);

  if (res == -1) {
    switch (errno) {
      case EBADF:
        spdlog::error("The provided file descriptor is invalid.");
        return tl::make_unexpected(FileStatError::BadFileDescriptor);
      case EIO:
        spdlog::error(
            "Unable to acquire file status since an io failure occurred "
            "while reading.");
        return tl::make_unexpected(FileStatError::IoFailure);
      case EOVERFLOW:
        spdlog::error(
            "Unable to acquire file status since the file size cannot be "
            "represented by the "
            "corresponding structure.");
        return tl::make_unexpected(FileStatError::FileTooLarge);
      default:
        spdlog::error(
            "Unable to acquire file status due to an unknown failure. errno: " +
            std::to_string(errno));
        return tl::make_unexpected(FileStatError::UnknownError);
    }
  }

  return file_status;
}
tl::expected<void, FileSetOwnerError> set_owner(const int fildes,
                                                const uid_t uid,
                                                const gid_t gid) noexcept {
  auto res = fchown(fildes, uid, gid);
  if (res == -1) {
    switch (errno) {
      case EBADF:
        spdlog::error("The provided file descriptor is invalid.");
        return tl::make_unexpected(FileSetOwnerError::BadFileDescriptor);
      case EPERM:
        spdlog::error("Unable to set owner due to insufficient permissions.");
        return tl::make_unexpected(FileSetOwnerError::PermissionDenied);
      case EROFS:
        spdlog::error(
            "Unable to set owner since it is a read-only filesystem.");
        return tl::make_unexpected(FileSetOwnerError::ReadOnlyFilesystem);
      case EINVAL:
        spdlog::error("Unable to set owner since the uid " +
                      std::to_string(uid) + " or the gid " +
                      std::to_string(gid) +
                      " are not supported by the OS implementation.");
        return tl::make_unexpected(FileSetOwnerError::InvalidUidOrGid);
      case EIO:
        spdlog::error("Unable to set owner due to an IO error.");
        return tl::make_unexpected(FileSetOwnerError::IoFailure);
      case EINTR:
        spdlog::error("Unable to set owner since an interrupt was received.");
        return tl::make_unexpected(FileSetOwnerError::Interrupt);
      default:
        spdlog::error(
            "Unable to set owner since an unknown error occurred. errno: " +
            std::to_string(errno));
        return tl::make_unexpected(FileSetOwnerError::UnknownError);
    }
  }
  return {};
}
tl::expected<void, FileSetPermissionError> set_permissions(
    const int fildes, const access_rights perms) noexcept {
  auto res = fchmod(fildes, perms.value());

  if (res == -1) {
    switch (errno) {
      case EBADF:
        spdlog::error("The provided file descriptor is invalid.");
        return tl::make_unexpected(FileSetPermissionError::BadFileDescriptor);
      case EPERM:
        spdlog::error(
            "Unable to adjust permissions due to insufficient permissions.");
        return tl::make_unexpected(FileSetPermissionError::PermissionDenied);
      case EROFS:
        spdlog::error(
            "Unable to adjust permissions since it is a read-only filesystem.");
        return tl::make_unexpected(FileSetPermissionError::ReadOnlyFilesystem);
      default:
        spdlog::error(
            "Unable to adjust permissions since an unknown error occurred. "
            "errno: " +
            std::to_string(errno));
        return tl::make_unexpected(FileSetPermissionError::UnknownError);
    }
  }
  return {};
}
}  // namespace details
}  // namespace shm