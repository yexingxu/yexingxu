#pragma once

#include <bits/types/struct_timespec.h>

#include <cstdint>
#include <limits>
#include <utility>

namespace ipc {
namespace infra {

enum class TimeSpecReference : uint8_t { kNone, kEpoch, kMonotonic };

class Duration {
 public:
  template <typename T>
  static constexpr Duration FromNanoseconds(const T value) noexcept;
  template <typename T>
  static constexpr Duration FromMicroseconds(const T value) noexcept;
  template <typename T>
  static constexpr Duration FromMilliseconds(const T value) noexcept;
  template <typename T>
  static constexpr Duration FromSeconds(const T value) noexcept;

  constexpr explicit Duration(const timespec& value) noexcept
      : Duration(static_cast<SecondsType>(value.tv_sec),
                 static_cast<NanosecondsType>(value.tv_nsec)) {}

  friend constexpr bool operator==(const Duration& lhs,
                                   const Duration& rhs) noexcept;
  friend constexpr bool operator!=(const Duration& lhs,
                                   const Duration& rhs) noexcept;
  friend constexpr bool operator<(const Duration& lhs,
                                  const Duration& rhs) noexcept;
  friend constexpr bool operator<=(const Duration& lhs,
                                   const Duration& rhs) noexcept;
  friend constexpr bool operator>(const Duration& lhs,
                                  const Duration& rhs) noexcept;
  friend constexpr bool operator>=(const Duration& lhs,
                                   const Duration& rhs) noexcept;

  constexpr Duration operator+(const Duration& rhs) const noexcept;

  constexpr Duration& operator+=(const Duration& rhs) noexcept;

  constexpr Duration operator-(const Duration& rhs) const noexcept;

  constexpr Duration& operator-=(const Duration& rhs) noexcept;

  constexpr uint64_t ToNanoseconds() const noexcept;

  constexpr uint64_t ToMicroseconds() const noexcept;

  constexpr uint64_t ToMilliseconds() const noexcept;

  constexpr uint64_t ToSeconds() const noexcept;

  inline constexpr Duration max() const noexcept {
    return Duration{std::numeric_limits<SecondsType>::max(),
                    kNanoSecPerSec - 1U};
  }

  timespec Timespec(const TimeSpecReference reference =
                        TimeSpecReference::kNone) const noexcept;

  static constexpr uint32_t SECS_PER_MINUTE{60U};
  static constexpr uint32_t SECS_PER_HOUR{3600U};
  static constexpr uint32_t HOURS_PER_DAY{24U};

  static constexpr uint32_t MILLISECS_PER_SEC{1000U};
  static constexpr uint32_t MICROSECS_PER_SEC{MILLISECS_PER_SEC * 1000U};

  static constexpr uint32_t NANOSECS_PER_MICROSEC{1000U};
  static constexpr uint32_t NANOSECS_PER_MILLISEC{NANOSECS_PER_MICROSEC *
                                                  1000U};
  static constexpr uint32_t kNanoSecPerSec{NANOSECS_PER_MILLISEC * 1000U};

 protected:
  using SecondsType = uint64_t;
  using NanosecondsType = uint32_t;

  constexpr Duration(const SecondsType seconds,
                     const NanosecondsType nanoseconds) noexcept
      : seconds_(seconds), nanoseconds_(nanoseconds) {
    if (nanoseconds >= kNanoSecPerSec) {
      const SecondsType additionalSeconds{
          static_cast<SecondsType>(nanoseconds) /
          static_cast<SecondsType>(kNanoSecPerSec)};
      if ((std::numeric_limits<SecondsType>::max() - additionalSeconds) <
          seconds_) {
        seconds_ = std::numeric_limits<SecondsType>::max();
        nanoseconds_ = kNanoSecPerSec - 1U;
      } else {
        seconds_ += additionalSeconds;
        nanoseconds_ = nanoseconds_ % kNanoSecPerSec;
      }
    }
  }

  static constexpr Duration CreateDuration(
      const SecondsType seconds, const NanosecondsType nanoseconds) noexcept {
    return Duration(seconds, nanoseconds);
  }

 private:
  template <typename T>
  static constexpr uint64_t positiveValueOrClampToZero(const T value) noexcept;

  template <typename T>
  constexpr Duration fromFloatingPointSeconds(
      const T floatingPointSeconds) const noexcept;
  template <typename From, typename To>
  constexpr bool wouldCastFromFloatingPointProbablyOverflow(
      const From floatingPoint) const noexcept;

 private:
  SecondsType seconds_{0U};
  NanosecondsType nanoseconds_{0U};
};

}  // namespace infra
}  // namespace ipc