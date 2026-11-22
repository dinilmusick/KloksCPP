#include <kloks/service.hpp>
#include <kloks/osc.hpp>
#include <kloks/transport_clock.hpp>
#include <kloks/midi.hpp>
#include <kloks/state_graph.hpp>
#include <iostream>
int main(){kloks::TransportClock c;c.start();if(c.advance(.5)!=480)return 1;auto packet=kloks::midi_encode({0x90,{60,100}});if(packet.size()!=3)return 2;auto msg=kloks::osc_decode(kloks::osc_encode({"/check",{std::int32_t(42)}}));if(msg.address!="/check")return 3;kloks::StateGraph g;g.add("ok",true);if(!std::get<bool>(g.get("ok")))return 4;std::cout<<"external consumer OK\n";}
