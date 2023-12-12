/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-12-12 17:18:31
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-12-12 17:19:14
 */
#pragma once

#include <system_error>
namespace serialize {

enum class errc {
  ok = 0,
  no_buffer_space,
  invalid_buffer,
  hash_conflict,
  seek_failed,
};

namespace detail {

class struct_pack_category : public std::error_category {
 public:
  virtual const char *name() const noexcept override {
    return "serialize::category";
  }

  virtual std::string message(int err_val) const override {
    switch (static_cast<errc>(err_val)) {
      case errc::ok:
        return "ok";
      case errc::no_buffer_space:
        return "no buffer space";
      case errc::invalid_buffer:
        return "invalid argument";
      case errc::hash_conflict:
        return "hash conflict";

      default:
        return "(unrecognized error)";
    }
  }
};

inline const std::error_category &category() {
  static serialize::detail::struct_pack_category instance;
  return instance;
}
}  // namespace detail

}  // namespace serialize