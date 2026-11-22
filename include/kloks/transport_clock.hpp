#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>
namespace kloks {
// Host-driven deterministic transport: never creates threads, timers or devices.
class TransportClock {
public:
 explicit TransportClock(double bpm=120.0,std::uint32_t ppqn=480):bpm_(checked_bpm(bpm)),ppqn_(checked_ppqn(ppqn)){}
 void start()noexcept{running_=true;}
 void stop()noexcept{running_=false;}
 void reset()noexcept{ticks_=0;remainder_=0;running_=false;}
 bool running()const noexcept{return running_;}
 std::uint64_t ticks()const noexcept{return ticks_;}
 std::uint32_t ppqn()const noexcept{return ppqn_;}
 double bpm()const noexcept{return bpm_;}
 long double beats()const noexcept{return (static_cast<long double>(ticks_)+remainder_)/ppqn_;}
 void set_bpm(double bpm){bpm_=checked_bpm(bpm);}
 std::uint64_t advance(double seconds){
  if(!std::isfinite(seconds)||seconds<0)throw std::invalid_argument("elapsed seconds must be finite nonnegative");
  if(!running_||seconds==0)return 0;
  const auto available=static_cast<long double>(seconds)*bpm_*ppqn_/60.L+remainder_;
  const auto limit=static_cast<long double>(std::numeric_limits<std::uint64_t>::max()-ticks_);
  if(!std::isfinite(available)||available>=limit+1.L)throw std::overflow_error("tick overflow");
  // Cross-compiler consistency for decimal host time increments: normalize only within
  // 32 double ULP of an *integer tick*, not across ordinary fractional boundaries.
  const auto rounded=std::round(available);
  const auto tolerance=32.L*std::numeric_limits<double>::epsilon()*std::max(1.L,std::fabs(available));
  const auto normalized=std::fabs(available-rounded)<=tolerance?rounded:available;
  const auto whole=static_cast<std::uint64_t>(std::floor(normalized));
  ticks_+=whole;remainder_=normalized-static_cast<long double>(whole);return whole;
 }
private:
 static double checked_bpm(double bpm){if(!std::isfinite(bpm)||bpm<=0)throw std::invalid_argument("invalid tempo");return bpm;}
 static std::uint32_t checked_ppqn(std::uint32_t ppqn){if(!ppqn)throw std::invalid_argument("invalid PPQN");return ppqn;}
 double bpm_;std::uint32_t ppqn_;bool running_=false;std::uint64_t ticks_=0;long double remainder_=0;
};
}
