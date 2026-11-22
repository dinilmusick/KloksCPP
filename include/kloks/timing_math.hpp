#pragma once
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>
namespace kloks {
class TimingMath final{
public:
 static std::uint64_t ticks_for_elapsed(std::uint64_t micros,double bpm,std::uint32_t ppqn){validate(bpm,ppqn);const long double ticks=static_cast<long double>(micros)*bpm*ppqn/60000000.L;if(!std::isfinite(ticks)||ticks>=std::ldexp(1.L,64))throw std::overflow_error("tick overflow");return static_cast<std::uint64_t>(ticks);}
 static long double microseconds_per_tick(double bpm,std::uint32_t ppqn){validate(bpm,ppqn);const long double duration=60000000.L/(static_cast<long double>(bpm)*ppqn);if(!std::isfinite(duration))throw std::overflow_error("tick duration overflow");return duration;}
 static std::uint64_t microseconds_for_ticks(std::uint64_t ticks,double bpm,std::uint32_t ppqn){validate(bpm,ppqn);if(!ticks)return 0;const auto micros=static_cast<long double>(ticks)*60000000.L/(static_cast<long double>(bpm)*ppqn);if(!std::isfinite(micros)||micros>=std::ldexp(1.L,64))throw std::overflow_error("time overflow");const auto integer=std::round(micros);const auto tolerance=8.L*std::numeric_limits<double>::epsilon()*std::max(1.L,std::fabs(micros));return static_cast<std::uint64_t>(std::ceil(std::fabs(integer-micros)<=tolerance?integer:micros));}
private:static void validate(double bpm,std::uint32_t ppqn){if(!std::isfinite(bpm)||bpm<=0||!ppqn)throw std::invalid_argument("invalid tempo or PPQN");}
};
}
