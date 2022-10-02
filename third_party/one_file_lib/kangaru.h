#ifndef KANGARU_PROXY_HEADER_H
#define KANGARU_PROXY_HEADER_H
//https://github.com/gracicot/kangaru/blob/master/doc/section13_structure.md
// alternative:
// [Boost].DI: C++14 Dependency Injection Library https://boost-experimental.github.io/di
// https://github.com/boost-experimental/di
#include <kangaru/kangaru.hpp> // include kangaru

// Here you can put your generic service.
// Example:
// 
// 

// You can optionally include `compatibility.hpp`
// #include <kangaru/compatibility.hpp>

// declare some recommended shortcut macros
//#define METHOD(...) ::kgr::Method<decltype(__VA_ARGS__), __VA_ARGS__>

// Or if you have C++17 available:
// template<auto F>
// using invoke = kgr::invoke<decltype(F), F>;

#endif // KANGARU_H
