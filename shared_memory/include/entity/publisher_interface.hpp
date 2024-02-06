
#pragma once

#include "entity/sample.hpp"

namespace shm {
namespace entity {

template <typename T, typename H>
class PublisherInterface {
 public:
  using SampleType = Sample<T, H>;

  /// @brief Publishes the given sample and then releases its loan.
  /// @param sample The sample to publish.
  virtual void publish(SampleType&& sample) noexcept = 0;

 protected:
  PublisherInterface() = default;
};
}  // namespace entity
}  // namespace shm
