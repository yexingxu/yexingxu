

#include "shm/user.hpp"

#include <grp.h>
#include <pwd.h>

#include <string>

#include "shm/system_call.hpp"

namespace shm {
namespace details {

tl::optional<uid_t> User::getUserID(const userName_t& name) noexcept {
  auto getpwnamCall = SYSTEM_CALL(getpwnam)(name.c_str())
                          .failureReturnValue(nullptr)
                          .evaluate();

  if (!getpwnamCall.has_value()) {
    spdlog::error("Could not find user '" + name + "'.");
    return tl::nullopt;
  }
  return tl::make_optional<uid_t>(getpwnamCall->value->pw_uid);
}

tl::optional<User::userName_t> User::getUserName(uid_t id) noexcept {
  auto getpwuidCall =
      SYSTEM_CALL(getpwuid)(id).failureReturnValue(nullptr).evaluate();

  if (!getpwuidCall.has_value()) {
    spdlog::error("Could not find user with id'" + std::to_string(id) + "'.");
    return tl::nullopt;
  }
  return tl::make_optional<userName_t>(
      userName_t(getpwuidCall->value->pw_name));
}

User::groupVector_t User::getGroups() const noexcept {
  auto userName = getUserName(m_id);
  if (!userName.has_value()) {
    return groupVector_t();
  }

  auto getpwnamCall = SYSTEM_CALL(getpwnam)(userName->c_str())
                          .failureReturnValue(nullptr)
                          .evaluate();
  if (!getpwnamCall.has_value()) {
    spdlog::error("getpwnam call failed");
    return groupVector_t();
  }

  gid_t userDefaultGroup = getpwnamCall->value->pw_gid;
  std::array<gid_t, MAX_NUMBER_OF_GROUPS>
      groups{};  // groups is initialized in iox_getgrouplist
  int numGroups = MAX_NUMBER_OF_GROUPS;

  auto getgrouplistCall =
      SYSTEM_CALL(getgrouplist)(userName->c_str(), userDefaultGroup, &groups[0],
                                &numGroups)
          .failureReturnValue(-1)
          .evaluate();
  if (!getgrouplistCall.has_value()) {
    spdlog::error("Could not obtain group list");
    return groupVector_t();
  }

  if (numGroups == -1) {
    spdlog::error("List with negative size returned");
    return groupVector_t();
  }

  groupVector_t vec;
  for (int32_t i = 0; i < numGroups; ++i) {
    vec.emplace_back(Group(groups[static_cast<uint64_t>(i)]));
  }

  return vec;
}

User::User(uid_t id) noexcept
    : m_id(id), m_doesExist(getUserName(id).has_value()) {}

User::User(const User::userName_t& name) noexcept {
  auto id = getUserID(name);
  if (id.has_value()) {
    m_id = id.value();
  } else {
    spdlog::error("User name not found");
    m_id = std::numeric_limits<gid_t>::max();
  }
}

User::userName_t User::getName() const noexcept {
  auto name = getUserName(m_id);
  if (name.has_value()) {
    return name.value();
  }

  return userName_t();
}

uid_t User::getID() const noexcept { return m_id; }

bool User::doesExist() const noexcept { return m_doesExist; }

User User::getUserOfCurrentProcess() noexcept { return User(geteuid()); }

}  // namespace details
}  // namespace shm