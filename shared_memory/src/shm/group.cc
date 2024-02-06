

#include "shm/group.hpp"

#include <grp.h>
#include <spdlog/spdlog.h>

#include <string>

#include "shm/system_call.hpp"
#include "types/optional.hpp"

namespace shm {
namespace details {

Group::Group(gid_t id) noexcept
    : m_id(id), m_doesExist(getGroupName(id).has_value()) {}

Group::Group(const Group::groupName_t& name) noexcept {
  auto id = getGroupID(name);
  if (id.has_value()) {
    m_id = id.value();
  } else {
    spdlog::error("Group name not found");
    m_id = std::numeric_limits<gid_t>::max();
  }
}

bool Group::operator==(const Group& other) const noexcept {
  return (m_id == other.m_id);
}

Group Group::getGroupOfCurrentProcess() noexcept { return Group(getgid()); }

tl::optional<gid_t> Group::getGroupID(const Group::groupName_t& name) noexcept {
  auto getgrnamCall = SYSTEM_CALL(getgrnam)(name.c_str())
                          .failureReturnValue(nullptr)
                          .evaluate();

  if (!getgrnamCall.has_value()) {
    spdlog::error("Could not find group '" + name + "'.");
    return tl::nullopt;
  }

  return tl::make_optional<gid_t>(getgrnamCall->value->gr_gid);
}

tl::optional<Group::groupName_t> Group::getGroupName(gid_t id) noexcept {
  auto getgrgidCall =
      SYSTEM_CALL(getgrgid)(id).failureReturnValue(nullptr).evaluate();

  if (!getgrgidCall.has_value()) {
    spdlog::error("Could not find group with id '" + std::to_string(id) + "'.");
    return tl::nullopt;
  }

  return tl::make_optional<groupName_t>(
      groupName_t(getgrgidCall->value->gr_name));
}

Group::groupName_t Group::getName() const noexcept {
  auto name = getGroupName(m_id);
  if (name.has_value()) {
    return name.value();
  }

  return groupName_t();
}

gid_t Group::getID() const noexcept { return m_id; }

bool Group::doesExist() const noexcept { return m_doesExist; }

}  // namespace details
}  // namespace shm