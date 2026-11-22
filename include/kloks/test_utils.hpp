#pragma once
#include <stdexcept>
namespace kloks { inline void require(bool ok,const char* reason){if(!ok)throw std::runtime_error(reason);} template<class E,class F>void require_throws(F&& run){bool seen=false;try{run();}catch(const E&){seen=true;}require(seen,"expected exception not raised");}}
