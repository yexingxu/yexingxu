/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-12-18 10:59:10
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-12-18 11:07:58
 */

#include "details/signal_handle.hpp"

#include <spdlog/spdlog.h>

#include <string>

#include "details/expected.hpp"

namespace shm {
namespace details {

SignalGuard::SignalGuard(const Signal signal,
                         const struct sigaction& previousAction) noexcept
    : m_signal{signal},
      m_previousAction{previousAction},
      m_doRestorePreviousAction(true) {}

SignalGuard::SignalGuard(SignalGuard&& rhs) noexcept
    : m_signal{rhs.m_signal},
      m_previousAction{rhs.m_previousAction},
      m_doRestorePreviousAction{rhs.m_doRestorePreviousAction} {
  rhs.m_doRestorePreviousAction = false;
}

SignalGuard::~SignalGuard() noexcept { restorePreviousAction(); }

void SignalGuard::restorePreviousAction() noexcept {
  if (m_doRestorePreviousAction) {
    m_doRestorePreviousAction = false;
    auto res =
        sigaction(static_cast<int>(m_signal), &m_previousAction, nullptr);
    if (res < 0) {
      spdlog::error("Unable to restore the previous signal handling state!");
    }
  }
}

tl::expected<SignalGuard, SignalGuardError> registerSignalHandler(
    const Signal signal, const SignalHandlerCallback_t callback) noexcept {
  struct sigaction action = {};

  // sigemptyset fails when a nullptr is provided and this should never happen
  // with this logic
  auto res = sigemptyset(&action.sa_mask);
  if (res < 0) {
    spdlog::error(
        "This should never happen! Unable to create an empty sigaction set "
        "while registering a signal handler "
        "for the signal [" +
        std::to_string(static_cast<int>(signal)) +
        "]. No signal handler will be registered!");

    return tl::make_unexpected(SignalGuardError::INVALID_SIGNAL_ENUM_VALUE);
  }

  // system struct, no way to avoid union
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
  action.sa_handler = callback;
  action.sa_flags = 0;

  struct sigaction previousAction = {};

  // sigaction fails when action is a nullptr (which we ensure that its not) or
  // when the signal SIGSTOP or SIGKILL should be registered which can also
  // never happen - ensured by the enum class.
  auto r = sigaction(static_cast<int>(signal), &action, &previousAction);
  if (r < 0) {
    spdlog::error(
        "This should never happen! An error occurred while registering a "
        "signal handler for the signal [" +
        std::to_string(static_cast<int>(signal)) + "]. ");
    return tl::make_unexpected(
        SignalGuardError::UNDEFINED_ERROR_IN_SYSTEM_CALL);
  }

  return SignalGuard(signal, previousAction);
}

}  // namespace details
}  // namespace shm