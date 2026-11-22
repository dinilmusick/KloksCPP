#include "kloks/service.hpp"
#include "kloks/state_graph.hpp"
#include "kloks/test_utils.hpp"
#include <string>
int main(){using namespace kloks;
 int starts=0,stops=0;ServiceRegistry r;ServiceDefinition dep;dep.id="dep";dep.name="Dep";dep.version="1";dep.on_start=[&]{starts++;};dep.on_stop=[&]{stops++;};r.register_service(std::move(dep));
 ServiceDefinition srv;srv.id="main";srv.name="Main";srv.version="1";srv.dependencies={"dep"};srv.capabilities={"math.add"};srv.operations["add"]=[](const Value& v,const ServiceContext& ctx)->Value{require(ctx.service_id=="main","context");return std::get<std::int64_t>(v)+1;};r.register_service(std::move(srv));
 auto sub=r.events().subscribe("service.started",[&](const Value&){++starts;});require(std::get<std::int64_t>(r.call("main","add",std::int64_t(41)))==42,"call");require(starts==3,"start events");r.require_capability("main","math.add");require_throws<ServiceError>([&]{r.require_capability("main","unknown");});require_throws<ServiceError>([&]{(void)r.call("main","missing");});
 ServiceDefinition a;a.id="a";a.name="A";a.version="1";a.dependencies={"b"};r.register_service(std::move(a));ServiceDefinition b;b.id="b";b.name="B";b.version="1";b.dependencies={"a"};r.register_service(std::move(b));require_throws<ServiceError>([&]{r.start("a");});require(!r.started("a"),"rollback");sub.dispose();r.stop_all();require(stops==1,"dispose");
 StateGraph graph;graph.add("root",std::int64_t(1));graph.add("child",std::int64_t(2));graph.connect("child","root");require(graph.dependencies("child").size()==1,"edge");require_throws<std::invalid_argument>([&]{graph.connect("root","child");});auto gsub=graph.changes().subscribe("root",[&](const Value& v){require(std::get<std::int64_t>(v)==3,"event");});graph.set("root",std::int64_t(3));gsub.dispose();graph.remove("root");require(graph.dependencies("child").empty(),"edge removed");
}
