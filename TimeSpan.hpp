#pragma once

#include <tuple>

struct TimeSpan {

    int hours{ 0 };
    int minutes{ 0 };
    int seconds{ 0 };

    constexpr TimeSpan() = default;

    constexpr TimeSpan(int hours, int minutes, int seconds) : hours{ hours }, minutes{ minutes }, seconds{ seconds } { }

    TimeSpan& operator--() {
        --seconds;
        if (seconds < 0) {
            seconds = 59;
            --minutes;
            if (minutes < 0) {
                minutes = 59;
                --hours;
            }
        }
        return *this;
    }

    TimeSpan operator--(int) {
        TimeSpan oldTimeSpan = *this;
        operator--();
        return oldTimeSpan;
    }

    [[nodiscard]] constexpr static TimeSpan zero() {
        return TimeSpan{ 0, 0, 0 };
    }

    [[nodiscard]] bool operator<(const TimeSpan& other) const {
        return std::tie(hours, minutes, seconds) < std::tie(other.hours, other.minutes, other.seconds);
    }

    [[nodiscard]] bool operator>(const TimeSpan& other) const {
        return std::tie(hours, minutes, seconds) > std::tie(other.hours, other.minutes, other.seconds);
    }

    [[nodiscard]] bool operator<=(const TimeSpan& other) const {
        return std::tie(hours, minutes, seconds) <= std::tie(other.hours, other.minutes, other.seconds);
    }

    [[nodiscard]] bool operator>=(const TimeSpan& other) const {
        return std::tie(hours, minutes, seconds) >= std::tie(other.hours, other.minutes, other.seconds);
    }

    [[nodiscard]] bool operator==(const TimeSpan& other) const {
        return std::tie(hours, minutes, seconds) == std::tie(other.hours, other.minutes, other.seconds);
    }
};
