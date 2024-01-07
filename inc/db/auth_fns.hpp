#pragma once
#include <string>
#include "alias.hpp"
#include "jsoncons/json.hpp"

namespace ok::db::auth {
std::tuple<SubDomain, int> getMemberKeyFromJwt(std::string const &jwtEncodedCookie);
std::tuple<ErrorMsg, int> registerFn(jsoncons::ojson const &o, SubDomain const &subDomain, int mgPort);
std::tuple<ErrorMsg, int> login(jsoncons::ojson const &o, int mgPort);
std::tuple<int, jsoncons::ojson> loginJwt(const std::string &jwtEncoded);
std::tuple<ErrorMsg, std::string> change_password(VertexId const &memberKey, int mgPort,
    jsoncons::ojson const &o);
std::tuple<ErrorMsg, jsoncons::ojson> user(int memberKey, int mgPort);
int getSubDomainMGPort(std::string const &subDomain);
}


