#pragma once
#include "kloks/service.hpp"
#include <algorithm>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>
namespace kloks {
// Renderer-neutral state graph. Values and edges are owned by graph, no global mutation.
class StateGraph {
 struct Node {Value value;std::set<std::string> inputs;};
 std::map<std::string,Node> nodes_;EventBus<Value> changes_;
 bool reachable(const std::string& from,const std::string& target,std::set<std::string>& seen)const{
  if(from==target)return true;
  if(!seen.insert(from).second)return false;
  const auto it=nodes_.find(from);if(it==nodes_.end())return false;
  for(const auto& edge:it->second.inputs)if(reachable(edge,target,seen))return true;
  return false;
 }
public:
 void add(std::string id,Value value={}){if(id.empty()||!nodes_.emplace(id,Node{std::move(value),{}}).second)throw std::invalid_argument("duplicate or empty graph node");}
 void connect(const std::string& target,const std::string& source){if(!nodes_.count(target)||!nodes_.count(source))throw std::out_of_range("graph node missing");std::set<std::string> seen;if(reachable(source,target,seen))throw std::invalid_argument("graph cycle");nodes_.at(target).inputs.insert(source);}
 void disconnect(const std::string& target,const std::string& source){auto it=nodes_.find(target);if(it!=nodes_.end())it->second.inputs.erase(source);}
 void set(const std::string& id,Value value){auto it=nodes_.find(id);if(it==nodes_.end())throw std::out_of_range("graph node missing");it->second.value=std::move(value);changes_.emit(id,it->second.value);}
 const Value& get(const std::string& id)const{return nodes_.at(id).value;}
 std::vector<std::string> dependencies(const std::string& id)const{const auto& deps=nodes_.at(id).inputs;return {deps.begin(),deps.end()};}
 bool contains(const std::string& id)const noexcept{return nodes_.count(id)!=0;}
 void remove(const std::string& id){nodes_.erase(id);for(auto& [_,n]:nodes_)n.inputs.erase(id);}
 EventBus<Value>& changes()noexcept{return changes_;}
};
}
