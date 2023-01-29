#pragma once
#include <string>
#include <jsoncons/json.hpp>

using VertexId = int;
using VertexLabel = std::string;
using EdgeId = int;
using EdgeStart = std::string;
using EdgeEnd = std::string;
using EdgeType = std::string;

using WsEvent = jsoncons::ojson;
using WsArguments = jsoncons::ojson;
using ErrorMsg = std::string;


namespace global_var {
    inline int mg_port = 0;
}
