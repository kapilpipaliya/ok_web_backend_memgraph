#pragma once
#include <string>
#include "alias.hpp"
#include "jsoncons/json.hpp"
namespace ok {
namespace db {

namespace auth {
int getMemberKeyFromJwt(std::string const &jwtEncodedCookie);
std::tuple<ErrorMsg, int> registerFn(jsoncons::ojson const &o);
std::tuple<ErrorMsg, int> login(jsoncons::ojson const &o);
std::tuple<int, jsoncons::ojson> loginJwt(const std::string &jwtEncoded);
std::tuple<ErrorMsg, std::string> change_password(VertexId const &memberKey,
    jsoncons::ojson const &o);
std::tuple<ErrorMsg, jsoncons::ojson> user(const int memberKey);
int getSubDomainPort(std::string const &subDomain);
}
}
}
