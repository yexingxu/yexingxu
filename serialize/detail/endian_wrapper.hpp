

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

template <std::size_t block_size>
constexpr bool is_little_endian_copyable =
    is_system_little_endian || block_size == 1;

template <typename T>
T swap_endian(T u) {
  union {
    T u;
    unsigned char u8[sizeof(T)];
  } source, dest;
  source.u = u;
  for (size_t k = 0; k < sizeof(T); k++)
    dest.u8[k] = source.u8[sizeof(T) - k - 1];
  return dest.u;
}

inline uint16_t bswap16(uint16_t raw) {
#if (__GNUC__ || __clang__)
  return __builtin_bswap16(raw);
#else
  return swap_endian(raw);
#endif
};

inline uint32_t bswap32(uint32_t raw) {
#if (__GNUC__ || __clang__)
  return __builtin_bswap32(raw);
#else
  return swap_endian(raw);
#endif
};

inline uint64_t bswap64(uint64_t raw) {
#if (__GNUC__ || __clang__)
  return __builtin_bswap64(raw);
#else
  return swap_endian(raw);
#endif
};

template <std::size_t block_size, typename writer_t,
          std::enable_if_t<block_size == 1, int> = 0>
void write_wrapper(writer_t& writer, const char* data) {
  writer.write(data, block_size);
}

template <std::size_t block_size, typename writer_t,
          std::enable_if_t<block_size == 2, int> = 0>
void write_wrapper(writer_t& writer, const char* data) {
  auto tmp = bswap16(*(std::uint16_t*)data);
  writer.write((char*)&tmp, block_size);
}

template <std::size_t block_size, typename writer_t,
          std::enable_if_t<block_size == 4, int> = 0>
void write_wrapper(writer_t& writer, const char* data) {
  auto tmp = bswap32(*(std::uint32_t*)data);
  writer.write((char*)&tmp, block_size);
}

template <std::size_t block_size, typename writer_t,
          std::enable_if_t<block_size == 8, int> = 0>
void write_wrapper(writer_t& writer, const char* data) {
  auto tmp = bswap64(*(std::uint64_t*)data);
  writer.write((char*)&tmp, block_size);
}

template <std::size_t block_size, typename writer_t,
          std::enable_if_t<block_size == 16, int> = 0>
void write_wrapper(writer_t& writer, const char* data) {
  auto tmp1 = bswap64(*(std::uint64_t*)data),
       tmp2 = bswap64(*(std::uint64_t*)(data + 8));
  writer.write((char*)&tmp2, block_size);
  writer.write((char*)&tmp1, block_size);
}

template <
    std::size_t block_size, typename writer_t,
    std::enable_if_t<!(block_size == 1 || block_size == 2 || block_size == 4 ||
                       block_size == 8 || block_size == 16),
                     int> = 0>
void write_wrapper(writer_t& writer, const char*) {
  static_assert(!sizeof(writer), "illegal block size(should be 1,2,4,8,16)");
}

template <typename writer_t>
void write_bytes_array(writer_t& writer, const char* data, std::size_t length) {
  if (length >= PTRDIFF_MAX)
    unreachable();
  else
    writer.write(data, length);
}

template <std::size_t block_size, typename reader_t,
          std::enable_if_t<block_size == 1, int> = 0>
bool read_wrapper(reader_t& reader, char* data) {
  return static_cast<bool>(reader.read(data, block_size));
  return true;
}

template <std::size_t block_size, typename reader_t,
          std::enable_if_t<block_size == 2, int> = 0>
bool read_wrapper(reader_t& reader, char* data) {
  std::array<char, block_size> tmp;
  bool res = static_cast<bool>(reader.read((char*)&tmp, block_size));
  if SER_UNLIKELY (!res) {
    return res;
  }
  *(uint16_t*)data = bswap16(*(uint16_t*)&tmp);
  return true;
}

template <std::size_t block_size, typename reader_t,
          std::enable_if_t<block_size == 4, int> = 0>
bool read_wrapper(reader_t& reader, char* data) {
  std::array<char, block_size> tmp;
  bool res = static_cast<bool>(reader.read((char*)&tmp, block_size));
  if SER_UNLIKELY (!res) {
    return res;
  }
  *(uint32_t*)data = bswap32(*(uint32_t*)&tmp);

  return true;
}

template <std::size_t block_size, typename reader_t,
          std::enable_if_t<block_size == 8, int> = 0>
bool read_wrapper(reader_t& reader, char* data) {
  std::array<char, block_size> tmp;
  bool res = static_cast<bool>(reader.read((char*)&tmp, block_size));
  if SER_UNLIKELY (!res) {
    return res;
  }

  *(uint64_t*)data = bswap64(*(uint64_t*)&tmp);
  return true;
}

template <std::size_t block_size, typename reader_t,
          std::enable_if_t<block_size == 16, int> = 0>
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
    std::size_t block_size, typename reader_t,
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
