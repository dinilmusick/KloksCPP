#pragma once
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>
namespace kloks {
using OscBlob=std::vector<std::uint8_t>;
using OscValue=std::variant<std::int32_t,float,std::string,OscBlob,bool,std::monostate>;
struct OscMessage {std::string address;std::vector<OscValue> arguments;};
struct OscBundle {std::uint64_t timetag=1;std::vector<OscMessage> messages;};
inline void osc_push_u32(OscBlob& o,std::uint32_t n){o.push_back(static_cast<std::uint8_t>(n>>24));o.push_back(static_cast<std::uint8_t>(n>>16));o.push_back(static_cast<std::uint8_t>(n>>8));o.push_back(static_cast<std::uint8_t>(n));}
inline std::uint32_t osc_read_u32(const OscBlob& b,std::size_t& at){if(b.size()-at<4)throw std::invalid_argument("truncated OSC integer");auto n=(std::uint32_t(b[at])<<24)|(std::uint32_t(b[at+1])<<16)|(std::uint32_t(b[at+2])<<8)|b[at+3];at+=4;return n;}
inline void osc_push_string(OscBlob& out,const std::string& s){if(s.find('\0')!=std::string::npos)throw std::invalid_argument("OSC string contains null");out.insert(out.end(),s.begin(),s.end());out.push_back(0);while(out.size()%4)out.push_back(0);}
inline std::string osc_read_string(const OscBlob& b,std::size_t& pos){if(pos>=b.size())throw std::invalid_argument("missing OSC string");auto begin=pos;while(pos<b.size()&&b[pos])++pos;if(pos==b.size())throw std::invalid_argument("unterminated OSC string");std::string s(b.begin()+static_cast<std::ptrdiff_t>(begin),b.begin()+static_cast<std::ptrdiff_t>(pos));++pos;while(pos%4){if(pos>=b.size()||b[pos]!=0)throw std::invalid_argument("invalid OSC string padding");++pos;}return s;}
inline void osc_validate_address(const std::string& address){if(address.empty()||address.front()!='/'||address.find('\0')!=std::string::npos)throw std::invalid_argument("OSC address must begin /");}
inline OscBlob osc_encode(const OscMessage& m){osc_validate_address(m.address);OscBlob out;osc_push_string(out,m.address);std::string tags=",";for(const auto& arg:m.arguments){switch(arg.index()){case 0:tags+='i';break;case 1:tags+='f';break;case 2:tags+='s';break;case 3:tags+='b';break;case 4:tags+=std::get<bool>(arg)?'T':'F';break;default:tags+='N';break;}}osc_push_string(out,tags);
 for(const auto& arg:m.arguments){switch(arg.index()){
  case 0:{std::uint32_t n;auto v=std::get<std::int32_t>(arg);std::memcpy(&n,&v,4);osc_push_u32(out,n);break;}
  case 1:{std::uint32_t n;auto v=std::get<float>(arg);std::memcpy(&n,&v,4);osc_push_u32(out,n);break;}
  case 2:osc_push_string(out,std::get<std::string>(arg));break;
  case 3:{const auto& b=std::get<OscBlob>(arg);if(b.size()>static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max()))throw std::length_error("OSC blob too large");osc_push_u32(out,static_cast<std::uint32_t>(b.size()));out.insert(out.end(),b.begin(),b.end());while(out.size()%4)out.push_back(0);break;}
  default:break;
 }}return out;}
inline OscMessage osc_decode(const OscBlob& data){if(data.size()>1024*1024||data.size()%4)throw std::invalid_argument("OSC packet size");std::size_t pos=0;OscMessage m;m.address=osc_read_string(data,pos);osc_validate_address(m.address);const auto tags=osc_read_string(data,pos);if(tags.empty()||tags[0]!=',')throw std::invalid_argument("OSC type tags missing");
 for(std::size_t i=1;i<tags.size();++i){switch(tags[i]){
  case 'i':{const auto n=osc_read_u32(data,pos);std::int32_t v;std::memcpy(&v,&n,4);m.arguments.emplace_back(v);break;}
  case 'f':{const auto n=osc_read_u32(data,pos);float v;std::memcpy(&v,&n,4);m.arguments.emplace_back(v);break;}
  case 's':m.arguments.emplace_back(osc_read_string(data,pos));break;
  case 'b':{const auto length=osc_read_u32(data,pos);if(length>data.size()-pos)throw std::invalid_argument("OSC blob length");OscBlob blob(data.begin()+static_cast<std::ptrdiff_t>(pos),data.begin()+static_cast<std::ptrdiff_t>(pos+length));pos+=length;while(pos%4){if(pos>=data.size()||data[pos]!=0)throw std::invalid_argument("OSC blob padding");++pos;}m.arguments.emplace_back(std::move(blob));break;}
  case 'T':m.arguments.emplace_back(true);break;case 'F':m.arguments.emplace_back(false);break;case 'N':m.arguments.emplace_back(std::monostate{});break;
  default:throw std::invalid_argument("unsupported OSC type tag");
 }}if(pos!=data.size())throw std::invalid_argument("trailing OSC data");return m;
}
inline OscBlob osc_encode(const OscBundle& bundle){OscBlob out;osc_push_string(out,"#bundle");osc_push_u32(out,static_cast<std::uint32_t>(bundle.timetag>>32));osc_push_u32(out,static_cast<std::uint32_t>(bundle.timetag));for(const auto& message:bundle.messages){const auto raw=osc_encode(message);if(raw.size()>std::numeric_limits<std::uint32_t>::max())throw std::length_error("OSC bundle element too large");osc_push_u32(out,static_cast<std::uint32_t>(raw.size()));out.insert(out.end(),raw.begin(),raw.end());}return out;}
inline OscBundle osc_decode_bundle(const OscBlob& data){if(data.size()<16||data.size()>1024*1024||data.size()%4)throw std::invalid_argument("invalid OSC bundle size");std::size_t pos=0;if(osc_read_string(data,pos)!="#bundle")throw std::invalid_argument("not an OSC bundle");OscBundle bundle;bundle.timetag=static_cast<std::uint64_t>(osc_read_u32(data,pos))<<32;bundle.timetag|=osc_read_u32(data,pos);
 while(pos<data.size()){auto length=osc_read_u32(data,pos);if(!length||length>data.size()-pos||length%4)throw std::invalid_argument("OSC bundle element size");OscBlob bytes(data.begin()+static_cast<std::ptrdiff_t>(pos),data.begin()+static_cast<std::ptrdiff_t>(pos+length));bundle.messages.push_back(osc_decode(bytes));pos+=length;}return bundle;
}
class OscHost {public:virtual ~OscHost()=default;virtual void send(const std::string& destination,const OscBlob& packet)=0;};
class OscRouter {OscHost& host_;public:explicit OscRouter(OscHost& host):host_(host){}void send(const std::string& destination,const OscMessage& m){if(destination.empty())throw std::invalid_argument("empty OSC host");host_.send(destination,osc_encode(m));}void send(const std::string& destination,const OscBundle& b){if(destination.empty())throw std::invalid_argument("empty OSC host");host_.send(destination,osc_encode(b));}};
}
