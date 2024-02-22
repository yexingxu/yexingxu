#include "entity/base_port.hpp"

namespace shm {
namespace entity {

BasePort::BasePort(MemberType_t* const basePortDataPtr) noexcept
    : m_basePortDataPtr(basePortDataPtr) {}

BasePort::BasePort(BasePort&& rhs) noexcept { *this = std::move(rhs); }

BasePort& BasePort::operator=(BasePort&& rhs) noexcept {
  if (this != &rhs) {
    m_basePortDataPtr = rhs.m_basePortDataPtr;
    rhs.m_basePortDataPtr = nullptr;
  }
  return *this;
}

const ServiceDescription& BasePort::getCaProServiceDescription()
    const noexcept {
  return getMembers()->m_serviceDescription;
}

const RuntimeName_t& BasePort::getRuntimeName() const noexcept {
  return getMembers()->m_runtimeName;
}

types::UniquePortId BasePort::getUniqueID() const noexcept {
  return getMembers()->m_uniqueId;
}

const NodeName_t& BasePort::getNodeName() const noexcept {
  return getMembers()->m_nodeName;
}

BasePort::operator bool() const noexcept {
  return m_basePortDataPtr != nullptr;
}

void BasePort::destroy() noexcept {
  getMembers()->m_toBeDestroyed.store(true, std::memory_order_relaxed);
}

bool BasePort::toBeDestroyed() const noexcept {
  return getMembers()->m_toBeDestroyed.load(std::memory_order_relaxed);
}

}  // namespace entity
}  // namespace shm