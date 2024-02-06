

#pragma once

#include <spdlog/spdlog.h>

#include <cstdint>
#include <cstring>
#include <string>

#include "shm/algorithm.hpp"
#include "types/expected.hpp"

namespace shm {

static constexpr uint32_t POSIX_CALL_ERROR_STRING_SIZE = 128U;
static constexpr uint64_t POSIX_CALL_EINTR_REPETITIONS = 5U;
static constexpr int32_t POSIX_CALL_INVALID_ERRNO = -1;

inline std::string errorLiteralToString(const int returnCode [[maybe_unused]],
                                        char* const buffer) {
  return std::string(buffer);
}

inline std::string errorLiteralToString(const char* msg,
                                        char* const buffer [[maybe_unused]]) {
  return std::string(msg);
}

template <typename ReturnType, typename... FunctionArguments>
class SystemCallBuilder;

/// @brief result of a posix call
template <typename T>
struct SystemCallResult {
  SystemCallResult() noexcept = default;

  /// @brief returns the result of std::strerror(errnum) which acquires a
  ///        human readable error string
  std::string getReadableErrnum() const noexcept {
    {
      /// NOLINTJUSTIFICATION needed by POSIX function which is wrapped here
      /// NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
      char buffer[POSIX_CALL_ERROR_STRING_SIZE];
      return errorLiteralToString(
          strerror_r(errnum, &buffer[0], POSIX_CALL_ERROR_STRING_SIZE),
          &buffer[0]);
    }
  }

  /// @brief the return value of the posix function call
  T value{};

  /// @brief the errno value which was set by the posix function call
  int32_t errnum = POSIX_CALL_INVALID_ERRNO;
};

namespace details {
template <typename ReturnType, typename... FunctionArguments>
SystemCallBuilder<ReturnType, FunctionArguments...> createSystemCallBuilder(
    ReturnType (*SYSTEM_CALL)(FunctionArguments...),
    const char* posixFunctionName, const char* file, const int32_t line,
    const char* callingFunction) noexcept {
  return SystemCallBuilder<ReturnType, FunctionArguments...>(
      SYSTEM_CALL, posixFunctionName, file, line, callingFunction);
}

template <typename ReturnType>
struct SystemCallDetails {
  SystemCallDetails(const char* functionName, const char* f, int l,
                    const char* calling) noexcept
      : posixFunctionName(functionName),
        file(f),
        callingFunction(calling),
        line(l) {}
  const char* posixFunctionName = nullptr;
  const char* file = nullptr;
  const char* callingFunction = nullptr;
  int32_t line = 0;
  bool hasSuccess = true;
  bool hasIgnoredErrno = false;
  bool hasSilentErrno = false;

  SystemCallResult<ReturnType> result;
};
}  // namespace details

/// @brief class which is created by the verificator to evaluate the result of a
/// posix call
template <typename ReturnType>
class [[nodiscard]] SystemCallEvaluator {
 public:
  /// @brief ignore specified errnos from the evaluation
  /// @tparam IgnoredErrnos a list of int32_t variables
  /// @param[in] ignoredErrnos the int32_t values of the errnos which should be
  /// ignored
  /// @return a SystemCallEvaluator for further setup of the evaluation
  template <typename... IgnoredErrnos>
  SystemCallEvaluator<ReturnType> ignoreErrnos(
      const IgnoredErrnos... ignoredErrnos) const&& noexcept {
    if (!m_details.hasSuccess) {
      m_details.hasIgnoredErrno |=
          details::doesContainValue(m_details.result.errnum, ignoredErrnos...);
    }

    return *this;
  }

  /// @brief silence specified errnos from printing error messages in the
  /// evaluation
  /// @tparam SilentErrnos a list of int32_t variables
  /// @param[in] silentErrnos the int32_t values of the errnos which should be
  /// silent and not cause an error log
  /// @return a SystemCallEvaluator for further setup of the evaluation
  template <typename... SilentErrnos>
  SystemCallEvaluator<ReturnType> suppressErrorMessagesForErrnos(
      const SilentErrnos... silentErrnos) const&& noexcept {
    if (!m_details.hasSuccess) {
      m_details.hasSilentErrno |=
          details::doesContainValue(m_details.result.errnum, silentErrnos...);
    }

    return *this;
  }

  /// @brief evaluate the result of a posix call
  /// @return returns an expected which contains in both cases a
  /// SystemCallResult<ReturnType> with the return value
  /// (.value) and the errno value (.errnum) of the function call
  tl::expected<SystemCallResult<ReturnType>, SystemCallResult<ReturnType>>
  evaluate() const&& noexcept {
    if (m_details.hasSuccess || m_details.hasIgnoredErrno) {
      return SystemCallResult<ReturnType>(m_details.result);
    }

    if (!m_details.hasSilentErrno) {
      spdlog::error(std::string(m_details.file) + ":" +
                    std::to_string(m_details.line) + " { " +
                    std::string(m_details.callingFunction) + " -> " +
                    std::string(m_details.posixFunctionName) + " }  :::  [ " +
                    std::to_string(m_details.result.errnum) + " ]  " +
                    m_details.result.getReadableErrnum());
    }
    return tl::make_unexpected(SystemCallResult<ReturnType>(m_details.result));
  }

