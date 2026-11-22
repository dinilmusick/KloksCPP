#include "kloks/transport_clock.hpp"
#include "kloks/transport_position.hpp"
#include "kloks/timing_math.hpp"
#include "kloks/test_utils.hpp"
#include <cmath>
#include <limits>
#include <stdexcept>
#include <cstdint>
int main(){using namespace kloks;
 TransportClock c(120,480);require(c.advance(1)==0,"not started");c.start();require(c.advance(.25)==240,"quarter");require(c.advance(.25)==240,"quarter2");require(c.ticks()==480,"ticks");c.set_bpm(60);require(c.advance(1)==480,"tempo change");c.stop();require(c.advance(10)==0,"stopped");c.reset();require(!c.running()&&c.ticks()==0,"reset");
 TransportClock f(60,3);f.start();for(int i=0;i<10;++i)(void)f.advance(.1);require(f.ticks()==3,"fractional ticks");
 require_throws<std::invalid_argument>([&]{c.set_bpm(std::numeric_limits<double>::quiet_NaN());});require_throws<std::invalid_argument>([&]{c.advance(-1);});require_throws<std::invalid_argument>([]{TransportClock bad(120,0);(void)bad;});c.start();require_throws<std::overflow_error>([&]{c.advance(std::numeric_limits<double>::max());});
 Meter m(7,8,480);require(m.ticks_per_beat()==240,"meter beat");auto p=m.locate(1680);require(p.bar==1&&p.beat==0,"meter locate");for(auto t:{std::uint64_t(0),std::uint64_t(1680),std::numeric_limits<std::uint64_t>::max()})require(m.to_ticks(m.locate(t))==t,"roundtrip");require_throws<std::invalid_argument>([&]{(void)m.to_ticks({0,7,0});});require_throws<std::overflow_error>([&]{(void)m.to_ticks({UINT64_MAX,0,0});});
 TransportClock posClock(120,480);TransportPosition pos(posClock,m);require(pos.ticks_to_next_bar()==1680,"next bar");posClock.start();require(posClock.advance(.5)==480,"advance pos");require(pos.ticks_to_next_bar()==1200,"next bar2");require_throws<std::invalid_argument>([&]{(void)pos.ticks_until({0,1,0});});
 require(TimingMath::ticks_for_elapsed(500000,120,480)==480,"elapsed tick");require(TimingMath::microseconds_for_ticks(480,120,480)==500000,"ticks to micros");require(TimingMath::microseconds_for_ticks(1,120,480)==1042,"ceil micros");require_throws<std::invalid_argument>([]{(void)TimingMath::ticks_for_elapsed(1,0,480);});require_throws<std::overflow_error>([]{(void)TimingMath::ticks_for_elapsed(UINT64_MAX,1e9,480);});
}
