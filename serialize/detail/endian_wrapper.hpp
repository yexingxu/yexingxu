

#include <endian.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <type_traits>

#include "macro.hpp"
#include "utils.hpp"

#ifndef SERIALIZE_DETAILS_ENDIAN_WAPPER_HPP_
#define SERIALIZE_DETAILS_ENDIAN_WAPPER_HPP_
namespace serialize {
namespace detail {

constexpr bool is_system_little_endian = (__BYTE_ORDER == __LITTLE_ENDIAN);

template <bool is_little_endian_config, std::size_t block_size>
constexpr bool is_little_endian_copyable =
    (is_system_little_endian && is_little_endian_config) || block_size == 1;

inline uint16_t bswap16(uint16_t raw) { return __builtin_bswap16(raw); };

inline uint32_t bswap32(uint32_t raw) { return __builtin_bswap32(raw); };

inline uint64_t bswap64(uint64_t raw) { return __builtin_bswap64(raw); };

template <bool is_little_endian, std::size_t block_size, typename writer_t,
          std::enable_if_t<!is_little_endian || block_size == 1, int> = 0>
void write_wrapper(writer_t& writer, const char* data) {
  writer.write(data, block_size);
}

template <bool is_little_endian, std::size_t block_size, typename writer_t,
          std::enable_if_t<is_little_endian && block_size == 2, int> = 0>
void write_wrapper(writer_t& writer, const char* data) {
  auto tmp = bswap16(*(std::uint16_t*)data);
  writer.write((char*)&tmp, block_size);
}

template <bool is_little_endian, std::size_t block_size, typename writer_t,
          std::enable_if_t<is_little_endian && block_size == 4, int> = 0>
void write_wrapper(writer_t& writer, const char* data) {
  auto tmp = bswap32(*(std::uint32_t*)data);
  writer.write((char*)&tmp, block_size);
}

template <bool is_little_endian, std::size_t block_size, typename writer_t,
          std::enable_if_t<is_little_endian && block_size == 8, int> = 0>
void write_wrapper(writer_t& writer, const char* data) {
  auto tmp = bswap64(*(std::uint64_t*)data);
  writer.write((char*)&tmp, block_size);
}

template <bool is_little_endian, std::size_t block_size, typename writer_t,
          std::enable_if_t<is_little_endian && block_size == 16, int> = 0>
void write_wrapper(writer_t& writer, const char* data) {
  auto tmp1 = bswap64(*(std::uint64_t*)data),
       tmp2 = bswap64(*(std::uint64_t*)(data + 8));
  writer.write((char*)&tmp2, block_size);
  writer.write((char*)&tmp1, block_size);
}

template <
    bool is_little_endian, std::size_t block_size, typename writer_t,
    std::enable_if_t<!(block_size == 1 || block_size == 2 || block_size == 4 ||
                       block_size == 8 || block_size == 16),
                     int> = 0>
void write_wrapper(writer_t& writer, const char*) {
  static_assert(!sizeof(writer), "illegal block size(should be 1,2,4,8,16)");
}

template <typename writer_t>
void write_bytes_array(writer_t& writer, const char* data, std::size_t length) {
  if (length >= PTRDIFF_MAX) {
    unreachable();
  } else {
    writer.write(data, length);
  }
}

template <bool is_little_endian, std::size_t block_size, typename reader_t,
          std::enable_if_t<!is_little_endian || block_size == 1, int> = 0>
bool read_wrapper(reader_t& reader, char* data) {
  return static_cast<bool>(reader.read(data, block_size));
  return true;
}

template <bool is_little_endian, std::size_t block_size, typename reader_t,
          std::enable_if_t<is_little_endian && block_size == 2, int> = 0>
bool read_wrapper(reader_t& reader, char* data) {
  std::array<char, block_size> tmp;
  bool res = static_cast<bool>(reader.read((char*)&tmp, block_size));
  if SER_UNLIKELY (!res) {
    return res;
  }
  *(uint16_t*)data = bswap16(*(uint16_t*)&tmp);
  return true;
}

template <bool is_little_endian, std::size_t block_size, typename reader_t,
          std::enable_if_t<is_little_endian && block_size == 4, int> = 0>
bool read_wrapper(reader_t& reader, char* data) {
  std::array<char, block_size> tmp;
  bool res = static_cast<bool>(reader.read((char*)&tmp, block_size));
  if SER_UNLIKELY (!res) {
    return res;
  }
  *(uint32_t*)data = bswap32(*(uint32_t*)&tmp);

  return true;
}

template <bool is_little_endian, std::size_t block_size, typename reader_t,
          std::enable_if_t<is_little_endian && block_size == 8, int> = 0>
bool read_wrapper(reader_t& reader, char* data) {
  std::array<char, block_size> tmp;
  bool res = static_cast<bool>(reader.read((char*)&tmp, block_size));
  if SER_UNLIKELY (!res) {
    return res;
  }

  *(uint64_t*)data = bswap64(*(uint64_t*)&tmp);
  return true;
}

template <bool is_little_endian, std::size_t block_size, typename reader_t,
          std::enable_if_t<is_little_endian && block_size == 16, int> = 0>
bool read_wrapper(reader_t& reader, char* data) {
  std::array<char, block_size> tmp;
  bool res = static_cast<bool>(reader.read((char*)&tmp, block_size));
  if SER_UNLIKELY (!res) {
    return res;
  }
  *(uint64_t*)(data + 8) = bswap64(*(uint64_t*)&tmp);
  *(uint64_t*)data = bswap64(*(uint64_t*)(&tmp + 8));
  return true;
}

template <
    bool is_little_endian, std::size_t block_size, typename reader_t,
    std::enable_if_t<!(block_size == 1 || block_size == 2 || block_size == 4 ||
                       block_size == 8 || block_size == 16),
                     int> = 0>
bool read_wrapper(reader_t& reader, char*) {
  static_assert(!sizeof(reader), "illegal block size(should be 1,2,4,8,16)");
  return false;
}

template <typename reader_t>
bool read_bytes_array(reader_t& reader, char* data, std::size_t length) {
  if (length >= PTRDIFF_MAX) {
    unreachable();
  } else {
    return static_cast<bool>(reader.read(data, length));
  }
}

}  // namespace detail
}  // namespace serialize

#endif
