

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "shm/group.hpp"
#include "types/optional.hpp"
namespace shm {
namespace details {

class User {
 public:
  static constexpr uint64_t MAX_NUMBER_OF_GROUPS = 888;
  using groupVector_t = std::vector<Group>;
  using userName_t = std::string;

  explicit User(const uid_t id) noexcept;
  explicit User(const userName_t& name) noexcept;

  groupVector_t getGroups() const noexcept;
  userName_t getName() const noexcept;
  uid_t getID() const noexcept;

  bool doesExist() const noexcept;

  static User getUserOfCurrentProcess() noexcept;

  static tl::optional<uid_t> getUserID(const userName_t& name) noexcept;
  static tl::optional<userName_t> getUserName(uid_t id) noexcept;

 private:
  uid_t m_id;
  bool m_doesExist{false};
};

}  // namespace details
}  // namespace shm