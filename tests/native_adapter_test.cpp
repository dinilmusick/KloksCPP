#include "kloks/native_adapters.hpp"
#include "kloks/test_utils.hpp"
#include <cstdint>
#include <string>
int main() {
 using namespace kloks;
 krnl::ServiceInfo meta{"native.echo","NativeEcho","1.0",{"echo"},{"memory"}};
 krnl::Service source(meta);
 krnl::OperationInfo op; op.name="echo";
 const auto accepted=source.add_operation<Value,Value>(op,
  [](const Value& input,const krnl::InvocationContext&,krnl::Cancellation)->krnl::Result<Value> { return input; });
 require(accepted.ok(),"KrnlCPP operation");
 {
  ServiceRegistry registry;
  registry.register_service(bind_krnl_service(source));
  require(!registry.started("native.echo"),"initial state");
  const auto result=registry.call("native.echo","echo",Value(std::int64_t{9}));
  require(std::get<std::int64_t>(result)==9,"KrnlCPP roundtrip");
  require(source.status()==krnl::LifecycleState::running,"KrnlCPP started");
  const auto gut=call_gut(registry,"native.echo","echo",Value(std::string("gut")));
  require(gut.ok()&&std::get<std::string>(gut.value())=="gut","GutCPP result");
  const auto kr=call_krnl(registry,"native.echo","echo",Value(true));
  require(kr.ok()&&std::get<bool>(kr.value()),"KrnlCPP result");
  const auto missing=call_gut(registry,"missing","echo");
  require(!missing.ok()&&missing.error().code=="not_found","GutCPP missing");
  const auto bad=call_krnl(registry,"missing","echo");
  require(!bad.ok()&&bad.error().code==krnl::ErrorCode::not_found,"KrnlCPP missing");
  const auto op_bad=call_gut(registry,"native.echo","missing");
  require(!op_bad.ok()&&op_bad.error().code=="missing_operation","missing operation");
  registry.dispose();
  require(source.status()==krnl::LifecycleState::stopped,"KrnlCPP stopped");
 }
 require(source.status()==krnl::LifecycleState::stopped,"borrowed lifetime");
}
