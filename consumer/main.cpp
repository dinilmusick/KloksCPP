#include <kloks/service.hpp>
#include <kloks/osc.hpp>
#include <kloks/transport_clock.hpp>
#include <kloks/midi.hpp>
#include <kloks/state_graph.hpp>
#include <iostream>
#include <stdexcept>
int main(){kloks::TransportClock c;c.start();if(c.advance(.5)!=480)return 1;auto packet=kloks::midi_encode({0x90,{60,100}});if(packet.size()!=3)return 2;auto msg=kloks::osc_decode(kloks::osc_encode({"/check",{std::int32_t(42)}}));if(msg.address!="/check")return 3;kloks::StateGraph g;g.add("ok",true);if(!std::get<bool>(g.get("ok")))return 4;
kloks::ServiceRegistry registry;
int source_stops=0, stage_attempts=0;
kloks::ServiceDefinition source;
source.id="source";source.name="Source";source.version="1";
source.capabilities={"value"};
source.operations["read"]=[](const kloks::Value&,const kloks::ServiceContext&)->kloks::Value {
 return std::int64_t{7};
};
source.on_stop=[&]{++source_stops;};
registry.register_service(std::move(source));
kloks::ServiceDefinition stage;
stage.id="stage";stage.name="Stage";stage.version="1";
stage.dependencies={"source"};
stage.operations["double"]=[](const kloks::Value& input,const kloks::ServiceContext&)->kloks::Value {
 return std::get<std::int64_t>(input)*2;
};
stage.on_stop=[&]{if(++stage_attempts==1)throw std::runtime_error("stop interrupted");};
registry.register_service(std::move(stage));
const auto data=registry.call("source","read");
const auto out=registry.call("stage","double",data);
if(std::get<std::int64_t>(out)!=14)return 5;
registry.require_capability("source","value");
try{registry.require_capability("stage","value");return 6;}
catch(const kloks::ServiceError& e){if(e.code()!=kloks::ErrorCode::missing_capability)return 7;}
try{registry.stop_all();return 8;}
catch(const kloks::ServiceError& e){if(e.code()!=kloks::ErrorCode::host_failure||e.service()!="stage")return 9;}
if(!registry.started("source")||!registry.started("stage")||source_stops!=0)return 10;
registry.dispose();
if(source_stops!=1||stage_attempts!=2)return 11;
std::cout<<"external consumer OK\n";}
