#pragma once

#include "entity/publisher_impl.hpp"

namespace shm {
namespace entity {

/// @brief The Publisher class for the publish-subscribe messaging pattern in
/// iceoryx.
/// @param[in] T user payload type
/// @param[in] H user header type
template <typename T, typename H = memory::NoUserHeader>
class Publisher : public PublisherImpl<T, H> {
 public:
  using PublisherImpl<T, H>::PublisherImpl;
};

}  // namespace entity
}  // namespace shm