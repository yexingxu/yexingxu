
#pragma once

#include "entity/publisher_port_user.hpp"
#include "types/unique_port_id.hpp"

namespace shm {
namespace entity {

using uid_t = types::UniquePortId;

///
/// @brief The BasePublisher class contains the common implementation for the
/// different publisher specializations.
/// @note Not intended for public usage! Use the 'Publisher' or
/// 'UntypedPublisher' instead!
///
template <typename port_t = PublisherPortUser>
class BasePublisher {
 public:
  using PortType = port_t;

  BasePublisher(const BasePublisher& other) = delete;
  BasePublisher& operator=(const BasePublisher&) = delete;
  BasePublisher(BasePublisher&& rhs) = delete;
  BasePublisher& operator=(BasePublisher&& rhs) = delete;
  virtual ~BasePublisher() noexcept;

  ///
  /// @brief uid Get the UID of the publisher.
  /// @return The publisher's UID.
  ///
  uid_t getUid() const noexcept;

  ///
  /// @brief getServiceDescription Get the service description of the publisher.
  /// @return The service description.
  ///
  //   capro::ServiceDescription getServiceDescription() const noexcept;

  ///
  /// @brief offer Offer the service to be subscribed to.
  ///
  void offer() noexcept;

  ///
  /// @brief stopOffer Stop offering the service.
  ///
  void stopOffer() noexcept;

  ///
  /// @brief isOffered
  /// @return True if service is currently being offered.
  ///
  bool isOffered() const noexcept;

  ///
  /// @brief hasSubscribers
  /// @return True if currently has subscribers to the service.
  ///
  bool hasSubscribers() const noexcept;

 protected:
  BasePublisher() = default;  // Required for testing.
  //   BasePublisher(const capro::ServiceDescription& service,
  //                 const PublisherOptions& publisherOptions);

  ///
  /// @brief port
  /// @return const accessor of the underlying port
  ///
  const port_t& port() const noexcept;

  ///
  /// @brief port
  /// @return accessor of the underlying port
  ///
  port_t& port() noexcept;

  port_t m_port{nullptr};
};

}  // namespace entity
}  // namespace shm
