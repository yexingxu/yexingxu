

#pragma once

#include <string>

#include "types/optional.hpp"

namespace shm {
namespace details {

class Group {
 public:
  using groupName_t = std::string;
  explicit Group(const gid_t id) noexcept;
  explicit Group(const groupName_t& name) noexcept;

  bool operator==(const Group& other) const noexcept;

  groupName_t getName() const noexcept;
  gid_t getID() const noexcept;

  bool doesExist() const noexcept;

  static Group getGroupOfCurrentProcess() noexcept;

  static tl::optional<uid_t> getGroupID(const groupName_t& name) noexcept;
  static tl::optional<groupName_t> getGroupName(gid_t id) noexcept;

 private:
  gid_t m_id;
  bool m_doesExist{false};
};

}  // namespace details
}  // namespace shm