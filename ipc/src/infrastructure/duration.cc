
#include "infrastructure/duration.h"

#include "utils/macro.h"
#include "utils/system_call.h"

namespace ipc {
namespace infra {

constexpr bool operator==(const Duration& lhs, const Duration& rhs) noexcept {
  return (lhs.seconds_ == rhs.seconds_) &&
         (lhs.nanoseconds_ == rhs.nanoseconds_);
}

constexpr bool operator!=(const Duration& lhs, const Duration& rhs) noexcept {
  return !(lhs == rhs);
}

constexpr bool operator<(const Duration& lhs, const Duration& rhs) noexcept {
  return (lhs.seconds_ < rhs.seconds_) ||
         ((lhs.seconds_ == rhs.seconds_) &&
          (lhs.nanoseconds_ < rhs.nanoseconds_));
}

constexpr bool operator>(const Duration& lhs, const Duration& rhs) noexcept {
  return (lhs.seconds_ > rhs.seconds_) ||
         ((lhs.seconds_ == rhs.seconds_) &&
          (lhs.nanoseconds_ > rhs.nanoseconds_));
}

constexpr bool operator<=(const Duration& lhs, const Duration& rhs) noexcept {
  return !(lhs > rhs);
}

constexpr bool operator>=(const Duration& lhs, const Duration& rhs) noexcept {
  return !(lhs < rhs);
}

inline constexpr Duration Duration::operator+(
    const Duration& rhs) const noexcept {
  SecondsType seconds{seconds_ + rhs.seconds_};
  NanosecondsType nanoseconds{nanoseconds_ + rhs.nanoseconds_};
  if (nanoseconds >= kNanoSecPerSec) {
    ++seconds;
    nanoseconds -= kNanoSecPerSec;
  }

  const auto sum = CreateDuration(seconds, nanoseconds);
  if (sum < *this) {
    return Duration::max();
  }
  return sum;
}

timespec Duration::Timespec(const TimeSpecReference reference) const noexcept {
  {
    using SecType = decltype(std::declval<struct timespec>().tv_sec);
    using NanoSecType = decltype(std::declval<struct timespec>().tv_nsec);

    if (reference == TimeSpecReference::kNone) {
      static_assert(sizeof(uint64_t) >= sizeof(SecType),
                    "casting might alter result");
      if (this->seconds_ >
          static_cast<uint64_t>(std::numeric_limits<SecType>::max())) {
        LOG_WARN()
            << ": Result of conversion would overflow, clamping to max value!";
        return {std::numeric_limits<SecType>::max(), kNanoSecPerSec - 1U};
      }

      const auto tv_sec = static_cast<SecType>(this->seconds_);
      const auto tv_nsec = static_cast<NanoSecType>(this->nanoseconds_);
      return {tv_sec, tv_nsec};
    }

    timespec referenceTime{};

    const clockid_t clockId{(reference == TimeSpecReference::kEpoch)
                                ? CLOCK_REALTIME
                                : CLOCK_MONOTONIC};
    ENSURES(SYSTEM_CALL(clock_gettime)(clockId, &referenceTime)
                .FailureReturnValue(-1)
                .Evaluate()
                .has_value());

    const auto targetTime = Duration(referenceTime) + *this;

    static_assert(sizeof(uint64_t) >= sizeof(SecType),
                  "casting might alter result");
    if (targetTime.seconds_ >
        static_cast<uint64_t>(std::numeric_limits<SecType>::max())) {
      LOG_WARN()
          << ": Result of conversion would overflow, clamping to max value!";
      return {std::numeric_limits<SecType>::max(), kNanoSecPerSec - 1U};
    }

    const auto tv_sec = static_cast<SecType>(targetTime.seconds_);
    const auto tv_nsec = static_cast<NanoSecType>(targetTime.nanoseconds_);
    return {tv_sec, tv_nsec};
  }
}

}  // namespace infra
}  // namespace ipc