#pragma once
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>

namespace kloks {
class TimingMath final {
public:
    static std::uint64_t ticks_for_elapsed(std::uint64_t elapsed_us, double bpm, std::uint32_t ppqn) {
        validate(bpm, ppqn);
        const long double ticks = static_cast<long double>(elapsed_us) *
            static_cast<long double>(bpm) * ppqn / 60000000.0L;
        if (!std::isfinite(ticks) || ticks >=
            static_cast<long double>(std::numeric_limits<std::uint64_t>::max()) + 1.0L)
            throw std::overflow_error("transport tick overflow");
        return static_cast<std::uint64_t>(ticks);
    }
    static long double microseconds_per_tick(double bpm, std::uint32_t ppqn) {
        validate(bpm, ppqn);
        const long double duration = 60000000.0L / (static_cast<long double>(bpm) * ppqn);
        if (!std::isfinite(duration))
            throw std::overflow_error("transport tick duration overflow");
        return duration;
    }
    // Ceil to an integral microsecond boundary; floating-point tempos are approximate.
    static std::uint64_t microseconds_for_ticks(std::uint64_t ticks, double bpm, std::uint32_t ppqn) {
        validate(bpm, ppqn);
        if (ticks == 0) return 0;
        const long double duration = microseconds_per_tick(bpm, ppqn);
        const long double elapsed = static_cast<long double>(ticks) * duration;
        const long double upper = std::ldexp(1.0L, 64);
        if (!std::isfinite(elapsed) || elapsed >= upper)
            throw std::overflow_error("transport elapsed microseconds overflow");
        return static_cast<std::uint64_t>(std::ceil(elapsed));
    }
private:
    static void validate(double bpm, std::uint32_t ppqn) {
        if (!std::isfinite(bpm) || bpm <= 0.0 || ppqn == 0)
            throw std::invalid_argument("bpm and ppqn must be finite and positive");
    }
};
} // namespace kloks
