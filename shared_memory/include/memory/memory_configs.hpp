#pragma once

#include <cstdint>

namespace shm {
namespace memory {

static constexpr uint32_t CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT{8U};
static constexpr uint32_t CHUNK_NO_USER_HEADER_SIZE{0U};
static constexpr uint32_t CHUNK_NO_USER_HEADER_ALIGNMENT{1U};

}  // namespace memory
}  // namespace shm