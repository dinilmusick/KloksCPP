#pragma once
#include "kloks/meter.hpp"
#include "kloks/transport_clock.hpp"
#include <cstdint>
#include <limits>
#include <stdexcept>

namespace kloks {
// An adapter between the host-driven transport and its musical meter.
// A meter is only meaningful when it uses the same PPQN as the clock.
class TransportPosition {
public:
  TransportPosition(const TransportClock& clock, const Meter& meter)
      : clock_(clock), meter_(meter) {
    if (clock.ppqn() != meter.ppqn())
      throw std::invalid_argument("transport and meter PPQN differ");
  }

  MusicalPosition current() const noexcept { return meter_.locate(clock_.ticks()); }

  // Absolute tick distance to a requested canonical musical position.
  // Past positions are rejected rather than silently wrapping unsigned ticks.
  std::uint64_t ticks_until(MusicalPosition target) const {
    const auto absolute = meter_.to_ticks(target);
    if (absolute < clock_.ticks())
      throw std::invalid_argument("target precedes transport position");
    return absolute - clock_.ticks();
  }

  // Returns ticks to the next bar, never zero, including at bar boundaries.
  std::uint64_t ticks_to_next_bar() const {
    const auto here = current();
    if (here.bar == std::numeric_limits<std::uint64_t>::max())
      throw std::overflow_error("next bar exceeds position range");
    return ticks_until({here.bar + 1, 0, 0});
  }

private:
  const TransportClock& clock_;
  const Meter& meter_;
};
} // namespace kloks
