#pragma once
#include "jsoncons/json.hpp"
#include "alias.hpp"
#include "mgclient.hpp"
#include "db/Session.hpp"

namespace ok::db::get
{

jsoncons::ojson getInitialData(
    const WsArguments &args,
    std::unique_ptr<mg::Client> &mgClient,
    const ok::smart_actor::connection::Session &session);
bool reconnect(std::unique_ptr<mg::Client> &connPtr);
}
