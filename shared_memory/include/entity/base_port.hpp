#pragma once

#include "entity/base_port_data.hpp"

namespace shm {
namespace entity {

/// @brief this class is the base for all ports. it is constructed from a member
///         pointer and is only movable.
///        only movable rational: a port has only one member, a pointer to its
///        data. if a port
///                               is copied then both ports would work on the
///                               same data even though these are two
///                               independent copies. this would case a weird
///                               shared state, race conditions and so on
///
///        before using a port it is, depending on the use case, important to
///        verify that the port member pointers are set
/// @code
///     auto port = std::move(GetPortFromSomewhereElse());
///     if ( port ) {
///         // do stuff
///     }
/// @endcode
class BasePort {
 public:
  using MemberType_t = BasePortData;

  explicit BasePort(MemberType_t* const basePortDataPtr) noexcept;

  BasePort(const BasePort& other) = delete;
  BasePort& operator=(const BasePort&) = delete;
  BasePort(BasePort&&) noexcept;
  BasePort& operator=(BasePort&&) noexcept;
  virtual ~BasePort() = default;

  /// @brief Checks whether the memberpointer is not null
  /// @return if the memberpointer is not null it returns true, otherwise false
  explicit operator bool() const noexcept;

  /// @brief Reads Type of actual CaPro Port (publisher/subscriber...)
  /// @return m_portType  Type of Port in struct BasePortType
  const ServiceDescription& getCaProServiceDescription() const noexcept;

  /// @brief Gets name of the application's runtime for the active port
  /// @return runtime name as String
  const RuntimeName_t& getRuntimeName() const noexcept;

  /// @brief Gets Id of the active port
  /// @return UniqueId name as Integer
  types::UniquePortId getUniqueID() const noexcept;

  /// @brief returns node name for the active port
  /// @return node name as a string
  const NodeName_t& getNodeName() const noexcept;

  /// @brief Indicate that this port can be destroyed
  void destroy() noexcept;

  /// @brief Checks whether port can be destroyed
  /// @return true if it shall be destroyed, false if not
  bool toBeDestroyed() const noexcept;

 protected:
  const MemberType_t* getMembers() const noexcept { return m_basePortDataPtr; }

  MemberType_t* getMembers() noexcept { return m_basePortDataPtr; }

 private:
  MemberType_t* m_basePortDataPtr;
};

}  // namespace entity
}  // namespace shm