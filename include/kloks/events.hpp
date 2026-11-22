#pragma once
#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
namespace kloks {
// Synchronous, single-threaded event bus; subscriptions are RAII and safe during dispatch.
template<class Value> class EventBus {
 struct Store {std::size_t next=0;std::map<std::string,std::map<std::size_t,std::function<void(const Value&)>>> listeners;};
 std::shared_ptr<Store> store_=std::make_shared<Store>();
public:
 class Subscription {
  std::weak_ptr<Store> store_;std::string topic_;std::size_t id_=0;
 public:
  Subscription()=default;
  Subscription(const std::shared_ptr<Store>& store,std::string topic,std::size_t id):store_(store),topic_(std::move(topic)),id_(id){}
  Subscription(const Subscription&)=delete;Subscription& operator=(const Subscription&)=delete;
  Subscription(Subscription&& other)noexcept:store_(std::move(other.store_)),topic_(std::move(other.topic_)),id_(other.id_){other.id_=0;}
  Subscription& operator=(Subscription&& other)noexcept {if(this!=&other){dispose();store_=std::move(other.store_);topic_=std::move(other.topic_);id_=other.id_;other.id_=0;}return *this;}
  ~Subscription(){dispose();}
  void dispose()noexcept{if(id_){if(auto s=store_.lock()){auto it=s->listeners.find(topic_);if(it!=s->listeners.end()){it->second.erase(id_);if(it->second.empty())s->listeners.erase(it);}}id_=0;}}
  bool active()const noexcept{return id_!=0&&!store_.expired();}
 };
 Subscription subscribe(std::string topic,std::function<void(const Value&)> fn){if(topic.empty()||!fn)throw std::invalid_argument("invalid subscription");const auto id=++store_->next;store_->listeners[topic][id]=std::move(fn);return {store_,std::move(topic),id};}
 void emit(const std::string& topic,const Value& value)const{auto it=store_->listeners.find(topic);if(it==store_->listeners.end())return;auto copy=it->second;for(const auto& [id,fn]:copy){auto live=store_->listeners.find(topic);if(live!=store_->listeners.end()&&live->second.count(id))fn(value);}}
 std::size_t subscriptions(const std::string& topic)const{auto it=store_->listeners.find(topic);return it==store_->listeners.end()?0:it->second.size();}
 void clear()noexcept{store_->listeners.clear();}
};
}
