#pragma once
#include "kloks/events.hpp"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>
namespace kloks {
using Bytes=std::vector<std::uint8_t>;
using Value=std::variant<std::monostate,bool,std::int64_t,double,std::string,Bytes>;
enum class ErrorCode {invalid_definition,duplicate_service,not_found,missing_capability,missing_operation,cycle,invalid_state,host_failure};
class ServiceError:public std::runtime_error {
 ErrorCode code_;std::string service_;
public:ServiceError(ErrorCode code,std::string service,std::string message):std::runtime_error(std::move(message)),code_(code),service_(std::move(service)){}
 ErrorCode code()const noexcept{return code_;}const std::string& service()const noexcept{return service_;}
};
struct ServiceContext {std::string service_id;std::string operation;void* host=nullptr;};
using Operation=std::function<Value(const Value&,const ServiceContext&)>;
struct ServiceDefinition {
 std::string id,name,version;std::set<std::string> capabilities;std::vector<std::string> dependencies;
 std::map<std::string,Operation> operations;std::function<void()> on_start;std::function<void()> on_stop;
};
class ServiceRegistry {
 struct Entry {ServiceDefinition definition;bool started=false;};
 std::map<std::string,Entry> services_;
 std::vector<std::string> startup_order_;
 EventBus<Value> events_;
 void start_recursive(const std::string& id,std::set<std::string>& visiting){
  auto it=services_.find(id);if(it==services_.end())throw ServiceError(ErrorCode::not_found,id,"service not found");
  if(it->second.started)return;
  if(!visiting.insert(id).second)throw ServiceError(ErrorCode::cycle,id,"service dependency cycle");
  for(const auto& dep:it->second.definition.dependencies)start_recursive(dep,visiting);
  visiting.erase(id);
  if(it->second.definition.on_start)it->second.definition.on_start();
  it->second.started=true;startup_order_.push_back(id);events_.emit("service.started",std::string(id));
 }
public:
 explicit ServiceRegistry(void* host=nullptr):host_(host){}
 ServiceRegistry(const ServiceRegistry&)=delete;ServiceRegistry& operator=(const ServiceRegistry&)=delete;
 ~ServiceRegistry(){stop_all();}
 void register_service(ServiceDefinition definition){
  if(definition.id.empty()||definition.version.empty()||definition.name.empty())throw ServiceError(ErrorCode::invalid_definition,definition.id,"id, name and version required");
  for(const auto& dep:definition.dependencies)if(dep.empty()||dep==definition.id)throw ServiceError(ErrorCode::invalid_definition,definition.id,"invalid dependency");
  for(const auto& [name,op]:definition.operations)if(name.empty()||!op)throw ServiceError(ErrorCode::invalid_definition,definition.id,"invalid operation");
  if(services_.count(definition.id))throw ServiceError(ErrorCode::duplicate_service,definition.id,"duplicate service");
  const auto id=definition.id;services_.emplace(id,Entry{std::move(definition),false});
 }
 void start(const std::string& id){std::set<std::string> visiting;const auto started_before=startup_order_.size();try{start_recursive(id,visiting);}catch(...){while(startup_order_.size()>started_before)stop_one(startup_order_.back());throw;}}
 Value call(const std::string& id,const std::string& operation,const Value& input={}){
  start(id);const auto& def=services_.at(id).definition;auto it=def.operations.find(operation);
  if(it==def.operations.end())throw ServiceError(ErrorCode::missing_operation,id,"operation not found");
  return it->second(input,ServiceContext{id,operation,host_});
 }
 void require_capability(const std::string& id,const std::string& capability)const {auto it=services_.find(id);if(it==services_.end())throw ServiceError(ErrorCode::not_found,id,"service not found");if(!it->second.definition.capabilities.count(capability))throw ServiceError(ErrorCode::missing_capability,id,"capability unavailable");}
 bool started(const std::string& id)const{auto it=services_.find(id);return it!=services_.end()&&it->second.started;}
 std::vector<std::string> service_ids()const{std::vector<std::string> ids;for(const auto& [id,_]:services_)ids.push_back(id);return ids;}
 void stop_one(const std::string& id)noexcept{auto it=services_.find(id);if(it==services_.end()||!it->second.started)return;
  it->second.started=false;try{if(it->second.definition.on_stop)it->second.definition.on_stop();}catch(...){};
  startup_order_.erase(std::remove(startup_order_.begin(),startup_order_.end(),id),startup_order_.end());
  try{events_.emit("service.stopped",std::string(id));}catch(...){}
 }
 void stop_all()noexcept{while(!startup_order_.empty())stop_one(startup_order_.back());}
 void dispose()noexcept{stop_all();events_.clear();services_.clear();}
 EventBus<Value>& events()noexcept{return events_;}
private:void* host_=nullptr;
};
}
