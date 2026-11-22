#pragma once
#include "kloks/events.hpp"
#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
namespace kloks {
struct MidiMessage {std::uint8_t status=0;std::vector<std::uint8_t> data;
 std::uint8_t channel()const noexcept{return status<0xF0?static_cast<std::uint8_t>(status&0x0F):0;}
};
inline int midi_data_length(std::uint8_t status){
 if(status<0x80)return -1;
 if(status<0xF0){const auto kind=status&0xF0;return kind==0xC0||kind==0xD0?1:2;}
 switch(status){case 0xF1:case 0xF3:return 1;case 0xF2:return 2;case 0xF6:case 0xF7:case 0xF8:case 0xF9:case 0xFA:case 0xFB:case 0xFC:case 0xFD:case 0xFE:case 0xFF:return 0;case 0xF0:return -2;default:return -1;}
}
inline void midi_validate(const MidiMessage& m){const int len=midi_data_length(m.status);if(len==-1)throw std::invalid_argument("invalid MIDI status");
 if(len==-2){if(m.data.empty()||m.data.back()!=0xF7)throw std::invalid_argument("unterminated sysex");for(std::size_t i=0;i+1<m.data.size();++i)if(m.data[i]&0x80)throw std::invalid_argument("invalid sysex data");}
 else{if(m.data.size()!=static_cast<std::size_t>(len))throw std::invalid_argument("invalid MIDI length");for(auto data:m.data)if(data&0x80)throw std::invalid_argument("invalid MIDI data");}
}
inline std::vector<std::uint8_t> midi_encode(const MidiMessage& m){midi_validate(m);std::vector<std::uint8_t> result{m.status};result.insert(result.end(),m.data.begin(),m.data.end());return result;}
// Complete-packet parser; accepts optional channel running status, no implicit global state.
class MidiDecoder{
 std::uint8_t running_status_=0;
public:
 void reset()noexcept{running_status_=0;}
 MidiMessage decode(const std::vector<std::uint8_t>& bytes){
  if(bytes.empty())throw std::invalid_argument("empty MIDI packet");
  std::size_t index=0;std::uint8_t status=bytes.front();
  if(status<0x80){status=running_status_;if(!status)throw std::invalid_argument("MIDI running status unavailable");}
  else index=1;
  const auto len=midi_data_length(status);
  if(len==-1)throw std::invalid_argument("unsupported MIDI status");
  if(status<0xF0)running_status_=status;
  else if(status<0xF8)running_status_=0;
  MidiMessage m{status,{bytes.begin()+static_cast<std::ptrdiff_t>(index),bytes.end()}};
  if(len==-2){midi_validate(m);return m;}
  if(m.data.size()!=static_cast<std::size_t>(len))throw std::invalid_argument("MIDI packet length");
  midi_validate(m);return m;
 }
};
class MidiHost {public:virtual ~MidiHost()=default;virtual void send(const std::string& port,const std::vector<std::uint8_t>& packet)=0;};
class MidiRouter{
 MidiHost& host_;std::map<std::string,std::string> routes_;
public:explicit MidiRouter(MidiHost& host):host_(host){}
 void route(std::string name,std::string port){if(name.empty()||port.empty())throw std::invalid_argument("invalid MIDI route");routes_[std::move(name)]=std::move(port);}
 void unroute(const std::string& name){routes_.erase(name);}
 void send(const std::string& name,const MidiMessage& message){const auto it=routes_.find(name);if(it==routes_.end())throw std::out_of_range("MIDI route missing");host_.send(it->second,midi_encode(message));}
};
}
