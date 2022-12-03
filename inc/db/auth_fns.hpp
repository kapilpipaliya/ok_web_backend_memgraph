#pragma once
#include <string>
#include "alias.hpp"
#include "jsoncons/json.hpp"
namespace ok {
namespace db {

namespace auth {
int getMemberKeyFromJwt(std::string const &jwtEncodedCookie);
std::tuple<std::string, int> registerFn(jsoncons::ojson const &o);
std::tuple<std::string, int> login(jsoncons::ojson const &o);
std::tuple<std::string, std::string> change_password(VertexId const &memberKey,
    jsoncons::ojson const &o);
std::tuple<std::string, jsoncons::ojson> user(const int memberKey);
}
}
}
