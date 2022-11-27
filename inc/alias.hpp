#pragma once
#include <string>
#include <jsoncons/json.hpp>

using VertexId = std::string;
using VertexLabel = std::string;
using EdgeId = std::string;
using EdgeStart = std::string;
using EdgeEnd = std::string;
using EdgeType = std::string;

using WsEvent = jsoncons::ojson;
using WsArguments = jsoncons::ojson;
using ErrorMsg = std::string;


