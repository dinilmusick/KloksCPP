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
        return 60000000.0L / (static_cast<long double>(bpm) * ppqn);
    }
private:
    static void validate(double bpm, std::uint32_t ppqn) {
        if (!std::isfinite(bpm) || bpm <= 0.0 || ppqn == 0)
            throw std::invalid_argument("bpm and ppqn must be finite and positive");
    }
};
} // namespace kloks
