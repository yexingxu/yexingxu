/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-12-18 10:21:39
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-12-18 11:07:04
 */

#pragma once

#include <csignal>
#include <functional>

#include "types/expected.hpp"

namespace shm {
namespace details {

using SignalHandlerCallback_t = void (*)(int);

/// @brief Corresponds to the SIG* macros defined in signal.h. The integer
/// values
///        are equal to the corresponding macro value.
enum class Signal : int {
  BUS = SIGBUS,
  INT = SIGINT,
  TERM = SIGTERM,
  HUP = SIGHUP,
  ABORT = SIGABRT,
  /// @attention never add SIGKILL or SIGSTOP into this list, they cannot be
  /// caught
  ///            and sigaction returns the errno EINVAL
};

enum class SignalGuardError {
  INVALID_SIGNAL_ENUM_VALUE,
  UNDEFINED_ERROR_IN_SYSTEM_CALL
};

/// @attention NEVER USE THIS CLASS AS A MEMBER VARIABLE! A class which should
/// be used
///            only in method/function scopes.
/// @brief The SignalGuard is a class returned by registerSignalHandler. When
/// it goes
///         out of scope it restores the previous signal action. Typical use
///         case: One would like to override the signal action in main() or
///         some C posix makes it necessary to override the standard signal
///         action before and after the call.
/// @code
///    {
///      auto signalGuard = registerSignalHandler(Signal::BUS,
///      printErrorMessage); my_c_call_which_can_cause_SIGBUS();
///    }
///    // here we are out of scope and the signal action for Signal::BUS is
///    restored
/// @endcode
class SignalGuard {
 public:
  SignalGuard(SignalGuard&& rhs) noexcept;
  SignalGuard(const SignalGuard&) = delete;
  ~SignalGuard() noexcept;

  SignalGuard& operator=(const SignalGuard& rhs) = delete;
  SignalGuard& operator=(SignalGuard&& rhs) = delete;

  friend tl::expected<SignalGuard, SignalGuardError> registerSignalHandler(
      const Signal, const SignalHandlerCallback_t) noexcept;

 private:
  void restorePreviousAction() noexcept;
  SignalGuard(const Signal signal,
              const struct sigaction& previousAction) noexcept;

 private:
  Signal m_signal;
  struct sigaction m_previousAction = {};
  bool m_doRestorePreviousAction{false};
};

/// @brief Register a callback for a specific posix signal (SIG***).
/// @attention if a signal callback was already registered for the provided
/// signal with registerSignalHandler or
///             with sigaction() or signal(), the signal callback is
///             overridden until the SignalGuard goes out of scope and
///             restores the previous callback. If you override the callbacks
///             multiple times and the created SignalGuards goes out of scope
///             in a different order then the callback is restored which was
///             active when the last SignalGuard which is going out of scope
///             was created.
/// @param[in] signal the 'Signal' to which the callback should be
/// attached
/// @param[in] callback the callback which should be called when the signal is
/// raised.
/// @return SignalGuard on success - when it goes out of scope the previous
/// signal action is restored. On error
///         SignalGuardError is returned which describes the error.
tl::expected<SignalGuard, SignalGuardError> registerSignalHandler(
    const Signal signal, const SignalHandlerCallback_t callback) noexcept;

}  // namespace details
}  // namespace shm