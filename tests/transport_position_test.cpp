#include "kloks/transport_position.hpp"
#include <cassert>
#include <cstdint>
#include <stdexcept>

template <typename Error, typename Fn>
void rejects(Fn fn) {
  bool caught = false;
  try { fn(); } catch (const Error&) { caught = true; }
  assert(caught);
}

int main() {
  kloks::TransportClock clock(120.0, 480);
  kloks::Meter meter(7, 8, 480);
  kloks::TransportPosition position(clock, meter);
  assert(position.current().bar == 0);
  assert(position.ticks_to_next_bar() == 1680);
  assert(position.ticks_until({1, 0, 0}) == 1680);
  clock.start();
  assert(clock.advance(0.5) == 480);
  const auto now = position.current();
  assert(now.bar == 0 && now.beat == 2 && now.tick == 0);
  assert(position.ticks_to_next_bar() == 1200);
  assert(position.ticks_until({0, 2, 0}) == 0);
  rejects<std::invalid_argument>([&] { position.ticks_until({0, 1, 0}); });
  rejects<std::invalid_argument>([&] { position.ticks_until({0, 7, 0}); });
  rejects<std::invalid_argument>([&] {
    kloks::Meter incompatible(4, 4, 960);
    kloks::TransportPosition invalid(clock, incompatible);
    (void)invalid;
  });
  return 0;
}
