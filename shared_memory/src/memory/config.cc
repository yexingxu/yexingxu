
#include "memory/config.hpp"

#include <spdlog/spdlog.h>

#include <cstdlib>

namespace shm {
namespace memory {

const Config::ConfigContainerType* Config::getMemPoolConfig() const noexcept {
  return &m_mempoolConfig;
}

void Config::addMemPool(Config::Entry f_entry) noexcept {
  if (m_mempoolConfig.size() < MAX_NUMBER_OF_MEMPOOLS) {
    m_mempoolConfig.push_back(f_entry);
  } else {
    spdlog::error(
        "Maxmimum number of mempools reached, no more mempools available");
    // std::exit(EXIT_FAILURE);
    // errorHandler(PoshError::MEPOO__MAXIMUM_NUMBER_OF_MEMPOOLS_REACHED,
    //              ErrorLevel::FATAL);
  }
}

/// this is the default memory pool configuration if no one is provided by the
/// user
Config& Config::setDefaults() noexcept {
  m_mempoolConfig.push_back({128, 10000});
  m_mempoolConfig.push_back({1024, 5000});
  m_mempoolConfig.push_back({1024 * 16, 1000});
  m_mempoolConfig.push_back({1024 * 128, 200});
  m_mempoolConfig.push_back({1024 * 512, 50});
  m_mempoolConfig.push_back({1024 * 1024, 30});
  m_mempoolConfig.push_back({1024 * 1024 * 4, 10});

  return *this;
}

Config& Config::optimize() noexcept {
  auto config = m_mempoolConfig;
  m_mempoolConfig.clear();

  std::sort(config.begin(), config.end(),
            [](const Entry& lhs, const Entry& rhs) {
              return lhs.m_size < rhs.m_size;
            });

  Config::Entry newEntry{0u, 0u};

  for (const auto& entry : config) {
    if (entry.m_size != newEntry.m_size) {
      if (newEntry.m_size != 0u) {
        m_mempoolConfig.push_back(newEntry);
      }
      newEntry.m_size = entry.m_size;
      newEntry.m_chunkCount = entry.m_chunkCount;
    } else {
      newEntry.m_chunkCount += entry.m_chunkCount;
    }
  }

  if (newEntry.m_size != 0u) {
    m_mempoolConfig.push_back(newEntry);
  }

  return *this;
}

}  // namespace memory
}  // namespace shm