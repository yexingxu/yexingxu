#pragma once

#include <cstdint>
#include <cstring>
#include <string>

#include "plog/Log.h"
#include "types/expected.hpp"
#include "utils/algorithm.h"

namespace ipc {

static constexpr uint32_t kSystemCallErrorStringSize = 128U;
static constexpr uint64_t kSystemCallEnterRepetitions = 5U;
static constexpr int32_t kSystemCallInvalidErrno = -1;

inline std::string ErrorLiteralToString(const int returnCode [[maybe_unused]],
                                        char* const buffer) {
  return std::string(buffer);
}

inline std::string ErrorLiteralToString(const char* msg,
                                        char* const buffer [[maybe_unused]]) {
  return std::string(msg);
}

template <typename ReturnType, typename... FunctionArguments>
class SystemCallBuilder;

template <typename T>
struct SystemCallResult {
  SystemCallResult() noexcept = default;
  std::string GetReadableErrnum() const noexcept {
    {
      char buffer[kSystemCallErrorStringSize];
      return ErrorLiteralToString(
          strerror_r(errnum, &buffer[0], kSystemCallErrorStringSize),
          &buffer[0]);
    }
  }

  T value{};
  int32_t errnum = kSystemCallInvalidErrno;
};

namespace details {
template <typename ReturnType, typename... FunctionArguments>
SystemCallBuilder<ReturnType, FunctionArguments...> CreateSystemCallBuilder(
    ReturnType (*SYSTEM_CALL)(FunctionArguments...), const char* functionName,
    const char* file, const int32_t line,
    const char* callingFunction) noexcept {
  return SystemCallBuilder<ReturnType, FunctionArguments...>(
      SYSTEM_CALL, functionName, file, line, callingFunction);
}

template <typename ReturnType>
struct SystemCallDetails {
  SystemCallDetails(const char* name, const char* f, int l,
                    const char* calling) noexcept
      : functionName(name), file(f), callingFunction(calling), line(l) {}
  const char* functionName = nullptr;
  const char* file = nullptr;
  const char* callingFunction = nullptr;
  int32_t line = 0;
  bool hasSuccess = true;
  bool hasIgnoredErrno = false;
  bool hasSilentErrno = false;

  SystemCallResult<ReturnType> result;
};
}  // namespace details

template <typename ReturnType>
class [[nodiscard]] SystemCallEvaluator {
 public:
  template <typename... IgnoredErrnos>
  SystemCallEvaluator<ReturnType> IgnoreErrnos(
      const IgnoredErrnos... ignoredErrnos) const&& noexcept {
    if (!details_.hasSuccess) {
      details_.hasIgnoredErrno |=
          DoesContainValue(details_.result.errnum, ignoredErrnos...);
    }

    return *this;
  }

  template <typename... SilentErrnos>
  SystemCallEvaluator<ReturnType> SuppressErrorMessagesForErrnos(
      const SilentErrnos... silentErrnos) const&& noexcept {
    if (!details_.hasSuccess) {
      details_.hasSilentErrno |=
          DoesContainValue(details_.result.errnum, silentErrnos...);
    }

    return *this;
  }

  tl::expected<SystemCallResult<ReturnType>, SystemCallResult<ReturnType>>
  Evaluate() const&& noexcept {
    if (details_.hasSuccess || details_.hasIgnoredErrno) {
      return SystemCallResult<ReturnType>(details_.result);
    }

    if (!details_.hasSilentErrno) {
      LOG_ERROR() << details_.file << ":" << details_.line << " { "
                  << details_.callingFunction << " -> " << details_.functionName
                  << " }  :::  [ " << details_.result.errnum << " ]  "
                  << details_.result.GetReadableErrnum();
    }
    return tl::make_unexpected(SystemCallResult<ReturnType>(details_.result));
  }

 private:
  template <typename>
  friend class SystemCallVerificator;

  explicit SystemCallEvaluator(
      details::SystemCallDetails<ReturnType>& details) noexcept
      : details_{details} {}

 private:
  details::SystemCallDetails<ReturnType>& details_;
};

template <typename ReturnType>
class [[nodiscard]] SystemCallVerificator {
 public:
  template <typename... SuccessReturnValues>
  SystemCallEvaluator<ReturnType> SuccessReturnValue(
      const SuccessReturnValues... successReturnValues) && noexcept {
    details_.hasSuccess =
        DoesContainValue(details_.result.value, successReturnValues...);

    return SystemCallEvaluator<ReturnType>(details_);
  }

  template <typename... FailureReturnValues>
  SystemCallEvaluator<ReturnType> FailureReturnValue(
      const FailureReturnValues... failureReturnValues) && noexcept {
    using ValueType = decltype(details_.result.value);
    details_.hasSuccess = !DoesContainValue(
        details_.result.value, static_cast<ValueType>(failureReturnValues)...);

    return SystemCallEvaluator<ReturnType>(details_);
  }

  SystemCallEvaluator<ReturnType> ReturnValueMatchesErrno() && noexcept {
    details_.hasSuccess = details_.result.value == 0;
    details_.result.errnum = static_cast<int32_t>(details_.result.value);

    return SystemCallEvaluator<ReturnType>(details_);
  }

 private:
  template <typename, typename...>
  friend class SystemCallBuilder;

  explicit SystemCallVerificator(
      details::SystemCallDetails<ReturnType>& details) noexcept
      : details_{details} {}

 private:
  details::SystemCallDetails<ReturnType>& details_;
};

template <typename ReturnType, typename... FunctionArguments>
class [[nodiscard]] SystemCallBuilder {
 public:
  using FunctionType_t = ReturnType (*)(FunctionArguments...);

  SystemCallVerificator<ReturnType> operator()(
      FunctionArguments... arguments) && noexcept {
    for (uint64_t i = 0U; i < kSystemCallEnterRepetitions; ++i) {
      errno = 0;
      details_.result.value = systemCall_(arguments...);
      details_.result.errnum = errno;

      if (details_.result.errnum != EINTR) {
        break;
      }
    }

    return SystemCallVerificator<ReturnType>(details_);
  }

 private:
  template <typename ReturnTypeFriend, typename... FunctionArgumentsFriend>
  friend SystemCallBuilder<ReturnTypeFriend, FunctionArgumentsFriend...>
  details::CreateSystemCallBuilder(
      ReturnTypeFriend (*SYSTEM_CALL)(FunctionArgumentsFriend...),
      const char* functionName, const char* file, const int32_t line,
      const char* callingFunction) noexcept;

  SystemCallBuilder(FunctionType_t call, const char* functionName,
                    const char* file, const int32_t line,
                    const char* callingFunction) noexcept
      : systemCall_{call},
        details_{functionName, file, line, callingFunction} {}

 private:
  FunctionType_t systemCall_ = nullptr;
  details::SystemCallDetails<ReturnType> details_;
};

}  // namespace ipc

#define SYSTEM_CALL(f)                                                  \
  ipc::details::CreateSystemCallBuilder(&(f), (#f), __FILE__, __LINE__, \
                                        __PRETTY_FUNCTION__)
