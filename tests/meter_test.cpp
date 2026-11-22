#include "kloks/meter.hpp"
#include <cassert>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <tuple>
using kloks::Meter;
int main() {
  const Meter four(4,4,480);
  assert(four.ticks_per_beat()==480);
  auto p=four.locate(1919); assert(p.bar==0 && p.beat==3 && p.tick==479);
  p=four.locate(1920); assert(p.bar==1 && p.beat==0 && p.tick==0);
  const Meter seven(7,8,480);
  p=seven.locate(1680); assert(p.bar==1 && p.beat==0 && p.tick==0);
  p=seven.locate(1679); assert(p.bar==0 && p.beat==6 && p.tick==239);
  const Meter huge(1,1,480);
  p=huge.locate(std::numeric_limits<std::uint64_t>::max());
  assert(p.beat==0 && p.tick==std::numeric_limits<std::uint64_t>::max()%1920);
  for (auto ticks : {std::uint64_t{0}, std::uint64_t{1}, std::uint64_t{1919},
                     std::uint64_t{1920}, std::uint64_t{123456789},
                     std::numeric_limits<std::uint64_t>::max()}) {
    assert(four.to_ticks(four.locate(ticks)) == ticks);
    assert(seven.to_ticks(seven.locate(ticks)) == ticks);
  }
  for (auto invalid : {kloks::MusicalPosition{0,4,0},
                       kloks::MusicalPosition{0,0,480}}) {
    bool rejected=false;
    try { (void)four.to_ticks(invalid); }
    catch (const std::invalid_argument&) { rejected=true; }
    assert(rejected);
  }
  bool overflow=false;
  try { (void)four.to_ticks({std::numeric_limits<std::uint64_t>::max(),0,0}); }
  catch (const std::overflow_error&) { overflow=true; }
  assert(overflow);
  for (auto [n,d,ppqn] : {std::tuple<unsigned,unsigned,unsigned>{0,4,480}, {4,3,480}, {4,8,1}, {4,4,0}}) {
    bool rejected=false;
    try { Meter invalid(n,d,ppqn); (void)invalid; }
    catch (const std::invalid_argument&) { rejected=true; }
    assert(rejected);
  }
}
