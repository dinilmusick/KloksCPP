#pragma once
// Opt-in integrations with the exact native GutCPP/KrnlCPP public contracts.
#include "kloks/service.hpp"
#include <gut/result.hpp>
#include <krnl/kernel.hpp>
#include <any>
#include <string>
namespace kloks {
namespace native_detail {
inline const char* error_name(ErrorCode c) noexcept {
 switch(c) {
  case ErrorCode::invalid_definition: return "invalid_definition";
  case ErrorCode::duplicate_service: return "duplicate_service";
  case ErrorCode::not_found: return "not_found";
  case ErrorCode::missing_capability: return "missing_capability";
  case ErrorCode::missing_operation: return "missing_operation";
  case ErrorCode::cycle: return "dependency_cycle";
  case ErrorCode::invalid_state: return "invalid_state";
  case ErrorCode::host_failure: return "host_failure";
 }
 return "unknown";
}
inline krnl::ErrorCode error_to_krnl(ErrorCode c) noexcept {
 switch(c) {
  case ErrorCode::invalid_definition: return krnl::ErrorCode::invalid_argument;
  case ErrorCode::duplicate_service: return krnl::ErrorCode::duplicate;
  case ErrorCode::not_found: return krnl::ErrorCode::not_found;
  case ErrorCode::missing_capability: return krnl::ErrorCode::invalid_argument;
  case ErrorCode::missing_operation: return krnl::ErrorCode::not_found;
  case ErrorCode::cycle: return krnl::ErrorCode::invalid_graph;
  case ErrorCode::invalid_state: return krnl::ErrorCode::invalid_state;
  case ErrorCode::host_failure: return krnl::ErrorCode::exception;
 }
 return krnl::ErrorCode::exception;
}
}
inline gut::Result<Value> call_gut(ServiceRegistry& registry,const std::string& id,const std::string& op,const Value& input={}) {
 try { return gut::Result<Value>::success(registry.call(id,op,input)); }
 catch(const ServiceError& e) { return gut::Result<Value>::failure(native_detail::error_name(e.code()),e.what()); }
 catch(const std::exception& e) { return gut::Result<Value>::failure("host_exception",e.what()); }
}
inline krnl::Result<Value> call_krnl(ServiceRegistry& registry,const std::string& id,const std::string& op,const Value& input={}) {
 try { return registry.call(id,op,input); }
 catch(const ServiceError& e) { return krnl::failure(native_detail::error_to_krnl(e.code()),e.what()); }
 catch(const std::exception& e) { return krnl::failure(krnl::ErrorCode::exception,e.what()); }
}
// Borrows externally managed service. Caller MUST retain source lifetime until
// the Kloks registry disposes the binding; Kloks never creates a Krnl instance.
inline ServiceDefinition bind_krnl_service(krnl::Service& source) {
 const auto& info=source.info();
 ServiceDefinition def;
 def.id=info.id; def.name=info.name; def.version=info.version;
 def.capabilities.insert(info.capabilities.begin(),info.capabilities.end());
 def.on_start=[&source] {
  if(source.status()==krnl::LifecycleState::uninitialized) {
   const auto r=source.initialize();
   if(!r.ok()) throw ServiceError(ErrorCode::host_failure,source.info().id,r.error().message);
  }
  if(source.status()!=krnl::LifecycleState::running) {
   const auto r=source.start();
   if(!r.ok()) throw ServiceError(ErrorCode::host_failure,source.info().id,r.error().message);
  }
 };
 def.on_stop=[&source] {
  if(source.status()==krnl::LifecycleState::running) {
   const auto r=source.stop();
   if(!r.ok()) throw ServiceError(ErrorCode::host_failure,source.info().id,r.error().message);
  }
 };
 for(const auto& operation:source.operations()) {
  const auto name=operation.name;
  def.operations[name]=[&source,name](const Value& input,const ServiceContext&)->Value {
   const auto result=source.call(name,std::any(input));
   if(!result.ok()) throw ServiceError(ErrorCode::host_failure,source.info().id,result.error().message);
   const auto* v=std::any_cast<Value>(&result.value());
   if(!v) throw ServiceError(ErrorCode::host_failure,source.info().id,"KrnlCPP output is not KloksCPP Value");
   return *v;
  };
 }
 return def;
}
}
