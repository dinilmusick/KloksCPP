#pragma once
#include "kloks/meter.hpp"
#include "kloks/transport_clock.hpp"
#include <cstdint>
#include <limits>
#include <stdexcept>
namespace kloks {
class TransportPosition{
public:
 TransportPosition(const TransportClock& clock,const Meter& meter):clock_(clock),meter_(meter){if(clock.ppqn()!=meter.ppqn())throw std::invalid_argument("meter PPQN mismatch");}
 MusicalPosition current()const noexcept{return meter_.locate(clock_.ticks());}
 std::uint64_t ticks_until(MusicalPosition p)const{auto absolute=meter_.to_ticks(p);if(absolute<clock_.ticks())throw std::invalid_argument("position in past");return absolute-clock_.ticks();}
 std::uint64_t ticks_to_next_bar()const{auto here=current();if(here.bar==std::numeric_limits<std::uint64_t>::max())throw std::overflow_error("bar overflow");return ticks_until({here.bar+1,0,0});}
private:const TransportClock& clock_;const Meter& meter_;
};
}
