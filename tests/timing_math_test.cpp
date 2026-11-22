#include "kloks/timing_math.hpp"
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>

void check(bool condition) { if (!condition) throw std::runtime_error("check failed"); }
template <typename E, typename F> void expect_throws(F f) {
    bool caught = false;
    try { f(); } catch (const E&) { caught = true; }
    if (!caught) throw std::runtime_error("expected exception");
}
int main() {
    using kloks::TimingMath;
    check(TimingMath::ticks_for_elapsed(0, 120.0, 480) == 0);
    check(TimingMath::ticks_for_elapsed(500000, 120.0, 480) == 480);
    check(TimingMath::ticks_for_elapsed(1000000, 60.0, 960) == 960);
    check(TimingMath::ticks_for_elapsed(250000, 120.0, 480) == 240);
    check(std::fabs(TimingMath::microseconds_per_tick(120.0, 480) - 1041.6666666666667L) < 1e-8L);
    expect_throws<std::invalid_argument>([] { TimingMath::ticks_for_elapsed(1, 0, 480); });
    expect_throws<std::invalid_argument>([] { TimingMath::ticks_for_elapsed(1, -1, 480); });
    expect_throws<std::invalid_argument>([] { TimingMath::ticks_for_elapsed(1, std::numeric_limits<double>::quiet_NaN(), 480); });
    expect_throws<std::invalid_argument>([] { TimingMath::ticks_for_elapsed(1, 120, 0); });
    expect_throws<std::overflow_error>([] { TimingMath::ticks_for_elapsed(UINT64_MAX, 1e9, 480); });
    // On MSVC long double has double range: extremely slow tempo can overflow duration.
    const double tiny_bpm = std::numeric_limits<double>::denorm_min();
    const long double expected_duration = 60000000.0L / static_cast<long double>(tiny_bpm);
    if (!std::isfinite(expected_duration)) {
        expect_throws<std::overflow_error>([&] { TimingMath::microseconds_per_tick(tiny_bpm, 1); });
    } else {
        check(std::isfinite(TimingMath::microseconds_per_tick(tiny_bpm, 1)));
    }

    check(TimingMath::microseconds_for_ticks(0, 120.0, 480) == 0);
    check(TimingMath::microseconds_for_ticks(480, 120.0, 480) == 500000);
    check(TimingMath::microseconds_for_ticks(1, 120.0, 480) == 1042);
    check(TimingMath::microseconds_for_ticks(960, 60.0, 960) == 1000000);
    expect_throws<std::invalid_argument>([] { TimingMath::microseconds_for_ticks(0, 0.0, 480); });
    expect_throws<std::invalid_argument>([] { TimingMath::microseconds_for_ticks(1, 120.0, 0); });
    expect_throws<std::overflow_error>([] { TimingMath::microseconds_for_ticks(UINT64_MAX, 1.0, 1); });
    expect_throws<std::overflow_error>([&] { TimingMath::microseconds_for_ticks(1, tiny_bpm, 1); });
}
