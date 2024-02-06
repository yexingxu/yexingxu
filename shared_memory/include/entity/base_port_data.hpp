#pragma once

#include <atomic>
#include <string>

#include "types/unique_port_id.hpp"

namespace shm {
namespace entity {

using ServiceDescription = std::string;
using RuntimeName_t = std::string;
using NodeName_t = std::string;

struct BasePortData {
  /// @brief Constructor for base port data members
  BasePortData() noexcept = default;

  /// @brief Constructor
  /// @param[in] serviceDescription creates the service service description
  /// @param[in] runtimeName Name of the application's runtime
  /// @param[in] nodeName Name of the node
  BasePortData(const ServiceDescription& serviceDescription,
               const RuntimeName_t& runtimeName,
               const NodeName_t& nodeName) noexcept;

  BasePortData(const BasePortData&) = delete;
  BasePortData& operator=(const BasePortData&) = delete;
  BasePortData(BasePortData&&) = delete;
  BasePortData& operator=(BasePortData&&) = delete;
  ~BasePortData() noexcept = default;

  ServiceDescription m_serviceDescription;
  RuntimeName_t m_runtimeName;
  NodeName_t m_nodeName;
  types::UniquePortId m_uniqueId;
  std::atomic_bool m_toBeDestroyed{false};
};

}  // namespace entity
}  // namespace shm