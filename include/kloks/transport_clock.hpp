#pragma once
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>
namespace kloks {
// Deterministic host-independent musical transport; elapsed time is supplied
// by the caller, so no device, browser, or background thread is required.
class TransportClock {
public:
 explicit TransportClock(double bpm=120.0, std::uint32_t ppqn=480)
   : bpm_(checked_bpm(bpm)), ppqn_(checked_ppqn(ppqn)) {}
 void start() noexcept { running_=true; }
 void stop() noexcept { running_=false; }
 void reset() noexcept { ticks_=0; remainder_=0; running_=false; }
 bool running() const noexcept { return running_; }
 std::uint64_t ticks() const noexcept { return ticks_; }
 std::uint32_t ppqn() const noexcept { return ppqn_; }
 double bpm() const noexcept { return bpm_; }
 long double beats() const noexcept { return (static_cast<long double>(ticks_)+remainder_)/ppqn_; }
 void set_bpm(double bpm) { bpm_=checked_bpm(bpm); }
 std::uint64_t advance(double seconds) {
   if(!std::isfinite(seconds)||seconds<0) throw std::invalid_argument("elapsed seconds must be finite and nonnegative");
   if(!running_||seconds==0) return 0;
   const long double available=static_cast<long double>(seconds)*bpm_*ppqn_/60.0L+remainder_;
   const long double capacity=static_cast<long double>(std::numeric_limits<std::uint64_t>::max()-ticks_);
   if(!std::isfinite(available)||available>=capacity+1.0L) throw std::overflow_error("transport tick counter overflow");
   const auto whole=static_cast<std::uint64_t>(std::floor(available));
   ticks_+=whole; remainder_=available-static_cast<long double>(whole);
   return whole;
 }
private:
 static double checked_bpm(double v) { if(!std::isfinite(v)||v<=0) throw std::invalid_argument("tempo must be finite and positive"); return v; }
 static std::uint32_t checked_ppqn(std::uint32_t v) { if(!v) throw std::invalid_argument("PPQN must be positive"); return v; }
 double bpm_; std::uint32_t ppqn_; bool running_=false;
 std::uint64_t ticks_=0; long double remainder_=0;
};
} // namespace kloks