 private:
  template <typename>
  friend class SystemCallVerificator;

  explicit SystemCallEvaluator(
      details::SystemCallDetails<ReturnType>& details) noexcept
      : m_details{details} {}

 private:
  details::SystemCallDetails<ReturnType>& m_details;
};

/// @brief class which verifies the return value of a posix function call
template <typename ReturnType>
class [[nodiscard]] SystemCallVerificator {
 public:
  /// @brief the posix function call defines success through a single value
  /// @param[in] successReturnValues a list of values which define success
  /// @return the SystemCallEvaluator which evaluates the errno values
  template <typename... SuccessReturnValues>
  SystemCallEvaluator<ReturnType> successReturnValue(
      const SuccessReturnValues... successReturnValues) && noexcept {
    {
      m_details.hasSuccess = details::doesContainValue(m_details.result.value,
                                                       successReturnValues...);

      return SystemCallEvaluator<ReturnType>(m_details);
    }
  }

  /// @brief the posix function call defines failure through a single value
  /// @param[in] failureReturnValues a list of values which define failure
  /// @return the SystemCallEvaluator which evaluates the errno values
  template <typename... FailureReturnValues>
  SystemCallEvaluator<ReturnType> failureReturnValue(
      const FailureReturnValues... failureReturnValues) && noexcept {
    {
      using ValueType = decltype(m_details.result.value);
      m_details.hasSuccess = !details::doesContainValue(
          m_details.result.value,
          static_cast<ValueType>(failureReturnValues)...);

      return SystemCallEvaluator<ReturnType>(m_details);
    }
  }

  /// @brief the posix function call defines failure through return of the errno
  /// value instead of setting the errno
  /// @return the SystemCallEvaluator which evaluates the errno values
  SystemCallEvaluator<ReturnType> returnValueMatchesErrno() && noexcept {
    {
      m_details.hasSuccess = m_details.result.value == 0;
      m_details.result.errnum = static_cast<int32_t>(m_details.result.value);

      return SystemCallEvaluator<ReturnType>(m_details);
    }
  }

 private:
  template <typename, typename...>
  friend class SystemCallBuilder;

  explicit SystemCallVerificator(
      details::SystemCallDetails<ReturnType>& details) noexcept
      : m_details{details} {}

 private:
  details::SystemCallDetails<ReturnType>& m_details;
};

template <typename ReturnType, typename... FunctionArguments>
class [[nodiscard]] SystemCallBuilder {
 public:
  /// @brief input function type
  using FunctionType_t = ReturnType (*)(FunctionArguments...);

  /// @brief Call the underlying function with the provided arguments. If the
  /// underlying function fails and sets the errno to EINTR the call is repeated
  /// at most POSIX_CALL_EINTR_REPETITIONS times
  /// @param[in] arguments arguments which will be provided to the posix
  /// function
  /// @return the SystemCallVerificator to verify the return value
  SystemCallVerificator<ReturnType> operator()(
      FunctionArguments... arguments) && noexcept {
    for (uint64_t i = 0U; i < POSIX_CALL_EINTR_REPETITIONS; ++i) {
      errno = 0;
      m_details.result.value = m_SHM_SYSTEM_CALL(arguments...);
      m_details.result.errnum = errno;

      if (m_details.result.errnum != EINTR) {
        break;
      }
    }

    return SystemCallVerificator<ReturnType>(m_details);
  }

 private:
  template <typename ReturnTypeFriend, typename... FunctionArgumentsFriend>
  friend SystemCallBuilder<ReturnTypeFriend, FunctionArgumentsFriend...>
  details::createSystemCallBuilder(
      ReturnTypeFriend (*SYSTEM_CALL)(FunctionArgumentsFriend...),
      const char* posixFunctionName, const char* file, const int32_t line,
      const char* callingFunction) noexcept;

  SystemCallBuilder(FunctionType_t SYSTEM_CALL, const char* posixFunctionName,
                    const char* file, const int32_t line,
                    const char* callingFunction) noexcept
      : m_SHM_SYSTEM_CALL{SYSTEM_CALL},
        m_details{posixFunctionName, file, line, callingFunction} {}

 private:
  FunctionType_t m_SHM_SYSTEM_CALL = nullptr;
  details::SystemCallDetails<ReturnType> m_details;
};

}  // namespace shm

#define SYSTEM_CALL(f)                   \
  shm::details::createSystemCallBuilder( \
      &(f), (#f), __FILE__, __LINE__,    \
      __PRETTY_FUNCTION__)  // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)

template <typename T>
inline void DISCARD_RESULT_IMPL(T&&) noexcept {}

#define DISCARD_RESULT(expr) DISCARD_RESULT_IMPL(expr)

inline uint64_t pageSize() noexcept {
  // sysconf fails when one provides an invalid name parameter. _SC_PAGESIZE
  // is a valid name parameter therefore it should never fail.
  auto res =
      SYSTEM_CALL(sysconf)(_SC_PAGESIZE).failureReturnValue(-1).evaluate();
  if (!res.has_value()) {
    spdlog::error("This should never happen: " +
                  res.error().getReadableErrnum());
    ENSURES("");
  }

  return static_cast<uint64_t>(res.value().value);
}