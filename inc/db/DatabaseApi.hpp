#pragma once
#include <map>
#include <vector>
#include <velocypack/Slice.h>
#include "velocypack/velocypack-aliases.h"
#include "fuerte/types.h"
#include "fuerte/message.h"
#include "utils/ErrorConstants.hpp"
//#include "alias.hpp" // dont use this
// https://svelte.dev/repl/05b810c537664439a5e31d05a25c6252?version=3.13.0
enum class ParamIn : short
{
  query,
  body,
  header,
  path
};
enum class ParamFormat : short
{
  stringFormat,
  booleanFormat,
  numberFormat,
  integerFormat
};
struct Parameter
{
  char const *description;
  ParamFormat format;
  ParamIn in;
  char const *name;
  bool required;
  ParamFormat type;
  int xDescriptionOffset = 0;
  char const *schema;
};
using ApiResult = std::tuple<ok::ErrorCode, std::unique_ptr<arangodb::fuerte::Response>>;
namespace Api
{
// clang-format off
namespace Cluster {
namespace GetHealth {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_admin/cluster/health";
  static constexpr auto summery = "Queries the health of cluster for monitoring";
  static constexpr auto operationId = "(QueriesTheHealthOfClusterForMonitoring";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Cluster/get_cluster_health.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Cluster"};
  
  enum GetHealthResponse : short {
    Unknown = 0,
    Response200 = 200
  };
  inline const static std::map<int, char const * > responses{{Response200, ""}};
  static constexpr auto description = R"(Queries the health of the cluster for monitoring purposes. The response is a JSON object, containing the standard `code`, `error`, `errorNum`, and `errorMessage` fields as appropriate. The endpoint-specific fields are as follows:

- `ClusterId`: A UUID string identifying the cluster
- `Health`: An object containing a descriptive sub-object for each node in the cluster.
  - `<nodeID>`: Each entry in `Health` will be keyed by the node ID and contain the following attributes:
    - `Endpoint`: A string representing the network endpoint of the server.
    - `Role`: The role the server plays. Possible values are `"AGENT"`, `"COORDINATOR"`, and `"DBSERVER"`.
    - `CanBeDeleted`: Boolean representing whether the node can safely be removed from the cluster.
    - `Version`: Version String of ArangoDB used by that node.
    - `Engine`: Storage Engine used by that node.
    - `Status`: A string indicating the health of the node as assessed by the supervision (agency). This should be considered primary source of truth for coordinator and dbservers node health. If the node is responding normally to requests, it is `"GOOD"`. If it has missed one heartbeat, it is `"BAD"`. If it has been declared failed by the supervision, which occurs after missing heartbeats for about 15 seconds, it will be marked `"FAILED"`.

    Additionally it will also have the following attributes for:

    **Coordinators** and **DBServers**
    - `SyncStatus`: The last sync status reported by the node. This value is primarily used to determine the value of `Status`. Possible values include `"UNKNOWN"`, `"UNDEFINED"`, `"STARTUP"`, `"STOPPING"`, `"STOPPED"`, `"SERVING"`, `"SHUTDOWN"`.
    - `LastAckedTime`: ISO 8601 timestamp specifying the last heartbeat received.
    - `ShortName`: A string representing the shortname of the server, e.g. `"Coordinator0001"`.
    - `Timestamp`: ISO 8601 timestamp specifying the last heartbeat received. (deprecated)
    - `Host`: An optional string, specifying the host machine if known.

    **Coordinators** only
    - `AdvertisedEndpoint`: A string representing the advertised endpoint, if set. (e.g. external IP address or load balancer, optional)

    **Agents**
    - `Leader`: ID of the agent this node regards as leader.
    - `Leading`: Whether this agent is the leader (true) or not (false).
    - `LastAckedTime`: Time since last `acked` in seconds.
)";


  ApiResult request(std::string const &databaseName);
};
namespace PutMaintenance {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_admin/cluster/maintenance";
  static constexpr auto summery = "Enable or disable the supervision maintenance mode";
  static constexpr auto operationId = "(EnableOrDisableTheSupervisionMaintenanceMode";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Cluster/put_cluster_maintenance.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Cluster"};
  
  enum PutMaintenanceResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400,
    Response501 = 501,
    Response504 = 504
  };
  inline const static std::map<int, char const * > responses{{Response200, ""},
{Response400, ""}, 
{Response501, ""}, 
{Response504, ""}};
  static constexpr auto description = R"(This API allows you to temporarily enable the supervision maintenance mode. Be aware that no 
automatic failovers of any kind will take place while the maintenance mode is enabled.
The _cluster_ supervision reactivates itself automatically _60 minutes_ after disabling it.

To enable the maintenance mode the request body must contain the string `"on"`. To disable it, send the string
`"off"` (Please note it _must_ be lowercase as well as include the quotes).)";


  ApiResult request(std::string const &databaseName);
};
namespace GetClusterStatistics {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_admin/clusterStatistics";
  static constexpr auto summery = "Queries statistics of DBserver";
  static constexpr auto operationId = "(QueriesStatisticsOfDbserver";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Cluster/get_cluster_statistics.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Cluster"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = "";
    P0.in = ParamIn::query;
    P0.name = "DBserver";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum GetClusterStatisticsResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400,
    Response403 = 403
  };
  inline const static std::map<int, char const * > responses{{Response200, ""},
{Response400, R"(ID of a DBserver)"}, 
{Response403, ""}};
  static constexpr auto description = R"(Queries the statistics of the given DBserver)";


  ApiResult request(std::string const &databaseName);
};
}
namespace Administration {
namespace GetTargetVersion {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_admin/database/target-version";
  static constexpr auto summery = "Return the required version of the database";
  static constexpr auto operationId = "(RestAdminDatabaseHandler";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Administration/get_admin_database_version.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Administration"};
  
  enum GetTargetVersionResponse : short {
    Unknown = 0,
    Response200 = 200
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Is returned in all cases.


)"}};
  static constexpr auto description = R"(Returns the database version that this server requires.
The version is returned in the *version* attribute of the result.)";


  ApiResult request(std::string const &databaseName);
};
namespace PostEcho {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Post;
  static constexpr auto url = "/_admin/echo";
  static constexpr auto summery = "Return current request";
  static constexpr auto operationId = "(adminEchoJs";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Administration/post_admin_echo.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Administration"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The body can be any type and is simply forwarded.)";
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"additionalProperties":{},"type":"object"})";
    P0.xDescriptionOffset = 0;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PostEchoResponse : short {
    Unknown = 0,
    Response200 = 200
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Echo was returned successfully.)"}};
  static constexpr auto description = R"(The call returns an object with the servers request information


**HTTP 200**
*A json document with these Properties is returned:*

Echo was returned successfully.

- **cookies**: list of the cookies you sent
- **protocol**: the transport, one of ['http', 'https', 'velocystream']
- **suffix** (anonymous json object): 
- **parameters**: Object containing the query parameters
- **database**: the database this request was executed on
- **url**: the raw request URL
- **internals**: contents of the server internals struct
- **server**:
  - **id**: a server generated id
  - **port**: port of the client side of the tcp connection
  - **address**: the ip address of the client
- **requestBody**: stringified version of the POST body we sent
- **headers**: the list of the HTTP headers you sent
- **prefix**: prefix of the database
- **client**:
- **authorized**: whether the session is authorized
- **requestType**: In this case *POST*, if you use another HTTP-Verb, you will se that (GET/DELETE, ...)
- **rawSuffix** (anonymous json object): 
- **path**: relative path of this request
- **rawRequestBody** (anonymous json object): List of digits of the sent characters
- **user**: the currently user that sent this request)";


  ApiResult request(std::string const &databaseName);
};
namespace PostExecute {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Post;
  static constexpr auto url = "/_admin/execute";
  static constexpr auto summery = "Execute program";
  static constexpr auto operationId = "(RestAdminExecuteHandler";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Administration/post_admin_execute.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Administration"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The body to be executed.)";
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"additionalProperties":{},"type":"object"})";
    P0.xDescriptionOffset = 0;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PostExecuteResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response403 = 403,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned when everything went well, or if a timeout occurred. In the
latter case a body of type application/json indicating the timeout
is returned. depending on *returnAsJSON* this is a json object or a plain string.)"}, 
{Response403, R"(is returned if ArangoDB is not running in cluster mode.)"}, 
{Response404, R"(is returned if ArangoDB was not compiled for cluster operation.

)"}};
  static constexpr auto description = R"(Executes the javascript code in the body on the server as the body
of a function with no arguments. If you have a *return* statement
then the return value you produce will be returned as content type
*application/json*. If the parameter *returnAsJSON* is set to
*true*, the result will be a JSON object describing the return value
directly, otherwise a string produced by JSON.stringify will be
returned.

Note that this API endpoint will only be present if the server was
started with the option `--javascript.allow-admin-execute true`.

The default value of this option is `false`, which disables the execution of 
user-defined code and disables this API endpoint entirely. 
This is also the recommended setting for production. )";


  ApiResult request(std::string const &databaseName);
};
namespace GetLog {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_admin/log";
  static constexpr auto summery = "Read global logs from the server";
  static constexpr auto operationId = "(setLogLevel:read";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Administration/get_admin_modules_flush.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Administration"};
  inline Parameter P0;
  inline Parameter P1;
  inline Parameter P2;
  inline Parameter P3;
  inline Parameter P4;
  inline Parameter P5;
  inline Parameter P6;
  inline void initParam(){
    P0.description = R"(Returns all log entries up to log level *upto*. Note that *upto* must be:
- *fatal* or *0*
- *error* or *1*
- *warning* or *2*
- *info* or *3*
- *debug*  or *4*
The default value is *info*.)";
    P0.in = ParamIn::query;
    P0.name = "upto";
    P0.required = false;
    P0.type = ParamFormat::stringFormat;

    P1.description = R"(Returns all log entries of log level *level*. Note that the query parameters
*upto* and *level* are mutually exclusive.)";
    P1.in = ParamIn::query;
    P1.name = "level";
    P1.required = false;
    P1.type = ParamFormat::stringFormat;

    P2.description = R"(Returns all log entries such that their log entry identifier (*lid* value)
is greater or equal to *start*.)";
    P2.in = ParamIn::query;
    P2.name = "start";
    P2.required = false;
    P2.type = ParamFormat::numberFormat;

    P3.description = R"(Restricts the result to at most *size* log entries.)";
    P3.in = ParamIn::query;
    P3.name = "size";
    P3.required = false;
    P3.type = ParamFormat::numberFormat;

    P4.description = R"(Starts to return log entries skipping the first *offset* log entries. *offset*
and *size* can be used for pagination.)";
    P4.in = ParamIn::query;
    P4.name = "offset";
    P4.required = false;
    P4.type = ParamFormat::numberFormat;

    P5.description = R"(Only return the log entries containing the text specified in *search*.)";
    P5.in = ParamIn::query;
    P5.name = "search";
    P5.required = false;
    P5.type = ParamFormat::stringFormat;

    P6.description = R"(Sort the log entries either ascending (if *sort* is *asc*) or descending
(if *sort* is *desc*) according to their *lid* values. Note that the *lid*
imposes a chronological order. The default value is *asc*.)";
    P6.in = ParamIn::query;
    P6.name = "sort";
    P6.required = false;
    P6.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1, P2, P3, P4, P5, P6};

  enum GetLogResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400,
    Response500 = 500
  };
  inline const static std::map<int, char const * > responses{{Response200, ""},
{Response400, R"(is returned if invalid values are specified for *upto* or *level*.)"}, 
{Response500, R"(is returned if the server cannot generate the result due to an out-of-memory
error.

)"}};
  static constexpr auto description = R"(Returns fatal, error, warning or info log messages from the server's global log.
The result is a JSON object with the following attributes:


**HTTP 200**
*A json document with these Properties is returned:*

- **lid** (string): a list of log entry identifiers. Each log message is uniquely
identified by its @LIT{lid} and the identifiers are in ascending
order.
- **level**: A list of the log levels for all log entries.
- **timestamp** (string): a list of the timestamps as seconds since 1970-01-01 for all log
entries.
- **topic**: a list of the topics of all log entries
- **text**: a list of the texts of all log entries
- **totalAmount**: the total amount of log entries before pagination.)";


  ApiResult request(std::string const &databaseName);
};
namespace GetLevel {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_admin/log/level";
  static constexpr auto summery = "Return the current server log level";
  static constexpr auto operationId = "(ReturnTheCurrentServerLogLevel";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Administration/get_admin_modules_flush.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Administration"};
  
  enum GetLevelResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response500 = 500
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned if the request is valid)"},
{Response500, R"(is returned if the server cannot generate the result due to an out-of-memory
error.

)"}};
  static constexpr auto description = R"(Returns the server's current log level settings.
The result is a JSON object with the log topics being the object keys, and
the log levels being the object values.)";


  ApiResult request(std::string const &databaseName);
};
namespace PutLevel {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_admin/log/level";
  static constexpr auto summery = "Modify and return the current server log level";
  static constexpr auto operationId = "(ModifyAndReturnTheCurrentServerLogLevel";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Administration/get_admin_modules_flush.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Administration"};
  inline Parameter P0;
  inline void initParam(){
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"$ref":"#/definitions/put_admin_loglevel"})";
    P0.xDescriptionOffset = 935;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PutLevelResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400,
    Response405 = 405,
    Response500 = 500
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned if the request is valid)"},
{Response400, R"(is returned when the request body contains invalid JSON.)"}, 
{Response405, R"(is returned when an invalid HTTP method is used.)"}, 
{Response500, R"(is returned if the server cannot generate the result due to an out-of-memory
error.)"}};
  static constexpr auto description = R"(Modifies and returns the server's current log level settings.
The request body must be a JSON object with the log topics being the object keys
and the log levels being the object values.

The result is a JSON object with the adjusted log topics being the object keys, and
the adjusted log levels being the object values.

It can set the log level of all facilities by only specifying the log level as string without json.

Possible log levels are:
 - FATAL - There will be no way out of this. ArangoDB will go down after this message.
 - ERROR - This is an error. you should investigate and fix it. It may harm your production.
 - WARNING - This may be serious application-wise, but we don't know.
 - INFO - Something has happened, take notice, but no drama attached.
 - DEBUG - output debug messages
 - TRACE - trace - prepare your log to be flooded - don't use in production.


**A JSON object with these properties is required:**

  - **audit-service**: One of the possible log levels.
  - **cache**: One of the possible log levels.
  - **syscall**: One of the possible log levels.
  - **communication**: One of the possible log levels.
  - **audit-authentication**: One of the possible log levels.
  - **agencycomm**: One of the possible log levels.
  - **startup**: One of the possible log levels.
  - **audit-authorization**: One of the possible log levels.
  - **general**: One of the possible log levels.
  - **cluster**: One of the possible log levels.
  - **audit-view**: One of the possible log levels.
  - **collector**: One of the possible log levels.
  - **engines**: One of the possible log levels.
  - **trx**: One of the possible log levels.
  - **mmap**: One of the possible log levels.
  - **agency**: One of the possible log levels.
  - **audit-document**: One of the possible log levels.
  - **authentication**: One of the possible log levels.
  - **memory**: One of the possible log levels.
  - **performance**: One of the possible log levels.
  - **config**: One of the possible log levels.
  - **authorization**: One of the possible log levels.
  - **development**: One of the possible log levels.
  - **datafiles**: One of the possible log levels.
  - **views**: One of the possible log levels.
  - **ldap**: One of the possible log levels.
  - **replication**: One of the possible log levels.
  - **threads**: One of the possible log levels.
  - **audit-database**: One of the possible log levels.
  - **v8**: One of the possible log levels.
  - **ssl**: One of the possible log levels.
  - **pregel**: One of the possible log levels.
  - **audit-collection**: One of the possible log levels.
  - **rocksdb**: One of the possible log levels.
  - **supervision**: One of the possible log levels.
  - **graphs**: One of the possible log levels.
  - **compactor**: One of the possible log levels.
  - **queries**: One of the possible log levels.
  - **heartbeat**: One of the possible log levels.
  - **requests**: One of the possible log levels.
)";


  ApiResult request(std::string const &databaseName);
};
namespace PostReload {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Post;
  static constexpr auto url = "/_admin/routing/reload";
  static constexpr auto summery = "Reloads the routing information";
  static constexpr auto operationId = "(RestAdminRoutingHandler";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Administration/get_admin_routing_reloads.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Administration"};
  
  enum PostReloadResponse : short {
    Unknown = 0,
    Response200 = 200
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Routing information was reloaded successfully.
)"}};
  static constexpr auto description = R"(Reloads the routing information from the collection *routing*.)";


  ApiResult request(std::string const &databaseName);
};
namespace GetAvailability {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_admin/server/availability";
  static constexpr auto summery = "Return whether or not a server is available";
  static constexpr auto operationId = "(handleAvailability";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Administration/get_admin_server_availability.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Administration"};
  
  enum GetAvailabilityResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response503
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(This API will return HTTP 200 in case the server is up and running and usable for
arbitrary operations, is not set to read-only mode and is currently not a follower 
in case of an active failover setup.)"}, 
{Response503, R"(HTTP 503 will be returned in case the server is during startup or during shutdown,
is set to read-only mode or is currently a follower in an active failover setup.
)"}};
  static constexpr auto description = R"(Return availability information about a server.

This is a public API so it does *not* require authentication. It is meant to be
used only in the context of server monitoring only.)";


  ApiResult request(std::string const &databaseName);
};
namespace GetId {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_admin/server/id";
  static constexpr auto summery = "Return id of a server in a cluster";
  static constexpr auto operationId = "(handleId";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Administration/get_admin_server_id.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Administration"};
  
  enum GetIdResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response500 = 500
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Is returned when the server is running in cluster mode.)"},
{Response500, R"(Is returned when the server is not running in cluster mode.)"}};
  static constexpr auto description = R"(Returns the id of a server in a cluster. The request will fail if the
server is not running in cluster mode.)";


  ApiResult request(std::string const &databaseName);
};
namespace GetMode {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_admin/server/mode";
  static constexpr auto summery = "Return whether or not a server is in read-only mode";
  static constexpr auto operationId = "(handleMode:get";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Administration/get_admin_server_mode.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Administration"};
  
  enum GetModeResponse : short {
    Unknown = 0,
    Response200 = 200
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(This API will return HTTP 200 if everything is ok)"}};
  static constexpr auto description = R"(Return mode information about a server. The json response will contain
a field `mode` with the value `readonly` or `default`. In a read-only server
all write operations will fail with an error code of `1004` (_ERROR_READ_ONLY_).
Creating or dropping of databases and collections will also fail with error code `11` (_ERROR_FORBIDDEN_).

This is a public API so it does *not* require authentication. )";


  ApiResult request(std::string const &databaseName);
};
namespace PutMode {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_admin/server/mode";
  static constexpr auto summery = "Update whether or not a server is in read-only mode";
  static constexpr auto operationId = "(handleMode:set";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Administration/put_admin_server_mode.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Administration"};
  inline Parameter P0;
  inline void initParam(){
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"$ref":"#/definitions/put_admin_server_mode"})";
    P0.xDescriptionOffset = 54;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PutModeResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response401 = 401
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(This API will return HTTP 200 if everything is ok)"},
{Response401, R"(if the request was not authenticated as a user with sufficient rights
)"}};
  static constexpr auto description = R"(
**A JSON object with these properties is required:**

  - **mode**: The mode of the server `readonly` or `default`.




Update mode information about a server. The json response will contain
a field `mode` with the value `readonly` or `default`. In a read-only server
all write operations will fail with an error code of `1004` (_ERROR_READ_ONLY_).
Creating or dropping of databases and collections will also fail with error code `11` (_ERROR_FORBIDDEN_).

This API so it *does require* authentication and administrative server rights.)";


  ApiResult request(std::string const &databaseName);
};
namespace GetRole {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_admin/server/role";
  static constexpr auto summery = "Return the role of a server in a cluster";
  static constexpr auto operationId = "(handleRole";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Administration/get_admin_server_role.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Administration"};
  
  enum GetRoleResponse : short {
    Unknown = 0,
    Response200 = 200
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Is returned in all cases.)"}};
  static constexpr auto description = R"(Returns the role of a server in a cluster.
The role is returned in the *role* attribute of the result.
Possible return values for *role* are:
- *SINGLE*: the server is a standalone server without clustering
- *COORDINATOR*: the server is a Coordinator in a cluster
- *PRIMARY*: the server is a DBServer in a cluster
- *SECONDARY*: this role is not used anymore
- *AGENT*: the server is an Agency node in a cluster
- *UNDEFINED*: in a cluster, *UNDEFINED* is returned if the server role cannot be
   determined.


**HTTP 200**
*A json document with these Properties is returned:*

Is returned in all cases.

- **errorNum**: the server error number
- **code**: the HTTP status code, always 200
- **role**: one of [ *SINGLE*, *COORDINATOR*, *PRIMARY*, *SECONDARY*, *AGENT*, *UNDEFINED*]
- **error**: always *false*)";


  ApiResult request(std::string const &databaseName);
};
namespace DeleteShutdown {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Delete;
  static constexpr auto url = "/_admin/shutdown";
  static constexpr auto summery = "Initiate shutdown sequence";
  static constexpr auto operationId = "(RestShutdownHandler";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Administration/delete_api_shutdown.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Administration"};
  
  enum DeleteShutdownResponse : short {
    Unknown = 0,
    Response200 = 200
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned in all cases, `OK` will be returned in the result buffer on success.

)"}};
  static constexpr auto description = R"(This call initiates a clean shutdown sequence. Requires administrive privileges)";


  ApiResult request(std::string const &databaseName);
};
namespace GetStatistics {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_admin/statistics";
  static constexpr auto summery = "Read the statistics";
  static constexpr auto operationId = "(getStatistics";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Administration/get_admin_statistics.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Administration"};
  
  enum GetStatisticsResponse : short {
    Unknown = 0,
    Response200 = 200
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Statistics were returned successfully.)"}};
  static constexpr auto description = R"(Returns the statistics information. The returned object contains the
statistics figures grouped together according to the description returned by
*_admin/statistics-description*. For instance, to access a figure *userTime*
from the group *system*, you first select the sub-object describing the
group stored in *system* and in that sub-object the value for *userTime* is
stored in the attribute of the same name.

In case of a distribution, the returned object contains the total count in
*count* and the distribution list in *counts*. The sum (or total) of the
individual values is returned in *sum*.


**HTTP 200**
*A json document with these Properties is returned:*

Statistics were returned successfully.

- **code**: the HTTP status code - 200 in this case
- **http**:
  - **requestsTotal**: total number of http requests
  - **requestsPatch**: No of requests using the PATCH-verb
  - **requestsPut**: No of requests using the PUT-verb
  - **requestsOther**: No of requests using the none of the above identified verbs
  - **requestsAsync**: total number of asynchronous http requests
  - **requestsPost**: No of requests using the POST-verb
  - **requestsOptions**: No of requests using the OPTIONS-verb
  - **requestsHead**: No of requests using the HEAD-verb
  - **requestsGet**: No of requests using the GET-verb
  - **requestsDelete**: No of requests using the DELETE-verb
- **errorMessage**: a descriptive error message
- **enabled**: *true* if the server has the statistics module enabled. If not, don't expect any values.
- **system**:
  - **minorPageFaults**: pagefaults
  - **majorPageFaults**: pagefaults
  - **userTime**: the user CPU time used by the server process
  - **systemTime**: the system CPU time used by the server process
  - **numberOfThreads**: the number of threads in the server
  - **virtualSize**: VSS of the process
  - **residentSize**: RSS of process
  - **residentSizePercent**: RSS of process in %
- **server**:
  - **threads**:
    - **in-progress**: The number of currently busy worker threads
    - **scheduler-threads**: The number of spawned worker threads
    - **queued**: The number of jobs queued up waiting for worker threads becomming available
  - **uptime**: time the server is up and running
  - **physicalMemory**: available physical memory on the server
  - **v8Context**:
    - **available**: the number of currently spawnen V8 contexts
    - **max**: the total number of V8 contexts we may spawn as configured by --javascript.v8-contexts
    - **busy**: the number of currently active V8 contexts
    - **dirty**: the number of contexts that were previously used, and should now be garbage collected before being re-used
    - **free**: the number of V8 contexts that are free to use
- **client**:
  - **totalTime**:
    - **count**: number of values summarized
    - **sum**: summarized value of all counts
    - **counts** (integer): array containing the values
  - **bytesReceived**:
    - **count**: number of values summarized
    - **sum**: summarized value of all counts
    - **counts** (integer): array containing the values
  - **requestTime**:
    - **count**: number of values summarized
    - **sum**: summarized value of all counts
    - **counts** (integer): array containing the values
  - **connectionTime**:
    - **count**: number of values summarized
    - **sum**: summarized value of all counts
    - **counts** (integer): array containing the values
  - **queueTime**:
    - **count**: number of values summarized
    - **sum**: summarized value of all counts
    - **counts** (integer): array containing the values
  - **httpConnections**: the number of open http connections
  - **bytesSent**:
    - **count**: number of values summarized
    - **sum**: summarized value of all counts
    - **counts** (integer): array containing the values
  - **ioTime**:
    - **count**: number of values summarized
    - **sum**: summarized value of all counts
    - **counts** (integer): array containing the values
- **error**: boolean flag to indicate whether an error occurred (*false* in this case)
- **time**: the current server timestamp


)";


  ApiResult request(std::string const &databaseName);
};
namespace GetStatisticsDescription {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_admin/statistics-description";
  static constexpr auto summery = "Statistics description";
  static constexpr auto operationId = "(getStatisticsDescription";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Administration/get_admin_statistics_description.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Administration"};
  
  enum GetStatisticsDescriptionResponse : short {
    Unknown = 0,
    Response200 = 200
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Description was returned successfully.
)"}};
  static constexpr auto description = R"(Returns a description of the statistics returned by */_admin/statistics*.
The returned objects contains an array of statistics groups in the attribute
*groups* and an array of statistics figures in the attribute *figures*.

A statistics group is described by

- *group*: The identifier of the group.
- *name*: The name of the group.
- *description*: A description of the group.

A statistics figure is described by

- *group*: The identifier of the group to which this figure belongs.
- *identifier*: The identifier of the figure. It is unique within the group.
- *name*: The name of the figure.
- *description*: A description of the figure.
- *type*: Either *current*, *accumulated*, or *distribution*.
- *cuts*: The distribution vector.
- *units*: Units in which the figure is measured.


**HTTP 200**
*A json document with these Properties is returned:*

Description was returned successfully.

- **code**: the HTTP status code
- **figures**: A statistics figure
  - **group**: The identifier of the group to which this figure belongs.
  - **name**: The name of the figure.
  - **cuts**: The distribution vector.
  - **units**: Units in which the figure is measured.
  - **identifier**: The identifier of the figure. It is unique within the group.
  - **type**: Either *current*, *accumulated*, or *distribution*.
  - **description**: A description of the figure.
- **groups**: A statistics group
  - **group**: The identifier of the group.
  - **name**: The name of the group.
  - **description**: A description of the group.
- **error**: the error, *false* in this case


)";


  ApiResult request(std::string const &databaseName);
};
namespace GetStatus {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_admin/status";
  static constexpr auto summery = "Return status information";
  static constexpr auto operationId = "(RestStatusHandler";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Administration/JSF_get_admin_status.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Administration"};
  
  enum GetStatusResponse : short {
    Unknown = 0,
    Response200 = 200
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Status information was returned successfully.

)"}};
  static constexpr auto description = R"(Returns status information about the server.

This is intended for manual use by the support and should
never be used for monitoring or automatic tests. The results
are subject to change without notice.

The call returns an object with the following attributes:

- *server*: always *arango*.

- *license*: either *community* or *enterprise*.

- *version*: the server version as string.

- *mode* : either *server* or *console*.

- *host*: the hostname, see *ServerState*.

- *serverInfo.role*: either *SINGLE*, *COORDINATOR*, *PRIMARY*, *AGENT*.

- *serverInfo.writeOpsEnabled*: boolean, true if writes are enabled.

- *serverInfo.maintenance*: boolean, true if maintenace mode is enabled.

- *agency.endpoints*: a list of possible agency endpoints.

An agent, coordinator or primary will also have

- *serverInfo.persistedId*: the persisted ide, e. g. *"CRDN-e427b441-5087-4a9a-9983-2fb1682f3e2a"*.

A coordinator or primary will also have

- *serverInfo.state*: *SERVING*

- *serverInfo.address*: the address of the server, e. g. *tcp://[::1]:8530*.

- *serverInfo.serverId*: the server ide, e. g. *"CRDN-e427b441-5087-4a9a-9983-2fb1682f3e2a"*.

A coordinator will also have

- *coordinator.foxxmaster*: the server id of the foxx master.

- *coordinator.isFoxxmaster*: boolean, true if the server is the foxx master.

An agent will also have

- *agent.id*: server id of this agent.

- *agent.leaderId*: server id of the leader.

- *agent.leading*: boolean, true if leading.

- *agent.endpoint*: the endpoint of this agent.

- *agent.term*: current term number.)";


  ApiResult request(std::string const &databaseName);
};
namespace GetTime {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_admin/time";
  static constexpr auto summery = "Return system time";
  static constexpr auto operationId = "(RestTimeHandler";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Administration/get_admin_time.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Administration"};
  
  enum GetTimeResponse : short {
    Unknown = 0,
    Response200 = 200
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Time was returned successfully.)"}};
  static constexpr auto description = R"(The call returns an object with the attribute *time*. This contains the
current system time as a Unix timestamp with microsecond precision.


**HTTP 200**
*A json document with these Properties is returned:*

Time was returned successfully.

- **code**: the HTTP status code
- **time**: The current system time as a Unix timestamp with microsecond precision of the server
- **error**: boolean flag to indicate whether an error occurred (*false* in this case))";


  ApiResult request(std::string const &databaseName);
};
}
namespace Wal{
namespace PutFlush {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_admin/wal/flush";
  static constexpr auto summery = "Flushes the write-ahead log";
  static constexpr auto operationId = "(RestWalHandler:flush";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/wal/put_admin_wal_flush.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"wal"};
  inline Parameter P0;
  inline Parameter P1;
  inline void initParam(){
    P0.description = R"(Whether or not the operation should block until the not-yet synchronized
data in the write-ahead log was synchronized to disk.)";
    P0.in = ParamIn::query;
    P0.name = "waitForSync";
    P0.required = false;
    P0.type = ParamFormat::booleanFormat;

    P1.description = R"(Whether or not the operation should block until the data in the flushed
log has been collected by the write-ahead log garbage collector. Note that
setting this option to *true* might block for a long time if there are
long-running transactions and the write-ahead log garbage collector cannot
finish garbage collection.)";
    P1.in = ParamIn::query;
    P1.name = "waitForCollector";
    P1.required = false;
    P1.type = ParamFormat::booleanFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1};

  enum PutFlushResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response405 = 405
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Is returned if the operation succeeds.)"},
{Response405, R"(is returned when an invalid HTTP method is used.
)"}};
  static constexpr auto description = R"(Flushes the write-ahead log. By flushing the currently active write-ahead
logfile, the data in it can be transferred to collection journals and
datafiles. This is useful to ensure that all data for a collection is
present in the collection journals and datafiles, for example, when dumping
the data of a collection.)";


  ApiResult request(std::string const &databaseName);
};
namespace GetProperties {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_admin/wal/properties";
  static constexpr auto summery = "Retrieves the configuration of the write-ahead log";
  static constexpr auto operationId = "(RestWalHandler:properties:get";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/wal/get_admin_wal_properties.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"wal"};
  
  enum GetPropertiesResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response405 = 405
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Is returned if the operation succeeds.)"},
{Response405, R"(is returned when an invalid HTTP method is used.
)"}};
  static constexpr auto description = R"(Retrieves the configuration of the write-ahead log. The result is a JSON
object with the following attributes:
- *allowOversizeEntries*: whether or not operations that are bigger than a
  single logfile can be executed and stored
- *logfileSize*: the size of each write-ahead logfile
- *historicLogfiles*: the maximum number of historic logfiles to keep
- *reserveLogfiles*: the maximum number of reserve logfiles that ArangoDB
  allocates in the background
- *syncInterval*: the interval for automatic synchronization of not-yet
  synchronized write-ahead log data (in milliseconds)
- *throttleWait*: the maximum wait time that operations will wait before
  they get aborted if case of write-throttling (in milliseconds)
- *throttleWhenPending*: the number of unprocessed garbage-collection
  operations that, when reached, will activate write-throttling. A value of
  *0* means that write-throttling will not be triggered.


)";


  ApiResult request(std::string const &databaseName);
};
namespace PutProperties {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_admin/wal/properties";
  static constexpr auto summery = "Configures the write-ahead log";
  static constexpr auto operationId = "(RestWalHandler:properties";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/wal/put_admin_wal_properties.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"wal"};
  
  enum PutPropertiesResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response405 = 405
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Is returned if the operation succeeds.)"},
{Response405, R"(is returned when an invalid HTTP method is used.
)"}};
  static constexpr auto description = R"(Configures the behavior of the write-ahead log. The body of the request
must be a JSON object with the following attributes:
- *allowOversizeEntries*: whether or not operations that are bigger than a
  single logfile can be executed and stored
- *logfileSize*: the size of each write-ahead logfile
- *historicLogfiles*: the maximum number of historic logfiles to keep
- *reserveLogfiles*: the maximum number of reserve logfiles that ArangoDB
  allocates in the background
- *throttleWait*: the maximum wait time that operations will wait before
  they get aborted if case of write-throttling (in milliseconds)
- *throttleWhenPending*: the number of unprocessed garbage-collection
  operations that, when reached, will activate write-throttling. A value of
  *0* means that write-throttling will not be triggered.

Specifying any of the above attributes is optional. Not specified attributes
will be ignored and the configuration for them will not be modified.


)";


  ApiResult request(std::string const &databaseName);
};
namespace GetTransactions {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_admin/wal/transactions";
  static constexpr auto summery = "Returns information about the currently running transactions";
  static constexpr auto operationId = "(RestWalHandler:transactions";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/wal/get_admin_wal_transactions.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"wal"};
  
  enum GetTransactionsResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response405 = 405
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Is returned if the operation succeeds.)"},
{Response405, R"(is returned when an invalid HTTP method is used.
)"}};
  static constexpr auto description = R"(Returns information about the currently running transactions. The result
is a JSON object with the following attributes:
- *runningTransactions*: number of currently running transactions
- *minLastCollected*: minimum id of the last collected logfile (at the
  start of each running transaction). This is *null* if no transaction is
  running.
- *minLastSealed*: minimum id of the last sealed logfile (at the
  start of each running transaction). This is *null* if no transaction is
  running.


)";


  ApiResult request(std::string const &databaseName);
};
}
namespace Analyzer{
namespace GetAnalyzer {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/analyzer";
  static constexpr auto summery = "List all analyzers";
  static constexpr auto operationId = "(RestAnalyzerHandler:List";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Analyzers/get_api_analyzers.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Analyzers"};
  
  enum GetAnalyzerResponse : short {
    Unknown = 0,
    Response200 = 200
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(The analyzer definitions was retrieved successfully.)"}};
  static constexpr auto description = R"(Retrieves a an array of all analyzer definitions.
The resulting array contains objects with the following attributes:
- *name*: the analyzer name
- *type*: the analyzer type
- *properties*: the properties used to configure the specified type
- *features*: the set of features to set on the analyzer generated fields


)";


  ApiResult request(std::string const &databaseName);
};
namespace PostAnalyzer {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Post;
  static constexpr auto url = "/_api/analyzer";
  static constexpr auto summery = "Create an analyzer with the suppiled definition";
  static constexpr auto operationId = "(RestAnalyzerHandler:Create";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Analyzers/post_api_analyzer.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Analyzers"};
  inline Parameter P0;
  inline void initParam(){
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"$ref":"#/definitions/post_api_analyzer"})";
    P0.xDescriptionOffset = 54;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PostAnalyzerResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response201 = 201,
    Response400 = 400,
    Response403 = 403
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(An analyzer with a matching name and definition already exists.)"},
{Response201, R"(A new analyzer definition was successfully created.)"}, 
{Response400, R"(One or more of the required parameters is missing or one or more of the parameters
is not valid.)"}, 
{Response403, R"(The user does not have permission to create and analyzer with this configuration.)"}};
  static constexpr auto description = R"(
**A JSON object with these properties is required:**

  - **features** (string): The set of features to set on the analyzer generated fields.
   The default value is an empty array.
  - **type**: The analyzer type.
  - **name**: The analyzer name.
  - **properties**: The properties used to configure the specified type.
   Value may be a string, an object or null.
   The default value is *null*.




Creates a new analyzer based on the provided configuration.


)";


  ApiResult request(std::string const &databaseName);
};
namespace DeleteAnalyzerName {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Delete;
  static constexpr auto url = "/_api/analyzer/{analyzer-name}";
  static constexpr auto summery = "Remove an analyzer";
  static constexpr auto operationId = "(RestAnalyzerHandler:Delete";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Analyzers/delete_api_analyzer.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Analyzers"};
  inline Parameter P0;
  inline Parameter P1;
  inline void initParam(){
    P0.description = R"(The name of the analyzer to remove.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "analyzer-name";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.description = R"(The analyzer configuration should be removed even if it is in-use.
The default value is *false*.)";
    P1.in = ParamIn::query;
    P1.name = "force";
    P1.required = false;
    P1.type = ParamFormat::booleanFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1};

  enum DeleteAnalyzerNameResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400,
    Response403 = 403,
    Response404 = 404,
    Response409 = 409
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(The analyzer configuration was removed successfully.)"},
{Response400, R"(The *analyzer-name* was not supplied or another request parameter was not
valid.)"}, 
{Response403, R"(The user does not have permission to remove this analyzer configuration.)"}, 
{Response404, R"(Such an analyzer configuration does not exist.)"}, 
{Response409, R"(The specified analyzer configuration is still in use and *force* was omitted or
*false* specified.)"}};
  static constexpr auto description = R"(Removes an analyzer configuration identified by *analyzer-name*.

If the analyzer definition was successfully dropped, an object is returned with
the following attributes:
- *error*: *false*
- *name*: The name of the removed analyzer


)";


  ApiResult request(std::string const &databaseName);
};
namespace GetAnalyzerName {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/analyzer/{analyzer-name}";
  static constexpr auto summery = "Return the analyzer definition";
  static constexpr auto operationId = "(RestAnalyzerHandler:GetDefinition";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Analyzers/get_api_analyzer.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Analyzers"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The name of the analyzer to retrieve.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "analyzer-name";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum GetAnalyzerNameResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(The analyzer definition was retrieved successfully.)"},
{Response404, R"(Such an analyzer configuration does not exist.)"}};
  static constexpr auto description = R"(Retrieves the full definition for the specified analyzer name.
The resulting object contains the following attributes:
- *name*: the analyzer name
- *type*: the analyzer type
- *properties*: the properties used to configure the specified type
- *features*: the set of features to set on the analyzer generated fields


)";


  ApiResult request(std::string const &databaseName);
};
}
namespace AQL {
namespace GetAqlfunction {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/aqlfunction";
  static constexpr auto summery = "Return registered AQL user functions";
  static constexpr auto operationId = "(RestAqlUserFunctionsHandler:List";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/AQL/get_api_aqlfunction.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"AQL"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(Returns all registered AQL user functions from namespace *namespace* under *result*.)";
    P0.in = ParamIn::query;
    P0.name = "namespace";
    P0.required = false;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum GetAqlfunctionResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(on success *HTTP 200* is returned.)"},
{Response400, R"(If the user function name is malformed, the server will respond with *HTTP 400*.)"}};
  static constexpr auto description = R"(Returns all registered AQL user functions.

The call will return a JSON array with status codes and all user functions found under *result*.


**HTTP 200**
*A json document with these Properties is returned:*

on success *HTTP 200* is returned.

- **code**: the HTTP status code
- **result**: All functions, or the ones matching the *namespace* parameter 
  - **isDeterministic**: an optional boolean value to indicate whether the function
   results are fully deterministic (function return value solely depends on
   the input value and return value is the same for repeated calls with same
   input). The *isDeterministic* attribute is currently not used but may be
   used later for optimizations.
  - **code**: A string representation of the function body
  - **name**: The fully qualified name of the user function
- **error**: boolean flag to indicate whether an error occurred (*false* in this case)


**HTTP 400**
*A json document with these Properties is returned:*

If the user function name is malformed, the server will respond with *HTTP 400*.

- **errorMessage**: a descriptive error message
- **errorNum**: the server error number
- **code**: the HTTP status code
- **error**: boolean flag to indicate whether an error occurred (*true* in this case)


)";


  ApiResult request(std::string const &databaseName);
};
namespace PostAqlfunction {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Post;
  static constexpr auto url = "/_api/aqlfunction";
  static constexpr auto summery = "Create AQL user function";
  static constexpr auto operationId = "(RestAqlUserFunctionsHandler:create";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/AQL/post_api_aqlfunction.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"AQL"};
  inline Parameter P0;
  inline void initParam(){
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"$ref":"#/definitions/post_api_aqlfunction"})";
    P0.xDescriptionOffset = 54;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PostAqlfunctionResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response201 = 201,
    Response400 = 400
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(If the function already existed and was replaced by the
call, the server will respond with *HTTP 200*.)"}, 
{Response201, R"(If the function can be registered by the server, the server will respond with
*HTTP 201*.)"}, 
{Response400, R"(If the JSON representation is malformed or mandatory data is missing from the
request, the server will respond with *HTTP 400*.)"}};
  static constexpr auto description = R"(
**A JSON object with these properties is required:**

  - **isDeterministic**: an optional boolean value to indicate whether the function
   results are fully deterministic (function return value solely depends on
   the input value and return value is the same for repeated calls with same
   input). The *isDeterministic* attribute is currently not used but may be
   used later for optimizations.
  - **code**: a string representation of the function body.
  - **name**: the fully qualified name of the user functions.





In case of success, HTTP 200 is returned.
If the function isn't valid etc. HTTP 400 including a detailed error message will be returned.


**HTTP 200**
*A json document with these Properties is returned:*

If the function already existed and was replaced by the
call, the server will respond with *HTTP 200*.

- **code**: the HTTP status code
- **isNewlyCreated**: boolean flag to indicate whether the function was newly created (*false* in this case)
- **error**: boolean flag to indicate whether an error occurred (*false* in this case)


**HTTP 201**
*A json document with these Properties is returned:*

If the function can be registered by the server, the server will respond with
*HTTP 201*.

- **code**: the HTTP status code
- **isNewlyCreated**: boolean flag to indicate whether the function was newly created (*true* in this case)
- **error**: boolean flag to indicate whether an error occurred (*false* in this case)


**HTTP 400**
*A json document with these Properties is returned:*

If the JSON representation is malformed or mandatory data is missing from the
request, the server will respond with *HTTP 400*.

- **errorMessage**: a descriptive error message
- **errorNum**: the server error number
- **code**: the HTTP status code
- **error**: boolean flag to indicate whether an error occurred (*true* in this case)


)";


  ApiResult request(std::string const &databaseName);
};
namespace DeleteName {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Delete;
  static constexpr auto url = "/_api/aqlfunction/{name}";
  static constexpr auto summery = "Remove existing AQL user function";
  static constexpr auto operationId = "(RestAqlUserFunctionsHandler:Remove";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/AQL/delete_api_aqlfunction.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"AQL"};
  inline Parameter P0;
  inline Parameter P1;
  inline void initParam(){
    P0.description = R"(the name of the AQL user function.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "name";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.description = R"(- *true*: The function name provided in *name* is treated as
  a namespace prefix, and all functions in the specified namespace will be deleted.
  The returned number of deleted functions may become 0 if none matches the string.
- *false*: The function name provided in *name* must be fully
  qualified, including any namespaces. If none matches the *name*, HTTP 404 is returned. )";
    P1.in = ParamIn::query;
    P1.name = "group";
    P1.required = false;
    P1.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1};

  enum DeleteNameResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(If the function can be removed by the server, the server will respond with
*HTTP 200*.)"}, 
{Response400, R"(If the user function name is malformed, the server will respond with *HTTP 400*.)"}, 
{Response404, R"(If the specified user user function does not exist, the server will respond with *HTTP 404*.)"}};
  static constexpr auto description = R"(Removes an existing AQL user function or function group, identified by *name*.


**HTTP 200**
*A json document with these Properties is returned:*

If the function can be removed by the server, the server will respond with
*HTTP 200*.

- **deletedCount**: The number of deleted user functions, always `1` when `group` is set to *false*. 
Any number `>= 0` when `group` is set to *true*
- **code**: the HTTP status code
- **error**: boolean flag to indicate whether an error occurred (*false* in this case)


**HTTP 400**
*A json document with these Properties is returned:*

If the user function name is malformed, the server will respond with *HTTP 400*.

- **errorMessage**: a descriptive error message
- **errorNum**: the server error number
- **code**: the HTTP status code
- **error**: boolean flag to indicate whether an error occurred (*true* in this case)


**HTTP 404**
*A json document with these Properties is returned:*

If the specified user user function does not exist, the server will respond with *HTTP 404*.

- **errorMessage**: a descriptive error message
- **errorNum**: the server error number
- **code**: the HTTP status code
- **error**: boolean flag to indicate whether an error occurred (*true* in this case)


)";


  ApiResult request(std::string const &databaseName);
};
namespace PostBatch {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Post;
  static constexpr auto url = "/_api/batch";
  static constexpr auto summery = "executes a batch request";
  static constexpr auto operationId = "(RestBatchHandler";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Bulk/batch_processing.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Bulk"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The multipart batch request, consisting of the envelope and the individual
batch parts.)";
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"additionalProperties":{},"type":"object"})";
    P0.xDescriptionOffset = 0;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PostBatchResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400,
    Response405 = 405
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned if the batch was received successfully. HTTP 200 is returned
even if one or multiple batch part actions failed.)"}, 
{Response400, R"(is returned if the batch envelope is malformed or incorrectly formatted.
This code will also be returned if the content-type of the overall batch
request or the individual MIME parts is not as expected.)"}, 
{Response405, R"(is returned when an invalid HTTP method is used.)"}};
  static constexpr auto description = R"(Executes a batch request. A batch request can contain any number of
other requests that can be sent to ArangoDB in isolation. The benefit of
using batch requests is that batching requests requires less client/server
roundtrips than when sending isolated requests.

All parts of a batch request are executed serially on the server. The
server will return the results of all parts in a single response when all
parts are finished.

Technically, a batch request is a multipart HTTP request, with
content-type `multipart/form-data`. A batch request consists of an
envelope and the individual batch part actions. Batch part actions
are "regular" HTTP requests, including full header and an optional body.
Multiple batch parts are separated by a boundary identifier. The
boundary identifier is declared in the batch envelope. The MIME content-type
for each individual batch part must be `application/x-arango-batchpart`.

Please note that when constructing the individual batch parts, you must
use CRLF (`\\\r\\\n`) as the line terminator as in regular HTTP messages.

The response sent by the server will be an `HTTP 200` response, with an
optional error summary header `x-arango-errors`. This header contains the
number of batch part operations that failed with an HTTP error code of at
least 400. This header is only present in the response if the number of
errors is greater than zero.

The response sent by the server is a multipart response, too. It contains
the individual HTTP responses for all batch parts, including the full HTTP
result header (with status code and other potential headers) and an
optional result body. The individual batch parts in the result are
seperated using the same boundary value as specified in the request.

The order of batch parts in the response will be the same as in the
original client request. Client can additionally use the `Content-Id`
MIME header in a batch part to define an individual id for each batch part.
The server will return this id is the batch part responses, too.


)";


  ApiResult request(std::string const &databaseName);
};
}
namespace Administration {
namespace GetEndpoints {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/cluster/endpoints";
  static constexpr auto summery = "Get information about all coordinator endpoints";
  static constexpr auto operationId = "(handleCommandEndpoints:listClusterEndpoints";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Administration/get_api_cluster_endpoints.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Administration"};
  
  enum GetEndpointsResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response403 = 403
  };
  inline const static std::map<int, char const * > responses{{Response200, ""},
{Response403, ""}};
  static constexpr auto description = R"(Returns an object with an attribute `endpoints`, which contains an
array of objects, which each have the attribute `endpoint`, whose value
is a string with the endpoint description. There is an entry for each
coordinator in the cluster. This method only works on coordinators in
cluster mode. In case of an error the `error` attribute is set to
`true`.


**HTTP 200**
*A json document with these Properties is returned:*

- **endpoints**: A list of active cluster endpoints.
  - **endpoint**: The bind of the coordinator, like `tcp://[::1]:8530`
- **code**: the HTTP status code - 200
- **error**: boolean flag to indicate whether an error occurred (*true* in this case))";


  ApiResult request(std::string const &databaseName);
};
}
namespace Collection {
namespace GetCollection {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/collection";
  static constexpr auto summery = "reads all collections";
  static constexpr auto operationId = "(handleCommandGet";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Collections/get_api_collections.md";
  static constexpr auto x_hints = R"({% hint 'warning' %}
Accessing collections by their numeric ID is deprecated from version 3.4.0 on.
You should reference them via their names instead.
{% endhint %}

)";
  inline const static std::vector<char const * > tags{"Collections"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(Whether or not system collections should be excluded from the result.)";
    P0.in = ParamIn::query;
    P0.name = "excludeSystem";
    P0.required = false;
    P0.type = ParamFormat::booleanFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum GetCollectionResponse : short {
    Unknown = 0,
    Response200 = 200
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(The list of collections)"}};
  static constexpr auto description = R"(Returns an object with an attribute *collections* containing an
array of all collection descriptions. The same information is also
available in the *names* as an object with the collection names
as keys.

By providing the optional query parameter *excludeSystem* with a value of
*true*, all system collections will be excluded from the response.


<!-- Hints Start -->

**Warning:**  
Accessing collections by their numeric ID is deprecated from version 3.4.0 on.
You should reference them via their names instead.



<!-- Hints End -->
)";


  ApiResult request(std::string const &databaseName);
};
namespace PostCollection {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Post;
  static constexpr auto url = "/_api/collection";
  static constexpr auto summery = "Create collection";
  static constexpr auto operationId = "(handleCommandPost:CreateCollection";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Collections/post_api_collection.md";
  static constexpr auto x_hints = R"({% hint 'warning' %}
Accessing collections by their numeric ID is deprecated from version 3.4.0 on.
You should reference them via their names instead.
{% endhint %}

)";
  inline const static std::vector<char const * > tags{"Collections"};
  inline Parameter P0;
  inline Parameter P1;
  inline Parameter P2;
  inline void initParam(){
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"$ref":"#/definitions/post_api_collection"})";
    P0.xDescriptionOffset = 167;

    P1.description = R"(Default is *1* which means the server will only report success back to the
client if all replicas have created the collection. Set to *0* if you want
faster server responses and don't care about full replication.)";
    P1.in = ParamIn::query;
    P1.name = "waitForSyncReplication";
    P1.required = false;
    P1.type = ParamFormat::integerFormat;

    P2.description = R"(Default is *1* which means the server will check if there are enough replicas
available at creation time and bail out otherwise. Set to *0* to disable
this extra check.)";
    P2.in = ParamIn::query;
    P2.name = "enforceReplicationFactor";
    P2.required = false;
    P2.type = ParamFormat::integerFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1, P2};

  enum PostCollectionResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response200, ""},
{Response400, R"(If the *collection-name* is missing, then a *HTTP 400* is
returned.)"}, 
{Response404, R"(If the *collection-name* is unknown, then a *HTTP 404* is returned.)"}};
  static constexpr auto description = R"(Creates a new collection with a given name. The request must contain an
object with the following attributes.


**A JSON object with these properties is required:**

  - **journalSize**: The maximal size of a journal or datafile in bytes. The value
   must be at least `1048576` (1 MiB). (The default is a configuration parameter)
   This option is meaningful for the MMFiles storage engine only.
  - **replicationFactor**: (The default is *1*): in a cluster, this attribute determines how many copies
   of each shard are kept on different DBServers. The value 1 means that only one
   copy (no synchronous replication) is kept. A value of k means that k-1 replicas
   are kept. Any two copies reside on different DBServers. Replication between them is 
   synchronous, that is, every write operation to the "leader" copy will be replicated 
   to all "follower" replicas, before the write operation is reported successful.
   If a server fails, this is detected automatically and one of the servers holding 
   copies take over, usually without an error being reported.
  - **keyOptions**:
    - **allowUserKeys**: if set to *true*, then it is allowed to supply own key values in the
    *_key* attribute of a document. If set to *false*, then the key generator
    will solely be responsible for generating keys and supplying own key values
    in the *_key* attribute of documents is considered an error.
    - **type**: specifies the type of the key generator. The currently available generators are
    *traditional*, *autoincrement*, *uuid* and *padded*.
    The *traditional* key generator generates numerical keys in ascending order.
    The *autoincrement* key generator generates numerical keys in ascending order, 
    the inital offset and the spacing can be configured
    The *padded* key generator generates keys of a fixed length (16 bytes) in
    ascending lexicographical sort order. This is ideal for usage with the _RocksDB_
    engine, which will slightly benefit keys that are inserted in lexicographically
    ascending order. The key generator can be used in a single-server or cluster.
    The *uuid* key generator generates universally unique 128 bit keys, which 
    are stored in hexadecimal human-readable format. This key generator can be used
    in a single-server or cluster to generate "seemingly random" keys. The keys 
    produced by this key generator are not lexicographically sorted.
    - **increment**: increment value for *autoincrement* key generator. Not used for other key
    generator types.
    - **offset**: Initial offset value for *autoincrement* key generator.
    Not used for other key generator types.
  - **name**: The name of the collection.
  - **waitForSync**: If *true* then the data is synchronized to disk before returning from a
   document create, update, replace or removal operation. (default: false)
  - **doCompact**: whether or not the collection will be compacted (default is *true*)
   This option is meaningful for the MMFiles storage engine only.
  - **shardingStrategy**: This attribute specifies the name of the sharding strategy to use for 
   the collection. Since ArangoDB 3.4 there are different sharding strategies 
   to select from when creating a new collection. The selected *shardingStrategy* 
   value will remain fixed for the collection and cannot be changed afterwards. 
   This is important to make the collection keep its sharding settings and
   always find documents already distributed to shards using the same
   initial sharding algorithm.
   The available sharding strategies are:
   - `community-compat`: default sharding used by ArangoDB
     Community Edition before version 3.4
   - `enterprise-compat`: default sharding used by ArangoDB
     Enterprise Edition before version 3.4
   - `enterprise-smart-edge-compat`: default sharding used by smart edge
     collections in ArangoDB Enterprise Edition before version 3.4
   - `hash`: default sharding used for new collections starting from version 3.4
     (excluding smart edge collections)
   - `enterprise-hash-smart-edge`: default sharding used for new
     smart edge collections starting from version 3.4
   If no sharding strategy is specified, the default will be *hash* for
   all collections, and *enterprise-hash-smart-edge* for all smart edge
   collections (requires the *Enterprise Edition* of ArangoDB). 
   Manually overriding the sharding strategy does not yet provide a 
   benefit, but it may later in case other sharding strategies are added.
  - **isVolatile**: If *true* then the collection data is kept in-memory only and not made persistent.
   Unloading the collection will cause the collection data to be discarded. Stopping
   or re-starting the server will also cause full loss of data in the
   collection. Setting this option will make the resulting collection be
   slightly faster than regular collections because ArangoDB does not
   enforce any synchronization to disk and does not calculate any CRC
   checksums for datafiles (as there are no datafiles). This option
   should therefore be used for cache-type collections only, and not
   for data that cannot be re-created otherwise.
   (The default is *false*)
   This option is meaningful for the MMFiles storage engine only.
  - **shardKeys**: (The default is *[ "_key" ]*): in a cluster, this attribute determines
   which document attributes are used to determine the target shard for documents.
   Documents are sent to shards based on the values of their shard key attributes.
   The values of all shard key attributes in a document are hashed,
   and the hash value is used to determine the target shard.
   **Note**: Values of shard key attributes cannot be changed once set.
     This option is meaningless in a single server setup.
  - **smartJoinAttribute**: In an *Enterprise Edition* cluster, this attribute determines an attribute
   of the collection that must contain the shard key value of the referred-to 
   smart join collection. Additionally, the shard key for a document in this 
   collection must contain the value of this attribute, followed by a colon, 
   followed by the actual primary key of the document.
   This feature can only be used in the *Enterprise Edition* and requires the
   *distributeShardsLike* attribute of the collection to be set to the name
   of another collection. It also requires the *shardKeys* attribute of the
   collection to be set to a single shard key attribute, with an additional ':'
   at the end.
   A further restriction is that whenever documents are stored or updated in the 
   collection, the value stored in the *smartJoinAttribute* must be a string.
  - **numberOfShards**: (The default is *1*): in a cluster, this value determines the
   number of shards to create for the collection. In a single
   server setup, this option is meaningless.
  - **isSystem**: If *true*, create a  system collection. In this case *collection-name*
   should start with an underscore. End users should normally create non-system
   collections only. API implementors may be required to create system
   collections in very special occasions, but normally a regular collection will do.
   (The default is *false*)
  - **type**: (The default is *2*): the type of the collection to create.
   The following values for *type* are valid:
   - *2*: document collection
   - *3*: edge collection
  - **indexBuckets**: The number of buckets into which indexes using a hash
   table are split. The default is 16 and this number has to be a
   power of 2 and less than or equal to 1024.
   For very large collections one should increase this to avoid long pauses
   when the hash table has to be initially built or resized, since buckets
   are resized individually and can be initially built in parallel. For
   example, 64 might be a sensible value for a collection with 100
   000 000 documents. Currently, only the edge index respects this
   value, but other index types might follow in future ArangoDB versions.
   Changes (see below) are applied when the collection is loaded the next
   time.
   This option is meaningful for the MMFiles storage engine only.
  - **distributeShardsLike**: (The default is *""*): in an Enterprise Edition cluster, this attribute binds
   the specifics of sharding for the newly created collection to follow that of a
   specified existing collection.
   **Note**: Using this parameter has consequences for the prototype
   collection. It can no longer be dropped, before the sharding-imitating
   collections are dropped. Equally, backups and restores of imitating
   collections alone will generate warnings (which can be overridden)
   about missing sharding prototype.



**HTTP 200**
*A json document with these Properties is returned:*

- **smartGraphAttribute**: Attribute that is used in smart graphs, *Cluster specific attribute.*
- **journalSize**: The maximal size setting for journals / datafiles
in bytes. This option is only present for the MMFiles storage engine.
- **replicationFactor**: contains how many copies of each shard are kept on different DBServers.; *Cluster specific attribute.*
- **keyOptions**:
  - **lastValue**: 
  - **allowUserKeys**: if set to *true*, then it is allowed to supply
   own key values in the *_key* attribute of a document. If set to
   *false*, then the key generator is solely responsible for
   generating keys and supplying own key values in the *_key* attribute
   of documents is considered an error.
  - **type**: specifies the type of the key generator. The currently
   available generators are *traditional*, *autoincrement*, *uuid*
   and *padded*.
- **name**: literal name of this collection
- **waitForSync**: If *true* then creating, changing or removing
documents will wait until the data has been synchronized to disk.
- **doCompact**: Whether or not the collection will be compacted.
This option is only present for the MMFiles storage engine.
- **shardingStrategy**: the sharding strategy selected for the collection; *Cluster specific attribute.*
One of 'hash' or 'enterprise-hash-smart-edge'
- **isVolatile**: If *true* then the collection data will be
kept in memory only and ArangoDB will not write or sync the data
to disk. This option is only present for the MMFiles storage engine.
- **indexBuckets**: the number of index buckets
*Only relevant for the MMFiles storage engine*
- **numberOfShards**: The number of shards of the collection; *Cluster specific attribute.*
- **status**: corrosponds to **statusString**; *Only relevant for the MMFiles storage engine*
  - 0: "unknown" - may be corrupted
  - 1: (deprecated, maps to "unknown")
  - 2: "unloaded"
  - 3: "loaded"
  - 4: "unloading"
  - 5: "deleted"
  - 6: "loading"
- **statusString**: any of: ["unloaded", "loading", "loaded", "unloading", "deleted", "unknown"] *Only relevant for the MMFiles storage engine*
- **globallyUniqueId**: Unique identifier of the collection
- **id**: unique identifier of the collection; *deprecated*
- **isSystem**: true if this is a system collection; usually *name* will start with an underscore.
- **type**: The type of the collection:
  - 0: "unknown"
  - 2: regular document collection
  - 3: edge collection
- **shardKeys** (string): contains the names of document attributes that are used to
determine the target shard for documents; *Cluster specific attribute.*


<!-- Hints Start -->

**Warning:**  
Accessing collections by their numeric ID is deprecated from version 3.4.0 on.
You should reference them via their names instead.



<!-- Hints End -->
)";
  enum type : short {
    document_collection = 2,
    edge_collection = 3
    };



  ApiResult request(std::string const &databaseName, const std::string &name, type type = document_collection);

};
namespace DeleteCollectionName {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Delete;
  static constexpr auto url = "/_api/collection/{collection-name}";
  static constexpr auto summery = "Drops a collection";
  static constexpr auto operationId = "(handleCommandDelete:collection";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Collections/delete_api_collection.md";
  static constexpr auto x_hints = R"({% hint 'warning' %}
Accessing collections by their numeric ID is deprecated from version 3.4.0 on.
You should reference them via their names instead.
{% endhint %}

)";
  inline const static std::vector<char const * > tags{"Collections"};
  inline Parameter P0;
  inline Parameter P1;
  inline void initParam(){
    P0.description = R"(The name of the collection to drop.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "collection-name";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.description = R"(Whether or not the collection to drop is a system collection. This parameter
must be set to *true* in order to drop a system collection.)";
    P1.in = ParamIn::query;
    P1.name = "isSystem";
    P1.required = false;
    P1.type = ParamFormat::booleanFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1};

  enum DeleteCollectionNameResponse : short {
    Unknown = 0,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response400, R"(If the *collection-name* is missing, then a *HTTP 400* is
returned.)"}, 
{Response404, R"(If the *collection-name* is unknown, then a *HTTP 404* is returned.)"}};
  static constexpr auto description = R"(Drops the collection identified by *collection-name*.

If the collection was successfully dropped, an object is returned with
the following attributes:

- *error*: *false*

- *id*: The identifier of the dropped collection.


<!-- Hints Start -->

**Warning:**  
Accessing collections by their numeric ID is deprecated from version 3.4.0 on.
You should reference them via their names instead.



<!-- Hints End -->
)";


  ApiResult request(std::string const &databaseName);
};
namespace GetCollectionName {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/collection/{collection-name}";
  static constexpr auto summery = "Return information about a collection";
  static constexpr auto operationId = "(handleCommandGet:collectionGetProperties";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Collections/get_api_collection_name.md";
  static constexpr auto x_hints = R"({% hint 'warning' %}
Accessing collections by their numeric ID is deprecated from version 3.4.0 on.
You should reference them via their names instead.
{% endhint %}

)";
  inline const static std::vector<char const * > tags{"Collections"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The name of the collection.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "collection-name";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum GetCollectionNameResponse : short {
    Unknown = 0,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response404, R"(If the *collection-name* is unknown, then a *HTTP 404* is
returned.
)"}};
  static constexpr auto description = R"(The result is an object describing the collection with the following
attributes:

- *id*: The identifier of the collection.

- *name*: The name of the collection.

- *status*: The status of the collection as number.
 - 1: new born collection
 - 2: unloaded
 - 3: loaded
 - 4: in the process of being unloaded
 - 5: deleted
 - 6: loading

Every other status indicates a corrupted collection.

- *type*: The type of the collection as number.
  - 2: document collection (normal case)
  - 3: edges collection

- *isSystem*: If *true* then the collection is a system collection.


<!-- Hints Start -->

**Warning:**  
Accessing collections by their numeric ID is deprecated from version 3.4.0 on.
You should reference them via their names instead.



<!-- Hints End -->)";


  ApiResult request(std::string const &databaseName);
};
namespace GetChecksum {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/collection/{collection-name}/checksum";
  static constexpr auto summery = "Return checksum for the collection";
  static constexpr auto operationId = "(handleCommandGet:collectionChecksum";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Collections/get_api_collection_checksum.md";
  static constexpr auto x_hints = R"({% hint 'warning' %}
Accessing collections by their numeric ID is deprecated from version 3.4.0 on.
You should reference them via their names instead.
{% endhint %}

)";
  inline const static std::vector<char const * > tags{"Collections"};
  inline Parameter P0;
  inline Parameter P1;
  inline Parameter P2;
  inline void initParam(){
    P0.description = R"(The name of the collection.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "collection-name";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.description = R"(Whether or not to include document revision ids in the checksum calculation.)";
    P1.in = ParamIn::query;
    P1.name = "withRevisions";
    P1.required = false;
    P1.type = ParamFormat::booleanFormat;

    P2.description = R"(Whether or not to include document body data in the checksum calculation.)";
    P2.in = ParamIn::query;
    P2.name = "withData";
    P2.required = false;
    P2.type = ParamFormat::booleanFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1, P2};

  enum GetChecksumResponse : short {
    Unknown = 0,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response400, R"(If the *collection-name* is missing, then a *HTTP 400* is
returned.)"}, 
{Response404, R"(If the *collection-name* is unknown, then a *HTTP 404*
is returned.)"}};
  static constexpr auto description = R"(Will calculate a checksum of the meta-data (keys and optionally revision ids) and
optionally the document data in the collection.

The checksum can be used to compare if two collections on different ArangoDB
instances contain the same contents. The current revision of the collection is
returned too so one can make sure the checksums are calculated for the same
state of data.

By default, the checksum will only be calculated on the *_key* system attribute
of the documents contained in the collection. For edge collections, the system
attributes *_from* and *_to* will also be included in the calculation.

By setting the optional query parameter *withRevisions* to *true*, then revision
ids (*_rev* system attributes) are included in the checksumming.

By providing the optional query parameter *withData* with a value of *true*,
the user-defined document attributes will be included in the calculation too.
**Note**: Including user-defined attributes will make the checksumming slower.

The response is a JSON object with the following attributes:

- *checksum*: The calculated checksum as a number.

- *revision*: The collection revision id as a string.

**Note**: this method is not available in a cluster.


<!-- Hints Start -->

**Warning:**  
Accessing collections by their numeric ID is deprecated from version 3.4.0 on.
You should reference them via their names instead.



<!-- Hints End -->
)";


  ApiResult request(std::string const &databaseName);
};
namespace GetCount {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/collection/{collection-name}/count";
  static constexpr auto summery = "Return number of documents in a collection";
  static constexpr auto operationId = "(handleCommandGet:getCollectionCount";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Collections/get_api_collection_count.md";
  static constexpr auto x_hints = R"({% hint 'warning' %}
Accessing collections by their numeric ID is deprecated from version 3.4.0 on.
You should reference them via their names instead.
{% endhint %}

)";
  inline const static std::vector<char const * > tags{"Collections"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The name of the collection.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "collection-name";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum GetCountResponse : short {
    Unknown = 0,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response400, R"(If the *collection-name* is missing, then a *HTTP 400* is
returned.)"}, 
{Response404, R"(If the *collection-name* is unknown, then a *HTTP 404*
is returned.)"}};
  static constexpr auto description = R"(In addition to the above, the result also contains the number of documents.
**Note** that this will always load the collection into memory.

- *count*: The number of documents inside the collection.


<!-- Hints Start -->

**Warning:**  
Accessing collections by their numeric ID is deprecated from version 3.4.0 on.
You should reference them via their names instead.



<!-- Hints End -->
)";


  ApiResult request(std::string const &databaseName);
};
namespace GetFigures {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/collection/{collection-name}/figures";
  static constexpr auto summery = "Return statistics for a collection";
  static constexpr auto operationId = "(handleCommandGet:collectionFigures";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Collections/get_api_collection_figures.md";
  static constexpr auto x_hints = R"({% hint 'warning' %}
Accessing collections by their numeric ID is deprecated from version 3.4.0 on.
You should reference them via their names instead.
{% endhint %}

)";
  inline const static std::vector<char const * > tags{"Collections"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The name of the collection.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "collection-name";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum GetFiguresResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Returns information about the collection:)"},
{Response400, R"(If the *collection-name* is missing, then a *HTTP 400* is
returned.)"}, 
{Response404, R"(If the *collection-name* is unknown, then a *HTTP 404*
is returned.)"}};
  static constexpr auto description = R"(In addition to the above, the result also contains the number of documents
and additional statistical information about the collection.
**Note** : This will always load the collection into memory.

**Note**: collection data that are stored in the write-ahead log only are
not reported in the results. When the write-ahead log is collected, documents
might be added to journals and datafiles of the collection, which may modify
the figures of the collection.

Additionally, the filesizes of collection and index parameter JSON files are
not reported. These files should normally have a size of a few bytes
each. Please also note that the *fileSize* values are reported in bytes
and reflect the logical file sizes. Some filesystems may use optimizations
(e.g. sparse files) so that the actual physical file size is somewhat
different. Directories and sub-directories may also require space in the
file system, but this space is not reported in the *fileSize* results.

That means that the figures reported do not reflect the actual disk
usage of the collection with 100% accuracy. The actual disk usage of
a collection is normally slightly higher than the sum of the reported
*fileSize* values. Still the sum of the *fileSize* values can still be
used as a lower bound approximation of the disk usage.


**HTTP 200**
*A json document with these Properties is returned:*

Returns information about the collection:

- **count**: The number of documents currently present in the collection.
- **journalSize**: The maximal size of a journal or datafile in bytes.
- **figures**:
  - **datafiles**:
    - **count**: The number of datafiles.
    - **fileSize**: The total filesize of datafiles (in bytes).
  - **uncollectedLogfileEntries**: The number of markers in the write-ahead
   log for this collection that have not been transferred to journals or datafiles.
  - **documentReferences**: The number of references to documents in datafiles that JavaScript code 
   currently holds. This information can be used for debugging compaction and 
   unload issues.
  - **compactionStatus**:
    - **message**: The action that was performed when the compaction was last run for the collection. 
    This information can be used for debugging compaction issues.
    - **time**: The point in time the compaction for the collection was last executed. 
    This information can be used for debugging compaction issues.
  - **compactors**:
    - **count**: The number of compactor files.
    - **fileSize**: The total filesize of all compactor files (in bytes).
  - **dead**:
    - **count**: The number of dead documents. This includes document
    versions that have been deleted or replaced by a newer version. Documents
    deleted or replaced that are contained the write-ahead log only are not reported
    in this figure.
    - **deletion**: The total number of deletion markers. Deletion markers
    only contained in the write-ahead log are not reporting in this figure.
    - **size**: The total size in bytes used by all dead documents.
  - **indexes**:
    - **count**: The total number of indexes defined for the collection, including the pre-defined
    indexes (e.g. primary index).
    - **size**: The total memory allocated for indexes in bytes.
  - **readcache**:
    - **count**: The number of revisions of this collection stored in the document revisions cache.
    - **size**: The memory used for storing the revisions of this collection in the document 
    revisions cache (in bytes). This figure does not include the document data but 
    only mappings from document revision ids to cache entry locations.
  - **waitingFor**: An optional string value that contains information about which object type is at the 
   head of the collection's cleanup queue. This information can be used for debugging 
   compaction and unload issues.
  - **alive**:
    - **count**: The number of currently active documents in all datafiles
    and journals of the collection. Documents that are contained in the
    write-ahead log only are not reported in this figure.
    - **size**: The total size in bytes used by all active documents of
    the collection. Documents that are contained in the write-ahead log only are
    not reported in this figure.
  - **lastTick**: The tick of the last marker that was stored in a journal
   of the collection. This might be 0 if the collection does not yet have
   a journal.
  - **journals**:
    - **count**: The number of journal files.
    - **fileSize**: The total filesize of all journal files (in bytes).
  - **revisions**:
    - **count**: The number of revisions of this collection managed by the storage engine.
    - **size**: The memory used for storing the revisions of this collection in the storage 
    engine (in bytes). This figure does not include the document data but only mappings 
    from document revision ids to storage engine datafile positions.


<!-- Hints Start -->

**Warning:**  
Accessing collections by their numeric ID is deprecated from version 3.4.0 on.
You should reference them via their names instead.



<!-- Hints End -->
)";


  ApiResult request(std::string const &databaseName);
};
namespace PutLoad {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/collection/{collection-name}/load";
  static constexpr auto summery = "Load collection";
  static constexpr auto operationId = "(handleCommandPut:loadCollection";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Collections/put_api_collection_load.md";
  static constexpr auto x_hints = R"({% hint 'warning' %}
Accessing collections by their numeric ID is deprecated from version 3.4.0 on.
You should reference them via their names instead.
{% endhint %}

)";
  inline const static std::vector<char const * > tags{"Collections"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The name of the collection.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "collection-name";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PutLoadResponse : short {
    Unknown = 0,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response400, R"(If the *collection-name* is missing, then a *HTTP 400* is
returned.)"}, 
{Response404, R"(If the *collection-name* is unknown, then a *HTTP 404*
is returned.)"}};
  static constexpr auto description = R"(Loads a collection into memory. Returns the collection on success.

The request body object might optionally contain the following attribute:

- *count*: If set, this controls whether the return value should include
  the number of documents in the collection. Setting *count* to
  *false* may speed up loading a collection. The default value for
  *count* is *true*.

On success an object with the following attributes is returned:

- *id*: The identifier of the collection.

- *name*: The name of the collection.

- *count*: The number of documents inside the collection. This is only
  returned if the *count* input parameters is set to *true* or has
  not been specified.

- *status*: The status of the collection as number.

- *type*: The collection type. Valid types are:
  - 2: document collection
  - 3: edges collection

- *isSystem*: If *true* then the collection is a system collection.


<!-- Hints Start -->

**Warning:**  
Accessing collections by their numeric ID is deprecated from version 3.4.0 on.
You should reference them via their names instead.



<!-- Hints End -->
)";


  ApiResult request(std::string const &databaseName);
};
namespace PutLoadIndexesIntoMemory {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/collection/{collection-name}/loadIndexesIntoMemory";
  static constexpr auto summery = "Load Indexes into Memory";
  static constexpr auto operationId = "(handleCommandPut:loadIndexes";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Collections/put_api_collection_load_indexes_in_memory.md";
  static constexpr auto x_hints = R"({% hint 'warning' %}
Accessing collections by their numeric ID is deprecated from version 3.4.0 on.
You should reference them via their names instead.
{% endhint %}

)";
  inline const static std::vector<char const * > tags{"Collections"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = "";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "collection-name";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PutLoadIndexesIntoMemoryResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(If the indexes have all been loaded)"},
{Response400, R"(If the *collection-name* is missing, then a *HTTP 400* is
returned.)"}, 
{Response404, R"(If the *collection-name* is unknown, then a *HTTP 404* is returned.)"}};
  static constexpr auto description = R"(This route tries to cache all index entries
of this collection into the main memory.
Therefore it iterates over all indexes of the collection
and stores the indexed values, not the entire document data,
in memory.
All lookups that could be found in the cache are much faster
than lookups not stored in the cache so you get a nice performance boost.
It is also guaranteed that the cache is consistent with the stored data.

For the time being this function is only useful on RocksDB storage engine,
as in MMFiles engine all indexes are in memory anyways.

On RocksDB this function honors all memory limits, if the indexes you want
to load are smaller than your memory limit this function guarantees that most
index values are cached.
If the index is larger than your memory limit this function will fill up values
up to this limit and for the time being there is no way to control which indexes
of the collection should have priority over others.

On sucess this function returns an object with attribute `result` set to `true`


<!-- Hints Start -->

**Warning:**  
Accessing collections by their numeric ID is deprecated from version 3.4.0 on.
You should reference them via their names instead.



<!-- Hints End -->
)";


  ApiResult request(std::string const &databaseName);
};
namespace GetProperties {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/collection/{collection-name}/properties";
  static constexpr auto summery = "Read properties of a collection";
  static constexpr auto operationId = "(handleCommandGet:collectionProperties";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Collections/get_api_collection_properties.md";
  static constexpr auto x_hints = R"({% hint 'warning' %}
Accessing collections by their numeric ID is deprecated from version 3.4.0 on.
You should reference them via their names instead.
{% endhint %}

)";
  inline const static std::vector<char const * > tags{"Collections"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The name of the collection.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "collection-name";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum GetPropertiesResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response200, ""},
{Response400, R"(If the *collection-name* is missing, then a *HTTP 400* is
returned.)"}, 
{Response404, R"(If the *collection-name* is unknown, then a *HTTP 404*
is returned.)"}};
  static constexpr auto description = R"(
**HTTP 200**
*A json document with these Properties is returned:*

- **smartGraphAttribute**: Attribute that is used in smart graphs, *Cluster specific attribute.*
- **journalSize**: The maximal size setting for journals / datafiles
in bytes. This option is only present for the MMFiles storage engine.
- **replicationFactor**: contains how many copies of each shard are kept on different DBServers.; *Cluster specific attribute.*
- **keyOptions**:
  - **lastValue**: 
  - **allowUserKeys**: if set to *true*, then it is allowed to supply
   own key values in the *_key* attribute of a document. If set to
   *false*, then the key generator is solely responsible for
   generating keys and supplying own key values in the *_key* attribute
   of documents is considered an error.
  - **type**: specifies the type of the key generator. The currently
   available generators are *traditional*, *autoincrement*, *uuid*
   and *padded*.
- **name**: literal name of this collection
- **waitForSync**: If *true* then creating, changing or removing
documents will wait until the data has been synchronized to disk.
- **doCompact**: Whether or not the collection will be compacted.
This option is only present for the MMFiles storage engine.
- **shardingStrategy**: the sharding strategy selected for the collection; *Cluster specific attribute.*
One of 'hash' or 'enterprise-hash-smart-edge'
- **isVolatile**: If *true* then the collection data will be
kept in memory only and ArangoDB will not write or sync the data
to disk. This option is only present for the MMFiles storage engine.
- **indexBuckets**: the number of index buckets
*Only relevant for the MMFiles storage engine*
- **numberOfShards**: The number of shards of the collection; *Cluster specific attribute.*
- **status**: corrosponds to **statusString**; *Only relevant for the MMFiles storage engine*
  - 0: "unknown" - may be corrupted
  - 1: (deprecated, maps to "unknown")
  - 2: "unloaded"
  - 3: "loaded"
  - 4: "unloading"
  - 5: "deleted"
  - 6: "loading"
- **statusString**: any of: ["unloaded", "loading", "loaded", "unloading", "deleted", "unknown"] *Only relevant for the MMFiles storage engine*
- **globallyUniqueId**: Unique identifier of the collection
- **id**: unique identifier of the collection; *deprecated*
- **isSystem**: true if this is a system collection; usually *name* will start with an underscore.
- **type**: The type of the collection:
  - 0: "unknown"
  - 2: regular document collection
  - 3: edge collection
- **shardKeys** (string): contains the names of document attributes that are used to
determine the target shard for documents; *Cluster specific attribute.*






<!-- Hints Start -->

**Warning:**  
Accessing collections by their numeric ID is deprecated from version 3.4.0 on.
You should reference them via their names instead.



<!-- Hints End -->
)";


  ApiResult request(std::string const &databaseName);
};
namespace PutProperties {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/collection/{collection-name}/properties";
  static constexpr auto summery = "Change properties of a collection";
  static constexpr auto operationId = "(handleCommandPut:modifyProperties";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Collections/put_api_collection_properties.md";
  static constexpr auto x_hints = R"({% hint 'warning' %}
Accessing collections by their numeric ID is deprecated from version 3.4.0 on.
You should reference them via their names instead.
{% endhint %}

)";
  inline const static std::vector<char const * > tags{"Collections"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The name of the collection.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "collection-name";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PutPropertiesResponse : short {
    Unknown = 0,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response400, R"(If the *collection-name* is missing, then a *HTTP 400* is
returned.)"}, 
{Response404, R"(If the *collection-name* is unknown, then a *HTTP 404*
is returned.)"}};
  static constexpr auto description = R"(Changes the properties of a collection. Expects an object with the
attribute(s)

- *waitForSync*: If *true* then creating or changing a
  document will wait until the data has been synchronized to disk.

- *journalSize*: The maximal size of a journal or datafile in bytes. 
  The value must be at least `1048576` (1 MB). Note that when
  changing the journalSize value, it will only have an effect for
  additional journals or datafiles that are created. Already
  existing journals or datafiles will not be affected.

On success an object with the following attributes is returned:

- *id*: The identifier of the collection.

- *name*: The name of the collection.

- *waitForSync*: The new value.

- *journalSize*: The new value.

- *status*: The status of the collection as number.

- *type*: The collection type. Valid types are:
  - 2: document collection
  - 3: edges collection

- *isSystem*: If *true* then the collection is a system collection.

- *isVolatile*: If *true* then the collection data will be
  kept in memory only and ArangoDB will not write or sync the data
  to disk.

- *doCompact*: Whether or not the collection will be compacted.

- *keyOptions*: JSON object which contains key generation options:
  - *type*: specifies the type of the key generator. The currently
    available generators are *traditional*, *autoincrement*, *uuid*
    and *padded*.
  - *allowUserKeys*: if set to *true*, then it is allowed to supply
    own key values in the *_key* attribute of a document. If set to
    *false*, then the key generator is solely responsible for
    generating keys and supplying own key values in the *_key* attribute
    of documents is considered an error.

**Note**: except for *waitForSync*, *journalSize* and *name*, collection
properties **cannot be changed** once a collection is created. To rename
a collection, the rename endpoint must be used.


<!-- Hints Start -->

**Warning:**  
Accessing collections by their numeric ID is deprecated from version 3.4.0 on.
You should reference them via their names instead.



<!-- Hints End -->
)";


  ApiResult request(std::string const &databaseName);
};
namespace PutRecalculateCount {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/collection/{collection-name}/recalculateCount";
  static constexpr auto summery = "Recalculate count of a collection";
  static constexpr auto operationId = "(handleCommandPut:recalculateCount";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Collections/put_api_collection_recalculate_count.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Collections"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The name of the collection.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "collection-name";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PutRecalculateCountResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(If the document count was recalculated successfully, *HTTP 200* is returned.)"},
{Response404, R"(If the *collection-name* is unknown, then a *HTTP 404* is returned.
)"}};
  static constexpr auto description = R"(Recalculates the document count of a collection, if it ever becomes inconsistent.

It returns an object with the attributes

- *result*: will be *true* if recalculating the document count succeeded.

**Note**: this method is specific for the RocksDB storage engine)";


  ApiResult request(std::string const &databaseName);
};
namespace PutRename {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/collection/{collection-name}/rename";
  static constexpr auto summery = "Rename collection";
  static constexpr auto operationId = "(handleCommandPut:renameCollection";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Collections/put_api_collection_rename.md";
  static constexpr auto x_hints = R"({% hint 'warning' %}
Accessing collections by their numeric ID is deprecated from version 3.4.0 on.
You should reference them via their names instead.
{% endhint %}

)";
  inline const static std::vector<char const * > tags{"Collections"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The name of the collection to rename.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "collection-name";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PutRenameResponse : short {
    Unknown = 0,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response400, R"(If the *collection-name* is missing, then a *HTTP 400* is
returned.)"}, 
{Response404, R"(If the *collection-name* is unknown, then a *HTTP 404*
is returned.
)"}};
  static constexpr auto description = R"(Renames a collection. Expects an object with the attribute(s)

- *name*: The new name.

It returns an object with the attributes

- *id*: The identifier of the collection.

- *name*: The new name of the collection.

- *status*: The status of the collection as number.

- *type*: The collection type. Valid types are:
  - 2: document collection
  - 3: edges collection

- *isSystem*: If *true* then the collection is a system collection.

If renaming the collection succeeds, then the collection is also renamed in 
all graph definitions inside the `_graphs` collection in the current database.

**Note**: this method is not available in a cluster.


<!-- Hints Start -->

**Warning:**  
Accessing collections by their numeric ID is deprecated from version 3.4.0 on.
You should reference them via their names instead.



<!-- Hints End -->
)";


  ApiResult request(std::string const &databaseName);
};
namespace PutResponsibleShard {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/collection/{collection-name}/responsibleShard";
  static constexpr auto summery = "Return responsible shard for a document";
  static constexpr auto operationId = "(getResponsibleShard:Collection";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Collections/get_api_collection_getResponsibleShard.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Collections"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The name of the collection.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "collection-name";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PutResponsibleShardResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Returns the ID of the responsible shard:)"},
{Response400, R"(If the *collection-name* is missing, then a *HTTP 400* is
returned.)"}, 
{Response404, R"(If the *collection-name* is unknown, then an *HTTP 404*
is returned.
)"}};
  static constexpr auto description = R"(Returns the ID of the shard that is responsible for the given document
(if the document exists) or that would be responsible if such document
existed.

The response is a JSON object with a *shardId* attribute, which will 
contain the ID of the responsible shard.

**Note** : This method is only available in a cluster coordinator.)";


  ApiResult request(std::string const &databaseName);
};
namespace GetRevision {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/collection/{collection-name}/revision";
  static constexpr auto summery = "Return collection revision id";
  static constexpr auto operationId = "(handleCommandGet:collectionRevision";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Collections/get_api_collection_revision.md";
  static constexpr auto x_hints = R"({% hint 'warning' %}
Accessing collections by their numeric ID is deprecated from version 3.4.0 on.
You should reference them via their names instead.
{% endhint %}

)";
  inline const static std::vector<char const * > tags{"Collections"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The name of the collection.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "collection-name";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum GetRevisionResponse : short {
    Unknown = 0,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response400, R"(If the *collection-name* is missing, then a *HTTP 400* is
returned.)"}, 
{Response404, R"(If the *collection-name* is unknown, then a *HTTP 404*
is returned.)"}};
  static constexpr auto description = R"(In addition to the above, the result will also contain the
collection's revision id. The revision id is a server-generated
string that clients can use to check whether data in a collection
has changed since the last revision check.

- *revision*: The collection revision id as a string.


<!-- Hints Start -->

**Warning:**  
Accessing collections by their numeric ID is deprecated from version 3.4.0 on.
You should reference them via their names instead.



<!-- Hints End -->
)";


  ApiResult request(std::string const &databaseName);
};
namespace PutRotate {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/collection/{collection-name}/rotate";
  static constexpr auto summery = "Rotate journal of a collection";
  static constexpr auto operationId = "(handleCommandPut:collectionRotate";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Collections/put_api_collection_rotate.md";
  static constexpr auto x_hints = R"({% hint 'warning' %}
Accessing collections by their numeric ID is deprecated from version 3.4.0 on.
You should reference them via their names instead.
{% endhint %}

)";
  inline const static std::vector<char const * > tags{"Collections"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The name of the collection.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "collection-name";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PutRotateResponse : short {
    Unknown = 0,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response400, R"(If the collection currently has no journal, *HTTP 400* is returned.)"},
{Response404, R"(If the *collection-name* is unknown, then a *HTTP 404* is returned.)"}};
  static constexpr auto description = R"(Rotates the journal of a collection. The current journal of the collection will be closed
and made a read-only datafile. The purpose of the rotate method is to make the data in
the file available for compaction (compaction is only performed for read-only datafiles, and
not for journals).

Saving new data in the collection subsequently will create a new journal file
automatically if there is no current journal.

It returns an object with the attributes

- *result*: will be *true* if rotation succeeded

**Note**: this method is specific for the MMFiles storage engine, and there
it is not available in a cluster.


<!-- Hints Start -->

**Warning:**  
Accessing collections by their numeric ID is deprecated from version 3.4.0 on.
You should reference them via their names instead.



<!-- Hints End -->
)";


  ApiResult request(std::string const &databaseName);
};
namespace PutTruncate {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/collection/{collection-name}/truncate";
  static constexpr auto summery = "Truncate collection";
  static constexpr auto operationId = "(handleCommandPut:truncateCollection";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Collections/put_api_collection_truncate.md";
  static constexpr auto x_hints = R"({% hint 'warning' %}
Accessing collections by their numeric ID is deprecated from version 3.4.0 on.
You should reference them via their names instead.
{% endhint %}

)";
  inline const static std::vector<char const * > tags{"Collections"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The name of the collection.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "collection-name";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PutTruncateResponse : short {
    Unknown = 0,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response400, R"(If the *collection-name* is missing, then a *HTTP 400* is
returned.)"}, 
{Response404, R"(If the *collection-name* is unknown, then a *HTTP 404*
is returned.)"}};
  static constexpr auto description = R"(Removes all documents from the collection, but leaves the indexes intact.


<!-- Hints Start -->

**Warning:**  
Accessing collections by their numeric ID is deprecated from version 3.4.0 on.
You should reference them via their names instead.



<!-- Hints End -->
)";


  ApiResult request(std::string const &databaseName);
};
namespace PutUnload {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/collection/{collection-name}/unload";
  static constexpr auto summery = "Unload collection";
  static constexpr auto operationId = "(handleCommandPut:collectionUnload";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Collections/put_api_collection_unload.md";
  static constexpr auto x_hints = R"({% hint 'warning' %}
Accessing collections by their numeric ID is deprecated from version 3.4.0 on.
You should reference them via their names instead.
{% endhint %}

)";
  inline const static std::vector<char const * > tags{"Collections"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = "";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "collection-name";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PutUnloadResponse : short {
    Unknown = 0,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response400, R"(If the *collection-name* is missing, then a *HTTP 400* is
returned.)"}, 
{Response404, R"(If the *collection-name* is unknown, then a *HTTP 404* is returned.)"}};
  static constexpr auto description = R"(Removes a collection from memory. This call does not delete any documents.
You can use the collection afterwards; in which case it will be loaded into
memory, again. On success an object with the following attributes is
returned:

- *id*: The identifier of the collection.

- *name*: The name of the collection.

- *status*: The status of the collection as number.

- *type*: The collection type. Valid types are:
  - 2: document collection
  - 3: edges collection

- *isSystem*: If *true* then the collection is a system collection.


<!-- Hints Start -->

**Warning:**  
Accessing collections by their numeric ID is deprecated from version 3.4.0 on.
You should reference them via their names instead.



<!-- Hints End -->
)";


  ApiResult request(std::string const &databaseName);
};
}
namespace Cursor{
namespace PostCursor {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Post;
  static constexpr auto url = "/_api/cursor";
  static constexpr auto summery = "Create cursor";
  static constexpr auto operationId = "(createQueryCursor";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Cursors/post_api_cursor.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Cursors"};
  inline Parameter P0;
  inline void initParam(){
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"$ref":"#/definitions/post_api_cursor"})";
    P0.xDescriptionOffset = 54;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PostCursorResponse : short {
    Unknown = 0,
    Response201 = 201,
    Response400 = 400,
    Response404 = 404,
    Response405 = 405,
    Response409 = 409 // Not documented
  };
  inline const static std::map<int, char const * > responses{{Response201, R"(is returned if the result set can be created by the server.)"},
{Response400, R"(is returned if the JSON representation is malformed or the query specification is
missing from the request.

If the JSON representation is malformed or the query specification is
missing from the request, the server will respond with *HTTP 400*.

The body of the response will contain a JSON object with additional error
details. The object has the following attributes:)"}, 
{Response404, R"(The server will respond with *HTTP 404* in case a non-existing collection is
accessed in the query.)"}, 
{Response405, R"(The server will respond with *HTTP 405* if an unsupported HTTP method is used.)"},
{Response409, R"(Not documented)"}};
  static constexpr auto description = R"(
**A JSON object with these properties is required:**

  - **count**: indicates whether the number of documents in the result set should be returned in
   the "count" attribute of the result.
   Calculating the "count" attribute might have a performance impact for some queries
   in the future so this option is turned off by default, and "count"
   is only returned when requested.
  - **batchSize**: maximum number of result documents to be transferred from
   the server to the client in one roundtrip. If this attribute is
   not set, a server-controlled default value will be used. A *batchSize* value of
   *0* is disallowed.
  - **cache**: flag to determine whether the AQL query results cache
   shall be used. If set to *false*, then any query cache lookup will be skipped
   for the query. If set to *true*, it will lead to the query cache being checked
   for the query if the query cache mode is either *on* or *demand*.
  - **memoryLimit**: the maximum number of memory (measured in bytes) that the query is allowed to
   use. If set, then the query will fail with error "resource limit exceeded" in
   case it allocates too much memory. A value of *0* indicates that there is no
   memory limit.
  - **ttl**: The time-to-live for the cursor (in seconds). The cursor will be
   removed on the server automatically after the specified amount of time. This
   is useful to ensure garbage collection of cursors that are not fully fetched
   by clients. If not set, a server-defined value will be used (default: 30 seconds).
  - **query**: contains the query string to be executed
  - **bindVars** (object): key/value pairs representing the bind parameters.
  - **options**:
    - **failOnWarning**: When set to *true*, the query will throw an exception and abort instead of producing
    a warning. This option should be used during development to catch potential issues
    early. When the attribute is set to *false*, warnings will not be propagated to
    exceptions and will be returned with the query result.
    There is also a server configuration option `--query.fail-on-warning` for setting the
    default value for *failOnWarning* so it does not need to be set on a per-query level.
    - **profile**: If set to *true* or *1*, then the additional query profiling information will be returned
    in the sub-attribute *profile* of the *extra* return attribute, if the query result
    is not served from the query cache. Set to *2* the query will include execution stats
    per query plan node in sub-attribute *stats.nodes* of the *extra* return attribute.
    Additionally the query plan is returned in the sub-attribute *extra.plan*.
    - **maxTransactionSize**: Transaction size limit in bytes. Honored by the RocksDB storage engine only.
    - **stream**: Specify *true* and the query will be executed in a **streaming** fashion. The query result is
    not stored on the server, but calculated on the fly. *Beware*: long-running queries will
    need to hold the collection locks for as long as the query cursor exists. 
    When set to *false* a query will be executed right away in its entirety. 
    In that case query results are either returned right away (if the result set is small enough),
    or stored on the arangod instance and accessible via the cursor API (with respect to the `ttl`). 
    It is advisable to *only* use this option on short-running queries or without exclusive locks 
    (write-locks on MMFiles).
    Please note that the query options `cache`, `count` and `fullCount` will not work on streaming queries.
    Additionally query statistics, warnings and profiling data will only be available after the query is finished.
    The default value is *false*
    - **skipInaccessibleCollections**: AQL queries (especially graph traversals) will treat collection to which a user has no access rights as if these collections were empty. Instead of returning a forbidden access error, your queries will execute normally. This is intended to help with certain use-cases: A graph contains several collections and different users execute AQL queries on that graph. You can now naturally limit the accessible results by changing the access rights of users on collections. This feature is only available in the Enterprise Edition.
    - **maxWarningCount**: Limits the maximum number of warnings a query will return. The number of warnings
    a query will return is limited to 10 by default, but that number can be increased
    or decreased by setting this attribute.
    - **intermediateCommitCount**: Maximum number of operations after which an intermediate commit is performed
    automatically. Honored by the RocksDB storage engine only.
    - **satelliteSyncWait**: This *Enterprise Edition* parameter allows to configure how long a DBServer will have time
    to bring the satellite collections involved in the query into sync.
    The default value is *60.0* (seconds). When the max time has been reached the query
    will be stopped.
    - **fullCount**: if set to *true* and the query contains a *LIMIT* clause, then the
    result will have an *extra* attribute with the sub-attributes *stats*
    and *fullCount*, `{ ... , "extra": { "stats": { "fullCount": 123 } } }`.
    The *fullCount* attribute will contain the number of documents in the result before the
    last top-level LIMIT in the query was applied. It can be used to count the number of 
    documents that match certain filter criteria, but only return a subset of them, in one go.
    It is thus similar to MySQL's *SQL_CALC_FOUND_ROWS* hint. Note that setting the option
    will disable a few LIMIT optimizations and may lead to more documents being processed,
    and thus make queries run longer. Note that the *fullCount* attribute may only
    be present in the result if the query has a top-level LIMIT clause and the LIMIT 
    clause is actually used in the query.
    - **intermediateCommitSize**: Maximum total size of operations after which an intermediate commit is performed
    automatically. Honored by the RocksDB storage engine only.
    - **optimizer.rules** (string): A list of to-be-included or to-be-excluded optimizer rules
    can be put into this attribute, telling the optimizer to include or exclude
    specific rules. To disable a rule, prefix its name with a `-`, to enable a rule, prefix it
    with a `+`. There is also a pseudo-rule `all`, which will match all optimizer rules.
    - **maxPlans**: Limits the maximum number of plans that are created by the AQL query optimizer.




The query details include the query string plus optional query options and
bind parameters. These values need to be passed in a JSON representation in
the body of the POST request.


**HTTP 201**
*A json document with these Properties is returned:*

is returned if the result set can be created by the server.

- **count**: the total number of result documents available (only
available if the query was executed with the *count* attribute set)
- **code**: the HTTP status code
- **extra**: an optional JSON object with extra information about the query result
contained in its *stats* sub-attribute. For data-modification queries, the
*extra.stats* sub-attribute will contain the number of modified documents and
the number of documents that could not be modified
due to an error (if *ignoreErrors* query option is specified)
- **cached**: a boolean flag indicating whether the query result was served
from the query cache or not. If the query result is served from the query
cache, the *extra* return attribute will not contain any *stats* sub-attribute
and no *profile* sub-attribute.
- **hasMore**: A boolean indicator whether there are more results
available for the cursor on the server
- **result** (anonymous json object): an array of result documents (might be empty if query has no results)
- **error**: A flag to indicate that an error occurred (*false* in this case)
- **id**: id of temporary cursor created on the server (optional, see above)


**HTTP 400**
*A json document with these Properties is returned:*

is returned if the JSON representation is malformed or the query specification is
missing from the request.
If the JSON representation is malformed or the query specification is
missing from the request, the server will respond with *HTTP 400*.
The body of the response will contain a JSON object with additional error
details. The object has the following attributes:

- **errorMessage**: a descriptive error message
If the query specification is complete, the server will process the query. If an
error occurs during query processing, the server will respond with *HTTP 400*.
Again, the body of the response will contain details about the error.
A [list of query errors can be found here](../../Manual/Appendix/ErrorCodes.html).
- **errorNum**: the server error number
- **code**: the HTTP status code
- **error**: boolean flag to indicate that an error occurred (*true* in this case)


)";

  ApiResult request(std::string const &databaseName,const std::string &body, const arangodb::velocypack::Slice bindVarsObject);

};
namespace DeleteCursorIdentifier {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Delete;
  static constexpr auto url = "/_api/cursor/{cursor-identifier}";
  static constexpr auto summery = "Delete cursor";
  static constexpr auto operationId = "(deleteQueryCursor";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Cursors/post_api_cursor_delete.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Cursors"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The id of the cursor)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "cursor-identifier";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum DeleteCursorIdentifierResponse : short {
    Unknown = 0,
    Response202 = 202,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response202, R"(is returned if the server is aware of the cursor.)"},
{Response404, R"(is returned if the server is not aware of the cursor. It is also
returned if a cursor is used after it has been destroyed.)"}};
  static constexpr auto description = R"(Deletes the cursor and frees the resources associated with it.

The cursor will automatically be destroyed on the server when the client has
retrieved all documents from it. The client can also explicitly destroy the
cursor at any earlier time using an HTTP DELETE request. The cursor id must
be included as part of the URL.

Note: the server will also destroy abandoned cursors automatically after a
certain server-controlled timeout to avoid resource leakage.


)";


  ApiResult request(std::string const &databaseName);
};
namespace PutCursorIdentifier {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/cursor/{cursor-identifier}";
  static constexpr auto summery = "Read next batch from cursor";
  static constexpr auto operationId = "(modifyQueryCursor";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Cursors/post_api_cursor_identifier.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Cursors"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The name of the cursor)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "cursor-identifier";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PutCursorIdentifierResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(The server will respond with *HTTP 200* in case of success.)"},
{Response400, R"(If the cursor identifier is omitted, the server will respond with *HTTP 404*.)"}, 
{Response404, R"(If no cursor with the specified identifier can be found, the server will respond
with *HTTP 404*.)"}};
  static constexpr auto description = R"(If the cursor is still alive, returns an object with the following
attributes:

- *id*: the *cursor-identifier*
- *result*: a list of documents for the current batch
- *hasMore*: *false* if this was the last batch
- *count*: if present the total number of elements

Note that even if *hasMore* returns *true*, the next call might
still return no documents. If, however, *hasMore* is *false*, then
the cursor is exhausted.  Once the *hasMore* attribute has a value of
*false*, the client can stop.


)";


  ApiResult request(std::string const &databaseName);
};
}
namespace Database {
namespace GetDatabase {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/database";
  static constexpr auto summery = "List of databases";
  static constexpr auto operationId = "(getDatabases:all";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Database/get_api_database_list.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Database"};
  
  enum GetDatabaseResponse : short {
    UnKnown = 0,
    Response200 = 200,
    Response400 = 400,
    Response403 = 403
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned if the list of database was compiled successfully.)"},
{Response400, R"(is returned if the request is invalid.)"}, 
{Response403, R"(is returned if the request was not executed in the *_system* database.)"}};
  static constexpr auto description = R"(Retrieves the list of all existing databases

**Note**: retrieving the list of databases is only possible from within the *_system* database.

**Note**: You should use the [*GET user API*](../UserManagement/README.md#list-the-accessible-databases-for-a-user) to fetch the list of the available databases now.


)";


  ApiResult request(std::string const &databaseName);
};
namespace PostDatabase {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Post;
  static constexpr auto url = "/_api/database";
  static constexpr auto summery = "Create database";
  static constexpr auto operationId = "(createDatabase";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Database/get_api_database_new.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Database"};
  inline Parameter P0;
  inline void initParam(){
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"$ref":"#/definitions/get_api_database_new"})";
    P0.xDescriptionOffset = 54;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PostDatabaseResponse : short {
    Unknown = 0,
    Response201 = 201,
    Response400 = 400,
    Response403 = 403,
    Response409 = 409
  };
  inline const static std::map<int, char const * > responses{{Response201, R"(is returned if the database was created successfully.)"},
{Response400, R"(is returned if the request parameters are invalid or if a database with the
specified name already exists.)"}, 
{Response403, R"(is returned if the request was not executed in the *_system* database.)"}, 
{Response409, R"(is returned if a database with the specified name already exists.)"}};
  static constexpr auto description = R"(
**A JSON object with these properties is required:**

  - **name**: Has to contain a valid database name.
  - **users**: Has to be an array of user objects to initially create for the new database.
   User information will not be changed for users that already exist.
   If *users* is not specified or does not contain any users, a default user
   *root* will be created with an empty string password. This ensures that the
   new database will be accessible after it is created.
   Each user object can contain the following attributes:
    - **username**: Login name of the user to be created
    - **passwd**: The user password as a string. If not specified, it will default to an empty string.
    - **active**: A flag indicating whether the user account should be activated or not.
    The default value is *true*. If set to *false*, the user won't be able to
    log into the database.
    - **extra**: A JSON object with extra user information. The data contained in *extra*
    will be stored for the user but not be interpreted further by ArangoDB.




Creates a new database

The response is a JSON object with the attribute *result* set to *true*.

**Note**: creating a new database is only possible from within the *_system* database.


)";


  ApiResult request(const std::string &name);

};
namespace GetCurrent {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/database/current";
  static constexpr auto summery = "Information of the database";
  static constexpr auto operationId = "(getDatabases:current";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Database/get_api_database_current.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Database"};
  
  enum GetCurrentResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned if the information was retrieved successfully.)"},
{Response400, R"(is returned if the request is invalid.)"}, 
{Response404, R"(is returned if the database could not be found.)"}};
  static constexpr auto description = R"(Retrieves information about the current database

The response is a JSON object with the following attributes:

- *name*: the name of the current database

- *id*: the id of the current database

- *path*: the filesystem path of the current database

- *isSystem*: whether or not the current database is the *_system* database


)";


  ApiResult request(std::string const &databaseName);
};
namespace GetUser {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/database/user";
  static constexpr auto summery = "List of accessible databases";
  static constexpr auto operationId = "(getDatabases:user";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Database/get_api_database_user.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Database"};
  
  enum GetUserResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned if the list of database was compiled successfully.)"},
{Response400, R"(is returned if the request is invalid.)"}};
  static constexpr auto description = R"(Retrieves the list of all databases the current user can access without
specifying a different username or password.


)";


  ApiResult request(std::string const &databaseName);
};
namespace DeleteDatabaseName {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Delete;
  static constexpr auto url = "/_api/database/{database-name}";
  static constexpr auto summery = "Drop database";
  static constexpr auto operationId = "(deleteDatabase";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Database/get_api_database_delete.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Database"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The name of the database)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "database-name";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum DeleteDatabaseNameResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400,
    Response403 = 403,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned if the database was dropped successfully.)"},
{Response400, R"(is returned if the request is malformed.)"}, 
{Response403, R"(is returned if the request was not executed in the *_system* database.)"}, 
{Response404, R"(is returned if the database could not be found.)"}};
  static constexpr auto description = R"(Drops the database along with all data stored in it.

**Note**: dropping a database is only possible from within the *_system* database.
The *_system* database itself cannot be dropped.


)";


  ApiResult request(const std::string &name);
};
}
namespace Document {
namespace DeleteCollection {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Delete;
  static constexpr auto url = "/_api/document/{collection}";
  static constexpr auto summery = "Removes multiple documents";
  static constexpr auto operationId = "(removeDocument";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Documents/delete_mutliple_documents.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Documents"};
  inline Parameter P0;
  inline Parameter P1;
  inline Parameter P2;
  inline Parameter P3;
  inline Parameter P4;
  inline void initParam(){
    P0.description = R"(A JSON array of strings or documents.)";
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"additionalProperties":{},"type":"object"})";
    P0.xDescriptionOffset = 0;

    P1.description = R"(Collection from which documents are removed.)";
    P1.format = ParamFormat::stringFormat;
    P1.in = ParamIn::path;
    P1.name = "collection";
    P1.required = true;
    P1.type = ParamFormat::stringFormat;

    P2.description = R"(Wait until deletion operation has been synced to disk.)";
    P2.in = ParamIn::query;
    P2.name = "waitForSync";
    P2.required = false;
    P2.type = ParamFormat::booleanFormat;

    P3.description = R"(Return additionally the complete previous revision of the changed 
document under the attribute *old* in the result.)";
    P3.in = ParamIn::query;
    P3.name = "returnOld";
    P3.required = false;
    P3.type = ParamFormat::booleanFormat;

    P4.description = R"(If set to *true*, ignore any *_rev* attribute in the selectors. No
revision check is performed.)";
    P4.in = ParamIn::query;
    P4.name = "ignoreRevs";
    P4.required = false;
    P4.type = ParamFormat::booleanFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1, P2, P3, P4};

  enum DeleteCollectionResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response202 = 202,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned if *waitForSync* was *true*.)"},
{Response202, R"(is returned if *waitForSync* was *false*.)"}, 
{Response404, R"(is returned if the collection was not found.
The response body contains an error document in this case.)"}};
  static constexpr auto description = R"(The body of the request is an array consisting of selectors for
documents. A selector can either be a string with a key or a string
with a document handle or an object with a *_key* attribute. This
API call removes all specified documents from *collection*. If the
selector is an object and has a *_rev* attribute, it is a
precondition that the actual revision of the removed document in the
collection is the specified one.

The body of the response is an array of the same length as the input
array. For each input selector, the output contains a JSON object
with the information about the outcome of the operation. If no error
occurred, an object is built in which the attribute *_id* contains
the known *document-handle* of the removed document, *_key* contains
the key which uniquely identifies a document in a given collection,
and the attribute *_rev* contains the document revision. In case of
an error, an object with the attribute *error* set to *true* and
*errorCode* set to the error code is built.

If the *waitForSync* parameter is not specified or set to *false*,
then the collection's default *waitForSync* behavior is applied.
The *waitForSync* query parameter cannot be used to disable
synchronization for collections that have a default *waitForSync*
value of *true*.

If the query parameter *returnOld* is *true*, then
the complete previous revision of the document
is returned under the *old* attribute in the result.

Note that if any precondition is violated or an error occurred with
some of the documents, the return code is still 200 or 202, but
the additional HTTP header *X-Arango-Error-Codes* is set, which
contains a map of the error codes that occurred together with their
multiplicities, as in: *1200:17,1205:10* which means that in 17
cases the error 1200 "revision conflict" and in 10 cases the error
1205 "illegal document handle" has happened.


)";


  ApiResult request(std::string const &databaseName);
};
namespace PatchCollection {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Patch;
  static constexpr auto url = "/_api/document/{collection}";
  static constexpr auto summery = "Update documents";
  static constexpr auto operationId = "(updateDocument";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Documents/patch_update_multiple_documents.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Documents"};
  inline Parameter P0;
  inline Parameter P1;
  inline Parameter P2;
  inline Parameter P3;
  inline Parameter P4;
  inline Parameter P5;
  inline Parameter P6;
  inline Parameter P7;
  inline void initParam(){
    P0.description = R"(A JSON representation of an array of document updates as objects.)";
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"additionalProperties":{},"type":"object"})";
    P0.xDescriptionOffset = 0;

    P1.description = R"(This URL parameter is the name of the collection in which the
documents are updated.)";
    P1.format = ParamFormat::stringFormat;
    P1.in = ParamIn::path;
    P1.name = "collection";
    P1.required = true;
    P1.type = ParamFormat::stringFormat;

    P2.description = R"(If the intention is to delete existing attributes with the patch
command, the URL query parameter *keepNull* can be used with a value
of *false*. This will modify the behavior of the patch command to
remove any attributes from the existing document that are contained
in the patch document with an attribute value of *null*.)";
    P2.in = ParamIn::query;
    P2.name = "keepNull";
    P2.required = false;
    P2.type = ParamFormat::booleanFormat;

    P3.description = R"(Controls whether objects (not arrays) will be merged if present in
both the existing and the patch document. If set to *false*, the
value in the patch document will overwrite the existing document's
value. If set to *true*, objects will be merged. The default is
*true*.)";
    P3.in = ParamIn::query;
    P3.name = "mergeObjects";
    P3.required = false;
    P3.type = ParamFormat::booleanFormat;

    P4.description = R"(Wait until the new documents have been synced to disk.)";
    P4.in = ParamIn::query;
    P4.name = "waitForSync";
    P4.required = false;
    P4.type = ParamFormat::booleanFormat;

    P5.description = R"(By default, or if this is set to *true*, the *_rev* attributes in 
the given documents are ignored. If this is set to *false*, then
any *_rev* attribute given in a body document is taken as a
precondition. The document is only updated if the current revision
is the one specified.)";
    P5.in = ParamIn::query;
    P5.name = "ignoreRevs";
    P5.required = false;
    P5.type = ParamFormat::booleanFormat;

    P6.description = R"(Return additionally the complete previous revision of the changed 
documents under the attribute *old* in the result.)";
    P6.in = ParamIn::query;
    P6.name = "returnOld";
    P6.required = false;
    P6.type = ParamFormat::booleanFormat;

    P7.description = R"(Return additionally the complete new documents under the attribute *new*
in the result.)";
    P7.in = ParamIn::query;
    P7.name = "returnNew";
    P7.required = false;
    P7.type = ParamFormat::booleanFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1, P2, P3, P4, P5, P6, P7};

  enum PatchCollectionResponse : short {
    Unknown = 0,
    Response201 = 201,
    Response202 = 202,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response201, R"(is returned if the documents were updated successfully and
*waitForSync* was *true*.)"}, 
{Response202, R"(is returned if the documents were updated successfully and
*waitForSync* was *false*.)"}, 
{Response400, R"(is returned if the body does not contain a valid JSON representation
of an array of documents. The response body contains
an error document in this case.)"}, 
{Response404, R"(is returned if the collection was not found.

)"}};
  static constexpr auto description = R"(Partially updates documents, the documents to update are specified
by the *_key* attributes in the body objects. The body of the
request must contain a JSON array of document updates with the
attributes to patch (the patch documents). All attributes from the
patch documents will be added to the existing documents if they do
not yet exist, and overwritten in the existing documents if they do
exist there.

Setting an attribute value to *null* in the patch documents will cause a
value of *null* to be saved for the attribute by default.

If *ignoreRevs* is *false* and there is a *_rev* attribute in a
document in the body and its value does not match the revision of
the corresponding document in the database, the precondition is
violated.

If the document exists and can be updated, then an *HTTP 201* or
an *HTTP 202* is returned (depending on *waitForSync*, see below).

Optionally, the query parameter *waitForSync* can be used to force
synchronization of the document replacement operation to disk even in case
that the *waitForSync* flag had been disabled for the entire collection.
Thus, the *waitForSync* query parameter can be used to force synchronization
of just specific operations. To use this, set the *waitForSync* parameter
to *true*. If the *waitForSync* parameter is not specified or set to
*false*, then the collection's default *waitForSync* behavior is
applied. The *waitForSync* query parameter cannot be used to disable
synchronization for collections that have a default *waitForSync* value
of *true*.

The body of the response contains a JSON array of the same length
as the input array with the information about the handle and the
revision of the updated documents. In each entry, the attribute
*_id* contains the known *document-handle* of each updated document,
*_key* contains the key which uniquely identifies a document in a
given collection, and the attribute *_rev* contains the new document
revision. In case of an error or violated precondition, an error
object with the attribute *error* set to *true* and the attribute
*errorCode* set to the error code is built.

If the query parameter *returnOld* is *true*, then, for each
generated document, the complete previous revision of the document
is returned under the *old* attribute in the result.

If the query parameter *returnNew* is *true*, then, for each
generated document, the complete new document is returned under
the *new* attribute in the result.

Note that if any precondition is violated or an error occurred with
some of the documents, the return code is still 201 or 202, but
the additional HTTP header *X-Arango-Error-Codes* is set, which
contains a map of the error codes that occurred together with their
multiplicities, as in: *1200:17,1205:10* which means that in 17
cases the error 1200 "revision conflict" and in 10 cases the error
1205 "illegal document handle" has happened.)";


  ApiResult request(std::string const &databaseName);
};
namespace PostCollection {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Post;
  static constexpr auto url = "/_api/document/{collection}";
  static constexpr auto summery = "Create document";
  static constexpr auto operationId = "(insertDocument";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Documents/post_create_document.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Documents"};
  inline Parameter P0;
  inline Parameter P1;
  inline Parameter P2;
  inline Parameter P3;
  inline Parameter P4;
  inline Parameter P5;
  inline Parameter P6;
  inline Parameter P7;
  inline void initParam(){
    P0.description = R"(The *collection* in which the collection is to be created.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "collection";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.description = R"(A JSON representation of a single document or of an array of documents.)";
    P1.in = ParamIn::body;
    P1.name = "Json Request Body";
    P1.required = true;
    P1.schema = R"({"additionalProperties":{},"type":"object"})";
    P1.xDescriptionOffset = 0;

    P2.description = R"(The name of the collection. This is only for backward compatibility.
In ArangoDB versions < 3.0, the URL path was */_api/document* and
this query parameter was required. This combination still works, but
the recommended way is to specify the collection in the URL path.)";
    P2.in = ParamIn::query;
    P2.name = "collection";
    P2.required = false;
    P2.type = ParamFormat::stringFormat;

    P3.description = R"(Wait until document has been synced to disk.)";
    P3.in = ParamIn::query;
    P3.name = "waitForSync";
    P3.required = false;
    P3.type = ParamFormat::booleanFormat;

    P4.description = R"(Additionally return the complete new document under the attribute *new*
in the result.)";
    P4.in = ParamIn::query;
    P4.name = "returnNew";
    P4.required = false;
    P4.type = ParamFormat::booleanFormat;

    P5.description = R"(Additionally return the complete old document under the attribute *old*
in the result. Only available if the overwrite option is used.)";
    P5.in = ParamIn::query;
    P5.name = "returnOld";
    P5.required = false;
    P5.type = ParamFormat::booleanFormat;

    P6.description = R"(If set to *true*, an empty object will be returned as response. No meta-data 
will be returned for the created document. This option can be used to
save some network traffic.)";
    P6.in = ParamIn::query;
    P6.name = "silent";
    P6.required = false;
    P6.type = ParamFormat::booleanFormat;

    P7.description = R"(If set to *true*, the insert becomes a replace-insert. If a document with the
same *_key* already exists the new document is not rejected with unique
constraint violated but will replace the old document.)";
    P7.in = ParamIn::query;
    P7.name = "overwrite";
    P7.required = false;
    P7.type = ParamFormat::booleanFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1, P2, P3, P4, P5, P6, P7};

  enum PostCollectionResponse : short {
    Unknown = 0,
    Response201 = 201,
    Response202 = 202,
    Response400 = 400,
    Response404 = 404,
    Response409 = 409
  };
  inline const static std::map<int, char const * > responses{{Response201, R"(is returned if the documents were created successfully and
*waitForSync* was *true*.)"}, 
{Response202, R"(is returned if the documents were created successfully and
*waitForSync* was *false*.)"}, 
{Response400, R"(is returned if the body does not contain a valid JSON representation
of one document or an array of documents. The response body contains
an error document in this case.)"}, 
{Response404, R"(is returned if the collection specified by *collection* is unknown.
The response body contains an error document in this case.)"}, 
{Response409, R"(is returned in the single document case if a document with the
same qualifiers in an indexed attribute conflicts with an already
existing document and thus violates that unique constraint. The
response body contains an error document in this case. In the array
case only 201 or 202 is returned, but if an error occurred, the
additional HTTP header *X-Arango-Error-Codes* is set, which
contains a map of the error codes that occurred together with their
multiplicities, as in: *1205:10,1210:17* which means that in 10
cases the error 1205 "illegal document handle" and in 17 cases the
error 1210 "unique constraint violated" has happened.)"}};
  static constexpr auto description = R"(Creates a new document from the document given in the body, unless there
is already a document with the *_key* given. If no *_key* is given, a new
unique *_key* is generated automatically.

The body can be an array of documents, in which case all
documents in the array are inserted with the same semantics as for a
single document. The result body will contain a JSON array of the
same length as the input array, and each entry contains the result
of the operation for the corresponding input. In case of an error
the entry is a document with attributes *error* set to *true* and
errorCode set to the error code that has happened.

Possibly given *_id* and *_rev* attributes in the body are always ignored,
the URL part or the query parameter collection respectively counts.

If the document was created successfully, then the *Location* header
contains the path to the newly created document. The *Etag* header field
contains the revision of the document. Both are only set in the single
document case.

If *silent* is not set to *true*, the body of the response contains a 
JSON object (single document case) with the following attributes:

  - *_id* contains the document handle of the newly created document
  - *_key* contains the document key
  - *_rev* contains the document revision

In the multi case the body is an array of such objects.

If the collection parameter *waitForSync* is *false*, then the call
returns as soon as the document has been accepted. It will not wait
until the documents have been synced to disk.

Optionally, the query parameter *waitForSync* can be used to force
synchronization of the document creation operation to disk even in
case that the *waitForSync* flag had been disabled for the entire
collection. Thus, the *waitForSync* query parameter can be used to
force synchronization of just this specific operations. To use this,
set the *waitForSync* parameter to *true*. If the *waitForSync*
parameter is not specified or set to *false*, then the collection's
default *waitForSync* behavior is applied. The *waitForSync* query
parameter cannot be used to disable synchronization for collections
that have a default *waitForSync* value of *true*.

If the query parameter *returnNew* is *true*, then, for each
generated document, the complete new document is returned under
the *new* attribute in the result.


)";


  ApiResult request(std::string const &databaseName, const std::string &collectionName, const arangodb::velocypack::Slice bindVarsObject);

};
namespace PutCollection {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/document/{collection}";
  static constexpr auto summery = "Replace documents";
  static constexpr auto operationId = "(replaceDocument:multiple";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Documents/put_replace_multiple_documents.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Documents"};
  inline Parameter P0;
  inline Parameter P1;
  inline Parameter P2;
  inline Parameter P3;
  inline Parameter P4;
  inline Parameter P5;
  inline void initParam(){
    P0.description = R"(A JSON representation of an array of documents.)";
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"additionalProperties":{},"type":"object"})";
    P0.xDescriptionOffset = 0;

    P1.description = R"(This URL parameter is the name of the collection in which the
documents are replaced.)";
    P1.format = ParamFormat::stringFormat;
    P1.in = ParamIn::path;
    P1.name = "collection";
    P1.required = true;
    P1.type = ParamFormat::stringFormat;

    P2.description = R"(Wait until the new documents have been synced to disk.)";
    P2.in = ParamIn::query;
    P2.name = "waitForSync";
    P2.required = false;
    P2.type = ParamFormat::booleanFormat;

    P3.description = R"(By default, or if this is set to *true*, the *_rev* attributes in 
the given documents are ignored. If this is set to *false*, then
any *_rev* attribute given in a body document is taken as a
precondition. The document is only replaced if the current revision
is the one specified.)";
    P3.in = ParamIn::query;
    P3.name = "ignoreRevs";
    P3.required = false;
    P3.type = ParamFormat::booleanFormat;

    P4.description = R"(Return additionally the complete previous revision of the changed 
documents under the attribute *old* in the result.)";
    P4.in = ParamIn::query;
    P4.name = "returnOld";
    P4.required = false;
    P4.type = ParamFormat::booleanFormat;

    P5.description = R"(Return additionally the complete new documents under the attribute *new*
in the result.)";
    P5.in = ParamIn::query;
    P5.name = "returnNew";
    P5.required = false;
    P5.type = ParamFormat::booleanFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1, P2, P3, P4, P5};

  enum PutCollectionResponse : short {
    Unknown = 0,
    Response201 = 201,
    Response202 = 202,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response201, R"(is returned if the documents were replaced successfully and
*waitForSync* was *true*.)"}, 
{Response202, R"(is returned if the documents were replaced successfully and
*waitForSync* was *false*.)"}, 
{Response400, R"(is returned if the body does not contain a valid JSON representation
of an array of documents. The response body contains
an error document in this case.)"}, 
{Response404, R"(is returned if the collection was not found.

)"}};
  static constexpr auto description = R"(Replaces multiple documents in the specified collection with the
ones in the body, the replaced documents are specified by the *_key*
attributes in the body documents.

If *ignoreRevs* is *false* and there is a *_rev* attribute in a
document in the body and its value does not match the revision of
the corresponding document in the database, the precondition is
violated.

If the document exists and can be updated, then an *HTTP 201* or
an *HTTP 202* is returned (depending on *waitForSync*, see below).

Optionally, the query parameter *waitForSync* can be used to force
synchronization of the document replacement operation to disk even in case
that the *waitForSync* flag had been disabled for the entire collection.
Thus, the *waitForSync* query parameter can be used to force synchronization
of just specific operations. To use this, set the *waitForSync* parameter
to *true*. If the *waitForSync* parameter is not specified or set to
*false*, then the collection's default *waitForSync* behavior is
applied. The *waitForSync* query parameter cannot be used to disable
synchronization for collections that have a default *waitForSync* value
of *true*.

The body of the response contains a JSON array of the same length
as the input array with the information about the handle and the
revision of the replaced documents. In each entry, the attribute
*_id* contains the known *document-handle* of each updated document,
*_key* contains the key which uniquely identifies a document in a
given collection, and the attribute *_rev* contains the new document
revision. In case of an error or violated precondition, an error
object with the attribute *error* set to *true* and the attribute
*errorCode* set to the error code is built.

If the query parameter *returnOld* is *true*, then, for each
generated document, the complete previous revision of the document
is returned under the *old* attribute in the result.

If the query parameter *returnNew* is *true*, then, for each
generated document, the complete new document is returned under
the *new* attribute in the result.

Note that if any precondition is violated or an error occurred with
some of the documents, the return code is still 201 or 202, but
the additional HTTP header *X-Arango-Error-Codes* is set, which
contains a map of the error codes that occurred together with their
multiplicities, as in: *1200:17,1205:10* which means that in 17
cases the error 1200 "revision conflict" and in 10 cases the error
1205 "illegal document handle" has happened.)";


  ApiResult request(std::string const &databaseName);
};
namespace DeleteDocumentHandle {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Delete;
  static constexpr auto url = "/_api/document/{document-handle}";
  static constexpr auto summery = "Removes a document";
  static constexpr auto operationId = "(removeDocument";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Documents/delete_remove_document.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Documents"};
  inline Parameter P0;
  inline Parameter P1;
  inline Parameter P2;
  inline Parameter P3;
  inline Parameter P4;
  inline void initParam(){
    P0.description = R"(Removes the document identified by *document-handle*.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "document-handle";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.description = R"(Wait until deletion operation has been synced to disk.)";
    P1.in = ParamIn::query;
    P1.name = "waitForSync";
    P1.required = false;
    P1.type = ParamFormat::booleanFormat;

    P2.description = R"(Return additionally the complete previous revision of the changed 
document under the attribute *old* in the result.)";
    P2.in = ParamIn::query;
    P2.name = "returnOld";
    P2.required = false;
    P2.type = ParamFormat::booleanFormat;

    P3.description = R"(If set to *true*, an empty object will be returned as response. No meta-data 
will be returned for the removed document. This option can be used to
save some network traffic.)";
    P3.in = ParamIn::query;
    P3.name = "silent";
    P3.required = false;
    P3.type = ParamFormat::booleanFormat;

    P4.description = R"(You can conditionally remove a document based on a target revision id by
using the *if-match* HTTP header.)";
    P4.in = ParamIn::header;
    P4.name = "If-Match";
    P4.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1, P2, P3, P4};

  enum DeleteDocumentHandleResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response202 = 202,
    Response404 = 404,
    Response412 = 412
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned if the document was removed successfully and
*waitForSync* was *true*.)"}, 
{Response202, R"(is returned if the document was removed successfully and
*waitForSync* was *false*.)"}, 
{Response404, R"(is returned if the collection or the document was not found.
The response body contains an error document in this case.)"}, 
{Response412, R"(is returned if a "If-Match" header or *rev* is given and the found
document has a different version. The response will also contain the found
document's current revision in the *_rev* attribute. Additionally, the
attributes *_id* and *_key* will be returned.)"}};
  static constexpr auto description = R"(If *silent* is not set to *true*, the body of the response contains a JSON 
object with the information about the handle and the revision. The attribute 
*_id* contains the known *document-handle* of the removed document, *_key* 
contains the key which uniquely identifies a document in a given collection, 
and the attribute *_rev* contains the document revision.

If the *waitForSync* parameter is not specified or set to *false*,
then the collection's default *waitForSync* behavior is applied.
The *waitForSync* query parameter cannot be used to disable
synchronization for collections that have a default *waitForSync*
value of *true*.

If the query parameter *returnOld* is *true*, then
the complete previous revision of the document
is returned under the *old* attribute in the result.


)";


  ApiResult request(std::string const &databaseName,const std::string &collectionName, std::string const &key);

};
namespace GetDocumentHandle {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/document/{document-handle}";
  static constexpr auto summery = "Read document";
  static constexpr auto operationId = "(readDocument";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Documents/get_read_document.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Documents"};
  inline Parameter P0;
  inline Parameter P1;
  inline Parameter P2;
  inline void initParam(){
    P0.description = R"(The handle of the document.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "document-handle";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.description = R"(If the "If-None-Match" header is given, then it must contain exactly one
Etag. The document is returned, if it has a different revision than the
given Etag. Otherwise an *HTTP 304* is returned.)";
    P1.in = ParamIn::header;
    P1.name = "If-None-Match";
    P1.type = ParamFormat::stringFormat;

    P2.description = R"(If the "If-Match" header is given, then it must contain exactly one
Etag. The document is returned, if it has the same revision as the
given Etag. Otherwise a *HTTP 412* is returned.)";
    P2.in = ParamIn::header;
    P2.name = "If-Match";
    P2.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1, P2};

  enum GetDocumentHandleResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response304 = 304,
    Response404 = 404,
    Response412 = 412
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned if the document was found)"},
{Response304, R"(is returned if the "If-None-Match" header is given and the document has
the same version)"}, 
{Response404, R"(is returned if the document or collection was not found)"}, 
{Response412, R"(is returned if an "If-Match" header is given and the found
document has a different version. The response will also contain the found
document's current revision in the *_rev* attribute. Additionally, the
attributes *_id* and *_key* will be returned.)"}};
  static constexpr auto description = R"(Returns the document identified by *document-handle*. The returned
document contains three special attributes: *_id* containing the document
handle, *_key* containing key which uniquely identifies a document
in a given collection and *_rev* containing the revision.


)";


  ApiResult request(std::string const &databaseName,const std::string &collectionName, std::string const &key);

};
namespace HeadDocumentHandle {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Head;
  static constexpr auto url = "/_api/document/{document-handle}";
  static constexpr auto summery = "Read document header";
  static constexpr auto operationId = "(checkDocument";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Documents/head_read_document_header.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Documents"};
  inline Parameter P0;
  inline Parameter P1;
  inline Parameter P2;
  inline void initParam(){
    P0.description = R"(The handle of the document.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "document-handle";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.description = R"(If the "If-None-Match" header is given, then it must contain exactly one
Etag. If the current document revision is not equal to the specified Etag,
an *HTTP 200* response is returned. If the current document revision is
identical to the specified Etag, then an *HTTP 304* is returned.)";
    P1.in = ParamIn::header;
    P1.name = "If-None-Match";
    P1.type = ParamFormat::stringFormat;

    P2.description = R"(If the "If-Match" header is given, then it must contain exactly one
Etag. The document is returned, if it has the same revision as the
given Etag. Otherwise a *HTTP 412* is returned.)";
    P2.in = ParamIn::header;
    P2.name = "If-Match";
    P2.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1, P2};

  enum HeadDocumentHandleResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response304 = 304,
    Response404 = 404,
    Response412 = 412
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned if the document was found)"},
{Response304, R"(is returned if the "If-None-Match" header is given and the document has
the same version)"}, 
{Response404, R"(is returned if the document or collection was not found)"}, 
{Response412, R"(is returned if an "If-Match" header is given and the found
document has a different version. The response will also contain the found
document's current revision in the *Etag* header.)"}};
  static constexpr auto description = R"(Like *GET*, but only returns the header fields and not the body. You
can use this call to get the current revision of a document or check if
the document was deleted.


)";


  ApiResult request(std::string const &databaseName);
};
namespace PatchDocumentHandle {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Patch;
  static constexpr auto url = "/_api/document/{document-handle}";
  static constexpr auto summery = "Update document";
  static constexpr auto operationId = "(updateDocument";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Documents/patch_update_document.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Documents"};
  inline Parameter P0;
  inline Parameter P1;
  inline Parameter P2;
  inline Parameter P3;
  inline Parameter P4;
  inline Parameter P5;
  inline Parameter P6;
  inline Parameter P7;
  inline Parameter P8;
  inline Parameter P9;
  inline void initParam(){
    P0.description = R"(A JSON representation of a document update as an object.)";
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"additionalProperties":{},"type":"object"})";
    P0.xDescriptionOffset = 0;

    P1.description = R"(This URL parameter must be a document handle.)";
    P1.format = ParamFormat::stringFormat;
    P1.in = ParamIn::path;
    P1.name = "document-handle";
    P1.required = true;
    P1.type = ParamFormat::stringFormat;

    P2.description = R"(If the intention is to delete existing attributes with the patch
command, the URL query parameter *keepNull* can be used with a value
of *false*. This will modify the behavior of the patch command to
remove any attributes from the existing document that are contained
in the patch document with an attribute value of *null*.)";
    P2.in = ParamIn::query;
    P2.name = "keepNull";
    P2.required = false;
    P2.type = ParamFormat::booleanFormat;

    P3.description = R"(Controls whether objects (not arrays) will be merged if present in
both the existing and the patch document. If set to *false*, the
value in the patch document will overwrite the existing document's
value. If set to *true*, objects will be merged. The default is
*true*.)";
    P3.in = ParamIn::query;
    P3.name = "mergeObjects";
    P3.required = false;
    P3.type = ParamFormat::booleanFormat;

    P4.description = R"(Wait until document has been synced to disk.)";
    P4.in = ParamIn::query;
    P4.name = "waitForSync";
    P4.required = false;
    P4.type = ParamFormat::booleanFormat;

    P5.description = R"(By default, or if this is set to *true*, the *_rev* attributes in 
the given document is ignored. If this is set to *false*, then
the *_rev* attribute given in the body document is taken as a
precondition. The document is only updated if the current revision
is the one specified.)";
    P5.in = ParamIn::query;
    P5.name = "ignoreRevs";
    P5.required = false;
    P5.type = ParamFormat::booleanFormat;

    P6.description = R"(Return additionally the complete previous revision of the changed 
document under the attribute *old* in the result.)";
    P6.in = ParamIn::query;
    P6.name = "returnOld";
    P6.required = false;
    P6.type = ParamFormat::booleanFormat;

    P7.description = R"(Return additionally the complete new document under the attribute *new*
in the result.)";
    P7.in = ParamIn::query;
    P7.name = "returnNew";
    P7.required = false;
    P7.type = ParamFormat::booleanFormat;

    P8.description = R"(If set to *true*, an empty object will be returned as response. No meta-data 
will be returned for the updated document. This option can be used to
save some network traffic.)";
    P8.in = ParamIn::query;
    P8.name = "silent";
    P8.required = false;
    P8.type = ParamFormat::booleanFormat;

    P9.description = R"(You can conditionally update a document based on a target revision id by
using the *if-match* HTTP header.)";
    P9.in = ParamIn::header;
    P9.name = "If-Match";
    P9.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1, P2, P3, P4, P5, P6, P7, P8, P9};

  enum PatchDocumentHandleResponse : short {
    Unknown = 0,
    Response201 = 201,
    Response202 = 202,
    Response400 = 400,
    Response404 = 404,
    Response412 = 412
  };
  inline const static std::map<int, char const * > responses{{Response201, R"(is returned if the document was updated successfully and
*waitForSync* was *true*.)"}, 
{Response202, R"(is returned if the document was updated successfully and
*waitForSync* was *false*.)"}, 
{Response400, R"(is returned if the body does not contain a valid JSON representation
of a document. The response body contains
an error document in this case.)"}, 
{Response404, R"(is returned if the collection or the document was not found.)"}, 
{Response412, R"(is returned if the precondition was violated. The response will
also contain the found documents' current revisions in the *_rev*
attributes. Additionally, the attributes *_id* and *_key* will be
returned.)"}};
  static constexpr auto description = R"(Partially updates the document identified by *document-handle*.
The body of the request must contain a JSON document with the
attributes to patch (the patch document). All attributes from the
patch document will be added to the existing document if they do not
yet exist, and overwritten in the existing document if they do exist
there.

Setting an attribute value to *null* in the patch document will cause a
value of *null* to be saved for the attribute by default.

If the *If-Match* header is specified and the revision of the
document in the database is unequal to the given revision, the
precondition is violated.

If *If-Match* is not given and *ignoreRevs* is *false* and there
is a *_rev* attribute in the body and its value does not match
the revision of the document in the database, the precondition is
violated.

If a precondition is violated, an *HTTP 412* is returned.

If the document exists and can be updated, then an *HTTP 201* or
an *HTTP 202* is returned (depending on *waitForSync*, see below),
the *Etag* header field contains the new revision of the document
(in double quotes) and the *Location* header contains a complete URL
under which the document can be queried.

Optionally, the query parameter *waitForSync* can be used to force
synchronization of the updated document operation to disk even in case
that the *waitForSync* flag had been disabled for the entire collection.
Thus, the *waitForSync* query parameter can be used to force synchronization
of just specific operations. To use this, set the *waitForSync* parameter
to *true*. If the *waitForSync* parameter is not specified or set to
*false*, then the collection's default *waitForSync* behavior is
applied. The *waitForSync* query parameter cannot be used to disable
synchronization for collections that have a default *waitForSync* value
of *true*.

If *silent* is not set to *true*, the body of the response contains a JSON 
object with the information about the handle and the revision. The attribute 
*_id* contains the known *document-handle* of the updated document, *_key* 
contains the key which uniquely identifies a document in a given collection, 
and the attribute *_rev* contains the new document revision.

If the query parameter *returnOld* is *true*, then
the complete previous revision of the document
is returned under the *old* attribute in the result.

If the query parameter *returnNew* is *true*, then
the complete new document is returned under
the *new* attribute in the result.

If the document does not exist, then a *HTTP 404* is returned and the
body of the response contains an error document.


)";


  ApiResult request(std::string const &databaseName);
};
namespace PutDocumentHandle {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/document/{document-handle}";
  static constexpr auto summery = "Replace document";
  static constexpr auto operationId = "(replaceDocument";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Documents/put_replace_document.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Documents"};
  inline Parameter P0;
  inline Parameter P1;
  inline Parameter P2;
  inline Parameter P3;
  inline Parameter P4;
  inline Parameter P5;
  inline Parameter P6;
  inline Parameter P7;
  inline void initParam(){
    P0.description = R"(A JSON representation of a single document.)";
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"additionalProperties":{},"type":"object"})";
    P0.xDescriptionOffset = 0;

    P1.description = R"(This URL parameter must be a document handle.)";
    P1.format = ParamFormat::stringFormat;
    P1.in = ParamIn::path;
    P1.name = "document-handle";
    P1.required = true;
    P1.type = ParamFormat::stringFormat;

    P2.description = R"(Wait until document has been synced to disk.)";
    P2.in = ParamIn::query;
    P2.name = "waitForSync";
    P2.required = false;
    P2.type = ParamFormat::booleanFormat;

    P3.description = R"(By default, or if this is set to *true*, the *_rev* attributes in 
the given document is ignored. If this is set to *false*, then
the *_rev* attribute given in the body document is taken as a
precondition. The document is only replaced if the current revision
is the one specified.)";
    P3.in = ParamIn::query;
    P3.name = "ignoreRevs";
    P3.required = false;
    P3.type = ParamFormat::booleanFormat;

    P4.description = R"(Return additionally the complete previous revision of the changed 
document under the attribute *old* in the result.)";
    P4.in = ParamIn::query;
    P4.name = "returnOld";
    P4.required = false;
    P4.type = ParamFormat::booleanFormat;

    P5.description = R"(Return additionally the complete new document under the attribute *new*
in the result.)";
    P5.in = ParamIn::query;
    P5.name = "returnNew";
    P5.required = false;
    P5.type = ParamFormat::booleanFormat;

    P6.description = R"(If set to *true*, an empty object will be returned as response. No meta-data 
will be returned for the replaced document. This option can be used to
save some network traffic.)";
    P6.in = ParamIn::query;
    P6.name = "silent";
    P6.required = false;
    P6.type = ParamFormat::booleanFormat;

    P7.description = R"(You can conditionally replace a document based on a target revision id by
using the *if-match* HTTP header.)";
    P7.in = ParamIn::header;
    P7.name = "If-Match";
    P7.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1, P2, P3, P4, P5, P6, P7};

  enum PutDocumentHandleResponse : short {
    Unknown = 0,
    Response201 = 201,
    Response202 = 202,
    Response400 = 400,
    Response404 = 404,
    Response412 = 412
  };
  inline const static std::map<int, char const * > responses{{Response201, R"(is returned if the document was replaced successfully and
*waitForSync* was *true*.)"}, 
{Response202, R"(is returned if the document was replaced successfully and
*waitForSync* was *false*.)"}, 
{Response400, R"(is returned if the body does not contain a valid JSON representation
of a document. The response body contains
an error document in this case.)"}, 
{Response404, R"(is returned if the collection or the document was not found.)"}, 
{Response412, R"(is returned if the precondition was violated. The response will
also contain the found documents' current revisions in the *_rev*
attributes. Additionally, the attributes *_id* and *_key* will be
returned.)"}};
  static constexpr auto description = R"(Replaces the document with handle <document-handle> with the one in
the body, provided there is such a document and no precondition is
violated.

If the *If-Match* header is specified and the revision of the
document in the database is unequal to the given revision, the
precondition is violated.

If *If-Match* is not given and *ignoreRevs* is *false* and there
is a *_rev* attribute in the body and its value does not match
the revision of the document in the database, the precondition is
violated.

If a precondition is violated, an *HTTP 412* is returned.

If the document exists and can be updated, then an *HTTP 201* or
an *HTTP 202* is returned (depending on *waitForSync*, see below),
the *Etag* header field contains the new revision of the document
and the *Location* header contains a complete URL under which the
document can be queried.

Optionally, the query parameter *waitForSync* can be used to force
synchronization of the document replacement operation to disk even in case
that the *waitForSync* flag had been disabled for the entire collection.
Thus, the *waitForSync* query parameter can be used to force synchronization
of just specific operations. To use this, set the *waitForSync* parameter
to *true*. If the *waitForSync* parameter is not specified or set to
*false*, then the collection's default *waitForSync* behavior is
applied. The *waitForSync* query parameter cannot be used to disable
synchronization for collections that have a default *waitForSync* value
of *true*.

If *silent* is not set to *true*, the body of the response contains a JSON 
object with the information about the handle and the revision. The attribute 
*_id* contains the known *document-handle* of the updated document, *_key* 
contains the key which uniquely identifies a document in a given collection, 
and the attribute *_rev* contains the new document revision.

If the query parameter *returnOld* is *true*, then
the complete previous revision of the document
is returned under the *old* attribute in the result.

If the query parameter *returnNew* is *true*, then
the complete new document is returned under
the *new* attribute in the result.

If the document does not exist, then a *HTTP 404* is returned and the
body of the response contains an error document.


)";


  ApiResult request(std::string const &databaseName);
};
}
namespace GraphEdge {
namespace GetCollectionId {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/edges/{collection-id}";
  static constexpr auto summery = "Read in- or outbound edges";
  static constexpr auto operationId = "(ReadIn-OrOutboundEdges";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Graph Edges/get_read_in_out_edges.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Graph Edges"};
  inline Parameter P0;
  inline Parameter P1;
  inline Parameter P2;
  inline void initParam(){
    P0.description = R"(The id of the collection.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "collection-id";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.description = R"(The id of the start vertex.)";
    P1.in = ParamIn::query;
    P1.name = "vertex";
    P1.required = true;
    P1.type = ParamFormat::stringFormat;

    P2.description = R"(Selects *in* or *out* direction for edges. If not set, any edges are
returned.)";
    P2.in = ParamIn::query;
    P2.name = "direction";
    P2.required = false;
    P2.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1, P2};

  enum GetCollectionIdResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned if the edge collection was found and edges were retrieved.)"},
{Response400, R"(is returned if the request contains invalid parameters.)"}, 
{Response404, R"(is returned if the edge collection was not found.)"}};
  static constexpr auto description = R"(Returns an array of edges starting or ending in the vertex identified by
*vertex-handle*.


)";


  ApiResult request(std::string const &databaseName);
};
}
namespace Administration {
namespace GetEndpoint {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/endpoint";
  static constexpr auto summery = "Return list of all endpoints";
  static constexpr auto operationId = "(retrieveEndpoints";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Administration/get_api_endpoint.md";
  static constexpr auto x_hints = R"({% hint 'warning' %}
This route should no longer be used.
It is considered as deprecated from version 3.4.0 on.
{% endhint %}

)";
  inline const static std::vector<char const * > tags{"Administration"};
  
  enum GetEndpointResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400,
    Response405 = 405
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned when the array of endpoints can be determined successfully.)"},
{Response400, R"(is returned if the action is not carried out in the system database.)"}, 
{Response405, R"(The server will respond with *HTTP 405* if an unsupported HTTP method is used.)"}};
  static constexpr auto description = R"(Returns an array of all configured endpoints the server is listening on.

The result is a JSON array of JSON objects, each with `"entrypoint"' as
the only attribute, and with the value being a string describing the
endpoint.

**Note**: retrieving the array of all endpoints is allowed in the system database
only. Calling this action in any other database will make the server return
an error.


<!-- Hints Start -->

**Warning:**  
This route should no longer be used.
It is considered as deprecated from version 3.4.0 on.



<!-- Hints End -->
)";


  ApiResult request(std::string const &databaseName);
};
namespace GetEngine {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/engine";
  static constexpr auto summery = "Return server database engine type";
  static constexpr auto operationId = "(RestEngineHandler";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Administration/get_engine.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Administration"};
  
  enum GetEngineResponse : short {
    Unknown = 0,
    Response200 = 200
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned in all cases.)"}};
  static constexpr auto description = R"(Returns the storage engine the server is configured to use.
The response is a JSON object with the following attributes:


**HTTP 200**
*A json document with these Properties is returned:*

is returned in all cases.

- **name**: will be *mmfiles* or *rocksdb*


)";


  ApiResult request(std::string const &databaseName);
};
namespace PostExplain {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Post;
  static constexpr auto url = "/_api/explain";
  static constexpr auto summery = "Explain an AQL query";
  static constexpr auto operationId = "(explainQuery";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/AQL/post_api_explain.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"AQL"};
  inline Parameter P0;
  inline void initParam(){
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"$ref":"#/definitions/post_api_explain"})";
    P0.xDescriptionOffset = 54;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PostExplainResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(If the query is valid, the server will respond with *HTTP 200* and
return the optimal execution plan in the *plan* attribute of the response.
If option *allPlans* was set in the request, an array of plans will be returned
in the *allPlans* attribute instead.)"}, 
{Response400, R"(The server will respond with *HTTP 400* in case of a malformed request,
or if the query contains a parse error. The body of the response will
contain the error details embedded in a JSON object.
Omitting bind variables if the query references any will also result
in an *HTTP 400* error.)"}, 
{Response404, R"(The server will respond with *HTTP 404* in case a non-existing collection is
accessed in the query.)"}};
  static constexpr auto description = R"(See online..)";


  ApiResult request(std::string const &databaseName);
};
}
namespace Bulk {
namespace PostExport {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Post;
  static constexpr auto url = "/_api/export";
  static constexpr auto summery = "Create export cursor";
  static constexpr auto operationId = "(createCursorExport";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Bulk/post_api_export.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Bulk"};
  inline Parameter P0;
  inline Parameter P1;
  inline void initParam(){
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"$ref":"#/definitions/post_api_export"})";
    P0.xDescriptionOffset = 54;

    P1.description = R"(The name of the collection to export.)";
    P1.in = ParamIn::query;
    P1.name = "collection";
    P1.required = true;
    P1.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1};

  enum PostExportResponse : short {
    Unknown = 0,
    Response201 = 201,
    Response400 = 400,
    Response404 = 404,
    Response405 = 405,
    Response501 = 501
  };
  inline const static std::map<int, char const * > responses{{Response201, R"(is returned if the result set can be created by the server.)"},
{Response400, R"(is returned if the JSON representation is malformed or the query specification is
missing from the request.)"}, 
{Response404, R"(The server will respond with *HTTP 404* in case a non-existing collection is
accessed in the query.)"}, 
{Response405, R"(The server will respond with *HTTP 405* if an unsupported HTTP method is used.)"}, 
{Response501, R"(The server will respond with *HTTP 501* if this API is called on a cluster
coordinator.

)"}};
  static constexpr auto description = R"(
**A JSON object with these properties is required:**

  - **count**: boolean flag that indicates whether the number of documents
   in the result set should be returned in the "count" attribute of the result
   (optional).
   Calculating the "count" attribute might in the future have a performance
   impact so this option is turned off by default, and "count" is only returned
   when requested.
  - **restrict**:
    - **fields** (string): Contains an array of attribute names to *include* or *exclude*. Matching of attribute names
    for *inclusion* or *exclusion* will be done on the top level only.
    Specifying names of nested attributes is not supported at the moment.
    - **type**: has to be be set to either *include* or *exclude* depending on which you want to use
  - **batchSize**: maximum number of result documents to be transferred from
   the server to the client in one roundtrip (optional). If this attribute is
   not set, a server-controlled default value will be used.
  - **flush**: if set to *true*, a WAL flush operation will be executed prior to the
   export. The flush operation will start copying documents from the WAL to the
   collection's datafiles. There will be an additional wait time of up
   to *flushWait* seconds after the flush to allow the WAL collector to change
   the adjusted document meta-data to point into the datafiles, too.
   The default value is *false* (i.e. no flush) so most recently inserted or
   updated
   documents from the collection might be missing in the export.
  - **flushWait**: maximum wait time in seconds after a flush operation. The default
   value is 10. This option only has an effect when *flush* is set to *true*.
  - **limit**: an optional limit value, determining the maximum number of documents to
   be included in the cursor. Omitting the *limit* attribute or setting it to 0 will
   lead to no limit being used. If a limit is used, it is undefined which documents
   from the collection will be included in the export and which will be excluded. 
   This is because there is no natural order of documents in a collection.
  - **ttl**: an optional time-to-live for the cursor (in seconds). The cursor will be
   removed on the server automatically after the specified amount of time. This
   is useful to ensure garbage collection of cursors that are not fully fetched
   by clients. If not set, a server-defined value will be used.




A call to this method creates a cursor containing all documents in the 
specified collection. In contrast to other data-producing APIs, the internal
data structures produced by the export API are more lightweight, so it is
the preferred way to retrieve all documents from a collection.

Documents are returned in a similar manner as in the `/_api/cursor` REST API. 
If all documents of the collection fit into the first batch, then no cursor
will be created, and the result object's *hasMore* attribute will be set to
*false*. If not all documents fit into the first batch, then the result 
object's *hasMore* attribute will be set to *true*, and the *id* attribute
of the result will contain a cursor id.

The order in which the documents are returned is not specified.

By default, only those documents from the collection will be returned that are
stored in the collection's datafiles. Documents that are present in the write-ahead
log (WAL) at the time the export is run will not be exported.

To export these documents as well, the caller can issue a WAL flush request
before calling the export API or set the *flush* attribute. Setting the *flush*
option will trigger a WAL flush before the export so documents get copied from 
the WAL to the collection datafiles.

If the result set can be created by the server, the server will respond with
*HTTP 201*. The body of the response will contain a JSON object with the
result set.

The returned JSON object has the following properties:

- *error*: boolean flag to indicate that an error occurred (*false*
  in this case)

- *code*: the HTTP status code

- *result*: an array of result documents (might be empty if the collection was empty)

- *hasMore*: a boolean indicator whether there are more results
  available for the cursor on the server

- *count*: the total number of result documents available (only
  available if the query was executed with the *count* attribute set)

- *id*: id of temporary cursor created on the server (optional, see above)

If the JSON representation is malformed or the query specification is
missing from the request, the server will respond with *HTTP 400*.

The body of the response will contain a JSON object with additional error
details. The object has the following attributes:

- *error*: boolean flag to indicate that an error occurred (*true* in this case)

- *code*: the HTTP status code

- *errorNum*: the server error number

- *errorMessage*: a descriptive error message

Clients should always delete an export cursor result as early as possible because a
lingering export cursor will prevent the underlying collection from being
compacted or unloaded. By default, unused cursors will be deleted automatically 
after a server-defined idle time, and clients can adjust this idle time by setting
the *ttl* value.

Note: this API is currently not supported on cluster coordinators.)";


  ApiResult request(std::string const &databaseName);
};
}
namespace Foxx {
namespace GetFoxx {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/foxx";
  static constexpr auto summery = "List installed services";
  static constexpr auto operationId = "(ListInstalledServices";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Foxx/api_foxx_service_list.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Foxx"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(Whether or not system services should be excluded from the result.)";
    P0.in = ParamIn::query;
    P0.name = "excludeSystem";
    P0.required = false;
    P0.type = ParamFormat::booleanFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum GetFoxxResponse : short {
    Unknown = 0,
    Response200 = 200
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Returned if the request was successful.
)"}};
  static constexpr auto description = R"(Fetches a list of services installed in the current database.

Returns a list of objects with the following attributes:

- *mount*: the mount path of the service
- *development*: *true* if the service is running in development mode
- *legacy*: *true* if the service is running in 2.8 legacy compatibility mode
- *provides*: the service manifest's *provides* value or an empty object

Additionally the object may contain the following attributes if they have been set on the manifest:

- *name*: a string identifying the service type
- *version*: a semver-compatible version string)";


  ApiResult request(std::string const &databaseName);
};
namespace PostFoxx {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Post;
  static constexpr auto url = "/_api/foxx";
  static constexpr auto summery = "Install new service";
  static constexpr auto operationId = "(InstallNewService";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Foxx/api_foxx_service_install.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Foxx"};
  inline Parameter P0;
  inline Parameter P1;
  inline Parameter P2;
  inline Parameter P3;
  inline void initParam(){
    P0.description = R"(Mount path the service should be installed at.)";
    P0.in = ParamIn::query;
    P0.name = "mount";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.description = R"(Set to `true` to enable development mode.)";
    P1.in = ParamIn::query;
    P1.name = "development";
    P1.required = false;
    P1.type = ParamFormat::booleanFormat;

    P2.description = R"(Set to `false` to not run the service's setup script.)";
    P2.in = ParamIn::query;
    P2.name = "setup";
    P2.required = false;
    P2.type = ParamFormat::booleanFormat;

    P3.description = R"(Set to `true` to install the service in 2.8 legacy compatibility mode.)";
    P3.in = ParamIn::query;
    P3.name = "legacy";
    P3.required = false;
    P3.type = ParamFormat::booleanFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1, P2, P3};

  enum PostFoxxResponse : short {
    Unknown = 0,
    Response201 = 201
  };
  inline const static std::map<int, char const * > responses{{Response201, R"(Returned if the request was successful.
)"}};
  static constexpr auto description = R"(Installs the given new service at the given mount path.

The request body can be any of the following formats:

- `application/zip`: a raw zip bundle containing a service
- `application/javascript`: a standalone JavaScript file
- `application/json`: a service definition as JSON
- `multipart/form-data`: a service definition as a multipart form

A service definition is an object or form with the following properties or fields:

- *configuration*: a JSON object describing configuration values
- *dependencies*: a JSON object describing dependency settings
- *source*: a fully qualified URL or an absolute path on the server's file system

When using multipart data, the *source* field can also alternatively be a file field
containing either a zip bundle or a standalone JavaScript file.

When using a standalone JavaScript file the given file will be executed
to define our service's HTTP endpoints. It is the same which would be defined
in the field `main` of the service manifest.

If *source* is a URL, the URL must be reachable from the server.
If *source* is a file system path, the path will be resolved on the server.
In either case the path or URL is expected to resolve to a zip bundle,
JavaScript file or (in case of a file system path) directory.

Note that when using file system paths in a cluster with multiple coordinators
the file system path must resolve to equivalent files on every coordinator.)";


  ApiResult request(std::string const &databaseName);
};
namespace PostCommit {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Post;
  static constexpr auto url = "/_api/foxx/commit";
  static constexpr auto summery = "Commit local service state";
  static constexpr auto operationId = "(CommitLocalServiceState";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Foxx/api_foxx_commit.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Foxx"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(Overwrite existing service files in database even if they already exist.)";
    P0.in = ParamIn::query;
    P0.name = "replace";
    P0.required = false;
    P0.type = ParamFormat::booleanFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PostCommitResponse : short {
    Unknown = 0,
    Response204
  };
  inline const static std::map<int, char const * > responses{{Response204, R"(Returned if the request was successful.
)"}};
  static constexpr auto description = R"(Commits the local service state of the coordinator to the database.

This can be used to resolve service conflicts between coordinators that can not be fixed automatically due to missing data.)";


  ApiResult request(std::string const &databaseName);
};
namespace GetConfiguration {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/foxx/configuration";
  static constexpr auto summery = "Get configuration options";
  static constexpr auto operationId = "(GetConfigurationOptions";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Foxx/api_foxx_configuration_get.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Foxx"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(Mount path of the installed service.)";
    P0.in = ParamIn::query;
    P0.name = "mount";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum GetConfigurationResponse : short {
    Unknown = 0,
    Response200 = 200
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Returned if the request was successful.
)"}};
  static constexpr auto description = R"(Fetches the current configuration for the service at the given mount path.

Returns an object mapping the configuration option names to their definitions
including a human-friendly *title* and the *current* value (if any).)";


  ApiResult request(std::string const &databaseName);
};
namespace PatchConfiguration {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Patch;
  static constexpr auto url = "/_api/foxx/configuration";
  static constexpr auto summery = "Update configuration options";
  static constexpr auto operationId = "(UpdateConfigurationOptions";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Foxx/api_foxx_configuration_update.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Foxx"};
  inline Parameter P0;
  inline Parameter P1;
  inline void initParam(){
    P0.description = R"(A JSON object mapping configuration option names to their new values.
Any omitted options will be ignored.)";
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"additionalProperties":{},"type":"object"})";
    P0.xDescriptionOffset = 126;

    P1.description = R"(Mount path of the installed service.)";
    P1.in = ParamIn::query;
    P1.name = "mount";
    P1.required = true;
    P1.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1};

  enum PatchConfigurationResponse : short {
    Unknown = 0,
    Response200 = 200
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Returned if the request was successful.
)"}};
  static constexpr auto description = R"(Replaces the given service's configuration.

Returns an object mapping all configuration option names to their new values.)";


  ApiResult request(std::string const &databaseName);
};
namespace PutConfiguration {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/foxx/configuration";
  static constexpr auto summery = "Replace configuration options";
  static constexpr auto operationId = "(ReplaceConfigurationOptions";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Foxx/api_foxx_configuration_replace.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Foxx"};
  inline Parameter P0;
  inline Parameter P1;
  inline void initParam(){
    P0.description = R"(A JSON object mapping configuration option names to their new values.
Any omitted options will be reset to their default values or marked as unconfigured.)";
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"additionalProperties":{},"type":"object"})";
    P0.xDescriptionOffset = 137;

    P1.description = R"(Mount path of the installed service.)";
    P1.in = ParamIn::query;
    P1.name = "mount";
    P1.required = true;
    P1.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1};

  enum PutConfigurationResponse : short {
    Unknown = 0,
    Response200 = 200
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Returned if the request was successful.
)"}};
  static constexpr auto description = R"(Replaces the given service's configuration completely.

Returns an object mapping all configuration option names to their new values.)";


  ApiResult request(std::string const &databaseName);
};
namespace GetDependencies {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/foxx/dependencies";
  static constexpr auto summery = "Get dependency options";
  static constexpr auto operationId = "(GetDependencyOptions";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Foxx/api_foxx_dependencies_get.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Foxx"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(Mount path of the installed service.)";
    P0.in = ParamIn::query;
    P0.name = "mount";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum GetDependenciesResponse : short {
    Unknown = 0,
    Response200 = 200
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Returned if the request was successful.
)"}};
  static constexpr auto description = R"(Fetches the current dependencies for service at the given mount path.

Returns an object mapping the dependency names to their definitions
including a human-friendly *title* and the *current* mount path (if any).)";


  ApiResult request(std::string const &databaseName);
};
namespace PatchDependencies {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Patch;
  static constexpr auto url = "/_api/foxx/dependencies";
  static constexpr auto summery = "Update dependencies options";
  static constexpr auto operationId = "(UpdateDependenciesOptions";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Foxx/api_foxx_dependencies_update.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Foxx"};
  inline Parameter P0;
  inline Parameter P1;
  inline void initParam(){
    P0.description = R"(A JSON object mapping dependency names to their new mount paths.
Any omitted dependencies will be ignored.)";
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"additionalProperties":{},"type":"object"})";
    P0.xDescriptionOffset = 120;

    P1.description = R"(Mount path of the installed service.)";
    P1.in = ParamIn::query;
    P1.name = "mount";
    P1.required = true;
    P1.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1};

  enum PatchDependenciesResponse : short {
    Unknown = 0,
    Response200 = 200
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Returned if the request was successful.
)"}};
  static constexpr auto description = R"(Replaces the given service's dependencies.

Returns an object mapping all dependency names to their new mount paths.)";


  ApiResult request(std::string const &databaseName);
};
namespace PutDependencies {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/foxx/dependencies";
  static constexpr auto summery = "Replace dependencies options";
  static constexpr auto operationId = "(ReplaceDependenciesOptions";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Foxx/api_foxx_dependencies_replace.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Foxx"};
  inline Parameter P0;
  inline Parameter P1;
  inline void initParam(){
    P0.description = R"(A JSON object mapping dependency names to their new mount paths.
Any omitted dependencies will be disabled.)";
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"additionalProperties":{},"type":"object"})";
    P0.xDescriptionOffset = 131;

    P1.description = R"(Mount path of the installed service.)";
    P1.in = ParamIn::query;
    P1.name = "mount";
    P1.required = true;
    P1.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1};

  enum PutDependenciesResponse : short {
    Unknown = 0,
    Response200 = 200
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Returned if the request was successful.
)"}};
  static constexpr auto description = R"(Replaces the given service's dependencies completely.

Returns an object mapping all dependency names to their new mount paths.)";


  ApiResult request(std::string const &databaseName);
};
namespace DeleteDevelopment {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Delete;
  static constexpr auto url = "/_api/foxx/development";
  static constexpr auto summery = "Disable development mode";
  static constexpr auto operationId = "(DisableDevelopmentMode";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Foxx/api_foxx_development_disable.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Foxx"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(Mount path of the installed service.)";
    P0.in = ParamIn::query;
    P0.name = "mount";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum DeleteDevelopmentResponse : short {
    Unknown = 0,
    Response200 = 200
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Returned if the request was successful.
)"}};
  static constexpr auto description = R"(Puts the service at the given mount path into production mode.

When running ArangoDB in a cluster with multiple coordinators this will
replace the service on all other coordinators with the version on this
coordinator.)";


  ApiResult request(std::string const &databaseName);
};
namespace PostDevelopment {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Post;
  static constexpr auto url = "/_api/foxx/development";
  static constexpr auto summery = "Enable development mode";
  static constexpr auto operationId = "(EnableDevelopmentMode";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Foxx/api_foxx_development_enable.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Foxx"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(Mount path of the installed service.)";
    P0.in = ParamIn::query;
    P0.name = "mount";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PostDevelopmentResponse : short {
    Unknown = 0,
    Response200 = 200
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Returned if the request was successful.
)"}};
  static constexpr auto description = R"(Puts the service into development mode.

While the service is running in development mode the service will be reloaded
from the filesystem and its setup script (if any) will be re-executed every
time the service handles a request.

When running ArangoDB in a cluster with multiple coordinators note that changes
to the filesystem on one coordinator will not be reflected across the other
coordinators. This means you should treat your coordinators as inconsistent
as long as any service is running in development mode.)";


  ApiResult request(std::string const &databaseName);
};
namespace PostDownload {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Post;
  static constexpr auto url = "/_api/foxx/download";
  static constexpr auto summery = "Download service bundle";
  static constexpr auto operationId = "(DownloadServiceBundle";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Foxx/api_foxx_bundle.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Foxx"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(Mount path of the installed service.)";
    P0.in = ParamIn::query;
    P0.name = "mount";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PostDownloadResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Returned if the request was successful.)"},
{Response400, R"(Returned if the mount path is unknown.
)"}};
  static constexpr auto description = R"(Downloads a zip bundle of the service directory.

When development mode is enabled, this always creates a new bundle.

Otherwise the bundle will represent the version of a service that
is installed on that ArangoDB instance.)";


  ApiResult request(std::string const &databaseName);
};
namespace GetReadme {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/foxx/readme";
  static constexpr auto summery = "Service README";
  static constexpr auto operationId = "(ServiceReadme";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Foxx/api_foxx_readme.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Foxx"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(Mount path of the installed service.)";
    P0.in = ParamIn::query;
    P0.name = "mount";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum GetReadmeResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response204
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Returned if the request was successful.)"},
{Response204, R"(Returned if no README file was found.
)"}};
  static constexpr auto description = R"(Fetches the service's README or README.md file's contents if any.)";


  ApiResult request(std::string const &databaseName);
};
namespace GetScripts {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/foxx/scripts";
  static constexpr auto summery = "List service scripts";
  static constexpr auto operationId = "(ListServiceScripts";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Foxx/api_foxx_scripts_list.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Foxx"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(Mount path of the installed service.)";
    P0.in = ParamIn::query;
    P0.name = "mount";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum GetScriptsResponse : short {
    Unknown = 0,
    Response200 = 200
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Returned if the request was successful.
)"}};
  static constexpr auto description = R"(Fetches a list of the scripts defined by the service.

Returns an object mapping the raw script names to human-friendly names.)";


  ApiResult request(std::string const &databaseName);
};
namespace PostName {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Post;
  static constexpr auto url = "/_api/foxx/scripts/{name}";
  static constexpr auto summery = "Run service script";
  static constexpr auto operationId = "(RunServiceScript";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Foxx/api_foxx_scripts_run.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Foxx"};
  inline Parameter P0;
  inline Parameter P1;
  inline Parameter P2;
  inline void initParam(){
    P0.description = R"(An arbitrary JSON value that will be parsed and passed to the
script as its first argument.)";
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = false;
    P0.schema = R"({"additionalProperties":{},"type":"object"})";
    P0.xDescriptionOffset = 0;

    P1.description = R"(Name of the script to run.)";
    P1.format = ParamFormat::stringFormat;
    P1.in = ParamIn::path;
    P1.name = "name";
    P1.required = true;
    P1.type = ParamFormat::stringFormat;

    P2.description = R"(Mount path of the installed service.)";
    P2.in = ParamIn::query;
    P2.name = "mount";
    P2.required = true;
    P2.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1, P2};

  enum PostNameResponse : short {
    Unknown = 0,
    Response200 = 200
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Returned if the request was successful.
)"}};
  static constexpr auto description = R"(Runs the given script for the service at the given mount path.

Returns the exports of the script, if any.)";


  ApiResult request(std::string const &databaseName);
};
namespace DeleteService {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Delete;
  static constexpr auto url = "/_api/foxx/service";
  static constexpr auto summery = "Uninstall service";
  static constexpr auto operationId = "(UninstallService";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Foxx/api_foxx_service_uninstall.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Foxx"};
  inline Parameter P0;
  inline Parameter P1;
  inline void initParam(){
    P0.description = R"(Mount path of the installed service.)";
    P0.in = ParamIn::query;
    P0.name = "mount";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.description = R"(Set to `false` to not run the service's teardown script.)";
    P1.in = ParamIn::query;
    P1.name = "teardown";
    P1.required = false;
    P1.type = ParamFormat::booleanFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1};

  enum DeleteServiceResponse : short {
    Unknown = 0,
    Response204
  };
  inline const static std::map<int, char const * > responses{{Response204, R"(Returned if the request was successful.
)"}};
  static constexpr auto description = R"(Removes the service at the given mount path from the database and file system.

Returns an empty response on success.)";


  ApiResult request(std::string const &databaseName);
};
namespace GetService {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/foxx/service";
  static constexpr auto summery = "Service description";
  static constexpr auto operationId = "(ServiceDescription";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Foxx/api_foxx_service_details.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Foxx"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(Mount path of the installed service.)";
    P0.in = ParamIn::query;
    P0.name = "mount";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum GetServiceResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Returned if the request was successful.)"},
{Response400, R"(Returned if the mount path is unknown.
)"}};
  static constexpr auto description = R"(Fetches detailed information for the service at the given mount path.

Returns an object with the following attributes:

- *mount*: the mount path of the service
- *path*: the local file system path of the service
- *development*: *true* if the service is running in development mode
- *legacy*: *true* if the service is running in 2.8 legacy compatibility mode
- *manifest*: the normalized JSON manifest of the service

Additionally the object may contain the following attributes if they have been set on the manifest:

- *name*: a string identifying the service type
- *version*: a semver-compatible version string)";


  ApiResult request(std::string const &databaseName);
};
namespace PatchService {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Patch;
  static constexpr auto url = "/_api/foxx/service";
  static constexpr auto summery = "Upgrade service";
  static constexpr auto operationId = "(UpgradeService";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Foxx/api_foxx_service_upgrade.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Foxx"};
  inline Parameter P0;
  inline Parameter P1;
  inline Parameter P2;
  inline Parameter P3;
  inline Parameter P4;
  inline void initParam(){
    P0.description = R"(Mount path of the installed service.)";
    P0.in = ParamIn::query;
    P0.name = "mount";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.description = R"(Set to `true` to run the old service's teardown script.)";
    P1.in = ParamIn::query;
    P1.name = "teardown";
    P1.required = false;
    P1.type = ParamFormat::booleanFormat;

    P2.description = R"(Set to `false` to not run the new service's setup script.)";
    P2.in = ParamIn::query;
    P2.name = "setup";
    P2.required = false;
    P2.type = ParamFormat::booleanFormat;

    P3.description = R"(Set to `true` to install the new service in 2.8 legacy compatibility mode.)";
    P3.in = ParamIn::query;
    P3.name = "legacy";
    P3.required = false;
    P3.type = ParamFormat::booleanFormat;

    P4.description = R"(Set to `true` to force service install even if no service is installed under given mount.)";
    P4.in = ParamIn::query;
    P4.name = "force";
    P4.required = false;
    P4.type = ParamFormat::booleanFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1, P2, P3, P4};

  enum PatchServiceResponse : short {
    Unknown = 0,
    Response200 = 200
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Returned if the request was successful.
)"}};
  static constexpr auto description = R"(Installs the given new service on top of the service currently installed at the given mount path.
This is only recommended for switching between different versions of the same service.

Unlike replacing a service, upgrading a service retains the old service's configuration
and dependencies (if any) and should therefore only be used to migrate an existing service
to a newer or equivalent service.

The request body can be any of the following formats:

- `application/zip`: a raw zip bundle containing a service
- `application/javascript`: a standalone JavaScript file
- `application/json`: a service definition as JSON
- `multipart/form-data`: a service definition as a multipart form

A service definition is an object or form with the following properties or fields:

- *configuration*: a JSON object describing configuration values
- *dependencies*: a JSON object describing dependency settings
- *source*: a fully qualified URL or an absolute path on the server's file system

When using multipart data, the *source* field can also alternatively be a file field
containing either a zip bundle or a standalone JavaScript file.

When using a standalone JavaScript file the given file will be executed
to define our service's HTTP endpoints. It is the same which would be defined
in the field `main` of the service manifest.

If *source* is a URL, the URL must be reachable from the server.
If *source* is a file system path, the path will be resolved on the server.
In either case the path or URL is expected to resolve to a zip bundle,
JavaScript file or (in case of a file system path) directory.

Note that when using file system paths in a cluster with multiple coordinators
the file system path must resolve to equivalent files on every coordinator.)";


  ApiResult request(std::string const &databaseName);
};
namespace PutService {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/foxx/service";
  static constexpr auto summery = "Replace service";
  static constexpr auto operationId = "(ReplaceService";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Foxx/api_foxx_service_replace.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Foxx"};
  inline Parameter P0;
  inline Parameter P1;
  inline Parameter P2;
  inline Parameter P3;
  inline Parameter P4;
  inline void initParam(){
    P0.description = R"(Mount path of the installed service.)";
    P0.in = ParamIn::query;
    P0.name = "mount";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.description = R"(Set to `false` to not run the old service's teardown script.)";
    P1.in = ParamIn::query;
    P1.name = "teardown";
    P1.required = false;
    P1.type = ParamFormat::booleanFormat;

    P2.description = R"(Set to `false` to not run the new service's setup script.)";
    P2.in = ParamIn::query;
    P2.name = "setup";
    P2.required = false;
    P2.type = ParamFormat::booleanFormat;

    P3.description = R"(Set to `true` to install the new service in 2.8 legacy compatibility mode.)";
    P3.in = ParamIn::query;
    P3.name = "legacy";
    P3.required = false;
    P3.type = ParamFormat::booleanFormat;

    P4.description = R"(Set to `true` to force service install even if no service is installed under given mount.)";
    P4.in = ParamIn::query;
    P4.name = "force";
    P4.required = false;
    P4.type = ParamFormat::booleanFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1, P2, P3, P4};

  enum PutServiceResponse : short {
    Unknown = 0,
    Response200 = 200
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Returned if the request was successful.
)"}};
  static constexpr auto description = R"(Removes the service at the given mount path from the database and file system.
Then installs the given new service at the same mount path.

This is a slightly safer equivalent to performing an uninstall of the old service
followed by installing the new service. The new service's main and script files
(if any) will be checked for basic syntax errors before the old service is removed.

The request body can be any of the following formats:

- `application/zip`: a raw zip bundle containing a service
- `application/javascript`: a standalone JavaScript file
- `application/json`: a service definition as JSON
- `multipart/form-data`: a service definition as a multipart form

A service definition is an object or form with the following properties or fields:

- *configuration*: a JSON object describing configuration values
- *dependencies*: a JSON object describing dependency settings
- *source*: a fully qualified URL or an absolute path on the server's file system

When using multipart data, the *source* field can also alternatively be a file field
containing either a zip bundle or a standalone JavaScript file.

When using a standalone JavaScript file the given file will be executed
to define our service's HTTP endpoints. It is the same which would be defined
in the field `main` of the service manifest.

If *source* is a URL, the URL must be reachable from the server.
If *source* is a file system path, the path will be resolved on the server.
In either case the path or URL is expected to resolve to a zip bundle,
JavaScript file or (in case of a file system path) directory.

Note that when using file system paths in a cluster with multiple coordinators
the file system path must resolve to equivalent files on every coordinator.)";


  ApiResult request(std::string const &databaseName);
};
namespace GetSwagger {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/foxx/swagger";
  static constexpr auto summery = "Swagger description";
  static constexpr auto operationId = "(SwaggerDescription";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Foxx/api_foxx_swagger.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Foxx"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(Mount path of the installed service.)";
    P0.in = ParamIn::query;
    P0.name = "mount";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum GetSwaggerResponse : short {
    Unknown = 0,
    Response200 = 200
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Returned if the request was successful.
)"}};
  static constexpr auto description = R"(Fetches the Swagger API description for the service at the given mount path.

The response body will be an OpenAPI 2.0 compatible JSON description of the service API.)";


  ApiResult request(std::string const &databaseName);
};
namespace PostTests {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Post;
  static constexpr auto url = "/_api/foxx/tests";
  static constexpr auto summery = "Run service tests";
  static constexpr auto operationId = "(RunServiceTests";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Foxx/api_foxx_tests_run.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Foxx"};
  inline Parameter P0;
  inline Parameter P1;
  inline Parameter P2;
  inline Parameter P3;
  inline void initParam(){
    P0.description = R"(Mount path of the installed service.)";
    P0.in = ParamIn::query;
    P0.name = "mount";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.description = R"(Test reporter to use.)";
    P1.in = ParamIn::query;
    P1.name = "reporter";
    P1.required = false;
    P1.type = ParamFormat::stringFormat;

    P2.description = R"(Use the matching format for the reporter, regardless of the *Accept* header.)";
    P2.in = ParamIn::query;
    P2.name = "idiomatic";
    P2.required = false;
    P2.type = ParamFormat::booleanFormat;

    P3.description = R"(Only run tests where the full name (including full test suites and test case)
matches this string.)";
    P3.in = ParamIn::query;
    P3.name = "filter";
    P3.required = false;
    P3.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1, P2, P3};

  enum PostTestsResponse : short {
    Unknown = 0,
    Response200 = 200
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Returned if the request was successful.
)"}};
  static constexpr auto description = R"(Runs the tests for the service at the given mount path and returns the results.

Supported test reporters are:

- *default*: a simple list of test cases
- *suite*: an object of test cases nested in suites
- *stream*: a raw stream of test results
- *xunit*: an XUnit/JUnit compatible structure
- *tap*: a raw TAP compatible stream

The *Accept* request header can be used to further control the response format:

When using the *stream* reporter `application/x-ldjson` will result
in the response body being formatted as a newline-delimited JSON stream.

When using the *tap* reporter `text/plain` or `text/*` will result
in the response body being formatted as a plain text TAP report.

When using the *xunit* reporter `application/xml` or `text/xml` will result
in the response body being formatted as XML instead of JSONML.

Otherwise the response body will be formatted as non-prettyprinted JSON.)";


  ApiResult request(std::string const &databaseName);
};
}
namespace Graph{
namespace GetGharial {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/gharial";
  static constexpr auto summery = "List all graphs";
  static constexpr auto operationId = "(ListAllGraphs";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Graph/general_graph_list_http_examples.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Graph"};
  
  enum GetGharialResponse : short {
    Unknown = 0,
    Response200 = 200
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Is returned if the module is available and the graphs could be listed.)"}};
  static constexpr auto description = R"(Lists all graphs stored in this database.


**HTTP 200**
*A json document with these Properties is returned:*

Is returned if the module is available and the graphs could be listed.

- **graphs**: 
  - **graph**:
    - **smartGraphAttribute**: The name of the sharding attribute in smart graph case (Enterprise Edition only)
    - **replicationFactor**: The replication factor used for every new collection in the graph.
    - **orphanCollections** (string): An array of additional vertex collections.
    Documents within these collections do not have edges within this graph.
    - **name**: The name of the graph.
    - **_rev**: The revision of this graph. Can be used to make sure to not override
    concurrent modifications to this graph.
    - **numberOfShards**: Number of shards created for every new collection in the graph.
    - **isSmart**: Flag if the graph is a SmartGraph (Enterprise Edition only) or not.
    - **_id**: The internal id value of this graph. 
    - **edgeDefinitions**: An array of definitions for the relations of the graph.
    Each has the following type:
      - **to** (string): List of vertex collection names.
     Edges in collection can only be inserted if their _to is in any of the collections here.
      - **from** (string): List of vertex collection names.
     Edges in collection can only be inserted if their _from is in any of the collections here.
      - **collection**: Name of the edge collection, where the edge are stored in.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is false in this response.


)";


  ApiResult request(std::string const &databaseName);
};
namespace PostGharial {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Post;
  static constexpr auto url = "/_api/gharial";
  static constexpr auto summery = "Create a graph";
  static constexpr auto operationId = "(CreateAGraph";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Graph/general_graph_create_http_examples.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Graph"};
  inline Parameter P0;
  inline Parameter P1;
  inline void initParam(){
    P0.description = R"(define if the request should wait until everything is synced to disc.
Will change the success response code.)";
    P0.in = ParamIn::query;
    P0.name = "waitForSync";
    P0.required = false;
    P0.type = ParamFormat::booleanFormat;

    P1.in = ParamIn::body;
    P1.name = "Json Request Body";
    P1.required = true;
    P1.schema = R"({"$ref":"#/definitions/general_graph_create_http_examples"})";
    P1.xDescriptionOffset = 240;
  }
  inline std::vector<Parameter> parameters{P0, P1};

  enum PostGharialResponse : short {
    Unknown = 0,
    Response201 = 201,
    Response202 = 202,
    Response400 = 400,
    Response403 = 403,
    Response409 = 409
  };
  inline const static std::map<int, char const * > responses{{Response201, R"(Is returned if the graph could be created and waitForSync is enabled
for the `_graphs` collection, or given in the request.
The response body contains the graph configuration that has been stored.)"}, 
{Response202, R"(Is returned if the graph could be created and waitForSync is disabled
for the `_graphs` collection and not given in the request.
The response body contains the graph configuration that has been stored.)"}, 
{Response400, R"(Returned if the request is in a wrong format.)"}, 
{Response403, R"(Returned if your user has insufficient rights.
In order to create a graph you at least need to have the following privileges:

  1. `Administrate` access on the Database.
  2. `Read Only` access on every collection used within this graph.)"}, 
{Response409, R"(Returned if there is a conflict storing the graph.  This can occur
either if a graph with this name is already stored, or if there is one
edge definition with a the same
[edge collection](../../Manual/Appendix/Glossary.html#edge-collection) but a
different signature used in any other graph.)"}};
  static constexpr auto description = R"(The creation of a graph requires the name of the graph and a
definition of its edges.
[See also edge definitions](../../Manual/Graphs/GeneralGraphs/Management.html#edge-definitions).


**A JSON object with these properties is required:**

  - **isSmart**: Define if the created graph should be smart.
   This only has effect in Enterprise Edition.
  - **edgeDefinitions**: An array of definitions for the relations of the graph.
   Each has the following type:
    - **to** (string): List of vertex collection names.
    Edges in collection can only be inserted if their _to is in any of the collections here.
    - **from** (string): List of vertex collection names.
    Edges in collection can only be inserted if their _from is in any of the collections here.
    - **collection**: Name of the edge collection, where the edge are stored in.
  - **name**: Name of the graph.
  - **options**:
    - **smartGraphAttribute**: Only has effect in Enterprise Edition and it is required if isSmart is true.
    The attribute name that is used to smartly shard the vertices of a graph.
    Every vertex in this SmartGraph has to have this attribute.
    Cannot be modified later.
    - **numberOfShards**: The number of shards that is used for every collection within this graph.
    Cannot be modified later.
    - **replicationFactor**: The replication factor used when initially creating collections for this graph.



**HTTP 201**
*A json document with these Properties is returned:*

Is returned if the graph could be created and waitForSync is enabled
for the `_graphs` collection, or given in the request.
The response body contains the graph configuration that has been stored.

- **graph**:
  - **smartGraphAttribute**: The name of the sharding attribute in smart graph case (Enterprise Edition only)
  - **replicationFactor**: The replication factor used for every new collection in the graph.
  - **orphanCollections** (string): An array of additional vertex collections.
   Documents within these collections do not have edges within this graph.
  - **name**: The name of the graph.
  - **_rev**: The revision of this graph. Can be used to make sure to not override
   concurrent modifications to this graph.
  - **numberOfShards**: Number of shards created for every new collection in the graph.
  - **isSmart**: Flag if the graph is a SmartGraph (Enterprise Edition only) or not.
  - **_id**: The internal id value of this graph. 
  - **edgeDefinitions**: An array of definitions for the relations of the graph.
   Each has the following type:
    - **to** (string): List of vertex collection names.
    Edges in collection can only be inserted if their _to is in any of the collections here.
    - **from** (string): List of vertex collection names.
    Edges in collection can only be inserted if their _from is in any of the collections here.
    - **collection**: Name of the edge collection, where the edge are stored in.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is false in this response.


**HTTP 202**
*A json document with these Properties is returned:*

Is returned if the graph could be created and waitForSync is disabled
for the `_graphs` collection and not given in the request.
The response body contains the graph configuration that has been stored.

- **graph**:
  - **smartGraphAttribute**: The name of the sharding attribute in smart graph case (Enterprise Edition only)
  - **replicationFactor**: The replication factor used for every new collection in the graph.
  - **orphanCollections** (string): An array of additional vertex collections.
   Documents within these collections do not have edges within this graph.
  - **name**: The name of the graph.
  - **_rev**: The revision of this graph. Can be used to make sure to not override
   concurrent modifications to this graph.
  - **numberOfShards**: Number of shards created for every new collection in the graph.
  - **isSmart**: Flag if the graph is a SmartGraph (Enterprise Edition only) or not.
  - **_id**: The internal id value of this graph. 
  - **edgeDefinitions**: An array of definitions for the relations of the graph.
   Each has the following type:
    - **to** (string): List of vertex collection names.
    Edges in collection can only be inserted if their _to is in any of the collections here.
    - **from** (string): List of vertex collection names.
    Edges in collection can only be inserted if their _from is in any of the collections here.
    - **collection**: Name of the edge collection, where the edge are stored in.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is false in this response.


**HTTP 400**
*A json document with these Properties is returned:*

Returned if the request is in a wrong format.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


**HTTP 403**
*A json document with these Properties is returned:*

Returned if your user has insufficient rights.
In order to create a graph you at least need to have the following privileges:
  1. `Administrate` access on the Database.
  2. `Read Only` access on every collection used within this graph.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


**HTTP 409**
*A json document with these Properties is returned:*

Returned if there is a conflict storing the graph.  This can occur
either if a graph with this name is already stored, or if there is one
edge definition with a the same
[edge collection](../../Manual/Appendix/Glossary.html#edge-collection) but a
different signature used in any other graph.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


)";


  ApiResult request(std::string const &databaseName);
};
namespace DeleteGraph {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Delete;
  static constexpr auto url = "/_api/gharial/{graph}";
  static constexpr auto summery = "Drop a graph";
  static constexpr auto operationId = "(DropAGraph";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Graph/general_graph_drop_http_examples.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Graph"};
  inline Parameter P0;
  inline Parameter P1;
  inline void initParam(){
    P0.description = R"(The name of the graph.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "graph";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.description = R"(Drop collections of this graph as well.  Collections will only be
dropped if they are not used in other graphs.)";
    P1.in = ParamIn::query;
    P1.name = "dropCollections";
    P1.required = false;
    P1.type = ParamFormat::booleanFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1};

  enum DeleteGraphResponse : short {
    Unknown = 0,
    Response201 = 201,
    Response202 = 202,
    Response403 = 403,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response201, R"(Is returned if the graph could be dropped and waitForSync is enabled
for the `_graphs` collection, or given in the request.)"}, 
{Response202, R"(Is returned if the graph could be dropped and waitForSync is disabled
for the `_graphs` collection and not given in the request.)"}, 
{Response403, R"(Returned if your user has insufficient rights.
In order to drop a graph you at least need to have the following privileges:
  1. `Administrate` access on the Database.)"}, 
{Response404, R"(Returned if no graph with this name could be found.)"}};
  static constexpr auto description = R"(Drops an existing graph object by name.
Optionally all collections not used by other graphs
can be dropped as well.


**HTTP 403**
*A json document with these Properties is returned:*

Returned if your user has insufficient rights.
In order to drop a graph you at least need to have the following privileges:
  1. `Administrate` access on the Database.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


**HTTP 404**
*A json document with these Properties is returned:*

Returned if no graph with this name could be found.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


)";


  ApiResult request(std::string const &databaseName);
};
namespace GetGraph {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/gharial/{graph}";
  static constexpr auto summery = "Get a graph";
  static constexpr auto operationId = "(GetAGraph";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Graph/general_graph_get_http_examples.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Graph"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The name of the graph.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "graph";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum GetGraphResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Returns the graph if it could be found.
The result will have the following format:)"}, 
{Response404, R"(Returned if no graph with this name could be found.)"}};
  static constexpr auto description = R"(Selects information for a given graph.
Will return the edge definitions as well as the orphan collections.
Or returns a 404 if the graph does not exist.


**HTTP 200**
*A json document with these Properties is returned:*

Returns the graph if it could be found.
The result will have the following format:

- **graph**:
  - **smartGraphAttribute**: The name of the sharding attribute in smart graph case (Enterprise Edition only)
  - **replicationFactor**: The replication factor used for every new collection in the graph.
  - **orphanCollections** (string): An array of additional vertex collections.
   Documents within these collections do not have edges within this graph.
  - **name**: The name of the graph.
  - **_rev**: The revision of this graph. Can be used to make sure to not override
   concurrent modifications to this graph.
  - **numberOfShards**: Number of shards created for every new collection in the graph.
  - **isSmart**: Flag if the graph is a SmartGraph (Enterprise Edition only) or not.
  - **_id**: The internal id value of this graph. 
  - **edgeDefinitions**: An array of definitions for the relations of the graph.
   Each has the following type:
    - **to** (string): List of vertex collection names.
    Edges in collection can only be inserted if their _to is in any of the collections here.
    - **from** (string): List of vertex collection names.
    Edges in collection can only be inserted if their _from is in any of the collections here.
    - **collection**: Name of the edge collection, where the edge are stored in.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is false in this response.


**HTTP 404**
*A json document with these Properties is returned:*

Returned if no graph with this name could be found.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


)";


  ApiResult request(std::string const &databaseName);
};
namespace GetEdge {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/gharial/{graph}/edge";
  static constexpr auto summery = "List edge definitions";
  static constexpr auto operationId = "(ListEdgeDefinitions";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Graph/general_graph_list_edge_http_examples.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Graph"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The name of the graph.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "graph";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum GetEdgeResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Is returned if the edge definitions could be listed.)"},
{Response404, R"(Returned if no graph with this name could be found.)"}};
  static constexpr auto description = R"(Lists all edge collections within this graph.


**HTTP 200**
*A json document with these Properties is returned:*

Is returned if the edge definitions could be listed.

- **code**: The response code.
- **collections** (string): The list of all vertex collections within this graph.
Includes collections in edgeDefinitions as well as orphans.
- **error**: Flag if there was an error (true) or not (false).
It is false in this response.


**HTTP 404**
*A json document with these Properties is returned:*

Returned if no graph with this name could be found.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


)";


  ApiResult request(std::string const &databaseName);
};
namespace PostEdge {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Post;
  static constexpr auto url = "/_api/gharial/{graph}/edge";
  static constexpr auto summery = "Add edge definition";
  static constexpr auto operationId = "(AddEdgeDefinition";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Graph/general_graph_edge_definition_add_http_examples.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Graph"};
  inline Parameter P0;
  inline Parameter P1;
  inline void initParam(){
    P0.description = R"(The name of the graph.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "graph";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.in = ParamIn::body;
    P1.name = "Json Request Body";
    P1.required = true;
    P1.schema = R"({"$ref":"#/definitions/general_graph_edge_definition_add_http_examples"})";
    P1.xDescriptionOffset = 486;
  }
  inline std::vector<Parameter> parameters{P0, P1};

  enum PostEdgeResponse : short {
    Unknown = 0,
    Response201 = 201,
    Response202 = 202,
    Response400 = 400,
    Response403 = 403,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response201, R"(Returned if the definition could be added successfully and
waitForSync is enabled for the `_graphs` collection.
The response body contains the graph configuration that has been stored.)"}, 
{Response202, R"(Returned if the definition could be added successfully and
waitForSync is disabled for the `_graphs` collection.
The response body contains the graph configuration that has been stored.)"}, 
{Response400, R"(Returned if the definition could not be added.
This could be because it is ill-formed, or
if the definition is used in an other graph with a different signature.)"}, 
{Response403, R"(Returned if your user has insufficient rights.
In order to modify a graph you at least need to have the following privileges:

  1. `Administrate` access on the Database.)"}, 
{Response404, R"(Returned if no graph with this name could be found.)"}};
  static constexpr auto description = R"(Adds an additional edge definition to the graph.

This edge definition has to contain a *collection* and an array of
each *from* and *to* vertex collections.  An edge definition can only
be added if this definition is either not used in any other graph, or
it is used with exactly the same definition. It is not possible to
store a definition "e" from "v1" to "v2" in the one graph, and "e"
from "v2" to "v1" in the other graph.


**A JSON object with these properties is required:**

  - **to** (string): One or many vertex collections that can contain target vertices.
  - **from** (string): One or many vertex collections that can contain source vertices.
  - **collection**: The name of the edge collection to be used.



**HTTP 201**
*A json document with these Properties is returned:*

Returned if the definition could be added successfully and
waitForSync is enabled for the `_graphs` collection.
The response body contains the graph configuration that has been stored.

- **graph**:
  - **smartGraphAttribute**: The name of the sharding attribute in smart graph case (Enterprise Edition only)
  - **replicationFactor**: The replication factor used for every new collection in the graph.
  - **orphanCollections** (string): An array of additional vertex collections.
   Documents within these collections do not have edges within this graph.
  - **name**: The name of the graph.
  - **_rev**: The revision of this graph. Can be used to make sure to not override
   concurrent modifications to this graph.
  - **numberOfShards**: Number of shards created for every new collection in the graph.
  - **isSmart**: Flag if the graph is a SmartGraph (Enterprise Edition only) or not.
  - **_id**: The internal id value of this graph. 
  - **edgeDefinitions**: An array of definitions for the relations of the graph.
   Each has the following type:
    - **to** (string): List of vertex collection names.
    Edges in collection can only be inserted if their _to is in any of the collections here.
    - **from** (string): List of vertex collection names.
    Edges in collection can only be inserted if their _from is in any of the collections here.
    - **collection**: Name of the edge collection, where the edge are stored in.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is false in this response.


**HTTP 202**
*A json document with these Properties is returned:*

Returned if the definition could be added successfully and
waitForSync is disabled for the `_graphs` collection.
The response body contains the graph configuration that has been stored.

- **graph**:
  - **smartGraphAttribute**: The name of the sharding attribute in smart graph case (Enterprise Edition only)
  - **replicationFactor**: The replication factor used for every new collection in the graph.
  - **orphanCollections** (string): An array of additional vertex collections.
   Documents within these collections do not have edges within this graph.
  - **name**: The name of the graph.
  - **_rev**: The revision of this graph. Can be used to make sure to not override
   concurrent modifications to this graph.
  - **numberOfShards**: Number of shards created for every new collection in the graph.
  - **isSmart**: Flag if the graph is a SmartGraph (Enterprise Edition only) or not.
  - **_id**: The internal id value of this graph. 
  - **edgeDefinitions**: An array of definitions for the relations of the graph.
   Each has the following type:
    - **to** (string): List of vertex collection names.
    Edges in collection can only be inserted if their _to is in any of the collections here.
    - **from** (string): List of vertex collection names.
    Edges in collection can only be inserted if their _from is in any of the collections here.
    - **collection**: Name of the edge collection, where the edge are stored in.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is false in this response.


**HTTP 400**
*A json document with these Properties is returned:*

Returned if the definition could not be added.
This could be because it is ill-formed, or
if the definition is used in an other graph with a different signature.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


**HTTP 403**
*A json document with these Properties is returned:*

Returned if your user has insufficient rights.
In order to modify a graph you at least need to have the following privileges:
  1. `Administrate` access on the Database.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


**HTTP 404**
*A json document with these Properties is returned:*

Returned if no graph with this name could be found.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


)";


  ApiResult request(std::string const &databaseName);
};
namespace PostCollection {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Post;
  static constexpr auto url = "/_api/gharial/{graph}/edge/{collection}";
  static constexpr auto summery = "Create an edge";
  static constexpr auto operationId = "(CreateAnEdge";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Graph/general_graph_edge_create_http_examples.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Graph"};
  inline Parameter P0;
  inline Parameter P1;
  inline Parameter P2;
  inline Parameter P3;
  inline Parameter P4;
  inline void initParam(){
    P0.description = R"(The name of the graph.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "graph";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.description = R"(The name of the edge collection the edge belongs to.)";
    P1.format = ParamFormat::stringFormat;
    P1.in = ParamIn::path;
    P1.name = "collection";
    P1.required = true;
    P1.type = ParamFormat::stringFormat;

    P2.description = R"(Define if the request should wait until synced to disk.)";
    P2.in = ParamIn::query;
    P2.name = "waitForSync";
    P2.required = false;
    P2.type = ParamFormat::booleanFormat;

    P3.description = R"(Define if the response should contain the complete
new version of the document.)";
    P3.in = ParamIn::query;
    P3.name = "returnNew";
    P3.required = false;
    P3.type = ParamFormat::booleanFormat;

    P4.in = ParamIn::body;
    P4.name = "Json Request Body";
    P4.required = true;
    P4.schema = R"({"$ref":"#/definitions/general_graph_edge_create_http_examples"})";
    P4.xDescriptionOffset = 345;
  }
  inline std::vector<Parameter> parameters{P0, P1, P2, P3, P4};

  enum PostCollectionResponse : short {
    Unknown = 0,
    Response201 = 201,
    Response202 = 202,
    Response400 = 400,
    Response403 = 403,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response201, R"(Returned if the edge could be created and waitForSync is true.)"},
{Response202, R"(Returned if the request was successful but waitForSync is false.)"}, 
{Response400, R"(Returned if the input document is invalid.
This can for instance be the case if `_from` or `_to` is mising.)"}, 
{Response403, R"(Returned if your user has insufficient rights.
In order to insert edges into the graph  you at least need to have the following privileges:

  1. `Read Only` access on the Database.
  2. `Write` access on the given collection.)"}, 
{Response404, R"(Returned in any of the following cases:
* no graph with this name could be found.
* this edge collection is not part of the graph.
* either `_from` or `_to` vertex does not exist.)"}};
  static constexpr auto description = R"(Creates a new edge in the collection.
Within the body the edge has to contain a *_from* and *_to* value referencing to valid vertices in the graph.
Furthermore the edge has to be valid in the definition of the used 
[edge collection](../../Manual/Appendix/Glossary.html#edge-collection).


**A JSON object with these properties is required:**

  - **_from**: The source vertex of this edge. Has to be valid within
   the used edge definition.
  - **_to**: The target vertex of this edge. Has to be valid within
   the used edge definition.



**HTTP 201**
*A json document with these Properties is returned:*

Returned if the edge could be created and waitForSync is true.

- **edge**:
  - **_key**: The _key value of the stored data.
  - **_rev**: The _rev value of the stored data.
  - **_id**: The _id value of the stored data.
  - **_from**: The _from value of the stored data.
  - **_to**: The _to value of the stored data.
- **code**: The response code.
- **new**:
  - **_key**: The _key value of the stored data.
  - **_rev**: The _rev value of the stored data.
  - **_id**: The _id value of the stored data.
  - **_from**: The _from value of the stored data.
  - **_to**: The _to value of the stored data.
- **error**: Flag if there was an error (true) or not (false).
It is false in this response.


**HTTP 202**
*A json document with these Properties is returned:*

Returned if the request was successful but waitForSync is false.

- **edge**:
  - **_key**: The _key value of the stored data.
  - **_rev**: The _rev value of the stored data.
  - **_id**: The _id value of the stored data.
  - **_from**: The _from value of the stored data.
  - **_to**: The _to value of the stored data.
- **code**: The response code.
- **new**:
  - **_key**: The _key value of the stored data.
  - **_rev**: The _rev value of the stored data.
  - **_id**: The _id value of the stored data.
  - **_from**: The _from value of the stored data.
  - **_to**: The _to value of the stored data.
- **error**: Flag if there was an error (true) or not (false).
It is false in this response.


**HTTP 400**
*A json document with these Properties is returned:*

Returned if the input document is invalid.
This can for instance be the case if `_from` or `_to` is mising.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


**HTTP 403**
*A json document with these Properties is returned:*

Returned if your user has insufficient rights.
In order to insert edges into the graph  you at least need to have the following privileges:
  1. `Read Only` access on the Database.
  2. `Write` access on the given collection.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


**HTTP 404**
*A json document with these Properties is returned:*

Returned in any of the following cases:
* no graph with this name could be found.
* this edge collection is not part of the graph.
* either `_from` or `_to` vertex does not exist.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


)";


  ApiResult request(std::string const &databaseName);
};
namespace DeleteEdge {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Delete;
  static constexpr auto url = "/_api/gharial/{graph}/edge/{collection}/{edge}";
  static constexpr auto summery = "Remove an edge";
  static constexpr auto operationId = "(RemoveAnEdge";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Graph/general_graph_edge_delete_http_examples.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Graph"};
  inline Parameter P0;
  inline Parameter P1;
  inline Parameter P2;
  inline Parameter P3;
  inline Parameter P4;
  inline Parameter P5;
  inline void initParam(){
    P0.description = R"(The name of the graph.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "graph";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.description = R"(The name of the edge collection the edge belongs to.)";
    P1.format = ParamFormat::stringFormat;
    P1.in = ParamIn::path;
    P1.name = "collection";
    P1.required = true;
    P1.type = ParamFormat::stringFormat;

    P2.description = R"(The *_key* attribute of the edge.)";
    P2.format = ParamFormat::stringFormat;
    P2.in = ParamIn::path;
    P2.name = "edge";
    P2.required = true;
    P2.type = ParamFormat::stringFormat;

    P3.description = R"(Define if the request should wait until synced to disk.)";
    P3.in = ParamIn::query;
    P3.name = "waitForSync";
    P3.required = false;
    P3.type = ParamFormat::booleanFormat;

    P4.description = R"(Define if a presentation of the deleted document should
be returned within the response object.)";
    P4.in = ParamIn::query;
    P4.name = "returnOld";
    P4.required = false;
    P4.type = ParamFormat::booleanFormat;

    P5.description = R"(If the "If-Match" header is given, then it must contain exactly one Etag. The document is updated,
if it has the same revision as the given Etag. Otherwise a HTTP 412 is returned. As an alternative
you can supply the Etag in an attribute rev in the URL.)";
    P5.in = ParamIn::header;
    P5.name = "if-match";
    P5.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1, P2, P3, P4, P5};

  enum DeleteEdgeResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response202 = 202,
    Response403 = 403,
    Response404 = 404,
    Response412 = 412
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Returned if the edge could be removed.)"},
{Response202, R"(Returned if the request was successful but waitForSync is false.)"}, 
{Response403, R"(Returned if your user has insufficient rights.
In order to delete vertices in the graph  you at least need to have the following privileges:

  1. `Read Only` access on the Database.
  2. `Write` access on the given collection.)"}, 
{Response404, R"(Returned in the following cases:
* No graph with this name could be found.
* This collection is not part of the graph.
* The edge to remove does not exist.)"}, 
{Response412, R"(Returned if if-match header is given, but the stored documents revision is different.)"}};
  static constexpr auto description = R"(Removes an edge from the collection.


**HTTP 200**
*A json document with these Properties is returned:*

Returned if the edge could be removed.

- **removed**: Is set to true if the remove was successful.
- **code**: The response code.
- **old**:
  - **_key**: The _key value of the stored data.
  - **_rev**: The _rev value of the stored data.
  - **_id**: The _id value of the stored data.
  - **_from**: The _from value of the stored data.
  - **_to**: The _to value of the stored data.
- **error**: Flag if there was an error (true) or not (false).
It is false in this response.


**HTTP 202**
*A json document with these Properties is returned:*

Returned if the request was successful but waitForSync is false.

- **removed**: Is set to true if the remove was successful.
- **code**: The response code.
- **old**:
  - **_key**: The _key value of the stored data.
  - **_rev**: The _rev value of the stored data.
  - **_id**: The _id value of the stored data.
  - **_from**: The _from value of the stored data.
  - **_to**: The _to value of the stored data.
- **error**: Flag if there was an error (true) or not (false).
It is false in this response.


**HTTP 403**
*A json document with these Properties is returned:*

Returned if your user has insufficient rights.
In order to delete vertices in the graph  you at least need to have the following privileges:
  1. `Read Only` access on the Database.
  2. `Write` access on the given collection.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


**HTTP 404**
*A json document with these Properties is returned:*

Returned in the following cases:
* No graph with this name could be found.
* This collection is not part of the graph.
* The edge to remove does not exist.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


**HTTP 412**
*A json document with these Properties is returned:*

Returned if if-match header is given, but the stored documents revision is different.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


)";


  ApiResult request(std::string const &databaseName);
};
namespace GetCollectionEdge {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/gharial/{graph}/edge/{collection}/{edge}";
  static constexpr auto summery = "Get an edge";
  static constexpr auto operationId = "(GetAnEdge";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Graph/general_graph_edge_get_http_examples.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Graph"};
  inline Parameter P0;
  inline Parameter P1;
  inline Parameter P2;
  inline Parameter P3;
  inline Parameter P4;
  inline Parameter P5;
  inline void initParam(){
    P0.description = R"(The name of the graph.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "graph";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.description = R"(The name of the edge collection the edge belongs to.)";
    P1.format = ParamFormat::stringFormat;
    P1.in = ParamIn::path;
    P1.name = "collection";
    P1.required = true;
    P1.type = ParamFormat::stringFormat;

    P2.description = R"(The *_key* attribute of the edge.)";
    P2.format = ParamFormat::stringFormat;
    P2.in = ParamIn::path;
    P2.name = "edge";
    P2.required = true;
    P2.type = ParamFormat::stringFormat;

    P3.description = R"(Must contain a revision.
If this is set a document is only returned if
it has exactly this revision.
Also see if-match header as an alternative to this.)";
    P3.in = ParamIn::query;
    P3.name = "rev";
    P3.required = false;
    P3.type = ParamFormat::stringFormat;

    P4.description = R"(If the "If-Match" header is given, then it must contain exactly one Etag. The document is returned,
if it has the same revision as the given Etag. Otherwise a HTTP 412 is returned. As an alternative
you can supply the Etag in an attribute rev in the URL.)";
    P4.in = ParamIn::header;
    P4.name = "if-match";
    P4.type = ParamFormat::stringFormat;

    P5.description = R"(If the "If-None-Match" header is given, then it must contain exactly one Etag. The document is returned,
only if it has a different revision as the given Etag. Otherwise a HTTP 304 is returned. )";
    P5.in = ParamIn::header;
    P5.name = "if-none-match";
    P5.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1, P2, P3, P4, P5};

  enum GetEdgeResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response304 = 304,
    Response403 = 403,
    Response404 = 404,
    Response412 = 412
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Returned if the edge could be found.)"},
{Response304, R"(Returned if the if-none-match header is given and the
currently stored edge still has this revision value.
So there was no update between the last time the edge
was fetched by the caller.)"}, 
{Response403, R"(Returned if your user has insufficient rights.
In order to update vertices in the graph  you at least need to have the following privileges:

  1. `Read Only` access on the Database.
  2. `Read Only` access on the given collection.)"}, 
{Response404, R"(Returned in the following cases:
* No graph with this name could be found.
* This collection is not part of the graph.
* The edge does not exist.)"}, 
{Response412, R"(Returned if if-match header is given, but the stored documents revision is different.)"}};
  static constexpr auto description = R"(Gets an edge from the given collection.


**HTTP 200**
*A json document with these Properties is returned:*

Returned if the edge could be found.

- **edge**:
  - **_key**: The _key value of the stored data.
  - **_rev**: The _rev value of the stored data.
  - **_id**: The _id value of the stored data.
  - **_from**: The _from value of the stored data.
  - **_to**: The _to value of the stored data.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is false in this response.


**HTTP 304**
*A json document with these Properties is returned:*

Returned if the if-none-match header is given and the
currently stored edge still has this revision value.
So there was no update between the last time the edge
was fetched by the caller.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


**HTTP 403**
*A json document with these Properties is returned:*

Returned if your user has insufficient rights.
In order to update vertices in the graph  you at least need to have the following privileges:
  1. `Read Only` access on the Database.
  2. `Read Only` access on the given collection.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


**HTTP 404**
*A json document with these Properties is returned:*

Returned in the following cases:
* No graph with this name could be found.
* This collection is not part of the graph.
* The edge does not exist.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


**HTTP 412**
*A json document with these Properties is returned:*

Returned if if-match header is given, but the stored documents revision is different.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


)";


  ApiResult request(std::string const &databaseName);
};
namespace PatchEdge {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Patch;
  static constexpr auto url = "/_api/gharial/{graph}/edge/{collection}/{edge}";
  static constexpr auto summery = "Modify an edge";
  static constexpr auto operationId = "(ModifyAnEdge";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Graph/general_graph_edge_modify_http_examples.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Graph"};
  inline Parameter P0;
  inline Parameter P1;
  inline Parameter P2;
  inline Parameter P3;
  inline Parameter P4;
  inline Parameter P5;
  inline Parameter P6;
  inline Parameter P7;
  inline Parameter P8;
  inline void initParam(){
    P0.description = R"(The name of the graph.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "graph";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.description = R"(The name of the edge collection the edge belongs to.)";
    P1.format = ParamFormat::stringFormat;
    P1.in = ParamIn::path;
    P1.name = "collection";
    P1.required = true;
    P1.type = ParamFormat::stringFormat;

    P2.description = R"(The *_key* attribute of the vertex.)";
    P2.format = ParamFormat::stringFormat;
    P2.in = ParamIn::path;
    P2.name = "edge";
    P2.required = true;
    P2.type = ParamFormat::stringFormat;

    P3.description = R"(Define if the request should wait until synced to disk.)";
    P3.in = ParamIn::query;
    P3.name = "waitForSync";
    P3.required = false;
    P3.type = ParamFormat::booleanFormat;

    P4.description = R"(Define if values set to null should be stored.
By default (true) the given documents attribute(s) will be set to null.
If this parameter is false the attribute(s) will instead be deleted from the
document.)";
    P4.in = ParamIn::query;
    P4.name = "keepNull";
    P4.required = false;
    P4.type = ParamFormat::booleanFormat;

    P5.description = R"(Define if a presentation of the deleted document should
be returned within the response object.)";
    P5.in = ParamIn::query;
    P5.name = "returnOld";
    P5.required = false;
    P5.type = ParamFormat::booleanFormat;

    P6.description = R"(Define if a presentation of the new document should
be returned within the response object.)";
    P6.in = ParamIn::query;
    P6.name = "returnNew";
    P6.required = false;
    P6.type = ParamFormat::booleanFormat;

    P7.description = R"(If the "If-Match" header is given, then it must contain exactly one Etag. The document is updated,
if it has the same revision as the given Etag. Otherwise a HTTP 412 is returned. As an alternative
you can supply the Etag in an attribute rev in the URL.)";
    P7.in = ParamIn::header;
    P7.name = "if-match";
    P7.type = ParamFormat::stringFormat;

    P8.description = R"(The body has to contain a JSON object containing exactly the attributes that should be overwritten, all other attributes remain unchanged.)";
    P8.in = ParamIn::body;
    P8.name = "Json Request Body";
    P8.required = true;
    P8.schema = R"({"additionalProperties":{},"type":"object"})";
    P8.xDescriptionOffset = 60;
  }
  inline std::vector<Parameter> parameters{P0, P1, P2, P3, P4, P5, P6, P7, P8};

  enum PatchEdgeResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response202 = 202,
    Response403 = 403,
    Response404 = 404,
    Response412 = 412
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Returned if the edge could be updated, and waitForSync is false.)"},
{Response202, R"(Returned if the request was successful but waitForSync is false.)"}, 
{Response403, R"(Returned if your user has insufficient rights.
In order to update edges in the graph  you at least need to have the following privileges:

  1. `Read Only` access on the Database.
  2. `Write` access on the given collection.)"}, 
{Response404, R"(Returned in the following cases:
* No graph with this name could be found.
* This collection is not part of the graph.
* The edge to update does not exist.
* either `_from` or `_to` vertex does not exist (if updated).)"}, 
{Response412, R"(Returned if if-match header is given, but the stored documents revision is different.)"}};
  static constexpr auto description = R"(Updates the data of the specific edge in the collection.


**HTTP 200**
*A json document with these Properties is returned:*

Returned if the edge could be updated, and waitForSync is false.

- **edge**:
  - **_key**: The _key value of the stored data.
  - **_rev**: The _rev value of the stored data.
  - **_id**: The _id value of the stored data.
  - **_from**: The _from value of the stored data.
  - **_to**: The _to value of the stored data.
- **code**: The response code.
- **old**:
  - **_key**: The _key value of the stored data.
  - **_rev**: The _rev value of the stored data.
  - **_id**: The _id value of the stored data.
  - **_from**: The _from value of the stored data.
  - **_to**: The _to value of the stored data.
- **new**:
  - **_key**: The _key value of the stored data.
  - **_rev**: The _rev value of the stored data.
  - **_id**: The _id value of the stored data.
  - **_from**: The _from value of the stored data.
  - **_to**: The _to value of the stored data.
- **error**: Flag if there was an error (true) or not (false).
It is false in this response.


**HTTP 202**
*A json document with these Properties is returned:*

Returned if the request was successful but waitForSync is false.

- **edge**:
  - **_key**: The _key value of the stored data.
  - **_rev**: The _rev value of the stored data.
  - **_id**: The _id value of the stored data.
  - **_from**: The _from value of the stored data.
  - **_to**: The _to value of the stored data.
- **code**: The response code.
- **old**:
  - **_key**: The _key value of the stored data.
  - **_rev**: The _rev value of the stored data.
  - **_id**: The _id value of the stored data.
  - **_from**: The _from value of the stored data.
  - **_to**: The _to value of the stored data.
- **new**:
  - **_key**: The _key value of the stored data.
  - **_rev**: The _rev value of the stored data.
  - **_id**: The _id value of the stored data.
  - **_from**: The _from value of the stored data.
  - **_to**: The _to value of the stored data.
- **error**: Flag if there was an error (true) or not (false).
It is false in this response.


**HTTP 403**
*A json document with these Properties is returned:*

Returned if your user has insufficient rights.
In order to update edges in the graph  you at least need to have the following privileges:
  1. `Read Only` access on the Database.
  2. `Write` access on the given collection.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


**HTTP 404**
*A json document with these Properties is returned:*

Returned in the following cases:
* No graph with this name could be found.
* This collection is not part of the graph.
* The edge to update does not exist.
* either `_from` or `_to` vertex does not exist (if updated).

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


**HTTP 412**
*A json document with these Properties is returned:*

Returned if if-match header is given, but the stored documents revision is different.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


)";


  ApiResult request(std::string const &databaseName);
};
namespace PutEdge {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/gharial/{graph}/edge/{collection}/{edge}";
  static constexpr auto summery = "Replace an edge";
  static constexpr auto operationId = "(ReplaceAnEdge";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Graph/general_graph_edge_replace_http_examples.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Graph"};
  inline Parameter P0;
  inline Parameter P1;
  inline Parameter P2;
  inline Parameter P3;
  inline Parameter P4;
  inline Parameter P5;
  inline Parameter P6;
  inline Parameter P7;
  inline Parameter P8;
  inline void initParam(){
    P0.description = R"(The name of the graph.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "graph";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.description = R"(The name of the edge collection the edge belongs to.)";
    P1.format = ParamFormat::stringFormat;
    P1.in = ParamIn::path;
    P1.name = "collection";
    P1.required = true;
    P1.type = ParamFormat::stringFormat;

    P2.description = R"(The *_key* attribute of the vertex.)";
    P2.format = ParamFormat::stringFormat;
    P2.in = ParamIn::path;
    P2.name = "edge";
    P2.required = true;
    P2.type = ParamFormat::stringFormat;

    P3.description = R"(Define if the request should wait until synced to disk.)";
    P3.in = ParamIn::query;
    P3.name = "waitForSync";
    P3.required = false;
    P3.type = ParamFormat::booleanFormat;

    P4.description = R"(Define if values set to null should be stored. By default the key is not removed from the document.)";
    P4.in = ParamIn::query;
    P4.name = "keepNull";
    P4.required = false;
    P4.type = ParamFormat::booleanFormat;

    P5.description = R"(Define if a presentation of the deleted document should
be returned within the response object.)";
    P5.in = ParamIn::query;
    P5.name = "returnOld";
    P5.required = false;
    P5.type = ParamFormat::booleanFormat;

    P6.description = R"(Define if a presentation of the new document should
be returned within the response object.)";
    P6.in = ParamIn::query;
    P6.name = "returnNew";
    P6.required = false;
    P6.type = ParamFormat::booleanFormat;

    P7.description = R"(If the "If-Match" header is given, then it must contain exactly one Etag. The document is updated,
if it has the same revision as the given Etag. Otherwise a HTTP 412 is returned. As an alternative
you can supply the Etag in an attribute rev in the URL.)";
    P7.in = ParamIn::header;
    P7.name = "if-match";
    P7.type = ParamFormat::stringFormat;

    P8.in = ParamIn::body;
    P8.name = "Json Request Body";
    P8.required = true;
    P8.schema = R"({"$ref":"#/definitions/general_graph_edge_replace_http_examples"})";
    P8.xDescriptionOffset = 105;
  }
  inline std::vector<Parameter> parameters{P0, P1, P2, P3, P4, P5, P6, P7, P8};

  enum PutEdgeResponse : short {
    Unknown = 0,
    Response201 = 201,
    Response202 = 202,
    Response403 = 403,
    Response404 = 404,
    Response412 = 412
  };
  inline const static std::map<int, char const * > responses{{Response201, R"(Returned if the request was successful but waitForSync is true.)"},
{Response202, R"(Returned if the request was successful but waitForSync is false.)"}, 
{Response403, R"(Returned if your user has insufficient rights.
In order to replace edges in the graph  you at least need to have the following privileges:

  1. `Read Only` access on the Database.
  2. `Write` access on the given collection.)"}, 
{Response404, R"(Returned in the following cases:
* No graph with this name could be found.
* This collection is not part of the graph.
* The edge to replace does not exist.
* either `_from` or `_to` vertex does not exist.)"}, 
{Response412, R"(Returned if if-match header is given, but the stored documents revision is different.)"}};
  static constexpr auto description = R"(Replaces the data of an edge in the collection.


**A JSON object with these properties is required:**

  - **_from**: The source vertex of this edge. Has to be valid within
   the used edge definition.
  - **_to**: The target vertex of this edge. Has to be valid within
   the used edge definition.



**HTTP 201**
*A json document with these Properties is returned:*

Returned if the request was successful but waitForSync is true.

- **edge**:
  - **_key**: The _key value of the stored data.
  - **_rev**: The _rev value of the stored data.
  - **_id**: The _id value of the stored data.
  - **_from**: The _from value of the stored data.
  - **_to**: The _to value of the stored data.
- **code**: The response code.
- **old**:
  - **_key**: The _key value of the stored data.
  - **_rev**: The _rev value of the stored data.
  - **_id**: The _id value of the stored data.
  - **_from**: The _from value of the stored data.
  - **_to**: The _to value of the stored data.
- **new**:
  - **_key**: The _key value of the stored data.
  - **_rev**: The _rev value of the stored data.
  - **_id**: The _id value of the stored data.
  - **_from**: The _from value of the stored data.
  - **_to**: The _to value of the stored data.
- **error**: Flag if there was an error (true) or not (false).
It is false in this response.


**HTTP 202**
*A json document with these Properties is returned:*

Returned if the request was successful but waitForSync is false.

- **edge**:
  - **_key**: The _key value of the stored data.
  - **_rev**: The _rev value of the stored data.
  - **_id**: The _id value of the stored data.
  - **_from**: The _from value of the stored data.
  - **_to**: The _to value of the stored data.
- **code**: The response code.
- **old**:
  - **_key**: The _key value of the stored data.
  - **_rev**: The _rev value of the stored data.
  - **_id**: The _id value of the stored data.
  - **_from**: The _from value of the stored data.
  - **_to**: The _to value of the stored data.
- **new**:
  - **_key**: The _key value of the stored data.
  - **_rev**: The _rev value of the stored data.
  - **_id**: The _id value of the stored data.
  - **_from**: The _from value of the stored data.
  - **_to**: The _to value of the stored data.
- **error**: Flag if there was an error (true) or not (false).
It is false in this response.


**HTTP 403**
*A json document with these Properties is returned:*

Returned if your user has insufficient rights.
In order to replace edges in the graph  you at least need to have the following privileges:
  1. `Read Only` access on the Database.
  2. `Write` access on the given collection.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


**HTTP 404**
*A json document with these Properties is returned:*

Returned in the following cases:
* No graph with this name could be found.
* This collection is not part of the graph.
* The edge to replace does not exist.
* either `_from` or `_to` vertex does not exist.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


**HTTP 412**
*A json document with these Properties is returned:*

Returned if if-match header is given, but the stored documents revision is different.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


)";


  ApiResult request(std::string const &databaseName);
};
namespace DeleteDefinition {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Delete;
  static constexpr auto url = "/_api/gharial/{graph}/edge/{definition}";
  static constexpr auto summery = "Remove an edge definition from the graph";
  static constexpr auto operationId = "(RemoveAnEdgeDefinitionFromTheGraph";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Graph/general_graph_edge_definition_remove_http_examples.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Graph"};
  inline Parameter P0;
  inline Parameter P1;
  inline Parameter P2;
  inline Parameter P3;
  inline void initParam(){
    P0.description = R"(The name of the graph.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "graph";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.description = R"(The name of the edge collection used in the definition.)";
    P1.format = ParamFormat::stringFormat;
    P1.in = ParamIn::path;
    P1.name = "definition";
    P1.required = true;
    P1.type = ParamFormat::stringFormat;

    P2.description = R"(Define if the request should wait until synced to disk.)";
    P2.in = ParamIn::query;
    P2.name = "waitForSync";
    P2.required = false;
    P2.type = ParamFormat::booleanFormat;

    P3.description = R"(Drop the collection as well.
Collection will only be dropped if it is not used in other graphs.)";
    P3.in = ParamIn::query;
    P3.name = "dropCollections";
    P3.required = false;
    P3.type = ParamFormat::booleanFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1, P2, P3};

  enum DeleteDefinitionResponse : short {
    Unknown = 0,
    Response201 = 201,
    Response202 = 202,
    Response403 = 403,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response201, R"(Returned if the edge definition could be removed from the graph
and waitForSync is true.)"}, 
{Response202, R"(Returned if the edge definition could be removed from the graph and
waitForSync is false.)"}, 
{Response403, R"(Returned if your user has insufficient rights.
In order to drop a vertex you at least need to have the following privileges:
  1. `Administrate` access on the Database.)"}, 
{Response404, R"(Returned if no graph with this name could be found,
or if no edge definition with this name is found in the graph.)"}};
  static constexpr auto description = R"(Remove one edge definition from the graph.  This will only remove the
edge collection, the vertex collections remain untouched and can still
be used in your queries.


**HTTP 201**
*A json document with these Properties is returned:*

Returned if the edge definition could be removed from the graph 
and waitForSync is true.

- **graph**:
  - **smartGraphAttribute**: The name of the sharding attribute in smart graph case (Enterprise Edition only)
  - **replicationFactor**: The replication factor used for every new collection in the graph.
  - **orphanCollections** (string): An array of additional vertex collections.
   Documents within these collections do not have edges within this graph.
  - **name**: The name of the graph.
  - **_rev**: The revision of this graph. Can be used to make sure to not override
   concurrent modifications to this graph.
  - **numberOfShards**: Number of shards created for every new collection in the graph.
  - **isSmart**: Flag if the graph is a SmartGraph (Enterprise Edition only) or not.
  - **_id**: The internal id value of this graph. 
  - **edgeDefinitions**: An array of definitions for the relations of the graph.
   Each has the following type:
    - **to** (string): List of vertex collection names.
    Edges in collection can only be inserted if their _to is in any of the collections here.
    - **from** (string): List of vertex collection names.
    Edges in collection can only be inserted if their _from is in any of the collections here.
    - **collection**: Name of the edge collection, where the edge are stored in.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is false in this response.


**HTTP 202**
*A json document with these Properties is returned:*

Returned if the edge definition could be removed from the graph and
waitForSync is false.

- **graph**:
  - **smartGraphAttribute**: The name of the sharding attribute in smart graph case (Enterprise Edition only)
  - **replicationFactor**: The replication factor used for every new collection in the graph.
  - **orphanCollections** (string): An array of additional vertex collections.
   Documents within these collections do not have edges within this graph.
  - **name**: The name of the graph.
  - **_rev**: The revision of this graph. Can be used to make sure to not override
   concurrent modifications to this graph.
  - **numberOfShards**: Number of shards created for every new collection in the graph.
  - **isSmart**: Flag if the graph is a SmartGraph (Enterprise Edition only) or not.
  - **_id**: The internal id value of this graph. 
  - **edgeDefinitions**: An array of definitions for the relations of the graph.
   Each has the following type:
    - **to** (string): List of vertex collection names.
    Edges in collection can only be inserted if their _to is in any of the collections here.
    - **from** (string): List of vertex collection names.
    Edges in collection can only be inserted if their _from is in any of the collections here.
    - **collection**: Name of the edge collection, where the edge are stored in.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is false in this response.


**HTTP 403**
*A json document with these Properties is returned:*

Returned if your user has insufficient rights.
In order to drop a vertex you at least need to have the following privileges:
  1. `Administrate` access on the Database.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


**HTTP 404**
*A json document with these Properties is returned:*

Returned if no graph with this name could be found,
or if no edge definition with this name is found in the graph.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


)";


  ApiResult request(std::string const &databaseName);
};
namespace PutDefinition {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/gharial/{graph}/edge/{definition}";
  static constexpr auto summery = "Replace an edge definition";
  static constexpr auto operationId = "(ReplaceAnEdgeDefinition";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Graph/general_graph_edge_definition_modify_http_examples.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Graph"};
  inline Parameter P0;
  inline Parameter P1;
  inline Parameter P2;
  inline Parameter P3;
  inline Parameter P4;
  inline void initParam(){
    P0.description = R"(The name of the graph.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "graph";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.description = R"(The name of the edge collection used in the definition.)";
    P1.format = ParamFormat::stringFormat;
    P1.in = ParamIn::path;
    P1.name = "definition";
    P1.required = true;
    P1.type = ParamFormat::stringFormat;

    P2.description = R"(Define if the request should wait until synced to disk.)";
    P2.in = ParamIn::query;
    P2.name = "waitForSync";
    P2.required = false;
    P2.type = ParamFormat::booleanFormat;

    P3.description = R"(Drop the collection as well.
Collection will only be dropped if it is not used in other graphs.)";
    P3.in = ParamIn::query;
    P3.name = "dropCollections";
    P3.required = false;
    P3.type = ParamFormat::booleanFormat;

    P4.in = ParamIn::body;
    P4.name = "Json Request Body";
    P4.required = true;
    P4.schema = R"({"$ref":"#/definitions/general_graph_edge_definition_modify_http_examples"})";
    P4.xDescriptionOffset = 184;
  }
  inline std::vector<Parameter> parameters{P0, P1, P2, P3, P4};

  enum PutDefinitionResponse : short {
    Unknown = 0,
    Response201 = 201,
    Response202 = 202,
    Response400 = 400,
    Response403 = 403,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response201, R"(Returned if the request was successful and waitForSync is true.)"},
{Response202, R"(Returned if the request was successful but waitForSync is false.)"}, 
{Response400, R"(Returned if no edge definition with this name is found in the graph,
or of the new definition is ill-formed and cannot be used.)"}, 
{Response403, R"(Returned if your user has insufficient rights.
In order to drop a vertex you at least need to have the following privileges:
  1. `Administrate` access on the Database.)"}, 
{Response404, R"(Returned if no graph with this name could be found.)"}};
  static constexpr auto description = R"(Change one specific edge definition.
This will modify all occurrences of this definition in all graphs known to your database.


**A JSON object with these properties is required:**

  - **to** (string): One or many vertex collections that can contain target vertices.
  - **from** (string): One or many vertex collections that can contain source vertices.
  - **collection**: The name of the edge collection to be used.



**HTTP 201**
*A json document with these Properties is returned:*

Returned if the request was successful and waitForSync is true.

- **graph**:
  - **smartGraphAttribute**: The name of the sharding attribute in smart graph case (Enterprise Edition only)
  - **replicationFactor**: The replication factor used for every new collection in the graph.
  - **orphanCollections** (string): An array of additional vertex collections.
   Documents within these collections do not have edges within this graph.
  - **name**: The name of the graph.
  - **_rev**: The revision of this graph. Can be used to make sure to not override
   concurrent modifications to this graph.
  - **numberOfShards**: Number of shards created for every new collection in the graph.
  - **isSmart**: Flag if the graph is a SmartGraph (Enterprise Edition only) or not.
  - **_id**: The internal id value of this graph. 
  - **edgeDefinitions**: An array of definitions for the relations of the graph.
   Each has the following type:
    - **to** (string): List of vertex collection names.
    Edges in collection can only be inserted if their _to is in any of the collections here.
    - **from** (string): List of vertex collection names.
    Edges in collection can only be inserted if their _from is in any of the collections here.
    - **collection**: Name of the edge collection, where the edge are stored in.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is false in this response.


**HTTP 202**
*A json document with these Properties is returned:*

Returned if the request was successful but waitForSync is false.

- **graph**:
  - **smartGraphAttribute**: The name of the sharding attribute in smart graph case (Enterprise Edition only)
  - **replicationFactor**: The replication factor used for every new collection in the graph.
  - **orphanCollections** (string): An array of additional vertex collections.
   Documents within these collections do not have edges within this graph.
  - **name**: The name of the graph.
  - **_rev**: The revision of this graph. Can be used to make sure to not override
   concurrent modifications to this graph.
  - **numberOfShards**: Number of shards created for every new collection in the graph.
  - **isSmart**: Flag if the graph is a SmartGraph (Enterprise Edition only) or not.
  - **_id**: The internal id value of this graph. 
  - **edgeDefinitions**: An array of definitions for the relations of the graph.
   Each has the following type:
    - **to** (string): List of vertex collection names.
    Edges in collection can only be inserted if their _to is in any of the collections here.
    - **from** (string): List of vertex collection names.
    Edges in collection can only be inserted if their _from is in any of the collections here.
    - **collection**: Name of the edge collection, where the edge are stored in.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is false in this response.


**HTTP 400**
*A json document with these Properties is returned:*

Returned if no edge definition with this name is found in the graph,
or of the new definition is ill-formed and cannot be used.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


**HTTP 403**
*A json document with these Properties is returned:*

Returned if your user has insufficient rights.
In order to drop a vertex you at least need to have the following privileges:
  1. `Administrate` access on the Database.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


**HTTP 404**
*A json document with these Properties is returned:*

Returned if no graph with this name could be found.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


)";


  ApiResult request(std::string const &databaseName);
};
namespace GetVertex {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/gharial/{graph}/vertex";
  static constexpr auto summery = "List vertex collections";
  static constexpr auto operationId = "(ListVertexCollections";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Graph/general_graph_list_vertex_http_examples.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Graph"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The name of the graph.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "graph";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum GetVertexResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Is returned if the collections could be listed.)"},
{Response404, R"(Returned if no graph with this name could be found.)"}};
  static constexpr auto description = R"(Lists all vertex collections within this graph.


**HTTP 200**
*A json document with these Properties is returned:*

Is returned if the collections could be listed.

- **code**: The response code.
- **collections** (string): The list of all vertex collections within this graph.
Includes collections in edgeDefinitions as well as orphans.
- **error**: Flag if there was an error (true) or not (false).
It is false in this response.


**HTTP 404**
*A json document with these Properties is returned:*

Returned if no graph with this name could be found.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


)";


  ApiResult request(std::string const &databaseName);
};
namespace PostVertex {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Post;
  static constexpr auto url = "/_api/gharial/{graph}/vertex";
  static constexpr auto summery = "Add vertex collection";
  static constexpr auto operationId = "(AddVertexCollection";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Graph/general_graph_vertex_collection_add_http_examples.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Graph"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The name of the graph.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "graph";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PostVertexResponse : short {
    Unknown = 0,
    Response201 = 201,
    Response202 = 202,
    Response400 = 400,
    Response403 = 403,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response201, R"(Is returned if the collection could be created and waitForSync is enabled
for the `_graphs` collection, or given in the request.
The response body contains the graph configuration that has been stored.)"}, 
{Response202, R"(Is returned if the collection could be created and waitForSync is disabled
for the `_graphs` collection, or given in the request.
The response body contains the graph configuration that has been stored.)"}, 
{Response400, R"(Returned if the request is in an invalid format.)"}, 
{Response403, R"(Returned if your user has insufficient rights.
In order to modify a graph you at least need to have the following privileges:

  1. `Administrate` access on the Database.
  2. `Read Only` access on every collection used within this graph.)"}, 
{Response404, R"(Returned if no graph with this name could be found.)"}};
  static constexpr auto description = R"(Adds a vertex collection to the set of orphan collections of the graph.
If the collection does not exist, it will be created.


**HTTP 201**
*A json document with these Properties is returned:*

Is returned if the collection could be created and waitForSync is enabled
for the `_graphs` collection, or given in the request.
The response body contains the graph configuration that has been stored.

- **graph**:
  - **smartGraphAttribute**: The name of the sharding attribute in smart graph case (Enterprise Edition only)
  - **replicationFactor**: The replication factor used for every new collection in the graph.
  - **orphanCollections** (string): An array of additional vertex collections.
   Documents within these collections do not have edges within this graph.
  - **name**: The name of the graph.
  - **_rev**: The revision of this graph. Can be used to make sure to not override
   concurrent modifications to this graph.
  - **numberOfShards**: Number of shards created for every new collection in the graph.
  - **isSmart**: Flag if the graph is a SmartGraph (Enterprise Edition only) or not.
  - **_id**: The internal id value of this graph. 
  - **edgeDefinitions**: An array of definitions for the relations of the graph.
   Each has the following type:
    - **to** (string): List of vertex collection names.
    Edges in collection can only be inserted if their _to is in any of the collections here.
    - **from** (string): List of vertex collection names.
    Edges in collection can only be inserted if their _from is in any of the collections here.
    - **collection**: Name of the edge collection, where the edge are stored in.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is false in this response.


**HTTP 202**
*A json document with these Properties is returned:*

Is returned if the collection could be created and waitForSync is disabled
for the `_graphs` collection, or given in the request.
The response body contains the graph configuration that has been stored.

- **graph**:
  - **smartGraphAttribute**: The name of the sharding attribute in smart graph case (Enterprise Edition only)
  - **replicationFactor**: The replication factor used for every new collection in the graph.
  - **orphanCollections** (string): An array of additional vertex collections.
   Documents within these collections do not have edges within this graph.
  - **name**: The name of the graph.
  - **_rev**: The revision of this graph. Can be used to make sure to not override
   concurrent modifications to this graph.
  - **numberOfShards**: Number of shards created for every new collection in the graph.
  - **isSmart**: Flag if the graph is a SmartGraph (Enterprise Edition only) or not.
  - **_id**: The internal id value of this graph. 
  - **edgeDefinitions**: An array of definitions for the relations of the graph.
   Each has the following type:
    - **to** (string): List of vertex collection names.
    Edges in collection can only be inserted if their _to is in any of the collections here.
    - **from** (string): List of vertex collection names.
    Edges in collection can only be inserted if their _from is in any of the collections here.
    - **collection**: Name of the edge collection, where the edge are stored in.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is false in this response.


**HTTP 400**
*A json document with these Properties is returned:*

Returned if the request is in an invalid format.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


**HTTP 403**
*A json document with these Properties is returned:*

Returned if your user has insufficient rights.
In order to modify a graph you at least need to have the following privileges:
  1. `Administrate` access on the Database.
  2. `Read Only` access on every collection used within this graph.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


**HTTP 404**
*A json document with these Properties is returned:*

Returned if no graph with this name could be found.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


)";


  ApiResult request(std::string const &databaseName);
};
namespace DeleteCollection {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Delete;
  static constexpr auto url = "/_api/gharial/{graph}/vertex/{collection}";
  static constexpr auto summery = "Remove vertex collection";
  static constexpr auto operationId = "(RemoveVertexCollection";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Graph/general_graph_vertex_collection_remove_http_examples.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Graph"};
  inline Parameter P0;
  inline Parameter P1;
  inline Parameter P2;
  inline void initParam(){
    P0.description = R"(The name of the graph.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "graph";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.description = R"(The name of the vertex collection.)";
    P1.format = ParamFormat::stringFormat;
    P1.in = ParamIn::path;
    P1.name = "collection";
    P1.required = true;
    P1.type = ParamFormat::stringFormat;

    P2.description = R"(Drop the collection as well.
Collection will only be dropped if it is not used in other graphs.)";
    P2.in = ParamIn::query;
    P2.name = "dropCollection";
    P2.required = false;
    P2.type = ParamFormat::booleanFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1, P2};

  enum DeleteCollectionResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response202 = 202,
    Response400 = 400,
    Response403 = 403,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Returned if the vertex collection was removed from the graph successfully
and waitForSync is true.)"}, 
{Response202, R"(Returned if the request was successful but waitForSync is false.)"}, 
{Response400, R"(Returned if the vertex collection is still used in an edge definition.
In this case it cannot be removed from the graph yet, it has to be
removed from the edge definition first.)"}, 
{Response403, R"(Returned if your user has insufficient rights.
In order to drop a vertex you at least need to have the following privileges:
  1. `Administrate` access on the Database.)"}, 
{Response404, R"(Returned if no graph with this name could be found.)"}};
  static constexpr auto description = R"(Removes a vertex collection from the graph and optionally deletes the collection,
if it is not used in any other graph.
It can only remove vertex collections that are no longer part of edge definitions,
if they are used in edge definitions you are required to modify those first.


**HTTP 200**
*A json document with these Properties is returned:*

Returned if the vertex collection was removed from the graph successfully
and waitForSync is true.

- **graph**:
  - **smartGraphAttribute**: The name of the sharding attribute in smart graph case (Enterprise Edition only)
  - **replicationFactor**: The replication factor used for every new collection in the graph.
  - **orphanCollections** (string): An array of additional vertex collections.
   Documents within these collections do not have edges within this graph.
  - **name**: The name of the graph.
  - **_rev**: The revision of this graph. Can be used to make sure to not override
   concurrent modifications to this graph.
  - **numberOfShards**: Number of shards created for every new collection in the graph.
  - **isSmart**: Flag if the graph is a SmartGraph (Enterprise Edition only) or not.
  - **_id**: The internal id value of this graph. 
  - **edgeDefinitions**: An array of definitions for the relations of the graph.
   Each has the following type:
    - **to** (string): List of vertex collection names.
    Edges in collection can only be inserted if their _to is in any of the collections here.
    - **from** (string): List of vertex collection names.
    Edges in collection can only be inserted if their _from is in any of the collections here.
    - **collection**: Name of the edge collection, where the edge are stored in.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is false in this response.


**HTTP 202**
*A json document with these Properties is returned:*

Returned if the request was successful but waitForSync is false.

- **graph**:
  - **smartGraphAttribute**: The name of the sharding attribute in smart graph case (Enterprise Edition only)
  - **replicationFactor**: The replication factor used for every new collection in the graph.
  - **orphanCollections** (string): An array of additional vertex collections.
   Documents within these collections do not have edges within this graph.
  - **name**: The name of the graph.
  - **_rev**: The revision of this graph. Can be used to make sure to not override
   concurrent modifications to this graph.
  - **numberOfShards**: Number of shards created for every new collection in the graph.
  - **isSmart**: Flag if the graph is a SmartGraph (Enterprise Edition only) or not.
  - **_id**: The internal id value of this graph. 
  - **edgeDefinitions**: An array of definitions for the relations of the graph.
   Each has the following type:
    - **to** (string): List of vertex collection names.
    Edges in collection can only be inserted if their _to is in any of the collections here.
    - **from** (string): List of vertex collection names.
    Edges in collection can only be inserted if their _from is in any of the collections here.
    - **collection**: Name of the edge collection, where the edge are stored in.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is false in this response.


**HTTP 400**
*A json document with these Properties is returned:*

Returned if the vertex collection is still used in an edge definition.
In this case it cannot be removed from the graph yet, it has to be
removed from the edge definition first.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


**HTTP 403**
*A json document with these Properties is returned:*

Returned if your user has insufficient rights.
In order to drop a vertex you at least need to have the following privileges:
  1. `Administrate` access on the Database.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


**HTTP 404**
*A json document with these Properties is returned:*

Returned if no graph with this name could be found.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


)";


  ApiResult request(std::string const &databaseName);
};
namespace PostVertexCollection {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Post;
  static constexpr auto url = "/_api/gharial/{graph}/vertex/{collection}";
  static constexpr auto summery = "Create a vertex";
  static constexpr auto operationId = "(CreateAVertex";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Graph/general_graph_vertex_create_http_examples.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Graph"};
  inline Parameter P0;
  inline Parameter P1;
  inline Parameter P2;
  inline Parameter P3;
  inline Parameter P4;
  inline void initParam(){
    P0.description = R"(The name of the graph.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "graph";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.description = R"(The name of the vertex collection the vertex should be inserted into.)";
    P1.format = ParamFormat::stringFormat;
    P1.in = ParamIn::path;
    P1.name = "collection";
    P1.required = true;
    P1.type = ParamFormat::stringFormat;

    P2.description = R"(Define if the request should wait until synced to disk.)";
    P2.in = ParamIn::query;
    P2.name = "waitForSync";
    P2.required = false;
    P2.type = ParamFormat::booleanFormat;

    P3.description = R"(Define if the response should contain the complete
new version of the document.)";
    P3.in = ParamIn::query;
    P3.name = "returnNew";
    P3.required = false;
    P3.type = ParamFormat::booleanFormat;

    P4.description = R"(The body has to be the JSON object to be stored.)";
    P4.in = ParamIn::body;
    P4.name = "Json Request Body";
    P4.required = true;
    P4.schema = R"({"additionalProperties":{},"type":"object"})";
    P4.xDescriptionOffset = 42;
  }
  inline std::vector<Parameter> parameters{P0, P1, P2, P3, P4};

  enum PostCollectionResponse : short {
    Unknown = 0,
    Response201 = 201,
    Response202 = 202,
    Response403 = 403,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response201, R"(Returned if the vertex could be added and waitForSync is true.)"},
{Response202, R"(Returned if the request was successful but waitForSync is false.)"}, 
{Response403, R"(Returned if your user has insufficient rights.
In order to insert vertices into the graph  you at least need to have the following privileges:

  1. `Read Only` access on the Database.
  2. `Write` access on the given collection.)"}, 
{Response404, R"(Returned if no graph with this name could be found.
Or if a graph is found but this collection is not part of the graph.)"}};
  static constexpr auto description = R"(Adds a vertex to the given collection.


**HTTP 201**
*A json document with these Properties is returned:*

Returned if the vertex could be added and waitForSync is true.

- **new**:
  - **_key**: The _key value of the stored data.
  - **_rev**: The _rev value of the stored data.
  - **_id**: The _id value of the stored data.
- **code**: The response code.
- **vertex**:
  - **_key**: The _key value of the stored data.
  - **_rev**: The _rev value of the stored data.
  - **_id**: The _id value of the stored data.
- **error**: Flag if there was an error (true) or not (false).
It is false in this response.


**HTTP 202**
*A json document with these Properties is returned:*

Returned if the request was successful but waitForSync is false.

- **new**:
  - **_key**: The _key value of the stored data.
  - **_rev**: The _rev value of the stored data.
  - **_id**: The _id value of the stored data.
- **code**: The response code.
- **vertex**:
  - **_key**: The _key value of the stored data.
  - **_rev**: The _rev value of the stored data.
  - **_id**: The _id value of the stored data.
- **error**: Flag if there was an error (true) or not (false).
It is false in this response.


**HTTP 403**
*A json document with these Properties is returned:*

Returned if your user has insufficient rights.
In order to insert vertices into the graph  you at least need to have the following privileges:
  1. `Read Only` access on the Database.
  2. `Write` access on the given collection.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


**HTTP 404**
*A json document with these Properties is returned:*

Returned if no graph with this name could be found.
Or if a graph is found but this collection is not part of the graph.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


)";


  ApiResult request(std::string const &databaseName);
};
namespace DeleteCollectionVertex {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Delete;
  static constexpr auto url = "/_api/gharial/{graph}/vertex/{collection}/{vertex}";
  static constexpr auto summery = "Remove a vertex";
  static constexpr auto operationId = "(RemoveAVertex";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Graph/general_graph_vertex_delete_http_examples.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Graph"};
  inline Parameter P0;
  inline Parameter P1;
  inline Parameter P2;
  inline Parameter P3;
  inline Parameter P4;
  inline Parameter P5;
  inline void initParam(){
    P0.description = R"(The name of the graph.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "graph";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.description = R"(The name of the vertex collection the vertex belongs to.)";
    P1.format = ParamFormat::stringFormat;
    P1.in = ParamIn::path;
    P1.name = "collection";
    P1.required = true;
    P1.type = ParamFormat::stringFormat;

    P2.description = R"(The *_key* attribute of the vertex.)";
    P2.format = ParamFormat::stringFormat;
    P2.in = ParamIn::path;
    P2.name = "vertex";
    P2.required = true;
    P2.type = ParamFormat::stringFormat;

    P3.description = R"(Define if the request should wait until synced to disk.)";
    P3.in = ParamIn::query;
    P3.name = "waitForSync";
    P3.required = false;
    P3.type = ParamFormat::booleanFormat;

    P4.description = R"(Define if a presentation of the deleted document should
be returned within the response object.)";
    P4.in = ParamIn::query;
    P4.name = "returnOld";
    P4.required = false;
    P4.type = ParamFormat::booleanFormat;

    P5.description = R"(If the "If-Match" header is given, then it must contain exactly one Etag. The document is updated,
if it has the same revision as the given Etag. Otherwise a HTTP 412 is returned. As an alternative
you can supply the Etag in an attribute rev in the URL.)";
    P5.in = ParamIn::header;
    P5.name = "if-match";
    P5.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1, P2, P3, P4, P5};

  enum DeleteVertexResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response202 = 202,
    Response403 = 403,
    Response404 = 404,
    Response412 = 412
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Returned if the vertex could be removed.)"},
{Response202, R"(Returned if the request was successful but waitForSync is false.)"}, 
{Response403, R"(Returned if your user has insufficient rights.
In order to delete vertices in the graph  you at least need to have the following privileges:

  1. `Read Only` access on the Database.
  2. `Write` access on the given collection.)"}, 
{Response404, R"(Returned in the following cases:
* No graph with this name could be found.
* This collection is not part of the graph.
* The vertex to remove does not exist.)"}, 
{Response412, R"(Returned if if-match header is given, but the stored documents revision is different.)"}};
  static constexpr auto description = R"(Removes a vertex from the collection.


**HTTP 200**
*A json document with these Properties is returned:*

Returned if the vertex could be removed.

- **removed**: Is set to true if the remove was successful.
- **code**: The response code.
- **old**:
  - **_key**: The _key value of the stored data.
  - **_rev**: The _rev value of the stored data.
  - **_id**: The _id value of the stored data.
- **error**: Flag if there was an error (true) or not (false).
It is false in this response.


**HTTP 202**
*A json document with these Properties is returned:*

Returned if the request was successful but waitForSync is false.

- **removed**: Is set to true if the remove was successful.
- **code**: The response code.
- **old**:
  - **_key**: The _key value of the stored data.
  - **_rev**: The _rev value of the stored data.
  - **_id**: The _id value of the stored data.
- **error**: Flag if there was an error (true) or not (false).
It is false in this response.


**HTTP 403**
*A json document with these Properties is returned:*

Returned if your user has insufficient rights.
In order to delete vertices in the graph  you at least need to have the following privileges:
  1. `Read Only` access on the Database.
  2. `Write` access on the given collection.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


**HTTP 404**
*A json document with these Properties is returned:*

Returned in the following cases:
* No graph with this name could be found.
* This collection is not part of the graph.
* The vertex to remove does not exist.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


**HTTP 412**
*A json document with these Properties is returned:*

Returned if if-match header is given, but the stored documents revision is different.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


)";


  ApiResult request(std::string const &databaseName);
};
namespace GetCollectionVertex {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/gharial/{graph}/vertex/{collection}/{vertex}";
  static constexpr auto summery = "Get a vertex";
  static constexpr auto operationId = "(GetAVertex";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Graph/general_graph_vertex_get_http_examples.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Graph"};
  inline Parameter P0;
  inline Parameter P1;
  inline Parameter P2;
  inline Parameter P3;
  inline Parameter P4;
  inline Parameter P5;
  inline void initParam(){
    P0.description = R"(The name of the graph.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "graph";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.description = R"(The name of the vertex collection the vertex belongs to.)";
    P1.format = ParamFormat::stringFormat;
    P1.in = ParamIn::path;
    P1.name = "collection";
    P1.required = true;
    P1.type = ParamFormat::stringFormat;

    P2.description = R"(The *_key* attribute of the vertex.)";
    P2.format = ParamFormat::stringFormat;
    P2.in = ParamIn::path;
    P2.name = "vertex";
    P2.required = true;
    P2.type = ParamFormat::stringFormat;

    P3.description = R"(Must contain a revision.
If this is set a document is only returned if
it has exactly this revision.
Also see if-match header as an alternative to this.)";
    P3.in = ParamIn::query;
    P3.name = "rev";
    P3.required = false;
    P3.type = ParamFormat::stringFormat;

    P4.description = R"(If the "If-Match" header is given, then it must contain exactly one Etag. The document is returned,
if it has the same revision as the given Etag. Otherwise a HTTP 412 is returned. As an alternative
you can supply the Etag in an query parameter *rev*.)";
    P4.in = ParamIn::header;
    P4.name = "if-match";
    P4.type = ParamFormat::stringFormat;

    P5.description = R"(If the "If-None-Match" header is given, then it must contain exactly one Etag. The document is returned,
only if it has a different revision as the given Etag. Otherwise a HTTP 304 is returned. )";
    P5.in = ParamIn::header;
    P5.name = "if-none-match";
    P5.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1, P2, P3, P4, P5};

  enum GetVertexResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response304 = 304,
    Response403 = 403,
    Response404 = 404,
    Response412 = 412
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Returned if the vertex could be found.)"},
{Response304, R"(Returned if the if-none-match header is given and the
currently stored vertex still has this revision value.
So there was no update between the last time the vertex
was fetched by the caller.)"}, 
{Response403, R"(Returned if your user has insufficient rights.
In order to update vertices in the graph  you at least need to have the following privileges:

  1. `Read Only` access on the Database.
  2. `Read Only` access on the given collection.)"}, 
{Response404, R"(Returned in the following cases:
* No graph with this name could be found.
* This collection is not part of the graph.
* The vertex does not exist.)"}, 
{Response412, R"(Returned if if-match header is given, but the stored documents revision is different.)"}};
  static constexpr auto description = R"(Gets a vertex from the given collection.


**HTTP 200**
*A json document with these Properties is returned:*

Returned if the vertex could be found.

- **code**: The response code.
- **vertex**:
  - **_key**: The _key value of the stored data.
  - **_rev**: The _rev value of the stored data.
  - **_id**: The _id value of the stored data.
- **error**: Flag if there was an error (true) or not (false).
It is false in this response.


**HTTP 304**
*A json document with these Properties is returned:*

Returned if the if-none-match header is given and the
currently stored vertex still has this revision value.
So there was no update between the last time the vertex
was fetched by the caller.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


**HTTP 403**
*A json document with these Properties is returned:*

Returned if your user has insufficient rights.
In order to update vertices in the graph  you at least need to have the following privileges:
  1. `Read Only` access on the Database.
  2. `Read Only` access on the given collection.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


**HTTP 404**
*A json document with these Properties is returned:*

Returned in the following cases:
* No graph with this name could be found.
* This collection is not part of the graph.
* The vertex does not exist.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


**HTTP 412**
*A json document with these Properties is returned:*

Returned if if-match header is given, but the stored documents revision is different.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


)";


  ApiResult request(std::string const &databaseName);
};
namespace PatchVertex {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Patch;
  static constexpr auto url = "/_api/gharial/{graph}/vertex/{collection}/{vertex}";
  static constexpr auto summery = "Update a vertex";
  static constexpr auto operationId = "(UpdateAVertex";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Graph/general_graph_vertex_modify_http_examples.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Graph"};
  inline Parameter P0;
  inline Parameter P1;
  inline Parameter P2;
  inline Parameter P3;
  inline Parameter P4;
  inline Parameter P5;
  inline Parameter P6;
  inline Parameter P7;
  inline Parameter P8;
  inline void initParam(){
    P0.description = R"(The name of the graph.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "graph";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.description = R"(The name of the vertex collection the vertex belongs to.)";
    P1.format = ParamFormat::stringFormat;
    P1.in = ParamIn::path;
    P1.name = "collection";
    P1.required = true;
    P1.type = ParamFormat::stringFormat;

    P2.description = R"(The *_key* attribute of the vertex.)";
    P2.format = ParamFormat::stringFormat;
    P2.in = ParamIn::path;
    P2.name = "vertex";
    P2.required = true;
    P2.type = ParamFormat::stringFormat;

    P3.description = R"(Define if the request should wait until synced to disk.)";
    P3.in = ParamIn::query;
    P3.name = "waitForSync";
    P3.required = false;
    P3.type = ParamFormat::booleanFormat;

    P4.description = R"(Define if values set to null should be stored.
By default (true) the given documents attribute(s) will be set to null.
If this parameter is false the attribute(s) will instead be delete from the
document.)";
    P4.in = ParamIn::query;
    P4.name = "keepNull";
    P4.required = false;
    P4.type = ParamFormat::booleanFormat;

    P5.description = R"(Define if a presentation of the deleted document should
be returned within the response object.)";
    P5.in = ParamIn::query;
    P5.name = "returnOld";
    P5.required = false;
    P5.type = ParamFormat::booleanFormat;

    P6.description = R"(Define if a presentation of the new document should
be returned within the response object.)";
    P6.in = ParamIn::query;
    P6.name = "returnNew";
    P6.required = false;
    P6.type = ParamFormat::booleanFormat;

    P7.description = R"(If the "If-Match" header is given, then it must contain exactly one Etag. The document is updated,
if it has the same revision as the given Etag. Otherwise a HTTP 412 is returned. As an alternative
you can supply the Etag in an attribute rev in the URL.)";
    P7.in = ParamIn::header;
    P7.name = "if-match";
    P7.type = ParamFormat::stringFormat;

    P8.description = R"(The body has to contain a JSON object containing exactly the attributes that should be overwritten, all other attributes remain unchanged.)";
    P8.in = ParamIn::body;
    P8.name = "Json Request Body";
    P8.required = true;
    P8.schema = R"({"additionalProperties":{},"type":"object"})";
    P8.xDescriptionOffset = 62;
  }
  inline std::vector<Parameter> parameters{P0, P1, P2, P3, P4, P5, P6, P7, P8};

  enum PatchVertexResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response202 = 202,
    Response403 = 403,
    Response404 = 404,
    Response412 = 412
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Returned if the vertex could be updated, and waitForSync is true.)"},
{Response202, R"(Returned if the request was successful, and waitForSync is false.)"}, 
{Response403, R"(Returned if your user has insufficient rights.
In order to update vertices in the graph  you at least need to have the following privileges:

  1. `Read Only` access on the Database.
  2. `Write` access on the given collection.)"}, 
{Response404, R"(Returned in the following cases:
* No graph with this name could be found.
* This collection is not part of the graph.
* The vertex to update does not exist.)"}, 
{Response412, R"(Returned if if-match header is given, but the stored documents revision is different.)"}};
  static constexpr auto description = R"(Updates the data of the specific vertex in the collection.


**HTTP 200**
*A json document with these Properties is returned:*

Returned if the vertex could be updated, and waitForSync is true.

- **new**:
  - **_key**: The _key value of the stored data.
  - **_rev**: The _rev value of the stored data.
  - **_id**: The _id value of the stored data.
- **old**:
  - **_key**: The _key value of the stored data.
  - **_rev**: The _rev value of the stored data.
  - **_id**: The _id value of the stored data.
- **code**: The response code.
- **vertex**:
  - **_key**: The _key value of the stored data.
  - **_rev**: The _rev value of the stored data.
  - **_id**: The _id value of the stored data.
- **error**: Flag if there was an error (true) or not (false).
It is false in this response.


**HTTP 202**
*A json document with these Properties is returned:*

Returned if the request was successful, and waitForSync is false.

- **new**:
  - **_key**: The _key value of the stored data.
  - **_rev**: The _rev value of the stored data.
  - **_id**: The _id value of the stored data.
- **old**:
  - **_key**: The _key value of the stored data.
  - **_rev**: The _rev value of the stored data.
  - **_id**: The _id value of the stored data.
- **code**: The response code.
- **vertex**:
  - **_key**: The _key value of the stored data.
  - **_rev**: The _rev value of the stored data.
  - **_id**: The _id value of the stored data.
- **error**: Flag if there was an error (true) or not (false).
It is false in this response.


**HTTP 403**
*A json document with these Properties is returned:*

Returned if your user has insufficient rights.
In order to update vertices in the graph  you at least need to have the following privileges:
  1. `Read Only` access on the Database.
  2. `Write` access on the given collection.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


**HTTP 404**
*A json document with these Properties is returned:*

Returned in the following cases:
* No graph with this name could be found.
* This collection is not part of the graph.
* The vertex to update does not exist.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


**HTTP 412**
*A json document with these Properties is returned:*

Returned if if-match header is given, but the stored documents revision is different.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


)";


  ApiResult request(std::string const &databaseName);
};
namespace PutVertex {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/gharial/{graph}/vertex/{collection}/{vertex}";
  static constexpr auto summery = "Replace a vertex";
  static constexpr auto operationId = "(ReplaceAVertex";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Graph/general_graph_vertex_replace_http_examples.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Graph"};
  inline Parameter P0;
  inline Parameter P1;
  inline Parameter P2;
  inline Parameter P3;
  inline Parameter P4;
  inline Parameter P5;
  inline Parameter P6;
  inline Parameter P7;
  inline Parameter P8;
  inline void initParam(){
    P0.description = R"(The name of the graph.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "graph";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.description = R"(The name of the vertex collection the vertex belongs to.)";
    P1.format = ParamFormat::stringFormat;
    P1.in = ParamIn::path;
    P1.name = "collection";
    P1.required = true;
    P1.type = ParamFormat::stringFormat;

    P2.description = R"(The *_key* attribute of the vertex.)";
    P2.format = ParamFormat::stringFormat;
    P2.in = ParamIn::path;
    P2.name = "vertex";
    P2.required = true;
    P2.type = ParamFormat::stringFormat;

    P3.description = R"(Define if the request should wait until synced to disk.)";
    P3.in = ParamIn::query;
    P3.name = "waitForSync";
    P3.required = false;
    P3.type = ParamFormat::booleanFormat;

    P4.description = R"(Define if values set to null should be stored. By default the key is not removed from the document.)";
    P4.in = ParamIn::query;
    P4.name = "keepNull";
    P4.required = false;
    P4.type = ParamFormat::booleanFormat;

    P5.description = R"(Define if a presentation of the deleted document should
be returned within the response object.)";
    P5.in = ParamIn::query;
    P5.name = "returnOld";
    P5.required = false;
    P5.type = ParamFormat::booleanFormat;

    P6.description = R"(Define if a presentation of the new document should
be returned within the response object.)";
    P6.in = ParamIn::query;
    P6.name = "returnNew";
    P6.required = false;
    P6.type = ParamFormat::booleanFormat;

    P7.description = R"(If the "If-Match" header is given, then it must contain exactly one Etag. The document is updated,
if it has the same revision as the given Etag. Otherwise a HTTP 412 is returned. As an alternative
you can supply the Etag in an attribute rev in the URL.)";
    P7.in = ParamIn::header;
    P7.name = "if-match";
    P7.type = ParamFormat::stringFormat;

    P8.description = R"(The body has to be the JSON object to be stored.)";
    P8.in = ParamIn::body;
    P8.name = "Json Request Body";
    P8.required = true;
    P8.schema = R"({"additionalProperties":{},"type":"object"})";
    P8.xDescriptionOffset = 52;
  }
  inline std::vector<Parameter> parameters{P0, P1, P2, P3, P4, P5, P6, P7, P8};

  enum PutVertexResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response202 = 202,
    Response403 = 403,
    Response404 = 404,
    Response412 = 412
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Returned if the vertex could be replaced, and waitForSync is true.)"},
{Response202, R"(Returned if the vertex could be replaced, and waitForSync is false.)"}, 
{Response403, R"(Returned if your user has insufficient rights.
In order to replace vertices in the graph  you at least need to have the following privileges:

  1. `Read Only` access on the Database.
  2. `Write` access on the given collection.)"}, 
{Response404, R"(Returned in the following cases:
* No graph with this name could be found.
* This collection is not part of the graph.
* The vertex to replace does not exist.)"}, 
{Response412, R"(Returned if if-match header is given, but the stored documents revision is different.)"}};
  static constexpr auto description = R"(Replaces the data of a vertex in the collection.


**HTTP 200**
*A json document with these Properties is returned:*

Returned if the vertex could be replaced, and waitForSync is true.

- **new**:
  - **_key**: The _key value of the stored data.
  - **_rev**: The _rev value of the stored data.
  - **_id**: The _id value of the stored data.
- **old**:
  - **_key**: The _key value of the stored data.
  - **_rev**: The _rev value of the stored data.
  - **_id**: The _id value of the stored data.
- **code**: The response code.
- **vertex**:
  - **_key**: The _key value of the stored data.
  - **_rev**: The _rev value of the stored data.
  - **_id**: The _id value of the stored data.
- **error**: Flag if there was an error (true) or not (false).
It is false in this response.


**HTTP 202**
*A json document with these Properties is returned:*

Returned if the vertex could be replaced, and waitForSync is false.

- **new**:
  - **_key**: The _key value of the stored data.
  - **_rev**: The _rev value of the stored data.
  - **_id**: The _id value of the stored data.
- **old**:
  - **_key**: The _key value of the stored data.
  - **_rev**: The _rev value of the stored data.
  - **_id**: The _id value of the stored data.
- **code**: The response code.
- **vertex**:
  - **_key**: The _key value of the stored data.
  - **_rev**: The _rev value of the stored data.
  - **_id**: The _id value of the stored data.
- **error**: Flag if there was an error (true) or not (false).
It is false in this response.


**HTTP 403**
*A json document with these Properties is returned:*

Returned if your user has insufficient rights.
In order to replace vertices in the graph  you at least need to have the following privileges:
  1. `Read Only` access on the Database.
  2. `Write` access on the given collection.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


**HTTP 404**
*A json document with these Properties is returned:*

Returned in the following cases:
* No graph with this name could be found.
* This collection is not part of the graph.
* The vertex to replace does not exist.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


**HTTP 412**
*A json document with these Properties is returned:*

Returned if if-match header is given, but the stored documents revision is different.

- **errorMessage**: A message created for this error.
- **errorNum**: ArangoDB error number for the error that occured.
- **code**: The response code.
- **error**: Flag if there was an error (true) or not (false).
It is true in this response.


)";


  ApiResult request(std::string const &databaseName);
};
}
namespace Bulk {
namespace PostImportDocument {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Post;
  static constexpr auto url = "/_api/import#document";
  static constexpr auto summery = "imports document values";
  static constexpr auto operationId = "(RestImportHandler#document";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Bulk/import_document.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Bulk"};
  inline Parameter P0;
  inline Parameter P1;
  inline Parameter P2;
  inline Parameter P3;
  inline Parameter P4;
  inline Parameter P5;
  inline Parameter P6;
  inline Parameter P7;
  inline Parameter P8;
  inline void initParam(){
    P0.description = R"(The body must consist of JSON-encoded arrays of attribute values, with one
line per document. The first row of the request must be a JSON-encoded
array of attribute names. These attribute names are used for the data in the
subsequent lines.)";
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"additionalProperties":{},"type":"object"})";
    P0.xDescriptionOffset = 0;

    P1.description = R"(The collection name.)";
    P1.in = ParamIn::query;
    P1.name = "collection";
    P1.required = true;
    P1.type = ParamFormat::stringFormat;

    P2.description = R"(An optional prefix for the values in `_from` attributes. If specified, the
value is automatically prepended to each `_from` input value. This allows
specifying just the keys for `_from`.)";
    P2.in = ParamIn::query;
    P2.name = "fromPrefix";
    P2.required = false;
    P2.type = ParamFormat::stringFormat;

    P3.description = R"(An optional prefix for the values in `_to` attributes. If specified, the
value is automatically prepended to each `_to` input value. This allows
specifying just the keys for `_to`.)";
    P3.in = ParamIn::query;
    P3.name = "toPrefix";
    P3.required = false;
    P3.type = ParamFormat::stringFormat;

    P4.description = R"(If this parameter has a value of `true` or `yes`, then all data in the
collection will be removed prior to the import. Note that any existing
index definitions will be preseved.)";
    P4.in = ParamIn::query;
    P4.name = "overwrite";
    P4.required = false;
    P4.type = ParamFormat::booleanFormat;

    P5.description = R"(Wait until documents have been synced to disk before returning.)";
    P5.in = ParamIn::query;
    P5.name = "waitForSync";
    P5.required = false;
    P5.type = ParamFormat::booleanFormat;

    P6.description = R"(Controls what action is carried out in case of a unique key constraint
violation. Possible values are:

- *error*: this will not import the current document because of the unique
  key constraint violation. This is the default setting.

- *update*: this will update an existing document in the database with the
  data specified in the request. Attributes of the existing document that
  are not present in the request will be preseved.

- *replace*: this will replace an existing document in the database with the
  data specified in the request.

- *ignore*: this will not update an existing document and simply ignore the
  error caused by the unique key constraint violation.

Note that *update*, *replace* and *ignore* will only work when the
import document in the request contains the *_key* attribute. *update* and
*replace* may also fail because of secondary unique key constraint
violations.)";
    P6.in = ParamIn::query;
    P6.name = "onDuplicate";
    P6.required = false;
    P6.type = ParamFormat::stringFormat;

    P7.description = R"(If set to `true` or `yes`, it will make the whole import fail if any error
occurs. Otherwise the import will continue even if some documents cannot
be imported.)";
    P7.in = ParamIn::query;
    P7.name = "complete";
    P7.required = false;
    P7.type = ParamFormat::booleanFormat;

    P8.description = R"(If set to `true` or `yes`, the result will include an attribute `details`
with details about documents that could not be imported.)";
    P8.in = ParamIn::query;
    P8.name = "details";
    P8.required = false;
    P8.type = ParamFormat::booleanFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1, P2, P3, P4, P5, P6, P7, P8};

  enum PostImportDocumentResponse : short {
    Unknown = 0,
    Response201 = 201,
    Response400 = 400,
    Response404 = 404,
    Response409 = 409,
    Response500 = 500
  };
  inline const static std::map<int, char const * > responses{{Response201, R"(is returned if all documents could be imported successfully.)"},
{Response400, R"(is returned if `type` contains an invalid value, no `collection` is
specified, the documents are incorrectly encoded, or the request
is malformed.)"}, 
{Response404, R"(is returned if `collection` or the `_from` or `_to` attributes of an
imported edge refer to an unknown collection.)"}, 
{Response409, R"(is returned if the import would trigger a unique key violation and
`complete` is set to `true`.)"}, 
{Response500, R"(is returned if the server cannot auto-generate a document key (out of keys
error) for a document with no user-defined key.)"}};
  static constexpr auto description = R"(**NOTE** Swagger examples won't work due to the anchor.


Creates documents in the collection identified by `collection-name`.
The first line of the request body must contain a JSON-encoded array of
attribute names. All following lines in the request body must contain
JSON-encoded arrays of attribute values. Each line is interpreted as a
separate document, and the values specified will be mapped to the array
of attribute names specified in the first header line.

The response is a JSON object with the following attributes:

- `created`: number of documents imported.

- `errors`: number of documents that were not imported due to an error.

- `empty`: number of empty lines found in the input (will only contain a
  value greater zero for types `documents` or `auto`).

- `updated`: number of updated/replaced documents (in case `onDuplicate`
  was set to either `update` or `replace`).

- `ignored`: number of failed but ignored insert operations (in case
  `onDuplicate` was set to `ignore`).

- `details`: if query parameter `details` is set to true, the result will
  contain a `details` attribute which is an array with more detailed
  information about which documents could not be inserted.


)";


  ApiResult request(std::string const &databaseName);
};
namespace PostImportJson {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Post;
  static constexpr auto url = "/_api/import#json";
  static constexpr auto summery = "imports documents from JSON";
  static constexpr auto operationId = "(RestImportHandler";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Bulk/import_json.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Bulk"};
  inline Parameter P0;
  inline Parameter P1;
  inline Parameter P2;
  inline Parameter P3;
  inline Parameter P4;
  inline Parameter P5;
  inline Parameter P6;
  inline Parameter P7;
  inline Parameter P8;
  inline Parameter P9;
  inline void initParam(){
    P0.description = R"(The body must either be a JSON-encoded array of objects or a string with
multiple JSON objects separated by newlines.)";
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"additionalProperties":{},"type":"object"})";
    P0.xDescriptionOffset = 0;

    P1.description = R"(Determines how the body of the request will be interpreted. `type` can have
the following values:
- `documents`: when this type is used, each line in the request body is
  expected to be an individual JSON-encoded document. Multiple JSON objects
  in the request body need to be separated by newlines.
- `list`: when this type is used, the request body must contain a single
  JSON-encoded array of individual objects to import.
- `auto`: if set, this will automatically determine the body type (either
  `documents` or `list`).)";
    P1.in = ParamIn::query;
    P1.name = "type";
    P1.required = true;
    P1.type = ParamFormat::stringFormat;

    P2.description = R"(The collection name.)";
    P2.in = ParamIn::query;
    P2.name = "collection";
    P2.required = true;
    P2.type = ParamFormat::stringFormat;

    P3.description = R"(An optional prefix for the values in `_from` attributes. If specified, the
value is automatically prepended to each `_from` input value. This allows
specifying just the keys for `_from`.)";
    P3.in = ParamIn::query;
    P3.name = "fromPrefix";
    P3.required = false;
    P3.type = ParamFormat::stringFormat;

    P4.description = R"(An optional prefix for the values in `_to` attributes. If specified, the
value is automatically prepended to each `_to` input value. This allows
specifying just the keys for `_to`.)";
    P4.in = ParamIn::query;
    P4.name = "toPrefix";
    P4.required = false;
    P4.type = ParamFormat::stringFormat;

    P5.description = R"(If this parameter has a value of `true` or `yes`, then all data in the
collection will be removed prior to the import. Note that any existing
index definitions will be preseved.)";
    P5.in = ParamIn::query;
    P5.name = "overwrite";
    P5.required = false;
    P5.type = ParamFormat::booleanFormat;

    P6.description = R"(Wait until documents have been synced to disk before returning.)";
    P6.in = ParamIn::query;
    P6.name = "waitForSync";
    P6.required = false;
    P6.type = ParamFormat::booleanFormat;

    P7.description = R"(Controls what action is carried out in case of a unique key constraint
violation. Possible values are:

- *error*: this will not import the current document because of the unique
  key constraint violation. This is the default setting.

- *update*: this will update an existing document in the database with the 
  data specified in the request. Attributes of the existing document that
  are not present in the request will be preseved.

- *replace*: this will replace an existing document in the database with the
  data specified in the request. 

- *ignore*: this will not update an existing document and simply ignore the
  error caused by a unique key constraint violation.

Note that that *update*, *replace* and *ignore* will only work when the
import document in the request contains the *_key* attribute. *update* and
*replace* may also fail because of secondary unique key constraint violations.)";
    P7.in = ParamIn::query;
    P7.name = "onDuplicate";
    P7.required = false;
    P7.type = ParamFormat::stringFormat;

    P8.description = R"(If set to `true` or `yes`, it will make the whole import fail if any error
occurs. Otherwise the import will continue even if some documents cannot
be imported.)";
    P8.in = ParamIn::query;
    P8.name = "complete";
    P8.required = false;
    P8.type = ParamFormat::booleanFormat;

    P9.description = R"(If set to `true` or `yes`, the result will include an attribute `details`
with details about documents that could not be imported.)";
    P9.in = ParamIn::query;
    P9.name = "details";
    P9.required = false;
    P9.type = ParamFormat::booleanFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1, P2, P3, P4, P5, P6, P7, P8, P9};

  enum PostImportJsonResponse : short {
    Unknown = 0,
    Response201 = 201,
    Response400 = 400,
    Response404 = 404,
    Response409 = 409,
    Response500 = 500
  };
  inline const static std::map<int, char const * > responses{{Response201, R"(is returned if all documents could be imported successfully.)"},
{Response400, R"(is returned if `type` contains an invalid value, no `collection` is
specified, the documents are incorrectly encoded, or the request
is malformed.)"}, 
{Response404, R"(is returned if `collection` or the `_from` or `_to` attributes of an
imported edge refer to an unknown collection.)"}, 
{Response409, R"(is returned if the import would trigger a unique key violation and
`complete` is set to `true`.)"}, 
{Response500, R"(is returned if the server cannot auto-generate a document key (out of keys
error) for a document with no user-defined key.)"}};
  static constexpr auto description = R"(**NOTE** Swagger examples won't work due to the anchor.


Creates documents in the collection identified by `collection-name`.
The JSON representations of the documents must be passed as the body of the
POST request. The request body can either consist of multiple lines, with
each line being a single stand-alone JSON object, or a singe JSON array with
sub-objects.

The response is a JSON object with the following attributes:

- `created`: number of documents imported.

- `errors`: number of documents that were not imported due to an error.

- `empty`: number of empty lines found in the input (will only contain a
  value greater zero for types `documents` or `auto`).

- `updated`: number of updated/replaced documents (in case `onDuplicate`
  was set to either `update` or `replace`).

- `ignored`: number of failed but ignored insert operations (in case
  `onDuplicate` was set to `ignore`).

- `details`: if query parameter `details` is set to true, the result will
  contain a `details` attribute which is an array with more detailed
  information about which documents could not be inserted.


)";


  ApiResult request(std::string const &databaseName);
};
}
namespace Index {
namespace GetIndex {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/index";
  static constexpr auto summery = "Read all indexes of a collection";
  static constexpr auto operationId = "(getIndexes";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Indexes/get_api_index.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Indexes"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The collection name.)";
    P0.in = ParamIn::query;
    P0.name = "collection";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum GetIndexResponse : short {
    Unknown = 0,
    Response200 = 200
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(returns a JSON object containing a list of indexes on that collection.)"}};
  static constexpr auto description = R"(Returns an object with an attribute *indexes* containing an array of all
index descriptions for the given collection. The same information is also
available in the *identifiers* as an object with the index handles as
keys.


)";


  ApiResult request(std::string const &databaseName);
};
namespace PostIndexFulltext {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Post;
  static constexpr auto url = "/_api/index#fulltext";
  static constexpr auto summery = "Create fulltext index";
  static constexpr auto operationId = "(createIndex#fulltext";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Indexes/post_api_index_fulltext.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Indexes"};
  inline Parameter P0;
  inline Parameter P1;
  inline void initParam(){
    P0.description = R"(The collection name.)";
    P0.in = ParamIn::query;
    P0.name = "collection-name";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.in = ParamIn::body;
    P1.name = "Json Request Body";
    P1.required = true;
    P1.schema = R"({"$ref":"#/definitions/post_api_index_fulltext"})";
    P1.xDescriptionOffset = 54;
  }
  inline std::vector<Parameter> parameters{P0, P1};

  enum PostIndexFulltextResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response201 = 201,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(If the index already exists, then a *HTTP 200* is
returned.)"}, 
{Response201, R"(If the index does not already exist and could be created, then a *HTTP 201*
is returned.)"}, 
{Response404, R"(If the *collection-name* is unknown, then a *HTTP 404* is returned.)"}};
  static constexpr auto description = R"(
**A JSON object with these properties is required:**

  - **fields** (string): an array of attribute names. Currently, the array is limited
   to exactly one attribute.
  - **type**: must be equal to *"fulltext"*.
  - **minLength**: Minimum character length of words to index. Will default
   to a server-defined value if unspecified. It is thus recommended to set
   this value explicitly when creating the index.




**NOTE** Swagger examples won't work due to the anchor.


Creates a fulltext index for the collection *collection-name*, if
it does not already exist. The call expects an object containing the index
details.


)";


  ApiResult request(std::string const &databaseName);
};
namespace PostIndexGeneral {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Post;
  static constexpr auto url = "/_api/index#general";
  static constexpr auto summery = "Create index";
  static constexpr auto operationId = "(createIndex:general";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Indexes/post_api_index.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Indexes"};
  inline Parameter P0;
  inline Parameter P1;
  inline void initParam(){
    P0.description = R"(The collection name.)";
    P0.in = ParamIn::query;
    P0.name = "collection";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.description = "";
    P1.in = ParamIn::body;
    P1.name = "Json Request Body";
    P1.required = true;
    P1.schema = R"({"additionalProperties":{},"type":"object"})";
    P1.xDescriptionOffset = 0;
  }
  inline std::vector<Parameter> parameters{P0, P1};

  enum PostIndexGeneralResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response201 = 201,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(If the index already exists, then an *HTTP 200* is returned.)"},
{Response201, R"(If the index does not already exist and could be created, then an *HTTP 201*
is returned.)"}, 
{Response400, R"(If an invalid index description is posted or attributes are used that the
target index will not support, then an *HTTP 400* is returned.)"}, 
{Response404, R"(If *collection* is unknown, then an *HTTP 404* is returned.)"}};
  static constexpr auto description = R"(**NOTE** Swagger examples won't work due to the anchor.


Creates a new index in the collection *collection*. Expects
an object containing the index details.

The type of the index to be created must specified in the *type*
attribute of the index details. Depending on the index type, additional
other attributes may need to specified in the request in order to create
the index.

Indexes require the to be indexed attribute(s) in the *fields* attribute
of the index details. Depending on the index type, a single attribute or
multiple attributes can be indexed. In the latter case, an array of
strings is expected.

Indexing the system attribute *_id* is not supported for user-defined indexes. 
Manually creating an index using *_id* as an index attribute will fail with 
an error.

Optionally, an index name may be specified as a string in the *name* attribute.
Index names have the same restrictions as collection names. If no value is
specified, one will be auto-generated.

Some indexes can be created as unique or non-unique variants. Uniqueness
can be controlled for most indexes by specifying the *unique* flag in the
index details. Setting it to *true* will create a unique index.
Setting it to *false* or omitting the *unique* attribute will
create a non-unique index.

**Note**: The following index types do not support uniqueness, and using
the *unique* attribute with these types may lead to an error:

- geo indexes
- fulltext indexes

**Note**: Unique indexes on non-shard keys are not supported in a
cluster.

Hash, skiplist and persistent indexes can optionally be created in a sparse
variant. A sparse index will be created if the *sparse* attribute in
the index details is set to *true*. Sparse indexes do not index documents
for which any of the index attributes is either not set or is *null*. 

The optional attribute **deduplicate** is supported by array indexes of
type *hash* or *skiplist*. It controls whether inserting duplicate index values 
from the same document into a unique array index will lead to a unique constraint
error or not. The default value is *true*, so only a single instance of each
non-unique index value will be inserted into the index per document. Trying to
insert a value into the index that already exists in the index will always fail,
regardless of the value of this attribute.)";


  ApiResult request(std::string const &databaseName, std::string const &collectionName);
};
namespace PostIndexGeo {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Post;
  static constexpr auto url = "/_api/index#geo";
  static constexpr auto summery = "Create geo-spatial index";
  static constexpr auto operationId = "(createIndex#geo";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Indexes/post_api_index_geo.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Indexes"};
  inline Parameter P0;
  inline Parameter P1;
  inline void initParam(){
    P0.description = R"(The collection name.
)";
    P0.in = ParamIn::query;
    P0.name = "collection";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.in = ParamIn::body;
    P1.name = "Json Request Body";
    P1.required = true;
    P1.schema = R"({"$ref":"#/definitions/post_api_index_geo"})";
    P1.xDescriptionOffset = 54;
  }
  inline std::vector<Parameter> parameters{P0, P1};

  enum PostIndexGeoResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response201 = 201,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(If the index already exists, then a *HTTP 200* is returned.)"},
{Response201, R"(If the index does not already exist and could be created, then a *HTTP 201*
is returned.)"}, 
{Response404, R"(If the *collection-name* is unknown, then a *HTTP 404* is returned.)"}};
  static constexpr auto description = R"(
**A JSON object with these properties is required:**

  - **fields** (string): An array with one or two attribute paths.
   If it is an array with one attribute path *location*, then a geo-spatial
   index on all documents is created using *location* as path to the
   coordinates. The value of the attribute must be an array with at least two
   double values. The array must contain the latitude (first value) and the
   longitude (second value). All documents, which do not have the attribute
   path or with value that are not suitable, are ignored.
   If it is an array with two attribute paths *latitude* and *longitude*,
   then a geo-spatial index on all documents is created using *latitude*
   and *longitude* as paths the latitude and the longitude. The value of
   the attribute *latitude* and of the attribute *longitude* must a
   double. All documents, which do not have the attribute paths or which
   values are not suitable, are ignored.
  - **type**: must be equal to *"geo"*.
  - **geoJson**: If a geo-spatial index on a *location* is constructed
   and *geoJson* is *true*, then the order within the array is longitude
   followed by latitude. This corresponds to the format described in
   http://geojson.org/geojson-spec.html#positions




**NOTE** Swagger examples won't work due to the anchor.


Creates a geo-spatial index in the collection *collection-name*, if
it does not already exist. Expects an object containing the index details.

Geo indexes are always sparse, meaning that documents that do not contain
the index attributes or have non-numeric values in the index attributes
will not be indexed.


)";


  ApiResult request(std::string const &databaseName);
};
namespace PostIndexHash {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Post;
  static constexpr auto url = "/_api/index#hash";
  static constexpr auto summery = "Create hash index";
  static constexpr auto operationId = "(createIndex:hash";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Indexes/post_api_index_hash.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Indexes"};
  inline Parameter P0;
  inline Parameter P1;
  inline void initParam(){
    P0.description = R"(The collection name.)";
    P0.in = ParamIn::query;
    P0.name = "collection-name";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.in = ParamIn::body;
    P1.name = "Json Request Body";
    P1.required = true;
    P1.schema = R"({"$ref":"#/definitions/post_api_index_hash"})";
    P1.xDescriptionOffset = 54;
  }
  inline std::vector<Parameter> parameters{P0, P1};

  enum PostIndexHashResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response201 = 201,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(If the index already exists, then a *HTTP 200* is returned.)"},
{Response201, R"(If the index does not already exist and could be created, then a *HTTP 201*
is returned.)"}, 
{Response400, R"(If the collection already contains documents and you try to create a unique
hash index in such a way that there are documents violating the uniqueness,
then a *HTTP 400* is returned.)"}, 
{Response404, R"(If the *collection-name* is unknown, then a *HTTP 404* is returned.)"}};
  static constexpr auto description = R"(
**A JSON object with these properties is required:**

  - **fields** (string): an array of attribute paths.
  - **unique**: if *true*, then create a unique index.
  - **type**: must be equal to *"hash"*.
  - **sparse**: if *true*, then create a sparse index.
  - **deduplicate**: if *false*, the deduplication of array values is turned off.




**NOTE** Swagger examples won't work due to the anchor.


Creates a hash index for the collection *collection-name* if it
does not already exist. The call expects an object containing the index
details.

In a sparse index all documents will be excluded from the index that do not 
contain at least one of the specified index attributes (i.e. *fields*) or that 
have a value of *null* in any of the specified index attributes. Such documents 
will not be indexed, and not be taken into account for uniqueness checks if
the *unique* flag is set.

In a non-sparse index, these documents will be indexed (for non-present
indexed attributes, a value of *null* will be used) and will be taken into
account for uniqueness checks if the *unique* flag is set.

**Note**: unique indexes on non-shard keys are not supported in a cluster.


)";


  ApiResult request(std::string const &databaseName);
};
namespace PostIndexPersistent {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Post;
  static constexpr auto url = "/_api/index#persistent";
  static constexpr auto summery = "Create a persistent index";
  static constexpr auto operationId = "(createIndex:persistent";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Indexes/post_api_index_persistent.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Indexes"};
  inline Parameter P0;
  inline Parameter P1;
  inline void initParam(){
    P0.description = R"(The collection name.)";
    P0.in = ParamIn::query;
    P0.name = "collection-name";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.in = ParamIn::body;
    P1.name = "Json Request Body";
    P1.required = true;
    P1.schema = R"({"$ref":"#/definitions/post_api_index_persistent"})";
    P1.xDescriptionOffset = 54;
  }
  inline std::vector<Parameter> parameters{P0, P1};

  enum PostIndexPersistentResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response201 = 201,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(If the index already exists, then a *HTTP 200* is
returned.)"}, 
{Response201, R"(If the index does not already exist and could be created, then a *HTTP 201*
is returned.)"}, 
{Response400, R"(If the collection already contains documents and you try to create a unique
persistent index in such a way that there are documents violating the
uniqueness, then a *HTTP 400* is returned.)"}, 
{Response404, R"(If the *collection-name* is unknown, then a *HTTP 404* is returned.)"}};
  static constexpr auto description = R"(
**A JSON object with these properties is required:**

  - **fields** (string): an array of attribute paths.
  - **unique**: if *true*, then create a unique index.
  - **type**: must be equal to *"persistent"*.
  - **sparse**: if *true*, then create a sparse index.





Creates a persistent index for the collection *collection-name*, if
it does not already exist. The call expects an object containing the index
details.

In a sparse index all documents will be excluded from the index that do not 
contain at least one of the specified index attributes (i.e. *fields*) or that 
have a value of *null* in any of the specified index attributes. Such documents 
will not be indexed, and not be taken into account for uniqueness checks if
the *unique* flag is set.

In a non-sparse index, these documents will be indexed (for non-present
indexed attributes, a value of *null* will be used) and will be taken into
account for uniqueness checks if the *unique* flag is set.

**Note**: unique indexes on non-shard keys are not supported in a cluster.


)";


  ApiResult request(std::string const &databaseName, const std::string &collectionName, std::vector<std::string> const &fieldNames);

};
namespace PostIndexSkiplist {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Post;
  static constexpr auto url = "/_api/index#skiplist";
  static constexpr auto summery = "Create skip list";
  static constexpr auto operationId = "(createIndex:skiplist";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Indexes/post_api_index_skiplist.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Indexes"};
  inline Parameter P0;
  inline Parameter P1;
  inline void initParam(){
    P0.description = R"(The collection name.)";
    P0.in = ParamIn::query;
    P0.name = "collection-name";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.in = ParamIn::body;
    P1.name = "Json Request Body";
    P1.required = true;
    P1.schema = R"({"$ref":"#/definitions/post_api_index_skiplist"})";
    P1.xDescriptionOffset = 54;
  }
  inline std::vector<Parameter> parameters{P0, P1};

  enum PostIndexSkiplistResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response201 = 201,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(If the index already exists, then a *HTTP 200* is
returned.)"}, 
{Response201, R"(If the index does not already exist and could be created, then a *HTTP 201*
is returned.)"}, 
{Response400, R"(If the collection already contains documents and you try to create a unique
skip-list index in such a way that there are documents violating the
uniqueness, then a *HTTP 400* is returned.)"}, 
{Response404, R"(If the *collection-name* is unknown, then a *HTTP 404* is returned.)"}};
  static constexpr auto description = R"(
**A JSON object with these properties is required:**

  - **fields** (string): an array of attribute paths.
  - **unique**: if *true*, then create a unique index.
  - **type**: must be equal to *"skiplist"*.
  - **sparse**: if *true*, then create a sparse index.
  - **deduplicate**: if *false*, the deduplication of array values is turned off.





Creates a skip-list index for the collection *collection-name*, if
it does not already exist. The call expects an object containing the index
details.

In a sparse index all documents will be excluded from the index that do not 
contain at least one of the specified index attributes (i.e. *fields*) or that 
have a value of *null* in any of the specified index attributes. Such documents 
will not be indexed, and not be taken into account for uniqueness checks if
the *unique* flag is set.

In a non-sparse index, these documents will be indexed (for non-present
indexed attributes, a value of *null* will be used) and will be taken into
account for uniqueness checks if the *unique* flag is set.

**Note**: unique indexes on non-shard keys are not supported in a cluster.


)";


  ApiResult request(std::string const &databaseName);
};
namespace PostIndexTtl {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Post;
  static constexpr auto url = "/_api/index#ttl";
  static constexpr auto summery = "Create TTL index";
  static constexpr auto operationId = "(createIndex:ttl";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Indexes/post_api_index_ttl.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Indexes"};
  inline Parameter P0;
  inline Parameter P1;
  inline void initParam(){
    P0.description = R"(The collection name.)";
    P0.in = ParamIn::query;
    P0.name = "collection-name";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.in = ParamIn::body;
    P1.name = "Json Request Body";
    P1.required = true;
    P1.schema = R"({"$ref":"#/definitions/post_api_index_ttl"})";
    P1.xDescriptionOffset = 54;
  }
  inline std::vector<Parameter> parameters{P0, P1};

  enum PostIndexTtlResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response201 = 201,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(If the index already exists, then a *HTTP 200* is returned.)"},
{Response201, R"(If the index does not already exist and could be created, then a *HTTP 201*
is returned.)"}, 
{Response400, R"(If the collection already contains another TTL index, then an *HTTP 400* is 
returned, as there can be at most one TTL index per collection.)"}, 
{Response404, R"(If the *collection-name* is unknown, then a *HTTP 404* is returned.)"}};
  static constexpr auto description = R"(
**A JSON object with these properties is required:**

  - **fields** (string): an array with exactly one attribute path.
  - **type**: must be equal to *"ttl"*.
  - **expireAfter**: The time (in seconds) after a document's creation after which the
   documents count as "expired".




**NOTE** Swagger examples won't work due to the anchor.

Creates a TTL index for the collection *collection-name* if it
does not already exist. The call expects an object containing the index
details.


)";


  ApiResult request(std::string const &databaseName);
};
namespace DeleteIndexHandle {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Delete;
  static constexpr auto url = "/_api/index/{index-handle}";
  static constexpr auto summery = "Delete index";
  static constexpr auto operationId = "(dropIndex";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Indexes/post_api_index_delete.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Indexes"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The index handle.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "index-handle";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum DeleteIndexHandleResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(If the index could be deleted, then an *HTTP 200* is
returned.)"}, 
{Response404, R"(If the *index-handle* is unknown, then an *HTTP 404* is returned.
)"}};
  static constexpr auto description = R"(Deletes an index with *index-handle*.


)";


  ApiResult request(std::string const &databaseName);
};
namespace GetIndexHandle {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/index/{index-handle}";
  static constexpr auto summery = "Read index";
  static constexpr auto operationId = "(getIndexes:handle";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Indexes/get_api_reads_index.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Indexes"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The index-handle.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "index-handle";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum GetIndexHandleResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(If the index exists, then a *HTTP 200* is returned.)"},
{Response404, R"(If the index does not exist, then a *HTTP 404*
is returned.)"}};
  static constexpr auto description = R"(The result is an object describing the index. It has at least the following
attributes:

- *id*: the identifier of the index

- *type*: the index type

All other attributes are type-dependent. For example, some indexes provide
*unique* or *sparse* flags, whereas others don't. Some indexes also provide 
a selectivity estimate in the *selectivityEstimate* attribute of the result.


)";


  ApiResult request(std::string const &databaseName);
};
}
namespace Job {
namespace GetJobId {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/job/{job-id}";
  static constexpr auto summery = "Returns async job";
  static constexpr auto operationId = "(getJobById";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/job/job_getStatusById.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"job"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The async job id.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "job-id";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum GetJobIdResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response204, 
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned if the job requested via job-id has been executed
and its result is ready to fetch.)"}, 
{Response204, R"(is returned if the job requested via job-id is still in the queue of pending
(or not yet finished) jobs.)"}, 
{Response404, R"(is returned if the job was not found or already deleted or fetched from the
job result list.)"}};
  static constexpr auto description = R"(Returns the processing status of the specified job. The processing status
can be
determined by peeking into the HTTP response code of the response.


)";


  ApiResult request(std::string const &databaseName);
};
namespace PutJobId {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/job/{job-id}";
  static constexpr auto summery = "Return result of an async job";
  static constexpr auto operationId = "(getJobResult";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/job/job_fetch_result.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"job"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The async job id.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "job-id";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PutJobIdResponse : short {
    Unknown = 0,
    Response204, 
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response204, R"(is returned if the job requested via job-id is still in the queue of pending
(or not yet finished) jobs. In this case, no x-arango-async-id HTTP header
will be returned.)"}, 
{Response400, R"(is returned if no job-id was specified in the request. In this case,
no x-arango-async-id HTTP header will be returned.)"}, 
{Response404, R"(is returned if the job was not found or already deleted or fetched from
the job result list. In this case, no x-arango-async-id HTTP header will
be returned.)"}};
  static constexpr auto description = R"(Returns the result of an async job identified by job-id. If the async job
result is present on the server, the result will be removed from the list of
result. That means this method can be called for each job-id once.
The method will return the original job result's headers and body, plus the
additional HTTP header x-arango-async-job-id. If this header is present,
then
the job was found and the response contains the original job's result. If
the header is not present, the job was not found and the response contains
status information from the job manager.


)";


  ApiResult request(std::string const &databaseName);
};
namespace PutCancel {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/job/{job-id}/cancel";
  static constexpr auto summery = "Cancel async job";
  static constexpr auto operationId = "(putJobMethod:cancel";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/job/job_cancel.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"job"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The async job id.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "job-id";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PutCancelResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(cancel has been initiated.)"},
{Response400, R"(is returned if no job-id was specified in the request. In this case,
no x-arango-async-id HTTP header will be returned.)"}, 
{Response404, R"(is returned if the job was not found or already deleted or fetched from
the job result list. In this case, no x-arango-async-id HTTP header will
be returned.)"}};
  static constexpr auto description = R"(Cancels the currently running job identified by job-id. Note that it still
might take some time to actually cancel the running async job.


)";


  ApiResult request(std::string const &databaseName);
};
namespace DeleteType {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Delete;
  static constexpr auto url = "/_api/job/{type}";
  static constexpr auto summery = "Deletes async job";
  static constexpr auto operationId = "(deleteJob:byType";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/job/job_delete.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"job"};
  inline Parameter P0;
  inline Parameter P1;
  inline void initParam(){
    P0.description = R"(The type of jobs to delete. type can be:
* *all*: Deletes all jobs results. Currently executing or queued async 
  jobs will not be stopped by this call.
* *expired*: Deletes expired results. To determine the expiration status of a 
  result, pass the stamp query parameter. stamp needs to be a UNIX timestamp, 
  and all async job results created at a lower timestamp will be deleted.
* *an actual job-id*: In this case, the call will remove the result of the
  specified async job. If the job is currently executing or queued, it will
  not be aborted.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "type";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.description = R"(A UNIX timestamp specifying the expiration threshold when type is expired.)";
    P1.in = ParamIn::query;
    P1.name = "stamp";
    P1.required = false;
    P1.type = ParamFormat::numberFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1};

  enum DeleteTypeResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned if the deletion operation was carried out successfully.
This code will also be returned if no results were deleted.)"}, 
{Response400, R"(is returned if type is not specified or has an invalid value.)"}, 
{Response404, R"(is returned if type is a job-id but no async job with the specified id was
found.)"}};
  static constexpr auto description = R"(Deletes either all job results, expired job results, or the result of a
specific job.
Clients can use this method to perform an eventual garbage collection of job
results.


)";


  ApiResult request(std::string const &databaseName);
};
namespace GetType {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/job/{type}";
  static constexpr auto summery = "Returns list of async jobs";
  static constexpr auto operationId = "(getJob";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/job/job_getByType.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"job"};
  inline Parameter P0;
  inline Parameter P1;
  inline void initParam(){
    P0.description = R"(The type of jobs to return. The type can be either done or pending. Setting
the type to done will make the method return the ids of already completed
async
jobs for which results can be fetched. Setting the type to pending will
return
the ids of not yet finished async jobs.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "type";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.description = R"(
The maximum number of ids to return per call. If not specified, a
server-defined maximum value will be used.)";
    P1.in = ParamIn::query;
    P1.name = "count";
    P1.required = false;
    P1.type = ParamFormat::numberFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1};

  enum GetTypeResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned if the list can be compiled successfully. Note: the list might
be empty.)"}, 
{Response400, R"(is returned if type is not specified or has an invalid value.)"}};
  static constexpr auto description = R"(Returns the list of ids of async jobs with a specific status (either done or
pending).
The list can be used by the client to get an overview of the job system
status and
to retrieve completed job results later.


)";


  ApiResult request(std::string const &databaseName);
};
}
namespace AQL {
namespace PostQuery {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Post;
  static constexpr auto url = "/_api/query";
  static constexpr auto summery = "Parse an AQL query";
  static constexpr auto operationId = "(parseQuery";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/AQL/PostApiQueryProperties.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"AQL"};
  inline Parameter P0;
  inline void initParam(){
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"$ref":"#/definitions/PostApiQueryProperties"})";
    P0.xDescriptionOffset = 152;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PostQueryResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(If the query is valid, the server will respond with *HTTP 200* and
return the names of the bind parameters it found in the query (if any) in
the *bindVars* attribute of the response. It will also return an array
of the collections used in the query in the *collections* attribute.
If a query can be parsed successfully, the *ast* attribute of the returned
JSON will contain the abstract syntax tree representation of the query.
The format of the *ast* is subject to change in future versions of
ArangoDB, but it can be used to inspect how ArangoDB interprets a given
query. Note that the abstract syntax tree will be returned without any
optimizations applied to it.)"}, 
{Response400, R"(The server will respond with *HTTP 400* in case of a malformed request,
or if the query contains a parse error. The body of the response will
contain the error details embedded in a JSON object.)"}};
  static constexpr auto description = R"(This endpoint is for query validation only. To actually query the database,
see `/api/cursor`.


**A JSON object with these properties is required:**

  - **query**: To validate a query string without executing it, the query string can be
   passed to the server via an HTTP POST request.



)";


  ApiResult request(std::string const &databaseName, const std::string &body);
};
namespace DeleteQueryCache {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Delete;
  static constexpr auto url = "/_api/query-cache";
  static constexpr auto summery = "Clears any results in the AQL query results cache";
  static constexpr auto operationId = "(clearCache";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/AQL/DeleteApiQueryCache.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"AQL"};
  
  enum DeleteQueryCacheResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(The server will respond with *HTTP 200* when the cache was cleared
successfully.)"}, 
{Response400, R"(The server will respond with *HTTP 400* in case of a malformed request.
)"}};
  static constexpr auto description = R"(clears the query results cache for the current database
)";


  ApiResult request(std::string const &databaseName);
};
namespace GetCacheEntries {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/query-cache/entries";
  static constexpr auto summery = "Returns the currently cached query results";
  static constexpr auto operationId = "(readQueries";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/AQL/GetApiQueryCacheCurrent.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"AQL"};
  
  enum GetEntriesResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Is returned when the list of results can be retrieved successfully.)"},
{Response400, R"(The server will respond with *HTTP 400* in case of a malformed request,

)"}};
  static constexpr auto description = R"(Returns an array containing the AQL query results currently stored in the query results
cache of the selected database. Each result is a JSON object with the following attributes:

- *hash*: the query result's hash

- *query*: the query string 

- *bindVars*: the query's bind parameters. this attribute is only shown if tracking for
  bind variables was enabled at server start

- *size*: the size of the query result and bind parameters, in bytes

- *results*: number of documents/rows in the query result

- *started*: the date and time when the query was stored in the cache

- *hits*: number of times the result was served from the cache (can be 
  *0* for queries that were only stored in the cache but were never accessed
  again afterwards)

- *runTime*: the query's run time

- *dataSources*: an array of collections/views the query was using)";


  ApiResult request(std::string const &databaseName);
};
namespace GetCacheProperties {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/query-cache/properties";
  static constexpr auto summery = "Returns the global properties for the AQL query results cache";
  static constexpr auto operationId = "(readProperties";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/AQL/GetApiQueryCacheProperties.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"AQL"};
  
  enum GetPropertiesResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Is returned if the properties can be retrieved successfully.)"},
{Response400, R"(The server will respond with *HTTP 400* in case of a malformed request,

)"}};
  static constexpr auto description = R"(Returns the global AQL query results cache configuration. The configuration is a
JSON object with the following properties:

- *mode*: the mode the AQL query results cache operates in. The mode is one of the following
  values: *off*, *on* or *demand*.

- *maxResults*: the maximum number of query results that will be stored per database-specific
  cache.

- *maxResultsSize*: the maximum cumulated size of query results that will be stored per 
  database-specific cache.

- *maxEntrySize*: the maximum individual result size of queries that will be stored per 
  database-specific cache.

- *includeSystem*: whether or not results of queries that involve system collections will be
  stored in the query results cache.)";


  ApiResult request(std::string const &databaseName);
};
namespace PutCacheProperties {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/query-cache/properties";
  static constexpr auto summery = "Globally adjusts the AQL query results cache properties";
  static constexpr auto operationId = "(replaceProperties:QueryCache";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/AQL/PutApiQueryCacheProperties.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"AQL"};
  inline Parameter P0;
  inline void initParam(){
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"$ref":"#/definitions/PutApiQueryCacheProperties"})";
    P0.xDescriptionOffset = 447;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PutPropertiesResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Is returned if the properties were changed successfully.)"},
{Response400, R"(The server will respond with *HTTP 400* in case of a malformed request,

)"}};
  static constexpr auto description = R"(After the properties have been changed, the current set of properties will
be returned in the HTTP response.

Note: changing the properties may invalidate all results in the cache.
The global properties for AQL query cache.
The properties need to be passed in the attribute *properties* in the body
of the HTTP request. *properties* needs to be a JSON object with the following
properties:


**A JSON object with these properties is required:**

  - **maxResultsSize**: the maximum cumulated size of query results that will be stored per database-specific cache.
  - **includeSystem**: whether or not to store results of queries that involve system collections.
  - **mode**:  the mode the AQL query cache should operate in. Possible values are *off*, *on* or *demand*.
  - **maxResults**: the maximum number of query results that will be stored per database-specific cache.
  - **maxEntrySize**: the maximum individual size of query results that will be stored per database-specific cache.
)";


  ApiResult request(std::string const &databaseName);
};
namespace GetCurrent {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/query/current";
  static constexpr auto summery = "Returns the currently running AQL queries";
  static constexpr auto operationId = "(readQuery:current";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/AQL/GetApiQueryCurrent.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"AQL"};
  
  enum GetCurrentResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Is returned when the list of queries can be retrieved successfully.)"},
{Response400, R"(The server will respond with *HTTP 400* in case of a malformed request,

)"}};
  static constexpr auto description = R"(Returns an array containing the AQL queries currently running in the selected
database. Each query is a JSON object with the following attributes:

- *id*: the query's id

- *query*: the query string (potentially truncated)

- *bindVars*: the bind parameter values used by the query

- *started*: the date and time when the query was started

- *runTime*: the query's run time up to the point the list of queries was
  queried

- *state*: the query's current execution state (as a string)

- *stream*: whether or not the query uses a streaming cursor)";


  ApiResult request(std::string const &databaseName);
};
namespace GetProperties {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/query/properties";
  static constexpr auto summery = "Returns the properties for the AQL query tracking";
  static constexpr auto operationId = "(readQueryProperties";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/AQL/GetApiQueryProperties.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"AQL"};
  
  enum GetPropertiesResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Is returned if properties were retrieved successfully.)"},
{Response400, R"(The server will respond with *HTTP 400* in case of a malformed request,

)"}};
  static constexpr auto description = R"(Returns the current query tracking configuration. The configuration is a
JSON object with the following properties:

- *enabled*: if set to *true*, then queries will be tracked. If set to
  *false*, neither queries nor slow queries will be tracked.

- *trackSlowQueries*: if set to *true*, then slow queries will be tracked
  in the list of slow queries if their runtime exceeds the value set in 
  *slowQueryThreshold*. In order for slow queries to be tracked, the *enabled*
  property must also be set to *true*.

- *trackBindVars*: if set to *true*, then bind variables used in queries will 
  be tracked.

- *maxSlowQueries*: the maximum number of slow queries to keep in the list
  of slow queries. If the list of slow queries is full, the oldest entry in
  it will be discarded when additional slow queries occur.

- *slowQueryThreshold*: the threshold value for treating a query as slow. A
  query with a runtime greater or equal to this threshold value will be
  put into the list of slow queries when slow query tracking is enabled.
  The value for *slowQueryThreshold* is specified in seconds.

- *maxQueryStringLength*: the maximum query string length to keep in the
  list of queries. Query strings can have arbitrary lengths, and this property
  can be used to save memory in case very long query strings are used. The
  value is specified in bytes.)";


  ApiResult request(std::string const &databaseName);
};
namespace PutProperties {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/query/properties";
  static constexpr auto summery = "Changes the properties for the AQL query tracking";
  static constexpr auto operationId = "(replaceProperties";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/AQL/PutApiQueryProperties.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"AQL"};
  inline Parameter P0;
  inline void initParam(){
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"$ref":"#/definitions/PutApiQueryProperties"})";
    P0.xDescriptionOffset = 54;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PutPropertiesResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Is returned if the properties were changed successfully.)"},
{Response400, R"(The server will respond with *HTTP 400* in case of a malformed request,

)"}};
  static constexpr auto description = R"(
**A JSON object with these properties is required:**

  - **maxSlowQueries**: The maximum number of slow queries to keep in the list
   of slow queries. If the list of slow queries is full, the oldest entry in
   it will be discarded when additional slow queries occur.
  - **slowQueryThreshold**: The threshold value for treating a query as slow. A
   query with a runtime greater or equal to this threshold value will be
   put into the list of slow queries when slow query tracking is enabled.
   The value for *slowQueryThreshold* is specified in seconds.
  - **enabled**: If set to *true*, then queries will be tracked. If set to
   *false*, neither queries nor slow queries will be tracked.
  - **maxQueryStringLength**: The maximum query string length to keep in the list of queries.
   Query strings can have arbitrary lengths, and this property
   can be used to save memory in case very long query strings are used. The
   value is specified in bytes.
  - **trackSlowQueries**: If set to *true*, then slow queries will be tracked
   in the list of slow queries if their runtime exceeds the value set in
   *slowQueryThreshold*. In order for slow queries to be tracked, the *enabled*
   property must also be set to *true*.
  - **trackBindVars**: If set to *true*, then the bind variables used in queries will be tracked 
   along with queries.




The properties need to be passed in the attribute *properties* in the body
of the HTTP request. *properties* needs to be a JSON object.

After the properties have been changed, the current set of properties will
be returned in the HTTP response.)";


  ApiResult request(std::string const &databaseName);
};
namespace DeleteSlow {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Delete;
  static constexpr auto url = "/_api/query/slow";
  static constexpr auto summery = "Clears the list of slow AQL queries";
  static constexpr auto operationId = "(deleteQuery";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/AQL/DeleteApiQuerySlow.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"AQL"};
  
  enum DeleteSlowResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(The server will respond with *HTTP 200* when the list of queries was
cleared successfully.)"}, 
{Response400, R"(The server will respond with *HTTP 400* in case of a malformed request.
)"}};
  static constexpr auto description = R"(Clears the list of slow AQL queries)";


  ApiResult request(std::string const &databaseName);
};
namespace GetSlow {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/query/slow";
  static constexpr auto summery = "Returns the list of slow AQL queries";
  static constexpr auto operationId = "(readQuery:Slow";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/AQL/GetApiQuerySlow.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"AQL"};
  
  enum GetSlowResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Is returned when the list of queries can be retrieved successfully.)"},
{Response400, R"(The server will respond with *HTTP 400* in case of a malformed request,

)"}};
  static constexpr auto description = R"(Returns an array containing the last AQL queries that are finished and
have exceeded the slow query threshold in the selected database.
The maximum amount of queries in the list can be controlled by setting
the query tracking property `maxSlowQueries`. The threshold for treating
a query as *slow* can be adjusted by setting the query tracking property
`slowQueryThreshold`.

Each query is a JSON object with the following attributes:

- *id*: the query's id

- *query*: the query string (potentially truncated)

- *bindVars*: the bind parameter values used by the query

- *started*: the date and time when the query was started

- *runTime*: the query's total run time 

- *state*: the query's current execution state (will always be "finished"
  for the list of slow queries)

- *stream*: whether or not the query uses a streaming cursor)";


  ApiResult request(std::string const &databaseName);
};
namespace DeleteQueryId {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Delete;
  static constexpr auto url = "/_api/query/{query-id}";
  static constexpr auto summery = "Kills a running AQL query";
  static constexpr auto operationId = "(deleteQuery";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/AQL/DeleteApiQueryKill.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"AQL"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The id of the query.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "query-id";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum DeleteQueryIdResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(The server will respond with *HTTP 200* when the query was still running when
the kill request was executed and the query's kill flag was set.)"}, 
{Response400, R"(The server will respond with *HTTP 400* in case of a malformed request.)"}, 
{Response404, R"(The server will respond with *HTTP 404* when no query with the specified
id was found.
)"}};
  static constexpr auto description = R"(Kills a running query. The query will be terminated at the next cancelation
point.)";


  ApiResult request(std::string const &databaseName);
};
}
namespace Replication {
namespace GetApplierConfig {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/replication/applier-config";
  static constexpr auto summery = "Return configuration of replication applier";
  static constexpr auto operationId = "(handleCommandApplierGetConfig";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Replication/put_api_replication_applier.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Replication"};
  
  enum GetApplierConfigResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response405 = 405,
    Response500 = 500
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned if the request was executed successfully.)"},
{Response405, R"(is returned when an invalid HTTP method is used.)"}, 
{Response500, R"(is returned if an error occurred while assembling the response.)"}};
  static constexpr auto description = R"(Returns the configuration of the replication applier.

The body of the response is a JSON object with the configuration. The
following attributes may be present in the configuration:

- *endpoint*: the logger server to connect to (e.g. "tcp://192.168.173.13:8529").

- *database*: the name of the database to connect to (e.g. "_system").

- *username*: an optional ArangoDB username to use when connecting to the endpoint.

- *password*: the password to use when connecting to the endpoint.

- *maxConnectRetries*: the maximum number of connection attempts the applier
  will make in a row. If the applier cannot establish a connection to the
  endpoint in this number of attempts, it will stop itself.

- *connectTimeout*: the timeout (in seconds) when attempting to connect to the
  endpoint. This value is used for each connection attempt.

- *requestTimeout*: the timeout (in seconds) for individual requests to the endpoint.

- *chunkSize*: the requested maximum size for log transfer packets that
  is used when the endpoint is contacted.

- *autoStart*: whether or not to auto-start the replication applier on
  (next and following) server starts

- *adaptivePolling*: whether or not the replication applier will use
  adaptive polling.

- *includeSystem*: whether or not system collection operations will be applied

- *autoResync*: whether or not the slave should perform a full automatic
  resynchronization with the master in case the master cannot serve log data
  requested by the slave, or when the replication is started and no tick
  value
  can be found.

- *autoResyncRetries*: number of resynchronization retries that will be performed
  in a row when automatic resynchronization is enabled and kicks in. Setting this
  to *0* will effectively disable *autoResync*. Setting it to some other value
  will limit the number of retries that are performed. This helps preventing endless
  retries in case resynchronizations always fail.

- *initialSyncMaxWaitTime*: the maximum wait time (in seconds) that the initial
  synchronization will wait for a response from the master when fetching initial
  collection data.
  This wait time can be used to control after what time the initial synchronization
  will give up waiting for a response and fail. This value is relevant even
  for continuous replication when *autoResync* is set to *true* because this
  may re-start the initial synchronization when the master cannot provide
  log data the slave requires.
  This value will be ignored if set to *0*.

- *connectionRetryWaitTime*: the time (in seconds) that the applier will
  intentionally idle before it retries connecting to the master in case of
  connection problems.
  This value will be ignored if set to *0*.

- *idleMinWaitTime*: the minimum wait time (in seconds) that the applier will
  intentionally idle before fetching more log data from the master in case
  the master has already sent all its log data. This wait time can be used
  to control the frequency with which the replication applier sends HTTP log
  fetch requests to the master in case there is no write activity on the master.
  This value will be ignored if set to *0*.

- *idleMaxWaitTime*: the maximum wait time (in seconds) that the applier will
  intentionally idle before fetching more log data from the master in case the
  master has already sent all its log data and there have been previous log
  fetch attempts that resulted in no more log data. This wait time can be used
  to control the maximum frequency with which the replication applier sends HTTP
  log fetch requests to the master in case there is no write activity on the
  master for longer periods. This configuration value will only be used if the
  option *adaptivePolling* is set to *true*.
  This value will be ignored if set to *0*.

- *requireFromPresent*: if set to *true*, then the replication applier will check
  at start whether the start tick from which it starts or resumes replication is
  still present on the master. If not, then there would be data loss. If
  *requireFromPresent* is *true*, the replication applier will abort with an
  appropriate error message. If set to *false*, then the replication applier will
  still start, and ignore the data loss.

- *verbose*: if set to *true*, then a log line will be emitted for all operations
  performed by the replication applier. This should be used for debugging
  replication
  problems only.

- *restrictType*: the configuration for *restrictCollections*

- *restrictCollections*: the optional array of collections to include or exclude,
  based on the setting of *restrictType*


)";


  ApiResult request(std::string const &databaseName);
};
namespace PutApplierConfig {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/replication/applier-config";
  static constexpr auto summery = "Adjust configuration of replication applier";
  static constexpr auto operationId = "(handleCommandApplierSetConfig";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Replication/put_api_replication_applier_adjust.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Replication"};
  inline Parameter P0;
  inline void initParam(){
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"$ref":"#/definitions/put_api_replication_applier_adjust"})";
    P0.xDescriptionOffset = 54;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PutApplierConfigResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400,
    Response405 = 405,
    Response500 = 500
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned if the request was executed successfully.)"},
{Response400, R"(is returned if the configuration is incomplete or malformed, or if the
replication applier is currently running.)"}, 
{Response405, R"(is returned when an invalid HTTP method is used.)"}, 
{Response500, R"(is returned if an error occurred while assembling the response.)"}};
  static constexpr auto description = R"(
**A JSON object with these properties is required:**

  - **username**: an optional ArangoDB username to use when connecting to the endpoint.
  - **includeSystem**: whether or not system collection operations will be applied
  - **endpoint**: the logger server to connect to (e.g. "tcp://192.168.173.13:8529"). The endpoint must be specified.
  - **verbose**: if set to *true*, then a log line will be emitted for all operations 
   performed by the replication applier. This should be used for debugging replication
   problems only.
  - **connectTimeout**: the timeout (in seconds) when attempting to connect to the
   endpoint. This value is used for each connection attempt.
  - **autoResync**: whether or not the slave should perform a full automatic resynchronization
   with the master in case the master cannot serve log data requested by the
   slave,
   or when the replication is started and no tick value can be found.
  - **database**: the name of the database on the endpoint. If not specified, defaults to the current local database name.
  - **idleMinWaitTime**: the minimum wait time (in seconds) that the applier will intentionally idle
   before fetching more log data from the master in case the master has
   already sent all its log data. This wait time can be used to control the
   frequency with which the replication applier sends HTTP log fetch requests
   to the master in case there is no write activity on the master.
   This value will be ignored if set to *0*.
  - **requestTimeout**: the timeout (in seconds) for individual requests to the endpoint.
  - **requireFromPresent**: if set to *true*, then the replication applier will check
   at start whether the start tick from which it starts or resumes replication is
   still present on the master. If not, then there would be data loss. If 
   *requireFromPresent* is *true*, the replication applier will abort with an
   appropriate error message. If set to *false*, then the replication applier will
   still start, and ignore the data loss.
  - **idleMaxWaitTime**: the maximum wait time (in seconds) that the applier will intentionally idle 
   before fetching more log data from the master in case the master has 
   already sent all its log data and there have been previous log fetch attempts
   that resulted in no more log data. This wait time can be used to control the
   maximum frequency with which the replication applier sends HTTP log fetch
   requests to the master in case there is no write activity on the master for
   longer periods. This configuration value will only be used if the option
   *adaptivePolling* is set to *true*.
   This value will be ignored if set to *0*.
  - **restrictCollections** (string): the array of collections to include or exclude,
   based on the setting of *restrictType*
  - **restrictType**: the configuration for *restrictCollections*; Has to be either *include* or *exclude*
  - **initialSyncMaxWaitTime**: the maximum wait time (in seconds) that the initial synchronization will
   wait for a response from the master when fetching initial collection data.
   This wait time can be used to control after what time the initial
   synchronization
   will give up waiting for a response and fail. This value is relevant even
   for continuous replication when *autoResync* is set to *true* because this
   may re-start the initial synchronization when the master cannot provide
   log data the slave requires.
   This value will be ignored if set to *0*.
  - **maxConnectRetries**: the maximum number of connection attempts the applier
   will make in a row. If the applier cannot establish a connection to the
   endpoint in this number of attempts, it will stop itself.
  - **autoStart**: whether or not to auto-start the replication applier on
   (next and following) server starts
  - **adaptivePolling**: if set to *true*, the replication applier will fall
   to sleep for an increasingly long period in case the logger server at the
   endpoint does not have any more replication events to apply. Using
   adaptive polling is thus useful to reduce the amount of work for both the
   applier and the logger server for cases when there are only infrequent
   changes. The downside is that when using adaptive polling, it might take
   longer for the replication applier to detect that there are new replication
   events on the logger server.
   Setting *adaptivePolling* to false will make the replication applier
   contact the logger server in a constant interval, regardless of whether
   the logger server provides updates frequently or seldom.
  - **password**: the password to use when connecting to the endpoint.
  - **connectionRetryWaitTime**: the time (in seconds) that the applier will intentionally idle before
   it retries connecting to the master in case of connection problems.
   This value will be ignored if set to *0*.
  - **autoResyncRetries**: number of resynchronization retries that will be performed in a row when
   automatic resynchronization is enabled and kicks in. Setting this to *0*
   will
   effectively disable *autoResync*. Setting it to some other value will limit
   the number of retries that are performed. This helps preventing endless
   retries
   in case resynchronizations always fail.
  - **chunkSize**: the requested maximum size for log transfer packets that
   is used when the endpoint is contacted.




Sets the configuration of the replication applier. The configuration can
only be changed while the applier is not running. The updated configuration
will be saved immediately but only become active with the next start of the
applier.

In case of success, the body of the response is a JSON object with the updated
configuration.


)";


  ApiResult request(std::string const &databaseName);
};
namespace PutApplierStart {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/replication/applier-start";
  static constexpr auto summery = "Start replication applier";
  static constexpr auto operationId = "(handleCommandApplierStart";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Replication/put_api_replication_applier_start.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Replication"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The remote *lastLogTick* value from which to start applying. If not specified,
the last saved tick from the previous applier run is used. If there is no
previous applier state saved, the applier will start at the beginning of the
logger server's log.)";
    P0.in = ParamIn::query;
    P0.name = "from";
    P0.required = false;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PutApplierStartResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400,
    Response405 = 405,
    Response500 = 500
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned if the request was executed successfully.)"},
{Response400, R"(is returned if the replication applier is not fully configured or the
configuration is invalid.)"}, 
{Response405, R"(is returned when an invalid HTTP method is used.)"}, 
{Response500, R"(is returned if an error occurred while assembling the response.)"}};
  static constexpr auto description = R"(Starts the replication applier. This will return immediately if the
replication applier is already running.

If the replication applier is not already running, the applier configuration
will be checked, and if it is complete, the applier will be started in a
background thread. This means that even if the applier will encounter any
errors while running, they will not be reported in the response to this
method.

To detect replication applier errors after the applier was started, use the
*/_api/replication/applier-state* API instead.


)";


  ApiResult request(std::string const &databaseName);
};
namespace GetApplierState {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/replication/applier-state";
  static constexpr auto summery = "State of the replication applier";
  static constexpr auto operationId = "(handleCommandApplierGetState";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Replication/get_api_replication_applier_state.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Replication"};
  
  enum GetApplierStateResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response405 = 405,
    Response500 = 500
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned if the request was executed successfully.)"},
{Response405, R"(is returned when an invalid HTTP method is used.)"}, 
{Response500, R"(is returned if an error occurred while assembling the response.)"}};
  static constexpr auto description = R"(Returns the state of the replication applier, regardless of whether the
applier is currently running or not.

The response is a JSON object with the following attributes:

- *state*: a JSON object with the following sub-attributes:

  - *running*: whether or not the applier is active and running

  - *lastAppliedContinuousTick*: the last tick value from the continuous
    replication log the applier has applied.

  - *lastProcessedContinuousTick*: the last tick value from the continuous
    replication log the applier has processed.

    Regularly, the last applied and last processed tick values should be
    identical. For transactional operations, the replication applier will first
    process incoming log events before applying them, so the processed tick
    value might be higher than the applied tick value. This will be the case
    until the applier encounters the *transaction commit* log event for the
    transaction.

  - *lastAvailableContinuousTick*: the last tick value the remote server can
    provide, for all databases.

  - *ticksBehind*: this attribute will be present only if the applier is currently
    running. It will provide the number of log ticks between what the applier
    has applied/seen and the last log tick value provided by the remote server.
    If this value is zero, then both servers are in sync. If this is non-zero,
    then the remote server has additional data that the applier has not yet
    fetched and processed, or the remote server may have more data that is not
    applicable to the applier.

    Client applications can use it to determine approximately how far the applier
    is behind the remote server, and can periodically check if the value is 
    increasing (applier is falling behind) or decreasing (applier is catching up).
    
    Please note that as the remote server will only keep one last log tick value 
    for all of its databases, but replication may be restricted to just certain 
    databases on the applier, this value is more meaningful when the global applier 
    is used.
    Additionally, the last log tick provided by the remote server may increase
    due to writes into system collections that are not replicated due to replication
    configuration. So the reported value may exaggerate the reality a bit for
    some scenarios. 

  - *time*: the time on the applier server.

  - *totalRequests*: the total number of requests the applier has made to the
    endpoint.

  - *totalFailedConnects*: the total number of failed connection attempts the
    applier has made.

  - *totalEvents*: the total number of log events the applier has processed.

  - *totalOperationsExcluded*: the total number of log events excluded because
    of *restrictCollections*.

  - *progress*: a JSON object with details about the replication applier progress.
    It contains the following sub-attributes if there is progress to report:

    - *message*: a textual description of the progress

    - *time*: the date and time the progress was logged

    - *failedConnects*: the current number of failed connection attempts

  - *lastError*: a JSON object with details about the last error that happened on
    the applier. It contains the following sub-attributes if there was an error:

    - *errorNum*: a numerical error code

    - *errorMessage*: a textual error description

    - *time*: the date and time the error occurred

    In case no error has occurred, *lastError* will be empty.

- *server*: a JSON object with the following sub-attributes:

  - *version*: the applier server's version

  - *serverId*: the applier server's id

- *endpoint*: the endpoint the applier is connected to (if applier is
  active) or will connect to (if applier is currently inactive)

- *database*: the name of the database the applier is connected to (if applier is
  active) or will connect to (if applier is currently inactive)

Please note that all "tick" values returned do not have a specific unit. Tick
values are only meaningful when compared to each other. Higher tick values mean
"later in time" than lower tick values.

)";


  ApiResult request(std::string const &databaseName);
};
namespace PutApplierStop {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/replication/applier-stop";
  static constexpr auto summery = "Stop replication applier";
  static constexpr auto operationId = "(handleCommandApplierStop";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Replication/put_api_replication_applier_stop.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Replication"};
  
  enum PutApplierStopResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response405 = 405,
    Response500 = 500
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned if the request was executed successfully.)"},
{Response405, R"(is returned when an invalid HTTP method is used.)"}, 
{Response500, R"(is returned if an error occurred while assembling the response.)"}};
  static constexpr auto description = R"(Stops the replication applier. This will return immediately if the
replication applier is not running.


)";


  ApiResult request(std::string const &databaseName);
};
namespace PostBatch {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Post;
  static constexpr auto url = "/_api/replication/batch";
  static constexpr auto summery = "Create new dump batch";
  static constexpr auto operationId = "(handleCommandBatch:Create";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Replication/post_batch_replication.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Replication"};
  inline Parameter P0;
  inline void initParam(){
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"$ref":"#/definitions/post_batch_replication"})";
    P0.xDescriptionOffset = 54;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PostBatchResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400,
    Response405 = 405
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned if the batch was created successfully.)"},
{Response400, R"(is returned if the ttl value is invalid or if *DBserver* attribute
is not specified or illegal on a coordinator.)"}, 
{Response405, R"(is returned when an invalid HTTP method is used.
)"}};
  static constexpr auto description = R"(
**A JSON object with these properties is required:**

  - **ttl**: the time-to-live for the new batch (in seconds)
   A JSON object with the batch configuration.




Creates a new dump batch and returns the batch's id.

The response is a JSON object with the following attributes:

- *id*: the id of the batch

**Note**: on a coordinator, this request must have the query parameter
*DBserver* which must be an ID of a DBserver.
The very same request is forwarded synchronously to that DBserver.
It is an error if this attribute is not bound in the coordinator case.)";


  ApiResult request(std::string const &databaseName);
};
namespace DeleteId {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Delete;
  static constexpr auto url = "/_api/replication/batch/{id}";
  static constexpr auto summery = "Deletes an existing dump batch";
  static constexpr auto operationId = "(handleCommandBatch:DELETE";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Replication/delete_batch_replication.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Replication"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The id of the batch.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "id";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum DeleteIdResponse : short {
    Unknown = 0,
    Response204, 
    Response400 = 400,
    Response405 = 405
  };
  inline const static std::map<int, char const * > responses{{Response204, R"(is returned if the batch was deleted successfully.)"},
{Response400, R"(is returned if the batch was not found.)"}, 
{Response405, R"(is returned when an invalid HTTP method is used.
)"}};
  static constexpr auto description = R"(Deletes the existing dump batch, allowing compaction and cleanup to resume.

**Note**: on a coordinator, this request must have the query parameter
*DBserver* which must be an ID of a DBserver.
The very same request is forwarded synchronously to that DBserver.
It is an error if this attribute is not bound in the coordinator case.)";


  ApiResult request(std::string const &databaseName);
};
namespace PutId {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/replication/batch/{id}";
  static constexpr auto summery = "Prolong existing dump batch";
  static constexpr auto operationId = "(handleCommandBatch:Prolong";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Replication/put_batch_replication.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Replication"};
  inline Parameter P0;
  inline Parameter P1;
  inline void initParam(){
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"$ref":"#/definitions/put_batch_replication"})";
    P0.xDescriptionOffset = 54;

    P1.description = R"(The id of the batch.)";
    P1.format = ParamFormat::stringFormat;
    P1.in = ParamIn::path;
    P1.name = "id";
    P1.required = true;
    P1.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1};

  enum PutIdResponse : short {
    Unknown = 0,
    Response204, 
    Response400 = 400,
    Response405 = 405
  };
  inline const static std::map<int, char const * > responses{{Response204, R"(is returned if the batch's ttl was extended successfully.)"},
{Response400, R"(is returned if the ttl value is invalid or the batch was not found.)"}, 
{Response405, R"(is returned when an invalid HTTP method is used.
)"}};
  static constexpr auto description = R"(
**A JSON object with these properties is required:**

  - **ttl**: the time-to-live for the new batch (in seconds)




Extends the ttl of an existing dump batch, using the batch's id and
the provided ttl value.

If the batch's ttl can be extended successfully, the response is empty.

**Note**: on a coordinator, this request must have the query parameter
*DBserver* which must be an ID of a DBserver.
The very same request is forwarded synchronously to that DBserver.
It is an error if this attribute is not bound in the coordinator case.)";


  ApiResult request(std::string const &databaseName);
};
namespace GetClusterInventory {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/replication/clusterInventory";
  static constexpr auto summery = "Return cluster inventory of collections and indexes";
  static constexpr auto operationId = "(handleCommandClusterInventory";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Replication/get_api_replication_cluster_inventory.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Replication"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(Include system collections in the result. The default value is *true*.)";
    P0.in = ParamIn::query;
    P0.name = "includeSystem";
    P0.required = false;
    P0.type = ParamFormat::booleanFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum GetClusterInventoryResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response405 = 405,
    Response500 = 500
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned if the request was executed successfully.)"},
{Response405, R"(is returned when an invalid HTTP method is used.)"}, 
{Response500, R"(is returned if an error occurred while assembling the response.
)"}};
  static constexpr auto description = R"(Returns the array of collections and indexes available on the cluster.

The response will be an array of JSON objects, one for each collection.
Each collection containscontains exactly two keys "parameters" and
"indexes". This
information comes from Plan/Collections/{DB-Name}/* in the agency,
just that the *indexes* attribute there is relocated to adjust it to
the data format of arangodump.)";


  ApiResult request(std::string const &databaseName);
};
namespace GetDump {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/replication/dump";
  static constexpr auto summery = "Return data of a collection";
  static constexpr auto operationId = "(handleCommandDump";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Replication/get_api_replication_dump.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Replication"};
  inline Parameter P0;
  inline Parameter P1;
  inline Parameter P2;
  inline Parameter P3;
  inline Parameter P4;
  inline Parameter P5;
  inline Parameter P6;
  inline Parameter P7;
  inline void initParam(){
    P0.description = R"(The name or id of the collection to dump.)";
    P0.in = ParamIn::query;
    P0.name = "collection";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.description = "";
    P1.in = ParamIn::query;
    P1.name = "chunkSize";
    P1.required = false;
    P1.type = ParamFormat::numberFormat;

    P2.description = R"(rocksdb only - The id of the snapshot to use)";
    P2.in = ParamIn::query;
    P2.name = "batchId";
    P2.required = true;
    P2.type = ParamFormat::numberFormat;

    P3.description = R"(mmfiles only - Lower bound tick value for results.)";
    P3.in = ParamIn::query;
    P3.name = "from";
    P3.required = false;
    P3.type = ParamFormat::numberFormat;

    P4.description = R"(mmfiles only - Upper bound tick value for results.)";
    P4.in = ParamIn::query;
    P4.name = "to";
    P4.required = false;
    P4.type = ParamFormat::numberFormat;

    P5.description = R"(mmfiles only - Include system collections in the result. The default value is *true*.)";
    P5.in = ParamIn::query;
    P5.name = "includeSystem";
    P5.required = false;
    P5.type = ParamFormat::booleanFormat;

    P6.description = R"(mmfiles only - Whether or not to include tick values in the dump. The default value is *true*.)";
    P6.in = ParamIn::query;
    P6.name = "ticks";
    P6.required = false;
    P6.type = ParamFormat::booleanFormat;

    P7.description = R"(mmfiles only - Whether or not to flush the WAL before dumping. The default value is *true*.)";
    P7.in = ParamIn::query;
    P7.name = "flush";
    P7.required = false;
    P7.type = ParamFormat::booleanFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1, P2, P3, P4, P5, P6, P7};

  enum GetDumpResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response204, 
    Response400 = 400,
    Response404 = 404,
    Response405 = 405,
    Response500 = 500
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned if the request was executed successfully and data was returned. The header
`x-arango-replication-lastincluded` is set to the tick of the last document returned.)"}, 
{Response204, R"(is returned if the request was executed successfully, but there was no content available.
The header `x-arango-replication-lastincluded` is `0` in this case.)"}, 
{Response400, R"(is returned if either the *from* or *to* values are invalid.)"}, 
{Response404, R"(is returned when the collection could not be found.)"}, 
{Response405, R"(is returned when an invalid HTTP method is used.)"}, 
{Response500, R"(is returned if an error occurred while assembling the response.)"}};
  static constexpr auto description = R"(Returns the data from the collection for the requested range.

When the *from* query parameter is not used, collection events are returned from
the beginning. When the *from* parameter is used, the result will only contain
collection entries which have higher tick values than the specified *from* value
(note: the log entry with a tick value equal to *from* will be excluded).

The *to* query parameter can be used to optionally restrict the upper bound of
the result to a certain tick value. If used, the result will only contain
collection entries with tick values up to (including) *to*.

The *chunkSize* query parameter can be used to control the size of the result.
It must be specified in bytes. The *chunkSize* value will only be honored
approximately. Otherwise a too low *chunkSize* value could cause the server
to not be able to put just one entry into the result and return it.
Therefore, the *chunkSize* value will only be consulted after an entry has
been written into the result. If the result size is then bigger than
*chunkSize*, the server will respond with as many entries as there are
in the response already. If the result size is still smaller than *chunkSize*,
the server will try to return more data if there's more data left to return.

If *chunkSize* is not specified, some server-side default value will be used.

The *Content-Type* of the result is *application/x-arango-dump*. This is an
easy-to-process format, with all entries going onto separate lines in the
response body.

Each line itself is a JSON object, with at least the following attributes:

- *tick*: the operation's tick attribute

- *key*: the key of the document/edge or the key used in the deletion operation

- *rev*: the revision id of the document/edge or the deletion operation

- *data*: the actual document/edge data for types 2300 and 2301. The full
  document/edge data will be returned even for updates.

- *type*: the type of entry. Possible values for *type* are:

  - 2300: document insertion/update

  - 2301: edge insertion/update

  - 2302: document/edge deletion

**Note**: there will be no distinction between inserts and updates when calling this method.


)";


  ApiResult request(std::string const &databaseName);
};
namespace GetInventory {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/replication/inventory";
  static constexpr auto summery = "Return inventory of collections and indexes";
  static constexpr auto operationId = "(handleCommandInventory";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Replication/put_api_replication_inventory.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Replication"};
  inline Parameter P0;
  inline Parameter P1;
  inline Parameter P2;
  inline void initParam(){
    P0.description = R"(Include system collections in the result. The default value is *true*.)";
    P0.in = ParamIn::query;
    P0.name = "includeSystem";
    P0.required = false;
    P0.type = ParamFormat::booleanFormat;

    P1.description = R"(Include alll databases in the response. Only works on `_system` The default value is *false*.)";
    P1.in = ParamIn::query;
    P1.name = "global";
    P1.required = false;
    P1.type = ParamFormat::booleanFormat;

    P2.description = R"(The RocksDB engine requires a valid batchId for this API call)";
    P2.in = ParamIn::query;
    P2.name = "batchId";
    P2.required = true;
    P2.type = ParamFormat::numberFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1, P2};

  enum GetInventoryResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response405 = 405,
    Response500 = 500
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned if the request was executed successfully.)"},
{Response405, R"(is returned when an invalid HTTP method is used.)"}, 
{Response500, R"(is returned if an error occurred while assembling the response.)"}};
  static constexpr auto description = R"(Returns the array of collections and indexes available on the server. This
array can be used by replication clients to initiate an initial sync with the
server.

The response will contain a JSON object with the *collection* and *state* and
*tick* attributes.

*collections* is an array of collections with the following sub-attributes:

- *parameters*: the collection properties

- *indexes*: an array of the indexes of a the collection. Primary indexes and edge indexes
   are not included in this array.

The *state* attribute contains the current state of the replication logger. It
contains the following sub-attributes:

- *running*: whether or not the replication logger is currently active. Note:
  since ArangoDB 2.2, the value will always be *true*

- *lastLogTick*: the value of the last tick the replication logger has written

- *time*: the current time on the server

Replication clients should note the *lastLogTick* value returned. They can then
fetch collections' data using the dump method up to the value of lastLogTick, and
query the continuous replication log for log events after this tick value.

To create a full copy of the collections on the server, a replication client
can execute these steps:

- call the */inventory* API method. This returns the *lastLogTick* value and the
  array of collections and indexes from the server.

- for each collection returned by */inventory*, create the collection locally and
  call */dump* to stream the collection data to the client, up to the value of
  *lastLogTick*.
  After that, the client can create the indexes on the collections as they were
  reported by */inventory*.

If the clients wants to continuously stream replication log events from the logger
server, the following additional steps need to be carried out:

- the client should call */logger-follow* initially to fetch the first batch of
  replication events that were logged after the client's call to */inventory*.

  The call to */logger-follow* should use a *from* parameter with the value of the
  *lastLogTick* as reported by */inventory*. The call to */logger-follow* will return the
  *x-arango-replication-lastincluded* which will contain the last tick value included
  in the response.

- the client can then continuously call */logger-follow* to incrementally fetch new
  replication events that occurred after the last transfer.

  Calls should use a *from* parameter with the value of the *x-arango-replication-lastincluded*
  header of the previous response. If there are no more replication events, the
  response will be empty and clients can go to sleep for a while and try again
  later.

**Note**: on a coordinator, this request must have the query parameter
*DBserver* which must be an ID of a DBserver.
The very same request is forwarded synchronously to that DBserver.
It is an error if this attribute is not bound in the coordinator case.

**Note:**: Using the `global` parameter the top-level object contains a key `databases`
under which each key represents a datbase name, and the value conforms to the above describtion.


)";


  ApiResult request(std::string const &databaseName);
};
namespace GetLoggerFirstTick {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/replication/logger-first-tick";
  static constexpr auto summery = "Returns the first available tick value";
  static constexpr auto operationId = "(handleCommandLoggerFirstTick";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Replication/get_api_replication_logger_first_tick.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Replication"};
  
  enum GetLoggerFirstTickResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response405 = 405,
    Response500 = 500,
    Response501 = 501
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned if the request was executed successfully.)"},
{Response405, R"(is returned when an invalid HTTP method is used.)"}, 
{Response500, R"(is returned if an error occurred while assembling the response.)"}, 
{Response501, R"(is returned when this operation is called on a coordinator in a cluster.)"}};
  static constexpr auto description = R"(Returns the first available tick value that can be served from the server's
replication log. This method can be called by replication clients after to
determine if certain data (identified by a tick value) is still available
for replication.

The result is a JSON object containing the attribute *firstTick*. This
attribute contains the minimum tick value available in the server's
replication
log.

**Note**: this method is not supported on a coordinator in a cluster.


)";


  ApiResult request(std::string const &databaseName);
};
namespace GetLoggerFollow {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/replication/logger-follow";
  static constexpr auto summery = "Returns log entries";
  static constexpr auto operationId = "(handleCommandLoggerFollow";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Replication/get_api_replication_logger_follow.md";
  static constexpr auto x_hints = R"({% hint 'warning' %}
This route should no longer be used.
It is considered as deprecated from version 3.4.0 on.
{% endhint %}

)";
  inline const static std::vector<char const * > tags{"Replication"};
  inline Parameter P0;
  inline Parameter P1;
  inline Parameter P2;
  inline Parameter P3;
  inline void initParam(){
    P0.description = R"(Exclusive lower bound tick value for results.)";
    P0.in = ParamIn::query;
    P0.name = "from";
    P0.required = false;
    P0.type = ParamFormat::numberFormat;

    P1.description = R"(Inclusive upper bound tick value for results.)";
    P1.in = ParamIn::query;
    P1.name = "to";
    P1.required = false;
    P1.type = ParamFormat::numberFormat;

    P2.description = R"(Approximate maximum size of the returned result.)";
    P2.in = ParamIn::query;
    P2.name = "chunkSize";
    P2.required = false;
    P2.type = ParamFormat::numberFormat;

    P3.description = R"(Include system collections in the result. The default value is *true*.)";
    P3.in = ParamIn::query;
    P3.name = "includeSystem";
    P3.required = false;
    P3.type = ParamFormat::booleanFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1, P2, P3};

  enum GetLoggerFollowResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response204, 
    Response400 = 400,
    Response405 = 405,
    Response500 = 500,
    Response501 = 501
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned if the request was executed successfully, and there are log
events available for the requested range. The response body will not be empty
in this case.)"}, 
{Response204, R"(is returned if the request was executed successfully, but there are no log
events available for the requested range. The response body will be empty
in this case.)"}, 
{Response400, R"(is returned if either the *from* or *to* values are invalid.)"}, 
{Response405, R"(is returned when an invalid HTTP method is used.)"}, 
{Response500, R"(is returned if an error occurred while assembling the response.)"}, 
{Response501, R"(is returned when this operation is called on a coordinator in a cluster.)"}};
  static constexpr auto description = R"(Returns data from the server's replication log. This method can be called
by replication clients after an initial synchronization of data. The method
will return all "recent" log entries from the logger server, and the clients
can replay and apply these entries locally so they get to the same data
state as the logger server.

Clients can call this method repeatedly to incrementally fetch all changes
from the logger server. In this case, they should provide the *from* value so
they will only get returned the log events since their last fetch.

When the *from* query parameter is not used, the logger server will return log
entries starting at the beginning of its replication log. When the *from*
parameter is used, the logger server will only return log entries which have
higher tick values than the specified *from* value (note: the log entry with a
tick value equal to *from* will be excluded). Use the *from* value when
incrementally fetching log data.

The *to* query parameter can be used to optionally restrict the upper bound of
the result to a certain tick value. If used, the result will contain only log events
with tick values up to (including) *to*. In incremental fetching, there is no
need to use the *to* parameter. It only makes sense in special situations,
when only parts of the change log are required.

The *chunkSize* query parameter can be used to control the size of the result.
It must be specified in bytes. The *chunkSize* value will only be honored
approximately. Otherwise a too low *chunkSize* value could cause the server
to not be able to put just one log entry into the result and return it.
Therefore, the *chunkSize* value will only be consulted after a log entry has
been written into the result. If the result size is then bigger than
*chunkSize*, the server will respond with as many log entries as there are
in the response already. If the result size is still smaller than *chunkSize*,
the server will try to return more data if there's more data left to return.

If *chunkSize* is not specified, some server-side default value will be used.

The *Content-Type* of the result is *application/x-arango-dump*. This is an
easy-to-process format, with all log events going onto separate lines in the
response body. Each log event itself is a JSON object, with at least the
following attributes:

- *tick*: the log event tick value

- *type*: the log event type

Individual log events will also have additional attributes, depending on the
event type. A few common attributes which are used for multiple events types
are:

- *cid*: id of the collection the event was for

- *tid*: id of the transaction the event was contained in

- *key*: document key

- *rev*: document revision id

- *data*: the original document data

A more detailed description of the individual replication event types and their
data structures can be found in [Operation Types](../Replications/WALAccess.html#operation-types).

The response will also contain the following HTTP headers:

- *x-arango-replication-active*: whether or not the logger is active. Clients
  can use this flag as an indication for their polling frequency. If the
  logger is not active and there are no more replication events available, it
  might be sensible for a client to abort, or to go to sleep for a long time
  and try again later to check whether the logger has been activated.

- *x-arango-replication-lastincluded*: the tick value of the last included
  value in the result. In incremental log fetching, this value can be used
  as the *from* value for the following request. **Note** that if the result is
  empty, the value will be *0*. This value should not be used as *from* value
  by clients in the next request (otherwise the server would return the log
  events from the start of the log again).

- *x-arango-replication-lasttick*: the last tick value the logger server has
  logged (not necessarily included in the result). By comparing the the last
  tick and last included tick values, clients have an approximate indication of
  how many events there are still left to fetch.

- *x-arango-replication-checkmore*: whether or not there already exists more
  log data which the client could fetch immediately. If there is more log data
  available, the client could call *logger-follow* again with an adjusted *from*
  value to fetch remaining log entries until there are no more.

  If there isn't any more log data to fetch, the client might decide to go
  to sleep for a while before calling the logger again.

**Note**: this method is not supported on a coordinator in a cluster.


<!-- Hints Start -->

**Warning:**  
This route should no longer be used.
It is considered as deprecated from version 3.4.0 on.



<!-- Hints End -->
)";


  ApiResult request(std::string const &databaseName);
};
namespace GetLoggerState {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/replication/logger-state";
  static constexpr auto summery = "Return replication logger state";
  static constexpr auto operationId = "(handleCommandLoggerState";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Replication/get_api_replication_logger_return_state.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Replication"};
  
  enum GetLoggerStateResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response405 = 405,
    Response500 = 500
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned if the logger state could be determined successfully.)"},
{Response405, R"(is returned when an invalid HTTP method is used.)"}, 
{Response500, R"(is returned if the logger state could not be determined.)"}};
  static constexpr auto description = R"(Returns the current state of the server's replication logger. The state will
include information about whether the logger is running and about the last
logged tick value. This tick value is important for incremental fetching of
data.

The body of the response contains a JSON object with the following
attributes:

- *state*: the current logger state as a JSON object with the following
  sub-attributes:

  - *running*: whether or not the logger is running

  - *lastLogTick*: the tick value of the latest tick the logger has logged.
    This value can be used for incremental fetching of log data.

  - *totalEvents*: total number of events logged since the server was started.
    The value is not reset between multiple stops and re-starts of the logger.

  - *time*: the current date and time on the logger server

- *server*: a JSON object with the following sub-attributes:

  - *version*: the logger server's version

  - *serverId*: the logger server's id

- *clients*: returns the last fetch status by replication clients connected to
  the logger. Each client is returned as a JSON object with the following attributes:

  - *serverId*: server id of client

  - *lastServedTick*: last tick value served to this client via the *logger-follow* API

  - *time*: date and time when this client last called the *logger-follow* API


)";


  ApiResult request(std::string const &databaseName);
};
namespace GetLoggerTickRanges {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/replication/logger-tick-ranges";
  static constexpr auto summery = "Return the tick ranges available in the WAL logfiles";
  static constexpr auto operationId = "(handleCommandLoggerTickRanges";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Replication/get_api_replication_logger_tick_ranges.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Replication"};
  
  enum GetLoggerTickRangesResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response405 = 405,
    Response500 = 500,
    Response501 = 501
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned if the tick ranges could be determined successfully.)"},
{Response405, R"(is returned when an invalid HTTP method is used.)"}, 
{Response500, R"(is returned if the logger state could not be determined.)"}, 
{Response501, R"(is returned when this operation is called on a coordinator in a cluster.)"}};
  static constexpr auto description = R"(Returns the currently available ranges of tick values for all currently
available WAL logfiles. The tick values can be used to determine if certain
data (identified by tick value) are still available for replication.

The body of the response contains a JSON array. Each array member is an
object
that describes a single logfile. Each object has the following attributes:

* *datafile*: name of the logfile

* *status*: status of the datafile, in textual form (e.g. "sealed", "open")

* *tickMin*: minimum tick value contained in logfile

* *tickMax*: maximum tick value contained in logfile


)";


  ApiResult request(std::string const &databaseName);
};
namespace PutMakeSlave {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/replication/make-slave";
  static constexpr auto summery = "Turn the server into a slave of another";
  static constexpr auto operationId = "(handleCommandMakeSlave";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Replication/put_api_replication_makeSlave.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Replication"};
  inline Parameter P0;
  inline void initParam(){
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"$ref":"#/definitions/put_api_replication_makeSlave"})";
    P0.xDescriptionOffset = 54;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PutMakeSlaveResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400,
    Response405 = 405,
    Response500 = 500,
    Response501 = 501
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned if the request was executed successfully.)"},
{Response400, R"(is returned if the configuration is incomplete or malformed.)"}, 
{Response405, R"(is returned when an invalid HTTP method is used.)"}, 
{Response500, R"(is returned if an error occurred during sychronization or when starting the
continuous replication.)"}, 
{Response501, R"(is returned when this operation is called on a coordinator in a cluster.
)"}};
  static constexpr auto description = R"(
**A JSON object with these properties is required:**

  - **username**: an optional ArangoDB username to use when connecting to the master.
  - **includeSystem**: whether or not system collection operations will be applied
  - **endpoint**: the master endpoint to connect to (e.g. "tcp://192.168.173.13:8529").
  - **verbose**: if set to *true*, then a log line will be emitted for all operations
   performed by the replication applier. This should be used for debugging
   replication
   problems only.
  - **connectTimeout**: the timeout (in seconds) when attempting to connect to the
   endpoint. This value is used for each connection attempt.
  - **autoResync**: whether or not the slave should perform an automatic resynchronization with
   the master in case the master cannot serve log data requested by the slave,
   or when the replication is started and no tick value can be found.
  - **database**: the database name on the master (if not specified, defaults to the
   name of the local current database).
  - **idleMinWaitTime**: the minimum wait time (in seconds) that the applier will intentionally idle
   before fetching more log data from the master in case the master has
   already sent all its log data. This wait time can be used to control the
   frequency with which the replication applier sends HTTP log fetch requests
   to the master in case there is no write activity on the master.
   This value will be ignored if set to *0*.
  - **requestTimeout**: the timeout (in seconds) for individual requests to the endpoint.
  - **restrictType**: an optional string value for collection filtering. When
   specified, the allowed values are *include* or *exclude*.
  - **idleMaxWaitTime**: the maximum wait time (in seconds) that the applier will intentionally idle
   before fetching more log data from the master in case the master has
   already sent all its log data and there have been previous log fetch attempts
   that resulted in no more log data. This wait time can be used to control the
   maximum frequency with which the replication applier sends HTTP log fetch
   requests to the master in case there is no write activity on the master for
   longer periods. This configuration value will only be used if the option
   *adaptivePolling* is set to *true*.
   This value will be ignored if set to *0*.
  - **initialSyncMaxWaitTime**: the maximum wait time (in seconds) that the initial synchronization will
   wait for a response from the master when fetching initial collection data.
   This wait time can be used to control after what time the initial synchronization
   will give up waiting for a response and fail. This value is relevant even
   for continuous replication when *autoResync* is set to *true* because this
   may re-start the initial synchronization when the master cannot provide
   log data the slave requires.
   This value will be ignored if set to *0*.
  - **restrictCollections** (string): an optional array of collections for use with *restrictType*.
   If *restrictType* is *include*, only the specified collections
   will be sychronised. If *restrictType* is *exclude*, all but the specified
   collections will be synchronized.
  - **requireFromPresent**: if set to *true*, then the replication applier will check
   at start of its continuous replication if the start tick from the dump phase
   is still present on the master. If not, then there would be data loss. If
   *requireFromPresent* is *true*, the replication applier will abort with an
   appropriate error message. If set to *false*, then the replication applier will
   still start, and ignore the data loss.
  - **adaptivePolling**: whether or not the replication applier will use adaptive polling.
  - **maxConnectRetries**: the maximum number of connection attempts the applier
   will make in a row. If the applier cannot establish a connection to the
   endpoint in this number of attempts, it will stop itself.
  - **password**: the password to use when connecting to the master.
  - **connectionRetryWaitTime**: the time (in seconds) that the applier will intentionally idle before
   it retries connecting to the master in case of connection problems.
   This value will be ignored if set to *0*.
  - **autoResyncRetries**: number of resynchronization retries that will be performed in a row when
   automatic resynchronization is enabled and kicks in. Setting this to *0* will
   effectively disable *autoResync*. Setting it to some other value will limit
   the number of retries that are performed. This helps preventing endless retries
   in case resynchronizations always fail.
  - **chunkSize**: the requested maximum size for log transfer packets that
   is used when the endpoint is contacted.




Starts a full data synchronization from a remote endpoint into the local ArangoDB
database and afterwards starts the continuous replication.
The operation works on a per-database level.

All local database data will be removed prior to the synchronization.

In case of success, the body of the response is a JSON object with the following
attributes:

- *state*: a JSON object with the following sub-attributes:

  - *running*: whether or not the applier is active and running

  - *lastAppliedContinuousTick*: the last tick value from the continuous
    replication log the applier has applied.

  - *lastProcessedContinuousTick*: the last tick value from the continuous
    replication log the applier has processed.

    Regularly, the last applied and last processed tick values should be
    identical. For transactional operations, the replication applier will first
    process incoming log events before applying them, so the processed tick
    value might be higher than the applied tick value. This will be the case
    until the applier encounters the *transaction commit* log event for the
    transaction.

  - *lastAvailableContinuousTick*: the last tick value the remote server can
    provide.

  - *ticksBehind*: this attribute will be present only if the applier is currently
    running. It will provide the number of log ticks between what the applier
    has applied/seen and the last log tick value provided by the remote server.
    If this value is zero, then both servers are in sync. If this is non-zero,
    then the remote server has additional data that the applier has not yet
    fetched and processed, or the remote server may have more data that is not
    applicable to the applier.
    
    Client applications can use it to determine approximately how far the applier
    is behind the remote server, and can periodically check if the value is 
    increasing (applier is falling behind) or decreasing (applier is catching up).
    
    Please note that as the remote server will only keep one last log tick value 
    for all of its databases, but replication may be restricted to just certain 
    databases on the applier, this value is more meaningful when the global applier 
    is used.
    Additionally, the last log tick provided by the remote server may increase
    due to writes into system collections that are not replicated due to replication
    configuration. So the reported value may exaggerate the reality a bit for
    some scenarios. 

  - *time*: the time on the applier server.

  - *totalRequests*: the total number of requests the applier has made to the
    endpoint.

  - *totalFailedConnects*: the total number of failed connection attempts the
    applier has made.

  - *totalEvents*: the total number of log events the applier has processed.

  - *totalOperationsExcluded*: the total number of log events excluded because
    of *restrictCollections*.

  - *progress*: a JSON object with details about the replication applier progress.
    It contains the following sub-attributes if there is progress to report:

    - *message*: a textual description of the progress

    - *time*: the date and time the progress was logged

    - *failedConnects*: the current number of failed connection attempts

  - *lastError*: a JSON object with details about the last error that happened on
    the applier. It contains the following sub-attributes if there was an error:

    - *errorNum*: a numerical error code

    - *errorMessage*: a textual error description

    - *time*: the date and time the error occurred

    In case no error has occurred, *lastError* will be empty.

- *server*: a JSON object with the following sub-attributes:

  - *version*: the applier server's version

  - *serverId*: the applier server's id

- *endpoint*: the endpoint the applier is connected to (if applier is
  active) or will connect to (if applier is currently inactive)

- *database*: the name of the database the applier is connected to (if applier is
  active) or will connect to (if applier is currently inactive)

Please note that all "tick" values returned do not have a specific unit. Tick
values are only meaningful when compared to each other. Higher tick values mean
"later in time" than lower tick values.

WARNING: calling this method will sychronize data from the collections found
on the remote master to the local ArangoDB database. All data in the local
collections will be purged and replaced with data from the master.

Use with caution!

Please also keep in mind that this command may take a long time to complete
and return. This is because it will first do a full data synchronization with
the master, which will take time roughly proportional to the amount of data.

**Note**: this method is not supported on a coordinator in a cluster.)";


  ApiResult request(std::string const &databaseName);
};
namespace GetServerId {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/replication/server-id";
  static constexpr auto summery = "Return server id";
  static constexpr auto operationId = "(handleCommandServerId";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Replication/put_api_replication_serverID.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Replication"};
  
  enum GetServerIdResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response405 = 405,
    Response500 = 500
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned if the request was executed successfully.)"},
{Response405, R"(is returned when an invalid HTTP method is used.)"}, 
{Response500, R"(is returned if an error occurred while assembling the response.)"}};
  static constexpr auto description = R"(Returns the servers id. The id is also returned by other replication API
methods, and this method is an easy means of determining a server's id.

The body of the response is a JSON object with the attribute *serverId*. The
server id is returned as a string.


)";


  ApiResult request(std::string const &databaseName);
};
namespace PutSync {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/replication/sync";
  static constexpr auto summery = "Synchronize data from a remote endpoint";
  static constexpr auto operationId = "(handleCommandSync";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Replication/put_api_replication_synchronize.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Replication"};
  inline Parameter P0;
  inline void initParam(){
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"$ref":"#/definitions/put_api_replication_synchronize"})";
    P0.xDescriptionOffset = 54;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PutSyncResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400,
    Response405 = 405,
    Response500 = 500,
    Response501 = 501
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned if the request was executed successfully.)"},
{Response400, R"(is returned if the configuration is incomplete or malformed.)"}, 
{Response405, R"(is returned when an invalid HTTP method is used.)"}, 
{Response500, R"(is returned if an error occurred during sychronization.)"}, 
{Response501, R"(is returned when this operation is called on a coordinator in a cluster.
)"}};
  static constexpr auto description = R"(
**A JSON object with these properties is required:**

  - **username**: an optional ArangoDB username to use when connecting to the endpoint.
  - **includeSystem**: whether or not system collection operations will be applied
  - **endpoint**: the master endpoint to connect to (e.g. "tcp://192.168.173.13:8529").
  - **initialSyncMaxWaitTime**: the maximum wait time (in seconds) that the initial synchronization will
   wait for a response from the master when fetching initial collection data.
   This wait time can be used to control after what time the initial synchronization
   will give up waiting for a response and fail.
   This value will be ignored if set to *0*.
  - **database**: the database name on the master (if not specified, defaults to the
   name of the local current database).
  - **restrictType**: an optional string value for collection filtering. When
   specified, the allowed values are *include* or *exclude*.
  - **incremental**: if set to *true*, then an incremental synchronization method will be used
   for synchronizing data in collections. This method is useful when
   collections already exist locally, and only the remaining differences need
   to be transferred from the remote endpoint. In this case, the incremental
   synchronization can be faster than a full synchronization.
   The default value is *false*, meaning that the complete data from the remote
   collection will be transferred.
  - **restrictCollections** (string): an optional array of collections for use with
   *restrictType*. If *restrictType* is *include*, only the specified collections
   will be sychronised. If *restrictType* is *exclude*, all but the specified
   collections will be synchronized.
  - **password**: the password to use when connecting to the endpoint.




Starts a full data synchronization from a remote endpoint into the local
ArangoDB database.

The *sync* method can be used by replication clients to connect an ArangoDB database
to a remote endpoint, fetch the remote list of collections and indexes, and collection
data. It will thus create a local backup of the state of data at the remote ArangoDB
database. *sync* works on a per-database level.

*sync* will first fetch the list of collections and indexes from the remote endpoint.
It does so by calling the *inventory* API of the remote database. It will then purge
data in the local ArangoDB database, and after start will transfer collection data
from the remote database to the local ArangoDB database. It will extract data from the
remote database by calling the remote database's *dump* API until all data are fetched.

In case of success, the body of the response is a JSON object with the following
attributes:

- *collections*: an array of collections that were transferred from the endpoint

- *lastLogTick*: the last log tick on the endpoint at the time the transfer
  was started. Use this value as the *from* value when starting the continuous
  synchronization later.

WARNING: calling this method will sychronize data from the collections found
on the remote endpoint to the local ArangoDB database. All data in the local
collections will be purged and replaced with data from the endpoint.

Use with caution!

**Note**: this method is not supported on a coordinator in a cluster.)";


  ApiResult request(std::string const &databaseName);
};
}
namespace SimpleQuery {
namespace PutAll {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/simple/all";
  static constexpr auto summery = "Return all documents";
  static constexpr auto operationId = "(ReturnAllDocuments";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Simple Queries/put_api_simple_all.md";
  static constexpr auto x_hints = R"({% hint 'warning' %}
This route should no longer be used.
All endpoints for Simple Queries are deprecated from version 3.4.0 on.
They are superseded by AQL queries.
{% endhint %}

)";
  inline const static std::vector<char const * > tags{"Simple Queries"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(Contains the query.)";
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"additionalProperties":{},"type":"object"})";
    P0.xDescriptionOffset = 0;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PutAllResponse : short {
    Unknown = 0,
    Response201 = 201,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response201, R"(is returned if the query was executed successfully.)"},
{Response400, R"(is returned if the body does not contain a valid JSON representation of a
query. The response body contains an error document in this case.)"}, 
{Response404, R"(is returned if the collection specified by *collection* is unknown.  The
response body contains an error document in this case.)"}};
  static constexpr auto description = R"(Returns all documents of a collections. Equivalent to the AQL query
`FOR doc IN collection RETURN doc`. The call expects a JSON object
as body with the following attributes:

- *collection*: The name of the collection to query.

- *skip*: The number of documents to skip in the query (optional).

- *limit*: The maximal amount of documents to return. The *skip*
  is applied before the *limit* restriction (optional).

- *batchSize*: The number of documents to return in one go. (optional)

- *ttl*: The time-to-live for the cursor (in seconds, optional). 

- *stream*: Create this cursor as a stream query (optional). 


Returns a cursor containing the result, see [HTTP Cursor](../AqlQueryCursor/README.md) for details.


<!-- Hints Start -->

**Warning:**  
This route should no longer be used.
All endpoints for Simple Queries are deprecated from version 3.4.0 on.
They are superseded by AQL queries.



<!-- Hints End -->
)";


  ApiResult request(std::string const &databaseName);
};
namespace PutAllKeys {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/simple/all-keys";
  static constexpr auto summery = "Read all documents";
  static constexpr auto operationId = "(allDocumentKeys";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Documents/put_read_all_documents.md";
  static constexpr auto x_hints = R"({% hint 'warning' %}
This route should no longer be used.
All endpoints for Simple Queries are deprecated from version 3.4.0 on.
They are superseded by AQL queries.
{% endhint %}

)";
  inline const static std::vector<char const * > tags{"Documents"};
  inline Parameter P0;
  inline Parameter P1;
  inline void initParam(){
    P0.description = R"(The name of the collection.
**This parameter is only for an easier migration path from old versions.**
In ArangoDB versions < 3.0, the URL path was `/_api/document` and
this was passed in via the query parameter "collection".
This combination was removed. The collection name can be passed to
`/_api/simple/all-keys` as body parameter (preferred) or as query parameter.)";
    P0.in = ParamIn::query;
    P0.name = "collection";
    P0.required = false;
    P0.type = ParamFormat::stringFormat;

    P1.in = ParamIn::body;
    P1.name = "Json Request Body";
    P1.required = true;
    P1.schema = R"({"$ref":"#/definitions/put_read_all_documents"})";
    P1.xDescriptionOffset = 54;
  }
  inline std::vector<Parameter> parameters{P0, P1};

  enum PutAllKeysResponse : short {
    Unknown = 0,
    Response201 = 201,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response201, R"(All went well.)"},
{Response404, R"(The collection does not exist.)"}};
  static constexpr auto description = R"(
**A JSON object with these properties is required:**

  - **type**: The type of the result. The following values are allowed:
     - *id*: returns an array of document ids (*_id* attributes)
     - *key*: returns an array of document keys (*_key* attributes)
     - *path*: returns an array of document URI paths. This is the default.
  - **collection**: The collection that should be queried




Returns an array of all keys, ids, or URI paths for all documents in the
collection identified by *collection*. The type of the result array is
determined by the *type* attribute.

Note that the results have no defined order and thus the order should
not be relied on.

Note: the *all-keys* simple query is **deprecated** as of ArangoDB 3.4.0.
This API may get removed in future versions of ArangoDB. You can use the
`/_api/cursor` endpoint instead with one of the below AQL queries depending
on the desired result:

- `FOR doc IN @@collection RETURN doc._id` to mimic *type: id*
- `FOR doc IN @@collection RETURN doc._key` to mimic *type: key*
- `FOR doc IN @@collection RETURN CONCAT("/_db/", CURRENT_DATABASE(), "/_api/document/", doc._id)`
  to mimic *type: path*


<!-- Hints Start -->

**Warning:**  
This route should no longer be used.
All endpoints for Simple Queries are deprecated from version 3.4.0 on.
They are superseded by AQL queries.



<!-- Hints End -->
)";


  ApiResult request(std::string const &databaseName);
};
namespace PutAny {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/simple/any";
  static constexpr auto summery = "Return a random document";
  static constexpr auto operationId = "(ReturnARandomDocument";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Simple Queries/put_api_simple_any.md";
  static constexpr auto x_hints = R"({% hint 'warning' %}
This route should no longer be used.
All endpoints for Simple Queries are deprecated from version 3.4.0 on.
They are superseded by AQL queries.
{% endhint %}

)";
  inline const static std::vector<char const * > tags{"Simple Queries"};
  inline Parameter P0;
  inline void initParam(){
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"$ref":"#/definitions/put_api_simple_any"})";
    P0.xDescriptionOffset = 172;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PutAnyResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned if the query was executed successfully.)"},
{Response400, R"(is returned if the body does not contain a valid JSON representation of a
query. The response body contains an error document in this case.)"}, 
{Response404, R"(is returned if the collection specified by *collection* is unknown.  The
response body contains an error document in this case.)"}};
  static constexpr auto description = R"(Returns a random document from a collection. The call expects a JSON object
as body with the following attributes:


**A JSON object with these properties is required:**

  - **collection**: The identifier or name of the collection to query.
   Returns a JSON object with the document stored in the attribute
   *document* if the collection contains at least one document. If
   the collection is empty, the *document* attribute contains null.



<!-- Hints Start -->

**Warning:**  
This route should no longer be used.
All endpoints for Simple Queries are deprecated from version 3.4.0 on.
They are superseded by AQL queries.



<!-- Hints End -->
)";


  ApiResult request(std::string const &databaseName);
};
namespace PutByExample {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/simple/by-example";
  static constexpr auto summery = "Simple query by-example";
  static constexpr auto operationId = "(SimpleQueryBy-example";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Simple Queries/put_api_simple_by_example.md";
  static constexpr auto x_hints = R"({% hint 'warning' %}
This route should no longer be used.
All endpoints for Simple Queries are deprecated from version 3.4.0 on.
They are superseded by AQL queries.
{% endhint %}

{% hint 'warning' %}
Till ArangoDB versions 3.2.13 and 3.3.7 this API is quite expensive.
A more lightweight alternative is to use the [HTTP Cursor API](../AqlQueryCursor/README.md).
Starting from versions 3.2.14 and 3.3.8 this performance impact is not
an issue anymore, as the internal implementation of the API has changed.
{% endhint %}

)";
  inline const static std::vector<char const * > tags{"Simple Queries"};
  inline Parameter P0;
  inline void initParam(){
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"$ref":"#/definitions/put_api_simple_by_example"})";
    P0.xDescriptionOffset = 54;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PutByExampleResponse : short {
    Unknown = 0,
    Response201 = 201,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response201, R"(is returned if the query was executed successfully.)"},
{Response400, R"(is returned if the body does not contain a valid JSON representation of a
query. The response body contains an error document in this case.)"}, 
{Response404, R"(is returned if the collection specified by *collection* is unknown.  The
response body contains an error document in this case.)"}};
  static constexpr auto description = R"(
**A JSON object with these properties is required:**

  - **skip**: The number of documents to skip in the query (optional).
  - **batchSize**: maximum number of result documents to be transferred from
   the server to the client in one roundtrip. If this attribute is
   not set, a server-controlled default value will be used. A *batchSize* value of
   *0* is disallowed.
  - **limit**: The maximal amount of documents to return. The *skip*
   is applied before the *limit* restriction. (optional)
  - **example**: The example document.
  - **collection**: The name of the collection to query.





This will find all documents matching a given example.

Returns a cursor containing the result, see [HTTP Cursor](../AqlQueryCursor/README.md) for details.


<!-- Hints Start -->

**Warning:**  
This route should no longer be used.
All endpoints for Simple Queries are deprecated from version 3.4.0 on.
They are superseded by AQL queries.




**Warning:**  
Till ArangoDB versions 3.2.13 and 3.3.7 this API is quite expensive.
A more lightweight alternative is to use the [HTTP Cursor API](../AqlQueryCursor/README.md).
Starting from versions 3.2.14 and 3.3.8 this performance impact is not
an issue anymore, as the internal implementation of the API has changed.



<!-- Hints End -->
)";


  ApiResult request(std::string const &databaseName);
};
namespace PutFirstExample {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/simple/first-example";
  static constexpr auto summery = "Find documents matching an example";
  static constexpr auto operationId = "(FindDocumentsMatchingAnExample";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Simple Queries/put_api_simple_first_example.md";
  static constexpr auto x_hints = R"({% hint 'warning' %}
This route should no longer be used.
All endpoints for Simple Queries are deprecated from version 3.4.0 on.
They are superseded by AQL queries.
{% endhint %}

{% hint 'warning' %}
Till ArangoDB versions 3.2.13 and 3.3.7 this API is quite expensive.
A more lightweight alternative is to use the [HTTP Cursor API](../AqlQueryCursor/README.md).
Starting from versions 3.2.14 and 3.3.8 this performance impact is not
an issue anymore, as the internal implementation of the API has changed.
{% endhint %}

)";
  inline const static std::vector<char const * > tags{"Simple Queries"};
  inline Parameter P0;
  inline void initParam(){
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"$ref":"#/definitions/put_api_simple_first_example"})";
    P0.xDescriptionOffset = 54;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PutFirstExampleResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned when the query was successfully executed.)"},
{Response400, R"(is returned if the body does not contain a valid JSON representation of a
query. The response body contains an error document in this case.)"}, 
{Response404, R"(is returned if the collection specified by *collection* is unknown.  The
response body contains an error document in this case.)"}};
  static constexpr auto description = R"(
**A JSON object with these properties is required:**

  - **example**: The example document.
  - **collection**: The name of the collection to query.





This will return the first document matching a given example.

Returns a result containing the document or *HTTP 404* if no
document matched the example.

If more than one document in the collection matches the specified example, only
one of these documents will be returned, and it is undefined which of the matching
documents is returned.


<!-- Hints Start -->

**Warning:**  
This route should no longer be used.
All endpoints for Simple Queries are deprecated from version 3.4.0 on.
They are superseded by AQL queries.




**Warning:**  
Till ArangoDB versions 3.2.13 and 3.3.7 this API is quite expensive.
A more lightweight alternative is to use the [HTTP Cursor API](../AqlQueryCursor/README.md).
Starting from versions 3.2.14 and 3.3.8 this performance impact is not
an issue anymore, as the internal implementation of the API has changed.



<!-- Hints End -->
)";


  ApiResult request(std::string const &databaseName);
};
namespace PutFulltext {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/simple/fulltext";
  static constexpr auto summery = "Fulltext index query";
  static constexpr auto operationId = "(FulltextIndexQuery";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Simple Queries/put_api_simple_fulltext.md";
  static constexpr auto x_hints = R"({% hint 'warning' %}
This route should no longer be used.
All endpoints for Simple Queries are deprecated from version 3.4.0 on.
They are superseded by AQL queries.
{% endhint %}

)";
  inline const static std::vector<char const * > tags{"Simple Queries"};
  inline Parameter P0;
  inline void initParam(){
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"$ref":"#/definitions/put_api_simple_fulltext"})";
    P0.xDescriptionOffset = 54;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PutFulltextResponse : short {
    Unknown = 0,
    Response201 = 201,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response201, R"(is returned if the query was executed successfully.)"},
{Response400, R"(is returned if the body does not contain a valid JSON representation of a
query. The response body contains an error document in this case.)"}, 
{Response404, R"(is returned if the collection specified by *collection* is unknown.  The
response body contains an error document in this case.)"}};
  static constexpr auto description = R"(
**A JSON object with these properties is required:**

  - **index**: The identifier of the fulltext-index to use.
  - **attribute**: The attribute that contains the texts.
  - **collection**: The name of the collection to query.
  - **limit**: The maximal amount of documents to return. The *skip*
   is applied before the *limit* restriction. (optional)
  - **skip**: The number of documents to skip in the query (optional).
  - **query**: The fulltext query. Please refer to [Fulltext queries](../../Manual/Appendix/Deprecated/SimpleQueries/FulltextQueries.html)
     for details.





This will find all documents from the collection that match the fulltext
query specified in *query*.

In order to use the *fulltext* operator, a fulltext index must be defined
for the collection and the specified attribute.

Returns a cursor containing the result, see [HTTP Cursor](../AqlQueryCursor/README.md) for details.

Note: the *fulltext* simple query is **deprecated** as of ArangoDB 2.6. 
This API may be removed in future versions of ArangoDB. The preferred
way for retrieving documents from a collection using the near operator is
to issue an AQL query using the *FULLTEXT* [AQL function](../../AQL/Functions/Fulltext.html) 
as follows:

    FOR doc IN FULLTEXT(@@collection, @attributeName, @queryString, @limit) 
      RETURN doc


<!-- Hints Start -->

**Warning:**  
This route should no longer be used.
All endpoints for Simple Queries are deprecated from version 3.4.0 on.
They are superseded by AQL queries.



<!-- Hints End -->
)";


  ApiResult request(std::string const &databaseName);
};
namespace PutLookupByKeys {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/simple/lookup-by-keys";
  static constexpr auto summery = "Find documents by their keys";
  static constexpr auto operationId = "(FindDocumentsByTheirKeys";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Simple Queries/RestLookupByKeys.md";
  static constexpr auto x_hints = R"({% hint 'warning' %}
This route should no longer be used.
All endpoints for Simple Queries are deprecated from version 3.4.0 on.
They are superseded by AQL queries.
{% endhint %}

)";
  inline const static std::vector<char const * > tags{"Simple Queries"};
  inline Parameter P0;
  inline void initParam(){
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"$ref":"#/definitions/RestLookupByKeys"})";
    P0.xDescriptionOffset = 54;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PutLookupByKeysResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response404 = 404,
    Response405 = 405
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned if the operation was carried out successfully.)"},
{Response404, R"(is returned if the collection was not found.  The response body
contains an error document in this case.)"}, 
{Response405, R"(is returned if the operation was called with a different HTTP METHOD than PUT.)"}};
  static constexpr auto description = R"(
**A JSON object with these properties is required:**

  - **keys** (string): array with the _keys of documents to remove.
  - **collection**: The name of the collection to look in for the documents




Looks up the documents in the specified collection
using the array of keys provided. All documents for which a matching
key was specified in the *keys* array and that exist in the collection
will be returned.  Keys for which no document can be found in the
underlying collection are ignored, and no exception will be thrown for
them.

Equivalent AQL query:

    FOR doc IN @@collection FILTER doc._key IN @keys RETURN doc

The body of the response contains a JSON object with a *documents*
attribute. The *documents* attribute is an array containing the
matching documents. The order in which matching documents are present
in the result array is unspecified.


<!-- Hints Start -->

**Warning:**  
This route should no longer be used.
All endpoints for Simple Queries are deprecated from version 3.4.0 on.
They are superseded by AQL queries.



<!-- Hints End -->
)";


  ApiResult request(std::string const &databaseName);
};
namespace PutNear {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/simple/near";
  static constexpr auto summery = "Returns documents near a coordinate";
  static constexpr auto operationId = "(ReturnsDocumentsNearACoordinate";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Simple Queries/put_api_simple_near.md";
  static constexpr auto x_hints = R"({% hint 'warning' %}
This route should no longer be used.
All endpoints for Simple Queries are deprecated from version 3.4.0 on.
They are superseded by AQL queries.
{% endhint %}

)";
  inline const static std::vector<char const * > tags{"Simple Queries"};
  inline Parameter P0;
  inline void initParam(){
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"$ref":"#/definitions/put_api_simple_near"})";
    P0.xDescriptionOffset = 54;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PutNearResponse : short {
    Unknown = 0,
    Response201 = 201,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response201, R"(is returned if the query was executed successfully.)"},
{Response400, R"(is returned if the body does not contain a valid JSON representation of a
query. The response body contains an error document in this case.)"}, 
{Response404, R"(is returned if the collection specified by *collection* is unknown.  The
response body contains an error document in this case.)"}};
  static constexpr auto description = R"(
**A JSON object with these properties is required:**

  - **distance**: If given, the attribute key used to return the distance to
   the given coordinate. (optional). If specified, distances are returned in meters.
  - **skip**: The number of documents to skip in the query. (optional)
  - **longitude**: The longitude of the coordinate.
  - **limit**: The maximal amount of documents to return. The *skip* is
   applied before the *limit* restriction. The default is 100. (optional)
  - **collection**: The name of the collection to query.
  - **latitude**: The latitude of the coordinate.
  - **geo**: If given, the identifier of the geo-index to use. (optional)





The default will find at most 100 documents near the given coordinate.  The
returned array is sorted according to the distance, with the nearest document
being first in the return array. If there are near documents of equal distance, documents
are chosen randomly from this set until the limit is reached.

In order to use the *near* operator, a geo index must be defined for the
collection. This index also defines which attribute holds the coordinates
for the document.  If you have more than one geo-spatial index, you can use
the *geo* field to select a particular index.


Returns a cursor containing the result, see [HTTP Cursor](../AqlQueryCursor/README.md) for details.

Note: the *near* simple query is **deprecated** as of ArangoDB 2.6. 
This API may be removed in future versions of ArangoDB. The preferred
way for retrieving documents from a collection using the near operator is
to issue an [AQL query](../../AQL/Functions/Geo.html) using the *NEAR* function as follows: 

    FOR doc IN NEAR(@@collection, @latitude, @longitude, @limit)
      RETURN doc`


<!-- Hints Start -->

**Warning:**  
This route should no longer be used.
All endpoints for Simple Queries are deprecated from version 3.4.0 on.
They are superseded by AQL queries.



<!-- Hints End -->
)";


  ApiResult request(std::string const &databaseName);
};
namespace PutRange {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/simple/range";
  static constexpr auto summery = "Simple range query";
  static constexpr auto operationId = "(SimpleRangeQuery";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Simple Queries/put_api_simple_range.md";
  static constexpr auto x_hints = R"({% hint 'warning' %}
This route should no longer be used.
All endpoints for Simple Queries are deprecated from version 3.4.0 on.
They are superseded by AQL queries.
{% endhint %}

)";
  inline const static std::vector<char const * > tags{"Simple Queries"};
  inline Parameter P0;
  inline void initParam(){
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"$ref":"#/definitions/put_api_simple_range"})";
    P0.xDescriptionOffset = 54;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PutRangeResponse : short {
    Unknown = 0,
    Response201 = 201,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response201, R"(is returned if the query was executed successfully.)"},
{Response400, R"(is returned if the body does not contain a valid JSON representation of a
query. The response body contains an error document in this case.)"}, 
{Response404, R"(is returned if the collection specified by *collection* is unknown or no
suitable index for the range query is present.  The response body contains 
an error document in this case.)"}};
  static constexpr auto description = R"(
**A JSON object with these properties is required:**

  - **right**: The upper bound.
  - **attribute**: The attribute path to check.
  - **collection**: The name of the collection to query.
  - **limit**: The maximal amount of documents to return. The *skip*
   is applied before the *limit* restriction. (optional)
  - **closed**: If *true*, use interval including *left* and *right*,
   otherwise exclude *right*, but include *left*.
  - **skip**: The number of documents to skip in the query (optional).
  - **left**: The lower bound.





This will find all documents within a given range. In order to execute a
range query, a skip-list index on the queried attribute must be present.

Returns a cursor containing the result, see [HTTP Cursor](../AqlQueryCursor/README.md) for details.

Note: the *range* simple query is **deprecated** as of ArangoDB 2.6. 
The function may be removed in future versions of ArangoDB. The preferred
way for retrieving documents from a collection within a specific range
is to use an AQL query as follows: 

    FOR doc IN @@collection 
      FILTER doc.value >= @left && doc.value < @right 
      LIMIT @skip, @limit 
      RETURN doc`


<!-- Hints Start -->

**Warning:**  
This route should no longer be used.
All endpoints for Simple Queries are deprecated from version 3.4.0 on.
They are superseded by AQL queries.



<!-- Hints End -->
)";


  ApiResult request(std::string const &databaseName);
};
namespace PutRemoveByExample {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/simple/remove-by-example";
  static constexpr auto summery = "Remove documents by example";
  static constexpr auto operationId = "(RemoveDocumentsByExample";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Simple Queries/put_api_simple_remove_by_example.md";
  static constexpr auto x_hints = R"({% hint 'warning' %}
This route should no longer be used.
All endpoints for Simple Queries are deprecated from version 3.4.0 on.
They are superseded by AQL queries.
{% endhint %}

)";
  inline const static std::vector<char const * > tags{"Simple Queries"};
  inline Parameter P0;
  inline void initParam(){
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"$ref":"#/definitions/put_api_simple_remove_by_example"})";
    P0.xDescriptionOffset = 54;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PutRemoveByExampleResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned if the query was executed successfully.)"},
{Response400, R"(is returned if the body does not contain a valid JSON representation of a
query. The response body contains an error document in this case.)"}, 
{Response404, R"(is returned if the collection specified by *collection* is unknown.  The
response body contains an error document in this case.)"}};
  static constexpr auto description = R"(
**A JSON object with these properties is required:**

  - **example**: An example document that all collection documents are compared against.
  - **collection**: The name of the collection to remove from.
  - **options**:
    - **limit**: an optional value that determines how many documents to
    delete at most. If *limit* is specified but is less than the number
    of documents in the collection, it is undefined which of the documents
    will be deleted.
    - **waitForSync**: if set to true, then all removal operations will
    instantly be synchronized to disk. If this is not specified, then the
    collection's default sync behavior will be applied.





This will find all documents in the collection that match the specified
example object.

Note: the *limit* attribute is not supported on sharded collections.
Using it will result in an error.

Returns the number of documents that were deleted.


<!-- Hints Start -->

**Warning:**  
This route should no longer be used.
All endpoints for Simple Queries are deprecated from version 3.4.0 on.
They are superseded by AQL queries.



<!-- Hints End -->
)";


  ApiResult request(std::string const &databaseName);
};
namespace PutRemoveByKeys {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/simple/remove-by-keys";
  static constexpr auto summery = "Remove documents by their keys";
  static constexpr auto operationId = "(RemoveDocumentsByTheirKeys";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Simple Queries/RestRemoveByKeys.md";
  static constexpr auto x_hints = R"({% hint 'warning' %}
This route should no longer be used.
All endpoints for Simple Queries are deprecated from version 3.4.0 on.
They are superseded by AQL queries.
{% endhint %}

)";
  inline const static std::vector<char const * > tags{"Simple Queries"};
  inline Parameter P0;
  inline void initParam(){
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"$ref":"#/definitions/RestRemoveByKeys"})";
    P0.xDescriptionOffset = 54;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PutRemoveByKeysResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response404 = 404,
    Response405 = 405
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned if the operation was carried out successfully. The number of removed
documents may still be 0 in this case if none of the specified document keys
were found in the collection.)"}, 
{Response404, R"(is returned if the collection was not found.
The response body contains an error document in this case.)"}, 
{Response405, R"(is returned if the operation was called with a different HTTP METHOD than PUT.)"}};
  static constexpr auto description = R"(
**A JSON object with these properties is required:**

  - **keys** (string): array with the _keys of documents to remove.
  - **options**:
    - **returnOld**: if set to *true* and *silent* above is *false*, then the above
    information about the removed documents contains the complete
    removed documents.
    - **silent**: if set to *false*, then the result will contain an additional
    attribute *old* which contains an array with one entry for each
    removed document. By default, these entries will have the *_id*,
    *_key* and *_rev* attributes.
    - **waitForSync**: if set to true, then all removal operations will
    instantly be synchronized to disk. If this is not specified, then the
    collection's default sync behavior will be applied.
  - **collection**: The name of the collection to look in for the documents to remove




Looks up the documents in the specified collection using the array of keys
provided, and removes all documents from the collection whose keys are
contained in the *keys* array. Keys for which no document can be found in
the underlying collection are ignored, and no exception will be thrown for
them.

Equivalent AQL query (the RETURN clause is optional):

    FOR key IN @keys REMOVE key IN @@collection
      RETURN OLD

The body of the response contains a JSON object with information how many
documents were removed (and how many were not). The *removed* attribute will
contain the number of actually removed documents. The *ignored* attribute 
will contain the number of keys in the request for which no matching document
could be found.


<!-- Hints Start -->

**Warning:**  
This route should no longer be used.
All endpoints for Simple Queries are deprecated from version 3.4.0 on.
They are superseded by AQL queries.



<!-- Hints End -->
)";


  ApiResult request(std::string const &databaseName);
};
namespace PutReplaceByExample {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/simple/replace-by-example";
  static constexpr auto summery = "Replace documents by example";
  static constexpr auto operationId = "(ReplaceDocumentsByExample";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Simple Queries/put_api_simple_replace_by_example.md";
  static constexpr auto x_hints = R"({% hint 'warning' %}
This route should no longer be used.
All endpoints for Simple Queries are deprecated from version 3.4.0 on.
They are superseded by AQL queries.
{% endhint %}

)";
  inline const static std::vector<char const * > tags{"Simple Queries"};
  inline Parameter P0;
  inline void initParam(){
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"$ref":"#/definitions/put_api_simple_replace_by_example"})";
    P0.xDescriptionOffset = 54;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PutReplaceByExampleResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned if the query was executed successfully.)"},
{Response400, R"(is returned if the body does not contain a valid JSON representation of a
query. The response body contains an error document in this case.)"}, 
{Response404, R"(is returned if the collection specified by *collection* is unknown.  The
response body contains an error document in this case.)"}};
  static constexpr auto description = R"(
**A JSON object with these properties is required:**

  - **options**:
    - **limit**: an optional value that determines how many documents to
    replace at most. If *limit* is specified but is less than the number
    of documents in the collection, it is undefined which of the documents
    will be replaced.
    - **waitForSync**: if set to true, then all removal operations will
     instantly be synchronized to disk. If this is not specified, then the
     collection's default sync behavior will be applied.
  - **example**: An example document that all collection documents are compared against.
  - **collection**: The name of the collection to replace within.
  - **newValue**: The replacement document that will get inserted in place
   of the "old" documents.





This will find all documents in the collection that match the specified
example object, and replace the entire document body with the new value
specified. Note that document meta-attributes such as *_id*, *_key*,
*_from*, *_to* etc. cannot be replaced.

Note: the *limit* attribute is not supported on sharded collections.
Using it will result in an error.

Returns the number of documents that were replaced.


<!-- Hints Start -->

**Warning:**  
This route should no longer be used.
All endpoints for Simple Queries are deprecated from version 3.4.0 on.
They are superseded by AQL queries.



<!-- Hints End -->
)";


  ApiResult request(std::string const &databaseName);
};
namespace PutUpdateByExample {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/simple/update-by-example";
  static constexpr auto summery = "Update documents by example";
  static constexpr auto operationId = "(UpdateDocumentsByExample";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Simple Queries/put_api_simple_update_by_example.md";
  static constexpr auto x_hints = R"({% hint 'warning' %}
This route should no longer be used.
All endpoints for Simple Queries are deprecated from version 3.4.0 on.
They are superseded by AQL queries.
{% endhint %}

)";
  inline const static std::vector<char const * > tags{"Simple Queries"};
  inline Parameter P0;
  inline void initParam(){
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"$ref":"#/definitions/put_api_simple_update_by_example"})";
    P0.xDescriptionOffset = 54;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PutUpdateByExampleResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned if the collection was updated successfully and *waitForSync* was
*true*.)"}, 
{Response400, R"(is returned if the body does not contain a valid JSON representation of a
query. The response body contains an error document in this case.)"}, 
{Response404, R"(is returned if the collection specified by *collection* is unknown.  The
response body contains an error document in this case.)"}};
  static constexpr auto description = R"(
**A JSON object with these properties is required:**

  - **options**:
    - **keepNull**: This parameter can be used to modify the behavior when
    handling *null* values. Normally, *null* values are stored in the
    database. By setting the *keepNull* parameter to *false*, this
    behavior can be changed so that all attributes in *data* with *null*
    values will be removed from the updated document.
    - **mergeObjects**: Controls whether objects (not arrays) will be merged if present in both the
    existing and the patch document. If set to false, the value in the
    patch document will overwrite the existing document's value. If set to
    true, objects will be merged. The default is true.
    - **limit**: an optional value that determines how many documents to
    update at most. If *limit* is specified but is less than the number
    of documents in the collection, it is undefined which of the documents
    will be updated.
    - **waitForSync**: if set to true, then all removal operations will
    instantly be synchronized to disk. If this is not specified, then the
    collection's default sync behavior will be applied.
  - **example**: An example document that all collection documents are compared against.
  - **collection**: The name of the collection to update within.
  - **newValue**: A document containing all the attributes to update in the found documents.





This will find all documents in the collection that match the specified
example object, and partially update the document body with the new value
specified. Note that document meta-attributes such as *_id*, *_key*,
*_from*, *_to* etc. cannot be replaced.

Note: the *limit* attribute is not supported on sharded collections.
Using it will result in an error.

Returns the number of documents that were updated.


<!-- Hints Start -->

**Warning:**  
This route should no longer be used.
All endpoints for Simple Queries are deprecated from version 3.4.0 on.
They are superseded by AQL queries.



<!-- Hints End -->
)";


  ApiResult request(std::string const &databaseName);
};
namespace PutWithin {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/simple/within";
  static constexpr auto summery = "Find documents within a radius around a coordinate";
  static constexpr auto operationId = "(FindDocumentsWithinARadiusAroundACoordinate";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Simple Queries/put_api_simple_within.md";
  static constexpr auto x_hints = R"({% hint 'warning' %}
This route should no longer be used.
All endpoints for Simple Queries are deprecated from version 3.4.0 on.
They are superseded by AQL queries.
{% endhint %}

)";
  inline const static std::vector<char const * > tags{"Simple Queries"};
  inline Parameter P0;
  inline void initParam(){
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"$ref":"#/definitions/put_api_simple_within"})";
    P0.xDescriptionOffset = 54;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PutWithinResponse : short {
    Unknown = 0,
    Response201 = 201,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response201, R"(is returned if the query was executed successfully.)"},
{Response400, R"(is returned if the body does not contain a valid JSON representation of a
query. The response body contains an error document in this case.)"}, 
{Response404, R"(is returned if the collection specified by *collection* is unknown.  The
response body contains an error document in this case.)"}};
  static constexpr auto description = R"(
**A JSON object with these properties is required:**

  - **distance**: If given, the attribute key used to return the distance to
   the given coordinate. (optional). If specified, distances are returned in meters.
  - **skip**: The number of documents to skip in the query. (optional)
  - **longitude**: The longitude of the coordinate.
  - **radius**: The maximal radius (in meters).
  - **collection**: The name of the collection to query.
  - **latitude**: The latitude of the coordinate.
  - **limit**: The maximal amount of documents to return. The *skip* is
   applied before the *limit* restriction. The default is 100. (optional)
  - **geo**: If given, the identifier of the geo-index to use. (optional)





This will find all documents within a given radius around the coordinate
(*latitude*, *longitude*). The returned list is sorted by distance.

In order to use the *within* operator, a geo index must be defined for
the collection. This index also defines which attribute holds the
coordinates for the document.  If you have more than one geo-spatial index,
you can use the *geo* field to select a particular index.


Returns a cursor containing the result, see [HTTP Cursor](../AqlQueryCursor/README.md) for details.

Note: the *within* simple query is **deprecated** as of ArangoDB 2.6. 
This API may be removed in future versions of ArangoDB. The preferred
way for retrieving documents from a collection using the near operator is
to issue an [AQL query](../../AQL/Functions/Geo.html) using the *WITHIN* function as follows: 

    FOR doc IN WITHIN(@@collection, @latitude, @longitude, @radius, @distanceAttributeName)
      RETURN doc


<!-- Hints Start -->

**Warning:**  
This route should no longer be used.
All endpoints for Simple Queries are deprecated from version 3.4.0 on.
They are superseded by AQL queries.



<!-- Hints End -->
)";


  ApiResult request(std::string const &databaseName);
};
namespace PutWithinRectangle {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/simple/within-rectangle";
  static constexpr auto summery = "Within rectangle query";
  static constexpr auto operationId = "(WithinRectangleQuery";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Simple Queries/put_api_simple_within_rectangle.md";
  static constexpr auto x_hints = R"({% hint 'warning' %}
This route should no longer be used.
All endpoints for Simple Queries are deprecated from version 3.4.0 on.
They are superseded by AQL queries.
{% endhint %}

)";
  inline const static std::vector<char const * > tags{"Simple Queries"};
  inline Parameter P0;
  inline void initParam(){
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"$ref":"#/definitions/put_api_simple_within_rectangle"})";
    P0.xDescriptionOffset = 54;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PutWithinRectangleResponse : short {
    Unknown = 0,
    Response201 = 201,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response201, R"(is returned if the query was executed successfully.)"},
{Response400, R"(is returned if the body does not contain a valid JSON representation of a
query. The response body contains an error document in this case.)"}, 
{Response404, R"(is returned if the collection specified by *collection* is unknown.  The
response body contains an error document in this case.)"}};
  static constexpr auto description = R"(
**A JSON object with these properties is required:**

  - **latitude1**: The latitude of the first rectangle coordinate.
  - **skip**: The number of documents to skip in the query. (optional)
  - **latitude2**: The latitude of the second rectangle coordinate.
  - **longitude2**: The longitude of the second rectangle coordinate.
  - **longitude1**: The longitude of the first rectangle coordinate.
  - **limit**: The maximal amount of documents to return. The *skip* is
   applied before the *limit* restriction. The default is 100. (optional)
  - **collection**: The name of the collection to query.
  - **geo**: If given, the identifier of the geo-index to use. (optional)





This will find all documents within the specified rectangle (determined by
the given coordinates (*latitude1*, *longitude1*, *latitude2*, *longitude2*). 

In order to use the *within-rectangle* query, a geo index must be defined for
the collection. This index also defines which attribute holds the
coordinates for the document.  If you have more than one geo-spatial index,
you can use the *geo* field to select a particular index.

Returns a cursor containing the result, see [HTTP Cursor](../AqlQueryCursor/README.md) for details.


<!-- Hints Start -->

**Warning:**  
This route should no longer be used.
All endpoints for Simple Queries are deprecated from version 3.4.0 on.
They are superseded by AQL queries.



<!-- Hints End -->
)";


  ApiResult request(std::string const &databaseName);
};
}
namespace Administration {
namespace PostTasks {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Post;
  static constexpr auto url = "/_api/tasks";
  static constexpr auto summery = "creates a task";
  static constexpr auto operationId = "(registerTask";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Administration/post_api_new_tasks.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Administration"};
  inline Parameter P0;
  inline void initParam(){
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"$ref":"#/definitions/post_api_new_tasks"})";
    P0.xDescriptionOffset = 54;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PostTasksResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(The task was registered)"},
{Response400, R"(If the post body is not accurate, a *HTTP 400* is returned.)"}};
  static constexpr auto description = R"(
**A JSON object with these properties is required:**

  - **params**: The parameters to be passed into command
  - **offset**: Number of seconds initial delay 
  - **command**: The JavaScript code to be executed
  - **name**: The name of the task
  - **period**: number of seconds between the executions




creates a new task with a generated id



**HTTP 200**
*A json document with these Properties is returned:*

The task was registered

- **code**: The status code, 200 in this case.
- **created**: The timestamp when this task was created
- **database**: the database this task belongs to
- **period**: this task should run each `period` seconds
- **command**: the javascript function for this task
- **error**: *false* in this case
- **offset**: time offset in seconds from the created timestamp
- **type**: What type of task is this [ `periodic`, `timed`]
 - periodic are tasks that repeat periodically
 - timed are tasks that execute once at a specific time
- **id**: A string identifying the task


)";


  ApiResult request(std::string const &databaseName);
};
namespace GetTasks {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/tasks/";
  static constexpr auto summery = "Fetch all tasks or one task";
  static constexpr auto operationId = "(getTasks";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Administration/get_api_tasks_all.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Administration"};
  
  enum GetResponse : short {
    Unknown = 0,
    Response200 = 200
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(The list of tasks)"}};
  static constexpr auto description = R"(fetches all existing tasks on the server


**HTTP 200**
*A json document with these Properties is returned:*

The list of tasks

[
  - **name**: The fully qualified name of the user function
  - **created**: The timestamp when this task was created
  - **database**: the database this task belongs to
  - **period**: this task should run each `period` seconds
  - **command**: the javascript function for this task
  - **offset**: time offset in seconds from the created timestamp
  - **type**: What type of task is this [ `periodic`, `timed`]
    - periodic are tasks that repeat periodically
    - timed are tasks that execute once at a specific time
  - **id**: A string identifying the task
]


)";


  ApiResult request(std::string const &databaseName);
};
namespace DeleteTaskId {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Delete;
  static constexpr auto url = "/_api/tasks/{id}";
  static constexpr auto summery = "deletes the task with id";
  static constexpr auto operationId = "(deleteTask";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Administration/delete_api_tasks.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Administration"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The id of the task to delete.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "id";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum DeleteIdResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(If the task was deleted, *HTTP 200* is returned.)"},
{Response404, R"(If the task *id* is unknown, then an *HTTP 404* is returned.)"}};
  static constexpr auto description = R"(See online..)";


  ApiResult request(std::string const &databaseName);
};
namespace GetTaskId {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/tasks/{id}";
  static constexpr auto summery = "Fetch one task with id";
  static constexpr auto operationId = "(getTasks";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Administration/get_api_tasks.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Administration"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The id of the task to fetch.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "id";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum GetIdResponse : short {
    Unknown = 0,
    Response200 = 200
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(The requested task)"}};
  static constexpr auto description = R"(See online..)";


  ApiResult request(std::string const &databaseName);
};
namespace PutTaskId {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/tasks/{id}";
  static constexpr auto summery = "creates a task with id";
  static constexpr auto operationId = "(registerTask:byId";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Administration/put_api_new_tasks.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Administration"};
  inline Parameter P0;
  inline Parameter P1;
  inline void initParam(){
    P0.description = R"(The id of the task to create)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "id";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.in = ParamIn::body;
    P1.name = "Json Request Body";
    P1.required = true;
    P1.schema = R"({"$ref":"#/definitions/put_api_new_tasks"})";
    P1.xDescriptionOffset = 54;
  }
  inline std::vector<Parameter> parameters{P0, P1};

  enum PutIdResponse : short {
    Unknown = 0,
    Response400 = 400
  };
  inline const static std::map<int, char const * > responses{{Response400, R"(If the task *id* already exists or the rest body is not accurate, *HTTP 400* is returned.)"}};
  static constexpr auto description = R"(See online..)";


  ApiResult request(std::string const &databaseName);
};
}
namespace Transaction {
namespace PostTransaction {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Post;
  static constexpr auto url = "/_api/transaction";
  static constexpr auto summery = "Execute transaction";
  static constexpr auto operationId = "(executeCommit";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Transactions/post_api_transaction.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Transactions"};
  inline Parameter P0;
  inline void initParam(){
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"$ref":"#/definitions/post_api_transaction"})";
    P0.xDescriptionOffset = 54;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PostTransactionResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400,
    Response404 = 404,
    Response500 = 500
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(If the transaction is fully executed and committed on the server,
*HTTP 200* will be returned.)"}, 
{Response400, R"(If the transaction specification is either missing or malformed, the server
will respond with *HTTP 400*.)"}, 
{Response404, R"(If the transaction specification contains an unknown collection, the server
will respond with *HTTP 404*.)"}, 
{Response500, R"(Exceptions thrown by users will make the server respond with a return code of
*HTTP 500*)"}};
  static constexpr auto description = R"(
**A JSON object with these properties is required:**

  - **maxTransactionSize**: Transaction size limit in bytes. Honored by the RocksDB storage engine only.
  - **lockTimeout**: an optional numeric value that can be used to set a
   timeout for waiting on collection locks. If not specified, a default
   value will be used. Setting *lockTimeout* to *0* will make ArangoDB
   not time out waiting for a lock.
  - **waitForSync**: an optional boolean flag that, if set, will force the
   transaction to write all data to disk before returning.
  - **allowImplicit**: Allow reading from undeclared collections.
  - **params**: optional arguments passed to *action*.
  - **action**: the actual transaction operations to be executed, in the
   form of stringified JavaScript code. The code will be executed on server
   side, with late binding. It is thus critical that the code specified in
   *action* properly sets up all the variables it needs.
   If the code specified in *action* ends with a return statement, the
   value returned will also be returned by the REST API in the *result*
   attribute if the transaction committed successfully.
  - **collections**: *collections* must be a JSON object that can have one or all sub-attributes
   *read*, *write* or *exclusive*, each being an array of collection names or a
   single collection name as string. Collections that will be written to in the
   transaction must be declared with the *write* or *exclusive* attribute or it
   will fail, whereas non-declared collections from which is solely read will be
   added lazily. The optional sub-attribute *allowImplicit* can be set to *false*
   to let transactions fail in case of undeclared collections for reading.
   Collections for reading should be fully declared if possible, to avoid
   deadlocks.
   See [locking and isolation](../../Manual/Transactions/LockingAndIsolation.html)
   for more information.




The transaction description must be passed in the body of the POST request.

If the transaction is fully executed and committed on the server,
*HTTP 200* will be returned. Additionally, the return value of the
code defined in *action* will be returned in the *result* attribute.

For successfully committed transactions, the returned JSON object has the
following properties:

- *error*: boolean flag to indicate if an error occurred (*false*
  in this case)

- *code*: the HTTP status code

- *result*: the return value of the transaction

If the transaction specification is either missing or malformed, the server
will respond with *HTTP 400*.

The body of the response will then contain a JSON object with additional error
details. The object has the following attributes:

- *error*: boolean flag to indicate that an error occurred (*true* in this case)

- *code*: the HTTP status code

- *errorNum*: the server error number

- *errorMessage*: a descriptive error message

If a transaction fails to commit, either by an exception thrown in the
*action* code, or by an internal error, the server will respond with
an error.
Any other errors will be returned with any of the return codes
*HTTP 400*, *HTTP 409*, or *HTTP 500*.


)";


  ApiResult request(std::string const &databaseName);
};
namespace PostBegin {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Post;
  static constexpr auto url = "/_api/transaction/begin";
  static constexpr auto summery = "Begin transaction";
  static constexpr auto operationId = "(executeBegin";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Transactions/post_api_transaction_begin.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Transactions"};
  inline Parameter P0;
  inline void initParam(){
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"$ref":"#/definitions/post_api_transaction_begin"})";
    P0.xDescriptionOffset = 54;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PostBeginResponse : short {
    Unknown = 0,
    Response201 = 201,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response201, R"(If the transaction is running on the server,
*HTTP 201* will be returned.)"}, 
{Response400, R"(If the transaction specification is either missing or malformed, the server
will respond with *HTTP 400*.)"}, 
{Response404, R"(If the transaction specification contains an unknown collection, the server
will respond with *HTTP 404*.)"}};
  static constexpr auto description = R"(
**A JSON object with these properties is required:**

  - **maxTransactionSize**: Transaction size limit in bytes. Honored by the RocksDB storage engine only.
  - **allowImplicit**: Allow reading from undeclared collections.
  - **collections**: *collections* must be a JSON object that can have one or all sub-attributes
   *read*, *write* or *exclusive*, each being an array of collection names or a
   single collection name as string. Collections that will be written to in the
   transaction must be declared with the *write* or *exclusive* attribute or it
   will fail, whereas non-declared collections from which is solely read will be
   added lazily. The optional sub-attribute *allowImplicit* can be set to *false*
   to let transactions fail in case of undeclared collections for reading.
   Collections for reading should be fully declared if possible, to avoid
   deadlocks.
   See [locking and isolation](../../Manual/Transactions/LockingAndIsolation.html)
   for more information.
  - **lockTimeout**: an optional numeric value that can be used to set a
   timeout for waiting on collection locks. If not specified, a default
   value will be used. Setting *lockTimeout* to *0* will make ArangoDB
   not time out waiting for a lock.
  - **waitForSync**: an optional boolean flag that, if set, will force the
   transaction to write all data to disk before returning.




The transaction description must be passed in the body of the POST request.
If the transaction can be started on the server, *HTTP 201* will be returned. 

For successfully started transactions, the returned JSON object has the
following properties:

- *error*: boolean flag to indicate if an error occurred (*false*
  in this case)

- *code*: the HTTP status code

- *result*: result containing
    - *id*: the identifier of the transaction
    - *status*: containing the string 'running'

If the transaction specification is either missing or malformed, the server
will respond with *HTTP 400* or *HTTP 404*.

The body of the response will then contain a JSON object with additional error
details. The object has the following attributes:

- *error*: boolean flag to indicate that an error occurred (*true* in this case)

- *code*: the HTTP status code

- *errorNum*: the server error number

- *errorMessage*: a descriptive error message



)";


  ApiResult request(std::string const &databaseName);
};
namespace DeleteTransactionId {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Delete;
  static constexpr auto url = "/_api/transaction/{transaction-id}";
  static constexpr auto summery = "Abort transaction";
  static constexpr auto operationId = "(executeAbort:transaction";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Transactions/delete_api_transaction.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Transactions"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The transaction identifier,)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "transaction-id";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum DeleteTransactionIdResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400,
    Response404 = 404,
    Response409 = 409
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(If the transaction was aborted,
*HTTP 200* will be returned.)"}, 
{Response400, R"(If the transaction cannot be aborted, the server
will respond with *HTTP 400*.)"}, 
{Response404, R"(If the transaction was not found, the server
will respond with *HTTP 404*.)"}, 
{Response409, R"(If the transaction was already committed, the server
will respond with *HTTP 409*.)"}};
  static constexpr auto description = R"(Abort a running server-side transaction. Aborting is an idempotent operation. 
It is not an error to abort a transaction more than once.

If the transaction can be aborted, *HTTP 200* will be returned. 
The returned JSON object has the following properties:

- *error*: boolean flag to indicate if an error occurred (*false*
  in this case)

- *code*: the HTTP status code

- *result*: result containing
    - *id*: the identifier of the transaction
    - *status*: containing the string 'aborted'

If the transaction cannot be found, aborting is not allowed or the 
transaction was already committed, the server
will respond with *HTTP 400*, *HTTP 404* or *HTTP 409*.

The body of the response will then contain a JSON object with additional error
details. The object has the following attributes:

- *error*: boolean flag to indicate that an error occurred (*true* in this case)

- *code*: the HTTP status code

- *errorNum*: the server error number

- *errorMessage*: a descriptive error message



)";


  ApiResult request(std::string const &databaseName);
};
namespace GetTransactionId {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/transaction/{transaction-id}";
  static constexpr auto summery = "Get transaction status";
  static constexpr auto operationId = "(executeGetState:transaction";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Transactions/get_api_transaction.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Transactions"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The transaction identifier.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "transaction-id";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum GetTransactionIdResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(If the transaction is fully executed and committed on the server,
*HTTP 200* will be returned.)"}, 
{Response400, R"(If the transaction identifier specified is either missing or malformed, the server
will respond with *HTTP 400*.)"}, 
{Response404, R"(If the transaction was not found with the specified identifier, the server
will respond with *HTTP 404*.)"}};
  static constexpr auto description = R"(The result is an object describing the status of the transaction. 
It has at least the following attributes:

- *id*: the identifier of the transaction

- *status*: the status of the transaction. One of "running", "committed" or "aborted".


)";


  ApiResult request(std::string const &databaseName);
};
namespace PutTransactionId {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/transaction/{transaction-id}";
  static constexpr auto summery = "Commit transaction";
  static constexpr auto operationId = "(executeCommit:Transaction";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Transactions/put_api_transaction.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Transactions"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The transaction identifier,)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "transaction-id";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PutTransactionIdResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400,
    Response404 = 404,
    Response409 = 409
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(If the transaction was committed,
*HTTP 200* will be returned.)"}, 
{Response400, R"(If the transaction cannot be committed, the server
will respond with *HTTP 400*.)"}, 
{Response404, R"(If the transaction was not found, the server
will respond with *HTTP 404*.)"}, 
{Response409, R"(If the transaction was already aborted, the server
will respond with *HTTP 409*.)"}};
  static constexpr auto description = R"(Commit a running server-side transaction. Committing is an idempotent operation. 
It is not an error to commit a transaction more than once.

If the transaction can be committed, *HTTP 200* will be returned. 
The returned JSON object has the following properties:

- *error*: boolean flag to indicate if an error occurred (*false*
  in this case)

- *code*: the HTTP status code

- *result*: result containing
    - *id*: the identifier of the transaction
    - *status*: containing the string 'committed'

If the transaction cannot be found, committing is not allowed or the 
transaction was aborted, the server
will respond with *HTTP 400*, *HTTP 404* or *HTTP 409*.

The body of the response will then contain a JSON object with additional error
details. The object has the following attributes:

- *error*: boolean flag to indicate that an error occurred (*true* in this case)

- *code*: the HTTP status code

- *errorNum*: the server error number

- *errorMessage*: a descriptive error message



)";


  ApiResult request(std::string const &databaseName);
};
}
namespace GraphTraversal {
namespace PostTraversal {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Post;
  static constexpr auto url = "/_api/traversal";
  static constexpr auto summery = "executes a traversal";
  static constexpr auto operationId = "(executesATraversal";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Graph Traversal/HTTP_API_TRAVERSAL.md";
  static constexpr auto x_hints = R"({% hint 'warning' %}
This route should no longer be used.
It is considered as deprecated from version 3.4.0 on.
It is superseded by AQL graph traversal.
{% endhint %}

)";
  inline const static std::vector<char const * > tags{"Graph Traversal"};
  inline Parameter P0;
  inline void initParam(){
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"$ref":"#/definitions/HTTP_API_TRAVERSAL"})";
    P0.xDescriptionOffset = 214;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PostTraversalResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400,
    Response404 = 404,
    Response500 = 500
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(If the traversal is fully executed
*HTTP 200* will be returned.)"}, 
{Response400, R"(If the traversal specification is either missing or malformed, the server
will respond with *HTTP 400*.)"}, 
{Response404, R"(The server will responded with *HTTP 404* if the specified edge collection
does not exist, or the specified start vertex cannot be found.)"}, 
{Response500, R"(The server will responded with *HTTP 500* when an error occurs inside the
traversal or if a traversal performs more than *maxIterations* iterations.)"}};
  static constexpr auto description = R"(Starts a traversal starting from a given vertex and following.
edges contained in a given edgeCollection. The request must
contain the following attributes.


**A JSON object with these properties is required:**

  - **sort**: body (JavaScript) code of a custom comparison function
   for the edges. The signature of this function is
   *(l, r) -> integer* (where l and r are edges) and must
   return -1 if l is smaller than, +1 if l is greater than,
   and 0 if l and r are equal. The reason for this is the
   following: The order of edges returned for a certain
   vertex is undefined. This is because there is no natural
   order of edges for a vertex with multiple connected edges.
   To explicitly define the order in which edges on the
   vertex are followed, you can specify an edge comparator
   function with this attribute. Note that the value here has
   to be a string to conform to the JSON standard, which in
   turn is parsed as function body on the server side. Furthermore
   note that this attribute is only used for the standard
   expanders. If you use your custom expander you have to
   do the sorting yourself within the expander code.
  - **direction**: direction for traversal
   - *if set*, must be either *"outbound"*, *"inbound"*, or *"any"*
   - *if not set*, the *expander* attribute must be specified
  - **minDepth**: ANDed with any existing filters):
   visits only nodes in at least the given depth
  - **startVertex**: id of the startVertex, e.g. *"users/foo"*.
  - **visitor**: body (JavaScript) code of custom visitor function
   function signature: *(config, result, vertex, path, connected) -> void*
   The visitor function can do anything, but its return value is ignored. To
   populate a result, use the *result* variable by reference. Note that the
   *connected* argument is only populated when the *order* attribute is set
   to *"preorder-expander"*.
  - **itemOrder**: item iteration order can be *"forward"* or *"backward"*
  - **strategy**: traversal strategy can be *"depthfirst"* or *"breadthfirst"*
  - **filter**: default is to include all nodes:
   body (JavaScript code) of custom filter function
   function signature: *(config, vertex, path) -> mixed*
   can return four different string values:
   - *"exclude"* -> this vertex will not be visited.
   - *"prune"* -> the edges of this vertex will not be followed.
   - *""* or *undefined* -> visit the vertex and follow its edges.
   - *Array* -> containing any combination of the above.
     If there is at least one *"exclude"* or *"prune"* respectively
     is contained, it's effect will occur.
  - **init**: body (JavaScript) code of custom result initialization function
   function signature: *(config, result) -> void*
   initialize any values in result with what is required
  - **maxIterations**: Maximum number of iterations in each traversal. This number can be
   set to prevent endless loops in traversal of cyclic graphs. When a traversal performs
   as many iterations as the *maxIterations* value, the traversal will abort with an
   error. If *maxIterations* is not set, a server-defined value may be used.
  - **maxDepth**: ANDed with any existing filters visits only nodes in at most the given depth
  - **uniqueness**: specifies uniqueness for vertices and edges visited.
   If set, must be an object like this:
   `"uniqueness": {"vertices": "none"|"global"|"path", "edges": "none"|"global"|"path"}`
  - **order**: traversal order can be *"preorder"*, *"postorder"* or *"preorder-expander"*
  - **graphName**: name of the graph that contains the edges.
   Either *edgeCollection* or *graphName* has to be given.
   In case both values are set the *graphName* is preferred.
  - **expander**: body (JavaScript) code of custom expander function
   *must* be set if *direction* attribute is **not** set
   function signature: *(config, vertex, path) -> array*
   expander must return an array of the connections for *vertex*
   each connection is an object with the attributes *edge* and *vertex*
  - **edgeCollection**: name of the collection that contains the edges.





If the Traversal is successfully executed *HTTP 200* will be returned.
Additionally the *result* object will be returned by the traversal.

For successful traversals, the returned JSON object has the
following properties:

- *error*: boolean flag to indicate if an error occurred (*false*
  in this case)

- *code*: the HTTP status code

- *result*: the return value of the traversal

If the traversal specification is either missing or malformed, the server
will respond with *HTTP 400*.

The body of the response will then contain a JSON object with additional error
details. The object has the following attributes:

- *error*: boolean flag to indicate that an error occurred (*true* in this case)

- *code*: the HTTP status code

- *errorNum*: the server error number

- *errorMessage*: a descriptive error message


<!-- Hints Start -->

**Warning:**  
This route should no longer be used.
It is considered as deprecated from version 3.4.0 on.
It is superseded by AQL graph traversal.



<!-- Hints End -->
)";


  ApiResult request(std::string const &databaseName);
};
}
namespace UserManagement {
namespace PostUser {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Post;
  static constexpr auto url = "/_api/user";
  static constexpr auto summery = "Create User";
  static constexpr auto operationId = "(CreateUser";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/User Management/README.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"User Management"};
  inline Parameter P0;
  inline void initParam(){
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"$ref":"#/definitions/UserHandling_create"})";
    P0.xDescriptionOffset = 54;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PostUserResponse : short {
    Unknown = 0,
    Response201 = 201,
    Response400 = 400,
    Response401 = 401,
    Response403 = 403,
    Response409 = 409
  };
  inline const static std::map<int, char const * > responses{{Response201, R"(Returned if the user can be added by the server)"},
{Response400, R"(If the JSON representation is malformed or mandatory data is missing
from the request.)"}, 
{Response401, R"(Returned if you have *No access* database access level to the *_system*
database.)"}, 
{Response403, R"(Returned if you have *No access* server access level.)"}, 
{Response409, R"(Returned if a user with the same name already exists.)"}};
  static constexpr auto description = R"(
**A JSON object with these properties is required:**

  - **passwd**: The user password as a string. If no password is specified, the empty string
   will be used. If you pass the special value *ARANGODB_DEFAULT_ROOT_PASSWORD*,
   then the password will be set the value stored in the environment variable
   `ARANGODB_DEFAULT_ROOT_PASSWORD`. This can be used to pass an instance
   variable into ArangoDB. For example, the instance identifier from Amazon.
  - **active**: An optional flag that specifies whether the user is active.  If not
   specified, this will default to true
  - **user**: The name of the user as a string. This is mandatory.
  - **extra**: An optional JSON object with arbitrary extra data about the user.




Create a new user. You need server access level *Administrate* in order to
execute this REST call.


)";


  ApiResult request(std::string const &databaseName);
};
namespace Get {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/user/";
  static constexpr auto summery = "List available Users";
  static constexpr auto operationId = "(ListAvailableUsers";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/User Management/README.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"User Management"};
  
  enum GetResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response401 = 401,
    Response403 = 403
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(The users that were found.)"},
{Response401, R"(Returned if you have *No access* database access level to the *_system*
database.)"}, 
{Response403, R"(Returned if you have *No access* server access level.)"}};
  static constexpr auto description = R"(Fetches data about all users.  You need the *Administrate* server access level
in order to execute this REST call.  Otherwise, you will only get information
about yourself.

The call will return a JSON object with at least the following
attributes on success:

- *user*: The name of the user as a string.
- *active*: An optional flag that specifies whether the user is active.
- *extra*: An optional JSON object with arbitrary extra data about the user.


)";


  ApiResult request(std::string const &databaseName);
};
namespace DeleteUser {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Delete;
  static constexpr auto url = "/_api/user/{user}";
  static constexpr auto summery = "Remove User";
  static constexpr auto operationId = "(RemoveUser";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/User Management/README.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"User Management"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The name of the user)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "user";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum DeleteUserResponse : short {
    Unknown = 0,
    Response202 = 202,
    Response401 = 401,
    Response403 = 403,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response202, R"(Is returned if the user was removed by the server)"},
{Response401, R"(Returned if you have *No access* database access level to the *_system*
database.)"}, 
{Response403, R"(Returned if you have *No access* server access level.)"}, 
{Response404, R"(The specified user does not exist)"}};
  static constexpr auto description = R"(Removes an existing user, identified by *user*.  You need *Administrate* for
the server access level in order to execute this REST call.


)";


  ApiResult request(std::string const &databaseName);
};
namespace GetUser {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/user/{user}";
  static constexpr auto summery = "Fetch User";
  static constexpr auto operationId = "(FetchUser";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/User Management/README.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"User Management"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The name of the user)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "user";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum GetUserResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response401 = 401,
    Response403 = 403,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(The user was found.)"},
{Response401, R"(Returned if you have *No access* database access level to the *_system*
database.)"}, 
{Response403, R"(Returned if you have *No access* server access level.)"}, 
{Response404, R"(The user with the specified name does not exist.)"}};
  static constexpr auto description = R"(Fetches data about the specified user. You can fetch information about
yourself or you need the *Administrate* server access level in order to
execute this REST call.


)";


  ApiResult request(std::string const &databaseName);
};
namespace PatchUser {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Patch;
  static constexpr auto url = "/_api/user/{user}";
  static constexpr auto summery = "Modify User";
  static constexpr auto operationId = "(ModifyUser";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/User Management/README.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"User Management"};
  inline Parameter P0;
  inline Parameter P1;
  inline void initParam(){
    P0.description = R"(The name of the user)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "user";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.in = ParamIn::body;
    P1.name = "Json Request Body";
    P1.required = true;
    P1.schema = R"({"$ref":"#/definitions/UserHandling_modify"})";
    P1.xDescriptionOffset = 54;
  }
  inline std::vector<Parameter> parameters{P0, P1};

  enum PatchUserResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400,
    Response401 = 401,
    Response403 = 403,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Is returned if the user data can be replaced by the server.)"},
{Response400, R"(The JSON representation is malformed or mandatory data is missing from the request.)"}, 
{Response401, R"(Returned if you have *No access* database access level to the *_system*
database.)"}, 
{Response403, R"(Returned if you have *No access* server access level.)"}, 
{Response404, R"(The specified user does not exist)"}};
  static constexpr auto description = R"(
**A JSON object with these properties is required:**

  - **passwd**: The user password as a string. Specifying a password is mandatory, but
   the empty string is allowed for passwords
  - **active**: An optional flag that specifies whether the user is active.  If not
   specified, this will default to true
  - **extra**: An optional JSON object with arbitrary extra data about the user.




Partially updates the data of an existing user. The name of an existing user
must be specified in *user*. You need server access level *Administrate* in
order to execute this REST call. Additionally, a user can change his/her own
data.


)";


  ApiResult request(std::string const &databaseName);
};
namespace PutUser {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/user/{user}";
  static constexpr auto summery = "Replace User";
  static constexpr auto operationId = "(ReplaceUser";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/User Management/README.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"User Management"};
  inline Parameter P0;
  inline Parameter P1;
  inline void initParam(){
    P0.description = R"(The name of the user)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "user";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.in = ParamIn::body;
    P1.name = "Json Request Body";
    P1.required = true;
    P1.schema = R"({"$ref":"#/definitions/UserHandling_replace"})";
    P1.xDescriptionOffset = 54;
  }
  inline std::vector<Parameter> parameters{P0, P1};

  enum PutUserResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400,
    Response401 = 401,
    Response403 = 403,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Is returned if the user data can be replaced by the server.)"},
{Response400, R"(The JSON representation is malformed or mandatory data is missing from the request)"}, 
{Response401, R"(Returned if you have *No access* database access level to the *_system*
database.)"}, 
{Response403, R"(Returned if you have *No access* server access level.)"}, 
{Response404, R"(The specified user does not exist)"}};
  static constexpr auto description = R"(
**A JSON object with these properties is required:**

  - **passwd**: The user password as a string. Specifying a password is mandatory, but
   the empty string is allowed for passwords
  - **active**: An optional flag that specifies whether the user is active.  If not
   specified, this will default to true
  - **extra**: An optional JSON object with arbitrary extra data about the user.




Replaces the data of an existing user. The name of an existing user must be
specified in *user*. You need server access level *Administrate* in order to
execute this REST call. Additionally, a user can change his/her own data.


)";


  ApiResult request(std::string const &databaseName);
};
namespace GetDatabases {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/user/{user}/database/";
  static constexpr auto summery = "List the accessible databases for a user";
  static constexpr auto operationId = "(ListTheAccessibleDatabasesForAUser";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/User Management/README.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"User Management"};
  inline Parameter P0;
  inline Parameter P1;
  inline void initParam(){
    P0.description = R"(The name of the user for which you want to query the databases.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "user";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.description = R"(Return the full set of access levels for all databases and all collections.)";
    P1.in = ParamIn::query;
    P1.name = "full";
    P1.required = false;
    P1.type = ParamFormat::booleanFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1};

  enum GetResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400,
    Response401 = 401,
    Response403 = 403
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Returned if the list of available databases can be returned.)"},
{Response400, R"(If the access privileges are not right etc.)"}, 
{Response401, R"(Returned if you have *No access* database access level to the *_system*
database.)"}, 
{Response403, R"(Returned if you have *No access* server access level.)"}};
  static constexpr auto description = R"(Fetch the list of databases available to the specified *user*. You need
*Administrate* for the server access level in order to execute this REST call.

The call will return a JSON object with the per-database access
privileges for the specified user. The *result* object will contain
the databases names as object keys, and the associated privileges
for the database as values.

In case you specified *full*, the result will contain the permissions
for the databases as well as the permissions for the collections.


)";


  ApiResult request(std::string const &databaseName);
};
namespace GetDatabase {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/user/{user}/database/{database}";
  static constexpr auto summery = "Get the database access level";
  static constexpr auto operationId = "(GetTheDatabaseAccessLevel";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/User Management/README.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"User Management"};
  inline Parameter P0;
  inline Parameter P1;
  inline void initParam(){
    P0.description = R"(The name of the user for which you want to query the databases.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "user";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.description = R"(The name of the database to query)";
    P1.format = ParamFormat::stringFormat;
    P1.in = ParamIn::path;
    P1.name = "database";
    P1.required = true;
    P1.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1};

  enum GetDatabaseResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400,
    Response401 = 401,
    Response403 = 403
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Returned if the acccess level can be returned)"},
{Response400, R"(If the access privileges are not right etc.)"}, 
{Response401, R"(Returned if you have *No access* database access level to the *_system*
database.)"}, 
{Response403, R"(Returned if you have *No access* server access level.)"}};
  static constexpr auto description = R"(Fetch the database access level for a specific database


)";


  ApiResult request(std::string const &databaseName);
};
namespace GetCollection {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/user/{user}/database/{database}/{collection}";
  static constexpr auto summery = "Get the specific collection access level";
  static constexpr auto operationId = "(GetTheSpecificCollectionAccessLevel";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/User Management/README.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"User Management"};
  inline Parameter P0;
  inline Parameter P1;
  inline Parameter P2;
  inline void initParam(){
    P0.description = R"(The name of the user for which you want to query the databases.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "user";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.description = R"(The name of the database to query)";
    P1.format = ParamFormat::stringFormat;
    P1.in = ParamIn::path;
    P1.name = "database";
    P1.required = true;
    P1.type = ParamFormat::stringFormat;

    P2.description = R"(The name of the collection)";
    P2.format = ParamFormat::stringFormat;
    P2.in = ParamIn::path;
    P2.name = "collection";
    P2.required = true;
    P2.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1, P2};

  enum GetCollectionResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400,
    Response401 = 401,
    Response403 = 403
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Returned if the acccess level can be returned)"},
{Response400, R"(If the access privileges are not right etc.)"}, 
{Response401, R"(Returned if you have *No access* database access level to the *_system*
database.)"}, 
{Response403, R"(Returned if you have *No access* server access level.)"}};
  static constexpr auto description = R"(Returns the collection access level for a specific collection


)";


  ApiResult request(std::string const &databaseName);
};
namespace DeleteDbname {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Delete;
  static constexpr auto url = "/_api/user/{user}/database/{dbname}";
  static constexpr auto summery = "Clear the database access level";
  static constexpr auto operationId = "(ClearTheDatabaseAccessLevel";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/User Management/README.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"User Management"};
  inline Parameter P0;
  inline Parameter P1;
  inline void initParam(){
    P0.description = R"(The name of the user.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "user";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.description = R"(The name of the database.)";
    P1.format = ParamFormat::stringFormat;
    P1.in = ParamIn::path;
    P1.name = "dbname";
    P1.required = true;
    P1.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1};

  enum DeleteDbnameResponse : short {
    Unknown = 0,
    Response202 = 202,
    Response400 = 400
  };
  inline const static std::map<int, char const * > responses{{Response202, R"(Returned if the access permissions were changed successfully.)"},
{Response400, R"(If the JSON representation is malformed or mandatory data is missing
from the request.)"}};
  static constexpr auto description = R"(Clears the database access level for the database *dbname* of user *user*. As
consequence the default database access level is used. If there is no defined
default database access level, it defaults to *No access*. You need permission
to the *_system* database in order to execute this REST call.


)";


  ApiResult request(std::string const &databaseName);
};
namespace PutDbname {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/user/{user}/database/{dbname}";
  static constexpr auto summery = "Set the database access level";
  static constexpr auto operationId = "(SetTheDatabaseAccessLevel";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/User Management/README.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"User Management"};
  inline Parameter P0;
  inline Parameter P1;
  inline Parameter P2;
  inline void initParam(){
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"$ref":"#/definitions/UserHandling_grantDatabase"})";
    P0.xDescriptionOffset = 54;

    P1.description = R"(The name of the user.)";
    P1.format = ParamFormat::stringFormat;
    P1.in = ParamIn::path;
    P1.name = "user";
    P1.required = true;
    P1.type = ParamFormat::stringFormat;

    P2.description = R"(The name of the database.)";
    P2.format = ParamFormat::stringFormat;
    P2.in = ParamIn::path;
    P2.name = "dbname";
    P2.required = true;
    P2.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1, P2};

  enum PutDbnameResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400,
    Response401 = 401,
    Response403 = 403
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Returned if the access level was changed successfully.)"},
{Response400, R"(If the JSON representation is malformed or mandatory data is missing
from the request.)"}, 
{Response401, R"(Returned if you have *No access* database access level to the *_system*
database.)"}, 
{Response403, R"(Returned if you have *No access* server access level.)"}};
  static constexpr auto description = R"(
**A JSON object with these properties is required:**

  - **grant**: Use "rw" to set the database access level to *Administrate* .
   Use "ro" to set the database access level to *Access*.
   Use "none" to set the database access level to *No access*.




Sets the database access levels for the database *dbname* of user *user*. You
need the *Administrate* server access level in order to execute this REST
call.


)";


  ApiResult request(std::string const &databaseName);
};
namespace DeleteCollection {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Delete;
  static constexpr auto url = "/_api/user/{user}/database/{dbname}/{collection}";
  static constexpr auto summery = "Clear the collection access level";
  static constexpr auto operationId = "(ClearTheCollectionAccessLevel";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/User Management/README.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"User Management"};
  inline Parameter P0;
  inline Parameter P1;
  inline Parameter P2;
  inline void initParam(){
    P0.description = R"(The name of the user.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "user";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.description = R"(The name of the database.)";
    P1.format = ParamFormat::stringFormat;
    P1.in = ParamIn::path;
    P1.name = "dbname";
    P1.required = true;
    P1.type = ParamFormat::stringFormat;

    P2.description = R"(The name of the collection.)";
    P2.format = ParamFormat::stringFormat;
    P2.in = ParamIn::path;
    P2.name = "collection";
    P2.required = true;
    P2.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1, P2};

  enum DeleteCollectionResponse : short {
    Unknown = 0,
    Response202 = 202,
    Response400 = 400
  };
  inline const static std::map<int, char const * > responses{{Response202, R"(Returned if the access permissions were changed successfully.)"},
{Response400, R"(If there was an error)"}};
  static constexpr auto description = R"(Clears the collection access level for the collection *collection* in the
database *dbname* of user *user*.  As consequence the default collection
access level is used. If there is no defined default collection access level,
it defaults to *No access*.  You need permissions to the *_system* database in
order to execute this REST call.


)";


  ApiResult request(std::string const &databaseName);
};
namespace PutCollection {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/user/{user}/database/{dbname}/{collection}";
  static constexpr auto summery = "Set the collection access level";
  static constexpr auto operationId = "(SetTheCollectionAccessLevel";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/User Management/README.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"User Management"};
  inline Parameter P0;
  inline Parameter P1;
  inline Parameter P2;
  inline Parameter P3;
  inline void initParam(){
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"$ref":"#/definitions/UserHandling_grantCollection"})";
    P0.xDescriptionOffset = 54;

    P1.description = R"(The name of the user.)";
    P1.format = ParamFormat::stringFormat;
    P1.in = ParamIn::path;
    P1.name = "user";
    P1.required = true;
    P1.type = ParamFormat::stringFormat;

    P2.description = R"(The name of the database.)";
    P2.format = ParamFormat::stringFormat;
    P2.in = ParamIn::path;
    P2.name = "dbname";
    P2.required = true;
    P2.type = ParamFormat::stringFormat;

    P3.description = R"(The name of the collection.)";
    P3.format = ParamFormat::stringFormat;
    P3.in = ParamIn::path;
    P3.name = "collection";
    P3.required = true;
    P3.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1, P2, P3};

  enum PutCollectionResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response400 = 400,
    Response401 = 401,
    Response403 = 403
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(Returned if the access permissions were changed successfully.)"},
{Response400, R"(If the JSON representation is malformed or mandatory data is missing
from the request.)"}, 
{Response401, R"(Returned if you have *No access* database access level to the *_system*
database.)"}, 
{Response403, R"(Returned if you have *No access* server access level.)"}};
  static constexpr auto description = R"(
**A JSON object with these properties is required:**

  - **grant**: Use "rw" to set the collection level access to *Read/Write*.
   Use "ro" to set the collection level access to  *Read Only*.
   Use "none" to set the collection level access to *No access*.




Sets the collection access level for the *collection* in the database *dbname*
for user *user*. You need the *Administrate* server access level in order to
execute this REST call.


)";


  ApiResult request(std::string const &databaseName);
};
}
namespace Administration {
namespace GetVersion {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/version";
  static constexpr auto summery = "Return server version";
  static constexpr auto operationId = "(RestVersionHandler";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Administration/get_api_return.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Administration"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(If set to *true*, the response will contain a *details* attribute with
additional information about included components and their versions. The
attribute names and internals of the *details* object may vary depending on
platform and ArangoDB version.)";
    P0.in = ParamIn::query;
    P0.name = "details";
    P0.required = false;
    P0.type = ParamFormat::booleanFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum GetVersionResponse : short {
    Unknown = 0,
    Response200 = 200
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned in all cases.)"}};
  static constexpr auto description = R"(Returns the server name and version number. The response is a JSON object
with the following attributes:


**HTTP 200**
*A json document with these Properties is returned:*

is returned in all cases.

- **version**: the server version string. The string has the format
"*major*.*minor*.*sub*". *major* and *minor* will be numeric, and *sub*
may contain a number or a textual version.
- **details**:
  - **failure-tests**: *false* for production binaries (the facility to invoke fatal errors is disabled)
  - **boost-version**: which boost version do we bind
  - **zlib-version**: the version of the bundled zlib
  - **sse42**: do we have a SSE 4.2 enabled cpu?
  - **assertions**: do we have assertions compiled in (=> developer version)
  - **jemalloc**: *true* if we use jemalloc
  - **arm**: *false* - this is not running on an ARM cpu
  - **rocksdb-version**: the rocksdb version this release bundles
  - **cplusplus**: C++ standards version
  - **sizeof int**: number of bytes for *integers*
  - **openssl-version**: which openssl version do we link?
  - **platform**: the host os - *linux*, *windows* or *darwin*
  - **endianness**: currently only *little* is supported
  - **vpack-version**: the version of the used velocypack implementation
  - **icu-version**: Which version of ICU do we bundle
  - **mode**: the mode we're runnig as - one of [*server*, *console*, *script*]
  - **unaligned-access**: does this system support unaligned memory access?
  - **build-repository**: reference to the git-ID this was compiled from
  - **asm-crc32**: do we have assembler implemented CRC functions?
  - **host**: the host ID
  - **fd-setsize**: if not *poll* the fd setsize is valid for the maximum number of filedescriptors
  - **maintainer-mode**: *false* if this is a production binary
  - **sizeof void***: number of bytes for *void pointers*
  - **asan**: has this been compiled with the asan address sanitizer turned on? (should be false)
  - **build-date**: the date when this binary was created
  - **compiler**: which compiler did we use
  - **server-version**: the ArangoDB release version
  - **fd-client-event-handler**: which method do we use to handle fd-sets, *poll* should be here on linux.
  - **reactor-type**: *epoll* TODO 
  - **architecture**: The CPU architecture, i.e. *64bit*
  - **debug**: *false* for production binaries
  - **full-version-string**: The full version string
  - **v8-version**: the bundled V8 javascript engine version
- **server**: will always contain *arango*


)";
  ApiResult request();
}
}
namespace View {
namespace GetView {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/view";
  static constexpr auto summery = "List all views";
  static constexpr auto operationId = "(getViews:AllViews";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Views/get_api_views.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Views"};
  
  enum GetViewResponse : short {
    Unknown = 0,
    Response200 = 200
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(The list of views)"}};
  static constexpr auto description = R"(Returns an object containing an array of all view descriptions.


)";


  ApiResult request(std::string const &databaseName);
};
namespace PostViewArangoSearch {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Post;
  static constexpr auto url = "/_api/view#ArangoSearch";
  static constexpr auto summery = "Create an ArangoSearch view";
  static constexpr auto operationId = "(createView";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Views/post_api_view_iresearch.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Views"};
  inline Parameter P0;
  inline void initParam(){
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"$ref":"#/definitions/post_api_view_iresearch"})";
    P0.xDescriptionOffset = 54;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PostViewArangoSearchResponse : short {
    Unknown = 0,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response400, R"(If the *view-name* is missing, then a *HTTP 400* is returned.)"},
{Response404, R"(If the *view-name* is unknown, then a *HTTP 404* is returned.)"}};
  static constexpr auto description = R"(
**A JSON object with these properties is required:**

  - **type**: The type of the view. must be equal to *"arangosearch"*
  - **name**: The name of the view.
  - **properties**:
    - **commitIntervalMsec**: Wait at least this many milliseconds between committing view data store
    changes and making documents visible to queries (default: 1000, to disable
    use: 0).
    For the case where there are a lot of inserts/updates, a lower value, until
    commit, will cause the index not to account for them and memory usage would
    continue to grow.
    For the case where there are a few inserts/updates, a higher value will impact
    performance and waste disk space for each commit call without any added
    benefits.<br/>
    _Background:_
      For data retrieval ArangoSearch views follow the concept of
      "eventually-consistent", i.e. eventually all the data in ArangoDB will be
      matched by corresponding query expressions.
      The concept of ArangoSearch view "commit" operation is introduced to
      control the upper-bound on the time until document addition/removals are
      actually reflected by corresponding query expressions.
      Once a "commit" operation is complete all documents added/removed prior to
      the start of the "commit" operation will be reflected by queries invoked in
      subsequent ArangoDB transactions, in-progress ArangoDB transactions will
      still continue to return a repeatable-read state.
    - **links**:
      - **[collection-name]**:
        - **analyzers** (string): The list of analyzers to be used for indexing of string values
      (default: ["identity"]).
        - **fields**:
          - **field-name** (object): This is a recursive structure for the specific attribute path, potentially
       containing any of the following attributes:
       *analyzers*, *includeAllFields*, *trackListPositions*, *storeValues*
       Any attributes not specified are inherited from the parent.
        - **includeAllFields**: The flag determines whether or not to index all fields on a particular level of
      depth (default: false).
        - **trackListPositions**: The flag determines whether or not values in a lists should be treated separate
      (default: false).
        - **storeValues**: How should the view track the attribute values, this setting allows for
      additional value retrieval optimizations, one of:
      - *none*: Do not store values by the view
      - *id*: Store only information about value presence, to allow use of the EXISTS() function
      (default "none").
    - **consolidationIntervalMsec**: Wait at least this many milliseconds between applying 'consolidationPolicy' to
    consolidate view data store and possibly release space on the filesystem
    (default: 60000, to disable use: 0).
    For the case where there are a lot of data modification operations, a higher
    value could potentially have the data store consume more space and file handles.
    For the case where there are a few data modification operations, a lower value
    will impact performance due to no segment candidates available for
    consolidation.<br/>
    _Background:_
      For data modification ArangoSearch views follow the concept of a
      "versioned data store". Thus old versions of data may be removed once there
      are no longer any users of the old data. The frequency of the cleanup and
      compaction operations are governed by 'consolidationIntervalMsec' and the
      candidates for compaction are selected via 'consolidationPolicy'.
    - **cleanupIntervalStep**: Wait at least this many commits between removing unused files in the
    ArangoSearch data directory (default: 10, to disable use: 0).
    For the case where the consolidation policies merge segments often (i.e. a lot
    of commit+consolidate), a lower value will cause a lot of disk space to be
    wasted.
    For the case where the consolidation policies rarely merge segments (i.e. few
    inserts/deletes), a higher value will impact performance without any added
    benefits.<br/>
    _Background:_
      With every "commit" or "consolidate" operation a new state of the view
      internal data-structures is created on disk.
      Old states/snapshots are released once there are no longer any users
      remaining.
      However, the files for the released states/snapshots are left on disk, and
      only removed by "cleanup" operation.
    - **consolidationPolicy**:
      - **type**: The segment candidates for the "consolidation" operation are selected based
     upon several possible configurable formulas as defined by their types.
     The currently supported types are (default: "bytes_accum"):
     - *bytes_accum*: consolidate if and only if (`{threshold}` range `[0.0, 1.0]`):
       `{threshold} > (segment_bytes + sum_of_merge_candidate_segment_bytes) / all_segment_bytes`
       i.e. the sum of all candidate segment byte size is less than the total
       segment byte size multiplied by the `{threshold}`
     - *tier*: consolidate based on segment byte size and live document count
       as dictated by the customization attributes.




Creates a new view with a given name and properties if it does not
already exist.

**Note**: view can't be created with the links. Please use PUT/PATCH for links
management.


)";


  ApiResult request(std::string const &databaseName);
};
namespace PostViewArangosearch {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Post;
  static constexpr auto url = "/_api/view#arangosearch";
  static constexpr auto summery = "Create an ArangoDB view";
  static constexpr auto operationId = "(createView:Create";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Views/post_api_view.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Views"};
  inline Parameter P0;
  inline void initParam(){
    P0.in = ParamIn::body;
    P0.name = "Json Request Body";
    P0.required = true;
    P0.schema = R"({"$ref":"#/definitions/post_api_view"})";
    P0.xDescriptionOffset = 54;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PostViewArangosearchResponse : short {
    Unknown = 0,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response400, R"(If the *view-name* is missing, then a *HTTP 400* is
returned.)"}, 
{Response404, R"(If the *view-name* is unknown, then a *HTTP 404* is returned.)"}};
  static constexpr auto description = R"(
**A JSON object with these properties is required:**

  - **type**: The type of the view. must be equal to one of the supported ArangoDB view
   types.
  - **name**: The name of the view.
  - **properties**:
    - **commitIntervalMsec**: Wait at least this many milliseconds between committing view data store
    changes and making documents visible to queries (default: 1000, to disable
    use: 0).
    For the case where there are a lot of inserts/updates, a lower value, until
    commit, will cause the index not to account for them and memory usage would
    continue to grow.
    For the case where there are a few inserts/updates, a higher value will impact
    performance and waste disk space for each commit call without any added
    benefits.<br/>
    _Background:_
      For data retrieval ArangoSearch views follow the concept of
      "eventually-consistent", i.e. eventually all the data in ArangoDB will be
      matched by corresponding query expressions.
      The concept of ArangoSearch view "commit" operation is introduced to
      control the upper-bound on the time until document addition/removals are
      actually reflected by corresponding query expressions.
      Once a "commit" operation is complete all documents added/removed prior to
      the start of the "commit" operation will be reflected by queries invoked in
      subsequent ArangoDB transactions, in-progress ArangoDB transactions will
      still continue to return a repeatable-read state.
    - **links**:
      - **[collection-name]**:
        - **analyzers** (string): The list of analyzers to be used for indexing of string values
      (default: ["identity"]).
        - **fields**:
          - **field-name** (object): This is a recursive structure for the specific attribute path, potentially
       containing any of the following attributes:
       *analyzers*, *includeAllFields*, *trackListPositions*, *storeValues*
       Any attributes not specified are inherited from the parent.
        - **includeAllFields**: The flag determines whether or not to index all fields on a particular level of
      depth (default: false).
        - **trackListPositions**: The flag determines whether or not values in a lists should be treated separate
      (default: false).
        - **storeValues**: How should the view track the attribute values, this setting allows for
      additional value retrieval optimizations, one of:
      - *none*: Do not store values by the view
      - *id*: Store only information about value presence, to allow use of the EXISTS() function
      (default "none").
    - **consolidationIntervalMsec**: Wait at least this many milliseconds between applying 'consolidationPolicy' to
    consolidate view data store and possibly release space on the filesystem
    (default: 60000, to disable use: 0).
    For the case where there are a lot of data modification operations, a higher
    value could potentially have the data store consume more space and file handles.
    For the case where there are a few data modification operations, a lower value
    will impact performance due to no segment candidates available for
    consolidation.<br/>
    _Background:_
      For data modification ArangoSearch views follow the concept of a
      "versioned data store". Thus old versions of data may be removed once there
      are no longer any users of the old data. The frequency of the cleanup and
      compaction operations are governed by 'consolidationIntervalMsec' and the
      candidates for compaction are selected via 'consolidationPolicy'.
    - **cleanupIntervalStep**: Wait at least this many commits between removing unused files in the
    ArangoSearch data directory (default: 10, to disable use: 0).
    For the case where the consolidation policies merge segments often (i.e. a lot
    of commit+consolidate), a lower value will cause a lot of disk space to be
    wasted.
    For the case where the consolidation policies rarely merge segments (i.e. few
    inserts/deletes), a higher value will impact performance without any added
    benefits.<br/>
    _Background:_
      With every "commit" or "consolidate" operation a new state of the view
      internal data-structures is created on disk.
      Old states/snapshots are released once there are no longer any users
      remaining.
      However, the files for the released states/snapshots are left on disk, and
      only removed by "cleanup" operation.
    - **consolidationPolicy**:
      - **type**: The segment candidates for the "consolidation" operation are selected based
     upon several possible configurable formulas as defined by their types.
     The currently supported types are (default: "bytes_accum"):
     - *bytes_accum*: consolidate if and only if (`{threshold}` range `[0.0, 1.0]`):
       `{threshold} > (segment_bytes + sum_of_merge_candidate_segment_bytes) / all_segment_bytes`
       i.e. the sum of all candidate segment byte size is less than the total
       segment byte size multiplied by the `{threshold}`
     - *tier*: consolidate based on segment byte size and live document count
       as dictated by the customization attributes.




Creates a new view with a given name and properties if it does not already
exist.

**Note**: view can't be created with the links. Please use PUT/PATCH
for links management.


)";


  ApiResult request(std::string const &databaseName);
};
namespace DeleteViewName {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Delete;
  static constexpr auto url = "/_api/view/{view-name}";
  static constexpr auto summery = "Drops a view";
  static constexpr auto operationId = "(deleteView";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Views/delete_api_view.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Views"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The name of the view to drop.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "view-name";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum DeleteViewNameResponse : short {
    Unknown = 0,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response400, R"(If the *view-name* is missing, then a *HTTP 400* is returned.)"},
{Response404, R"(If the *view-name* is unknown, then a *HTTP 404* is returned.)"}};
  static constexpr auto description = R"(Drops the view identified by *view-name*.

If the view was successfully dropped, an object is returned with
the following attributes:
- *error*: *false*
- *id*: The identifier of the dropped view


)";


  ApiResult request(std::string const &databaseName);
};
namespace GetViewName {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/view/{view-name}";
  static constexpr auto summery = "Return information about a view";
  static constexpr auto operationId = "(getViews:Properties";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Views/get_api_view_name.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Views"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The name of the view.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "view-name";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum GetViewNameResponse : short {
    Unknown = 0,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response404, R"(If the *view-name* is unknown, then a *HTTP 404* is returned.)"}};
  static constexpr auto description = R"(The result is an object describing the view with the following attributes:
- *id*: The identifier of the view
- *name*: The name of the view
- *type*: The type of the view as string


)";


  ApiResult request(std::string const &databaseName);
};
namespace GetProperties {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/view/{view-name}/properties";
  static constexpr auto summery = "Read properties of a view";
  static constexpr auto operationId = "(getView";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Views/get_api_view_properties.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Views"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The name of the view.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "view-name";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum GetPropertiesResponse : short {
    Unknown = 0,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response400, R"(If the *view-name* is missing, then a *HTTP 400* is returned.)"},
{Response404, R"(If the *view-name* is unknown, then a *HTTP 404* is returned.)"}};
  static constexpr auto description = R"(Returns an object containing the definition of the view identified by *view-name*.



The result is an object describing the view with the following attributes:
- *id*: The identifier of the view
- *name*: The name of the view
- *type*: The type of the view as string
- any additional view implementation specific properties


)";


  ApiResult request(std::string const &databaseName);
};
namespace PatchPropertiesArangoSearch {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Patch;
  static constexpr auto url = "/_api/view/{view-name}/properties#ArangoSearch";
  static constexpr auto summery = "Partially changes properties of an ArangoSearch view";
  static constexpr auto operationId = "(modifyView";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Views/patch_api_view_properties_iresearch.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Views"};
  inline Parameter P0;
  inline Parameter P1;
  inline void initParam(){
    P0.description = R"(The name of the view.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "view-name";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.in = ParamIn::body;
    P1.name = "Json Request Body";
    P1.required = true;
    P1.schema = R"({"$ref":"#/definitions/patch_api_view_properties_iresearch"})";
    P1.xDescriptionOffset = 54;
  }
  inline std::vector<Parameter> parameters{P0, P1};

  enum PatchPropertiesArangoSearchResponse : short {
    Unknown = 0,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response400, R"(If the *view-name* is missing, then a *HTTP 400* is returned.)"},
{Response404, R"(If the *view-name* is unknown, then a *HTTP 404* is returned.)"}};
  static constexpr auto description = R"(
**A JSON object with these properties is required:**

  - **properties**:
    - **commitIntervalMsec**: Wait at least this many milliseconds between committing view data store
    changes and making documents visible to queries (default: 1000, to disable
    use: 0).
    For the case where there are a lot of inserts/updates, a lower value, until
    commit, will cause the index not to account for them and memory usage would
    continue to grow.
    For the case where there are a few inserts/updates, a higher value will impact
    performance and waste disk space for each commit call without any added
    benefits.<br/>
    _Background:_
      For data retrieval ArangoSearch views follow the concept of
      "eventually-consistent", i.e. eventually all the data in ArangoDB will be
      matched by corresponding query expressions.
      The concept of ArangoSearch view "commit" operation is introduced to
      control the upper-bound on the time until document addition/removals are
      actually reflected by corresponding query expressions.
      Once a "commit" operation is complete all documents added/removed prior to
      the start of the "commit" operation will be reflected by queries invoked in
      subsequent ArangoDB transactions, in-progress ArangoDB transactions will
      still continue to return a repeatable-read state.
    - **links**:
      - **[collection-name]**:
        - **analyzers** (string): The list of analyzers to be used for indexing of string values
      (default: ["identity"]).
        - **fields**:
          - **field-name** (object): This is a recursive structure for the specific attribute path, potentially
       containing any of the following attributes:
       *analyzers*, *includeAllFields*, *trackListPositions*, *storeValues*
       Any attributes not specified are inherited from the parent.
        - **includeAllFields**: The flag determines whether or not to index all fields on a particular level of
      depth (default: false).
        - **trackListPositions**: The flag determines whether or not values in a lists should be treated separate
      (default: false).
        - **storeValues**: How should the view track the attribute values, this setting allows for
      additional value retrieval optimizations, one of:
      - *none*: Do not store values by the view
      - *id*: Store only information about value presence, to allow use of the EXISTS() function
      (default "none").
    - **consolidationIntervalMsec**: Wait at least this many milliseconds between applying 'consolidationPolicy' to
    consolidate view data store and possibly release space on the filesystem
    (default: 60000, to disable use: 0).
    For the case where there are a lot of data modification operations, a higher
    value could potentially have the data store consume more space and file handles.
    For the case where there are a few data modification operations, a lower value
    will impact performance due to no segment candidates available for
    consolidation.<br/>
    _Background:_
      For data modification ArangoSearch views follow the concept of a
      "versioned data store". Thus old versions of data may be removed once there
      are no longer any users of the old data. The frequency of the cleanup and
      compaction operations are governed by 'consolidationIntervalMsec' and the
      candidates for compaction are selected via 'consolidationPolicy'.
    - **cleanupIntervalStep**: Wait at least this many commits between removing unused files in the
    ArangoSearch data directory (default: 10, to disable use: 0).
    For the case where the consolidation policies merge segments often (i.e. a lot
    of commit+consolidate), a lower value will cause a lot of disk space to be
    wasted.
    For the case where the consolidation policies rarely merge segments (i.e. few
    inserts/deletes), a higher value will impact performance without any added
    benefits.<br/>
    _Background:_
      With every "commit" or "consolidate" operation a new state of the view
      internal data-structures is created on disk.
      Old states/snapshots are released once there are no longer any users
      remaining.
      However, the files for the released states/snapshots are left on disk, and
      only removed by "cleanup" operation.
    - **consolidationPolicy**:
      - **type**: The segment candidates for the "consolidation" operation are selected based
     upon several possible configurable formulas as defined by their types.
     The currently supported types are (default: "bytes_accum"):
     - *bytes_accum*: consolidate if and only if (`{threshold}` range `[0.0, 1.0]`):
       `{threshold} > (segment_bytes + sum_of_merge_candidate_segment_bytes) / all_segment_bytes`
       i.e. the sum of all candidate segment byte size is less than the total
       segment byte size multiplied by the `{threshold}`
     - *tier*: consolidate based on segment byte size and live document count
       as dictated by the customization attributes.




Changes the properties of a view.

On success an object with the following attributes is returned:
- *id*: The identifier of the view
- *name*: The name of the view
- *type*: The view type
- all additional arangosearch view implementation specific properties


)";


  ApiResult request(std::string const &databaseName);
};
namespace PutPropertiesArangoSearch {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/view/{view-name}/properties#ArangoSearch";
  static constexpr auto summery = "Change properties of an ArangoSearch view";
  static constexpr auto operationId = "(modifyView";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Views/put_api_view_properties_iresearch.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Views"};
  inline Parameter P0;
  inline Parameter P1;
  inline void initParam(){
    P0.description = R"(The name of the view.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "view-name";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;

    P1.in = ParamIn::body;
    P1.name = "Json Request Body";
    P1.required = true;
    P1.schema = R"({"$ref":"#/definitions/put_api_view_properties_iresearch"})";
    P1.xDescriptionOffset = 54;
  }
  inline std::vector<Parameter> parameters{P0, P1};

  enum PutPropertiesArangoSearchResponse : short {
    Unknown = 0,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response400, R"(If the *view-name* is missing, then a *HTTP 400* is returned.)"},
{Response404, R"(If the *view-name* is unknown, then a *HTTP 404* is returned.)"}};
  static constexpr auto description = R"(
**A JSON object with these properties is required:**

  - **properties**:
    - **commitIntervalMsec**: Wait at least this many milliseconds between committing view data store
    changes and making documents visible to queries (default: 1000, to disable
    use: 0).
    For the case where there are a lot of inserts/updates, a lower value, until
    commit, will cause the index not to account for them and memory usage would
    continue to grow.
    For the case where there are a few inserts/updates, a higher value will impact
    performance and waste disk space for each commit call without any added
    benefits.<br/>
    _Background:_
      For data retrieval ArangoSearch views follow the concept of
      "eventually-consistent", i.e. eventually all the data in ArangoDB will be
      matched by corresponding query expressions.
      The concept of ArangoSearch view "commit" operation is introduced to
      control the upper-bound on the time until document addition/removals are
      actually reflected by corresponding query expressions.
      Once a "commit" operation is complete all documents added/removed prior to
      the start of the "commit" operation will be reflected by queries invoked in
      subsequent ArangoDB transactions, in-progress ArangoDB transactions will
      still continue to return a repeatable-read state.
    - **links**:
      - **[collection-name]**:
        - **analyzers** (string): The list of analyzers to be used for indexing of string values
      (default: ["identity"]).
        - **fields**:
          - **field-name** (object): This is a recursive structure for the specific attribute path, potentially
       containing any of the following attributes:
       *analyzers*, *includeAllFields*, *trackListPositions*, *storeValues*
       Any attributes not specified are inherited from the parent.
        - **includeAllFields**: The flag determines whether or not to index all fields on a particular level of
      depth (default: false).
        - **trackListPositions**: The flag determines whether or not values in a lists should be treated separate
      (default: false).
        - **storeValues**: How should the view track the attribute values, this setting allows for
      additional value retrieval optimizations, one of:
      - *none*: Do not store values by the view
      - *id*: Store only information about value presence, to allow use of the EXISTS() function
      (default "none").
    - **consolidationIntervalMsec**: Wait at least this many milliseconds between applying 'consolidationPolicy' to
    consolidate view data store and possibly release space on the filesystem
    (default: 60000, to disable use: 0).
    For the case where there are a lot of data modification operations, a higher
    value could potentially have the data store consume more space and file handles.
    For the case where there are a few data modification operations, a lower value
    will impact performance due to no segment candidates available for
    consolidation.<br/>
    _Background:_
      For data modification ArangoSearch views follow the concept of a
      "versioned data store". Thus old versions of data may be removed once there
      are no longer any users of the old data. The frequency of the cleanup and
      compaction operations are governed by 'consolidationIntervalMsec' and the
      candidates for compaction are selected via 'consolidationPolicy'.
    - **cleanupIntervalStep**: Wait at least this many commits between removing unused files in the
    ArangoSearch data directory (default: 10, to disable use: 0).
    For the case where the consolidation policies merge segments often (i.e. a lot
    of commit+consolidate), a lower value will cause a lot of disk space to be
    wasted.
    For the case where the consolidation policies rarely merge segments (i.e. few
    inserts/deletes), a higher value will impact performance without any added
    benefits.<br/>
    _Background:_
      With every "commit" or "consolidate" operation a new state of the view
      internal data-structures is created on disk.
      Old states/snapshots are released once there are no longer any users
      remaining.
      However, the files for the released states/snapshots are left on disk, and
      only removed by "cleanup" operation.
    - **consolidationPolicy**:
      - **type**: The segment candidates for the "consolidation" operation are selected based
     upon several possible configurable formulas as defined by their types.
     The currently supported types are (default: "bytes_accum"):
     - *bytes_accum*: consolidate if and only if (`{threshold}` range `[0.0, 1.0]`):
       `{threshold} > (segment_bytes + sum_of_merge_candidate_segment_bytes) / all_segment_bytes`
       i.e. the sum of all candidate segment byte size is less than the total
       segment byte size multiplied by the `{threshold}`
     - *tier*: consolidate based on segment byte size and live document count
       as dictated by the customization attributes.




Changes the properties of a view.

On success an object with the following attributes is returned:
- *id*: The identifier of the view
- *name*: The name of the view
- *type*: The view type
- all additional arangosearch view implementation specific properties


)";


  ApiResult request(std::string const &databaseName);
};
namespace PatchPropertiesArangosearch {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Patch;
  static constexpr auto url = "/_api/view/{view-name}/properties#arangosearch";
  static constexpr auto summery = "Partially changes properties of an ArangoDB view";
  static constexpr auto operationId = "(modifyView:partially";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Views/patch_api_view_properties.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Views"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The name of the view.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "view-name";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PatchPropertiesArangosearchResponse : short {
    Unknown = 0,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response400, R"(If the *view-name* is missing, then a *HTTP 400* is returned.)"},
{Response404, R"(If the *view-name* is unknown, then a *HTTP 404* is returned.)"}};
  static constexpr auto description = R"(Changes the properties of a view.

On success an object with the following attributes is returned:
- *id*: The identifier of the view
- *name*: The name of the view
- *type*: The view type
- any additional view implementation specific properties


)";


  ApiResult request(std::string const &databaseName);
};
namespace PutPropertiesArangosearch {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/view/{view-name}/properties#arangosearch";
  static constexpr auto summery = "Change properties of an ArangoDB view";
  static constexpr auto operationId = "(modifyView:One";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Views/put_api_view_properties.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Views"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The name of the view.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "view-name";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PutPropertiesArangosearchResponse : short {
    Unknown = 0,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response400, R"(If the *view-name* is missing, then a *HTTP 400* is returned.)"},
{Response404, R"(If the *view-name* is unknown, then a *HTTP 404* is returned.)"}};
  static constexpr auto description = R"(Changes the properties of a view.

On success an object with the following attributes is returned:
- *id*: The identifier of the view
- *name*: The name of the view
- *type*: The view type
- any additional view implementation specific properties


)";


  ApiResult request(std::string const &databaseName);
};
namespace PutRename {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Put;
  static constexpr auto url = "/_api/view/{view-name}/rename";
  static constexpr auto summery = "Rename a view";
  static constexpr auto operationId = "(modifyView:rename";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Views/put_api_view_rename.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Views"};
  inline Parameter P0;
  inline void initParam(){
    P0.description = R"(The name of the view to rename.)";
    P0.format = ParamFormat::stringFormat;
    P0.in = ParamIn::path;
    P0.name = "view-name";
    P0.required = true;
    P0.type = ParamFormat::stringFormat;
  }
  inline std::vector<Parameter> parameters{P0};

  enum PutRenameResponse : short {
    Unknown = 0,
    Response400 = 400,
    Response404 = 404
  };
  inline const static std::map<int, char const * > responses{{Response400, R"(If the *view-name* is missing, then a *HTTP 400* is returned.)"},
{Response404, R"(If the *view-name* is unknown, then a *HTTP 404* is returned.)"}};
  static constexpr auto description = R"(Renames a view. Expects an object with the attribute(s)
- *name*: The new name

It returns an object with the attributes
- *id*: The identifier of the view.
- *name*: The new name of the view.
- *type*: The view type.

**Note**: this method is not available in a cluster.


)";


  ApiResult request(std::string const &databaseName);
};
namespace GetLastTick {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/wal/lastTick";
  static constexpr auto summery = "Return last available tick value";
  static constexpr auto operationId = "(handleCommandLastTick";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Replication/get_api_wal_access_last_tick.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Replication"};
  
  enum GetLastTickResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response405 = 405,
    Response500 = 500,
    Response501 = 501
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned if the request was executed successfully.)"},
{Response405, R"(is returned when an invalid HTTP method is used.)"}, 
{Response500, R"(is returned if an error occurred while assembling the response.)"}, 
{Response501, R"(is returned when this operation is called on a coordinator in a cluster.)"}};
  static constexpr auto description = R"(Returns the last available tick value that can be served from the server's
replication log. This corresponds to the tick of the latest successfull operation.

The result is a JSON object containing the attributes *tick*, *time* and *server*. 
* *tick*: contains the last available tick, *time* 
* *time*: the server time as string in format "YYYY-MM-DDTHH:MM:SSZ"
* *server*: An object with fields *version* and *serverId*

**Note**: this method is not supported on a coordinator in a cluster.


)";


  ApiResult request(std::string const &databaseName);
};
namespace GetRange {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/wal/range";
  static constexpr auto summery = "Return tick ranges available in the operations of WAL";
  static constexpr auto operationId = "(handleCommandTickRange";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Replication/get_api_wal_access_range.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Replication"};
  
  enum GetRangeResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response405 = 405,
    Response500 = 500,
    Response501 = 501
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned if the tick ranges could be determined successfully.)"},
{Response405, R"(is returned when an invalid HTTP method is used.)"}, 
{Response500, R"(is returned if the server operations state could not be determined.)"}, 
{Response501, R"(is returned when this operation is called on a coordinator in a cluster.)"}};
  static constexpr auto description = R"(Returns the currently available ranges of tick values for all WAL files.
The tick values can be used to determine if certain
data (identified by tick value) are still available for replication.

The body of the response contains a JSON object. 
* *tickMin*: minimum tick available
* *tickMax: maximum tick available
* *time*: the server time as string in format "YYYY-MM-DDTHH:MM:SSZ"
* *server*: An object with fields *version* and *serverId*


)";


  ApiResult request(std::string const &databaseName);
};
namespace GetTail {

  static constexpr auto restVerb = arangodb::fuerte::RestVerb::Get;
  static constexpr auto url = "/_api/wal/tail";
  static constexpr auto summery = "Tail recent server operations";
  static constexpr auto operationId = "(handleCommandTail";
  static constexpr auto x_filename = "/home/willi/src/devel/Documentation/DocuBlocks/Rest/Replication/get_api_wal_access_tail.md";
  static constexpr auto x_hints = R"()";
  inline const static std::vector<char const * > tags{"Replication"};
  inline Parameter P0;
  inline Parameter P1;
  inline Parameter P2;
  inline Parameter P3;
  inline Parameter P4;
  inline Parameter P5;
  inline Parameter P6;
  inline void initParam(){
    P0.description = R"(Exclusive lower bound tick value for results. On successive calls
to this API you should set this to the value returned
with the *x-arango-replication-lastincluded* header (Unless that header
contains 0).)";
    P0.in = ParamIn::query;
    P0.name = "from";
    P0.required = false;
    P0.type = ParamFormat::numberFormat;

    P1.description = R"(Inclusive upper bound tick value for results.)";
    P1.in = ParamIn::query;
    P1.name = "to";
    P1.required = false;
    P1.type = ParamFormat::numberFormat;

    P2.description = R"(Should be set to the value of the *x-arango-replication-lastscanned* header
or alternatively 0 on first try. This allows the rocksdb engine to break up
large transactions over multiple responses. )";
    P2.in = ParamIn::query;
    P2.name = "lastScanned";
    P2.required = false;
    P2.type = ParamFormat::numberFormat;

    P3.description = R"(Whether operations for all databases should be included. When set to *false*
only the operations for the current database are included. The value *true* is
only valid on the *_system* database. The default is *false*.)";
    P3.in = ParamIn::query;
    P3.name = "global";
    P3.required = false;
    P3.type = ParamFormat::booleanFormat;

    P4.description = R"(Approximate maximum size of the returned result.)";
    P4.in = ParamIn::query;
    P4.name = "chunkSize";
    P4.required = false;
    P4.type = ParamFormat::numberFormat;

    P5.description = R"(Id of the client used to tail results. The server will use this to 
keep operations until the client has fetched them. **Note** this is required
to have a chance at fetching reading all operations with the rocksdb storage engine)";
    P5.in = ParamIn::query;
    P5.name = "serverId";
    P5.required = false;
    P5.type = ParamFormat::numberFormat;

    P6.description = R"(Id of barrier used to keep WAL entries around. **Note** this is only required for the 
MMFiles storage engine)";
    P6.in = ParamIn::query;
    P6.name = "barrierId";
    P6.required = false;
    P6.type = ParamFormat::numberFormat;
  }
  inline std::vector<Parameter> parameters{P0, P1, P2, P3, P4, P5, P6};

  enum GetTailResponse : short {
    Unknown = 0,
    Response200 = 200,
    Response204, 
    Response400 = 400,
    Response405 = 405,
    Response500 = 500,
    Response501 = 501
  };
  inline const static std::map<int, char const * > responses{{Response200, R"(is returned if the request was executed successfully, and there are log
events available for the requested range. The response body will not be empty
in this case.)"}, 
{Response204, R"(is returned if the request was executed successfully, but there are no log
events available for the requested range. The response body will be empty
in this case.)"}, 
{Response400, R"(is returned if either the *from* or *to* values are invalid.)"}, 
{Response405, R"(is returned when an invalid HTTP method is used.)"}, 
{Response500, R"(is returned if an error occurred while assembling the response.)"}, 
{Response501, R"(is returned when this operation is called on a coordinator in a cluster.)"}};
  static constexpr auto description = R"(Returns data from the server's write-ahead log (also named replication log). This method can be called
by replication clients after an initial synchronization of data. The method
will return all "recent" logged operations from the server. Clients
can replay and apply these operations locally so they get to the same data
state as the server.

Clients can call this method repeatedly to incrementally fetch all changes
from the server. In this case, they should provide the *from* value so
they will only get returned the log events since their last fetch.

When the *from* query parameter is not used, the server will return log
entries starting at the beginning of its replication log. When the *from*
parameter is used, the server will only return log entries which have
higher tick values than the specified *from* value (note: the log entry with a
tick value equal to *from* will be excluded). Use the *from* value when
incrementally fetching log data.

The *to* query parameter can be used to optionally restrict the upper bound of
the result to a certain tick value. If used, the result will contain only log events
with tick values up to (including) *to*. In incremental fetching, there is no
need to use the *to* parameter. It only makes sense in special situations,
when only parts of the change log are required.

The *chunkSize* query parameter can be used to control the size of the result.
It must be specified in bytes. The *chunkSize* value will only be honored
approximately. Otherwise a too low *chunkSize* value could cause the server
to not be able to put just one log entry into the result and return it.
Therefore, the *chunkSize* value will only be consulted after a log entry has
been written into the result. If the result size is then bigger than
*chunkSize*, the server will respond with as many log entries as there are
in the response already. If the result size is still smaller than *chunkSize*,
the server will try to return more data if there's more data left to return.

If *chunkSize* is not specified, some server-side default value will be used.

The *Content-Type* of the result is *application/x-arango-dump*. This is an
easy-to-process format, with all log events going onto separate lines in the
response body. Each log event itself is a JSON object, with at least the
following attributes:

- *tick*: the log event tick value

- *type*: the log event type

Individual log events will also have additional attributes, depending on the
event type. A few common attributes which are used for multiple events types
are:

- *cuid*: globally unique id of the view or collection the event was for

- *db*: the database name the event was for

- *tid*: id of the transaction the event was contained in

- *data*: the original document data

A more detailed description of the individual replication event types and their
data structures can be found in [Operation Types](#operation-types).

The response will also contain the following HTTP headers:

- *x-arango-replication-active*: whether or not the logger is active. Clients
  can use this flag as an indication for their polling frequency. If the
  logger is not active and there are no more replication events available, it
  might be sensible for a client to abort, or to go to sleep for a long time
  and try again later to check whether the logger has been activated.

- *x-arango-replication-lastincluded*: the tick value of the last included
  value in the result. In incremental log fetching, this value can be used
  as the *from* value for the following request. **Note** that if the result is
  empty, the value will be *0*. This value should not be used as *from* value
  by clients in the next request (otherwise the server would return the log
  events from the start of the log again).

- *x-arango-replication-lastscanned*: the last tick the server scanned while
  computing the operation log. This might include operations the server did not
  returned to you due to various reasons (i.e. the value was filtered or skipped).
  You may use this value in the *lastScanned* header to allow the rocksdb engine
  to break up requests over multiple responses.

- *x-arango-replication-lasttick*: the last tick value the server has
  logged in its write ahead log (not necessarily included in the result). By comparing the the last
  tick and last included tick values, clients have an approximate indication of
  how many events there are still left to fetch.

- *x-arango-replication-frompresent*: is set to _true_ if server returned
  all tick values starting from the specified tick in the _from_ parameter.
  Should this be set to false the server did not have these operations anymore
  and the client might have missed operations.

- *x-arango-replication-checkmore*: whether or not there already exists more
  log data which the client could fetch immediately. If there is more log data
  available, the client could call *logger-follow* again with an adjusted *from*
  value to fetch remaining log entries until there are no more.

  If there isn't any more log data to fetch, the client might decide to go
  to sleep for a while before calling the logger again.

**Note**: this method is not supported on a coordinator in a cluster.


)";


  ApiResult request(std::string const &databaseName);
};

}  // namespace Replication
//clang-format on
}  // namespace Api

