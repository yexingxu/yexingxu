/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-12-12 17:18:31
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-12-28 23:33:20
 */
#pragma once

#include <system_error>

namespace serialize {

enum class return_code {
  ok = 0,
  no_buffer_space,
  invalid_buffer,
  seek_failed,
  length_error,
};

namespace detail {

class serialize_category : public std::error_category {
 public:
  virtual const char *name() const noexcept override {
    return "serialize::category";
  }

  virtual std::string message(int err_val) const override {
    switch (static_cast<return_code>(err_val)) {
      case return_code::ok:
        return "ok";
      case return_code::no_buffer_space:
        return "no buffer space";
      case return_code::invalid_buffer:
        return "invalid argument";

      default:
        return "(unrecognized error)";
    }
  }
};

inline const std::error_category &category() {
  static serialize::detail::serialize_category instance;
  return instance;
}
}  // namespace detail

}  // namespace serialize