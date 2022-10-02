#pragma once
#include <string>
#include <jsoncons/json.hpp>

using Collection = std::string;
using Database = std::string;
using SchemaKey = std::string;
using DocumentKey = std::string;
using Email = std::string;
using DocumentRev = std::string;
using Edge = std::string;
using EdgeKey = std::string;
using EdgeFrom = std::string;
using EdgeTo = std::string;
using EdgeRev = std::string;
using WsEvent = jsoncons::ojson;
using WsArguments = jsoncons::ojson;
using ErrorMsg = std::string;
using ID = std::string;
using AQLQuery = std::string;
using Selector = std::string;
using Label = std::string;
using Description = std::string;
struct DocumentBase
{
  DocumentKey _key;
  DocumentRev _rev;
};
struct EdgeBase
{
  EdgeKey _key;
  EdgeRev _rev;
  EdgeFrom _from;
  EdgeTo _to;
};
JSONCONS_ALL_MEMBER_TRAITS(DocumentBase, _key, _rev)
JSONCONS_ALL_MEMBER_TRAITS(EdgeBase, _key, _rev, _from, _to)
