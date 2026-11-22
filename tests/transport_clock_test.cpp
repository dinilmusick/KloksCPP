#include "kloks/transport_clock.hpp"
#include <cassert>
#include <cmath>
#include <limits>
#include <stdexcept>
int main() {
 kloks::TransportClock c(120.0,480);
 assert(c.advance(1.0)==0);
 c.start();
 assert(c.advance(0.25)==240);
 assert(c.advance(0.25)==240);
 assert(c.ticks()==480);
 c.set_bpm(60);
 assert(c.advance(1.0)==480);
 assert(std::abs(static_cast<double>(c.beats())-2.0)<1e-12);
 c.stop(); assert(c.advance(10)==0);
 c.start(); c.reset();
 assert(!c.running()&&c.ticks()==0);
 kloks::TransportClock fractional(60,3); fractional.start();
 for(int i=0;i<10;++i) fractional.advance(0.1);
 assert(fractional.ticks()==3);
 bool rejected=false;
 try { c.set_bpm(std::numeric_limits<double>::quiet_NaN()); }
 catch(const std::invalid_argument&) { rejected=true; }
 assert(rejected); rejected=false;
 try { c.advance(-1); } catch(const std::invalid_argument&) { rejected=true; }
 assert(rejected); rejected=false;
 try { kloks::TransportClock invalid(120,0); }
 catch(const std::invalid_argument&) { rejected=true; }
 assert(rejected); rejected=false;
 c.start();
 try { c.advance(std::numeric_limits<double>::max()); }
 catch(const std::overflow_error&) { rejected=true; }
 assert(rejected);
}
