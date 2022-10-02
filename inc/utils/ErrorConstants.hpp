#pragma once
#include <unordered_map>
#include <iostream>
#include <cassert>
namespace ok
{
enum class ErrorCode
{
  ERROR_NO_ERROR = 0,
  ERROR_FAILED = 1,
  ERROR_SYS_ERROR = 2,
  ERROR_OUT_OF_MEMORY = 3,
  ERROR_INTERNAL = 4,
  ERROR_ILLEGAL_NUMBER = 5,
  ERROR_NUMERIC_OVERFLOW = 6,
  ERROR_ILLEGAL_OPTION = 7,
  ERROR_DEAD_PID = 8,
  ERROR_NOT_IMPLEMENTED = 9,
  ERROR_BAD_PARAMETER = 10,
  ERROR_FORBIDDEN = 11,
  ERROR_OUT_OF_MEMORY_MMAP = 12,
  ERROR_CORRUPTED_CSV = 13,
  ERROR_FILE_NOT_FOUND = 14,
  ERROR_CANNOT_WRITE_FILE = 15,
  ERROR_CANNOT_OVERWRITE_FILE = 16,
  ERROR_TYPE_ERROR = 17,
  ERROR_LOCK_TIMEOUT = 18,
  ERROR_CANNOT_CREATE_DIRECTORY = 19,
  ERROR_CANNOT_CREATE_TEMP_FILE = 20,
  ERROR_REQUEST_CANCELED = 21,
  ERROR_DEBUG = 22,
  ERROR_IP_ADDRESS_INVALID = 25,
  ERROR_FILE_EXISTS = 27,
  ERROR_LOCKED = 28,
  ERROR_DEADLOCK = 29,
  ERROR_SHUTTING_DOWN = 30,
  ERROR_ONLY_ENTERPRISE = 31,
  ERROR_RESOURCE_LIMIT = 32,
  ERROR_ARANGO_ICU_ERROR = 33,
  ERROR_CANNOT_READ_FILE = 34,
  ERROR_INCOMPATIBLE_VERSION = 35,
  ERROR_DISABLED = 36,
  ERROR_HTTP_BAD_PARAMETER = 400,
  ERROR_HTTP_UNAUTHORIZED = 401,
  ERROR_HTTP_FORBIDDEN = 403,
  ERROR_HTTP_NOT_FOUND = 404,
  ERROR_HTTP_METHOD_NOT_ALLOWED = 405,
  ERROR_HTTP_NOT_ACCEPTABLE = 406,
  ERROR_HTTP_PRECONDITION_FAILED = 412,
  ERROR_HTTP_SERVER_ERROR = 500,
  ERROR_HTTP_SERVICE_UNAVAILABLE = 503,
  ERROR_HTTP_GATEWAY_TIMEOUT = 504,
  ERROR_HTTP_CORRUPTED_JSON = 600,
  ERROR_HTTP_SUPERFLUOUS_SUFFICES = 601,
  BAD_QUERY = 1001,
  CANNOT_LOAD_SCHEMA = 1002,
  CANNOT_CONNECT_TO_DATABASE = 1003,
  PASSWORD_NOT_MATCH = 1004,
  EXPIRED = 1005,
  UNIQUE_ERROR = 1006,
  SCHEMA_ERROR = 2001,
  ERROR_FORM_EMPTY = 2002,
  ERROR_FORM_FIELD_ERROR = 2003
};
/// @brief the error messages, will be read-only after initialization
inline std::unordered_map<ErrorCode, std::string> ErrorMessages;  // static cause it uninitialized on tests!
inline void set_errno_string(ErrorCode code, std::string msg)
{
  if (!ErrorMessages.try_emplace(code, msg).second)
  {
    // logic error, error number is redeclared
    std::cout << "ErrorCode:: duplicate declaration of error code " << static_cast<int>(code) << " in " << __FILE__ << ":" << __LINE__ << std::endl;
    exit(EXIT_FAILURE);
  }
}
/// @brief return an error message for an error code
inline std::string& errno_string(ErrorCode code) noexcept
{
  static std::string unknown{"unknown error"};
  auto it = ErrorMessages.find(code);
  if (it == std::end(ErrorMessages)) return unknown;
  return (*it).second;
}
/// register all errors
inline void InitializeErrorMessages()
{
  set_errno_string(ErrorCode::ERROR_NO_ERROR, "no error");
  set_errno_string(ErrorCode::ERROR_FAILED, "failed");
  set_errno_string(ErrorCode::ERROR_SYS_ERROR, "system error");
  set_errno_string(ErrorCode::ERROR_OUT_OF_MEMORY, "out of memory");
  set_errno_string(ErrorCode::ERROR_INTERNAL, "internal error");
  set_errno_string(ErrorCode::ERROR_ILLEGAL_NUMBER, "illegal number");
  set_errno_string(ErrorCode::ERROR_NUMERIC_OVERFLOW, "numeric overflow");
  set_errno_string(ErrorCode::ERROR_ILLEGAL_OPTION, "illegal option");
  set_errno_string(ErrorCode::ERROR_DEAD_PID, "dead process identifier");
  set_errno_string(ErrorCode::ERROR_NOT_IMPLEMENTED, "not implemented");
  set_errno_string(ErrorCode::ERROR_BAD_PARAMETER, "bad parameter");
  set_errno_string(ErrorCode::ERROR_FORBIDDEN, "forbidden");
  set_errno_string(ErrorCode::ERROR_OUT_OF_MEMORY_MMAP, "out of memory in mmap");
  set_errno_string(ErrorCode::ERROR_CORRUPTED_CSV, "csv is corrupt");
  set_errno_string(ErrorCode::ERROR_FILE_NOT_FOUND, "file not found");
  set_errno_string(ErrorCode::ERROR_CANNOT_WRITE_FILE, "cannot write file");
  set_errno_string(ErrorCode::ERROR_CANNOT_OVERWRITE_FILE, "cannot overwrite file");
  set_errno_string(ErrorCode::ERROR_TYPE_ERROR, "type error");
  set_errno_string(ErrorCode::ERROR_LOCK_TIMEOUT, "lock timeout");
  set_errno_string(ErrorCode::ERROR_CANNOT_CREATE_DIRECTORY, "cannot create directory");
  set_errno_string(ErrorCode::ERROR_CANNOT_CREATE_TEMP_FILE, "cannot create temporary file");
  set_errno_string(ErrorCode::ERROR_REQUEST_CANCELED, "canceled request");
  set_errno_string(ErrorCode::ERROR_DEBUG, "intentional debug error");
  set_errno_string(ErrorCode::ERROR_IP_ADDRESS_INVALID, "IP address is invalid");
  set_errno_string(ErrorCode::ERROR_FILE_EXISTS, "file exists");
  set_errno_string(ErrorCode::ERROR_LOCKED, "locked");
  set_errno_string(ErrorCode::ERROR_DEADLOCK, "deadlock detected");
  set_errno_string(ErrorCode::ERROR_SHUTTING_DOWN, "shutdown in progress");
  set_errno_string(ErrorCode::ERROR_ONLY_ENTERPRISE, "only enterprise version");
  set_errno_string(ErrorCode::ERROR_RESOURCE_LIMIT, "resource limit exceeded");
  set_errno_string(ErrorCode::ERROR_ARANGO_ICU_ERROR, "icu ErrorCode:: %s");
  set_errno_string(ErrorCode::ERROR_CANNOT_READ_FILE, "cannot read file");
  set_errno_string(ErrorCode::ERROR_INCOMPATIBLE_VERSION, "incompatible server version");
  set_errno_string(ErrorCode::ERROR_DISABLED, "disabled");
  set_errno_string(ErrorCode::ERROR_HTTP_BAD_PARAMETER, "bad parameter");
  set_errno_string(ErrorCode::ERROR_HTTP_UNAUTHORIZED, "unauthorized");
  set_errno_string(ErrorCode::ERROR_HTTP_FORBIDDEN, "forbidden");
  set_errno_string(ErrorCode::ERROR_HTTP_NOT_FOUND, "not found");
  set_errno_string(ErrorCode::ERROR_HTTP_METHOD_NOT_ALLOWED, "method not supported");
  set_errno_string(ErrorCode::ERROR_HTTP_NOT_ACCEPTABLE, "request not acceptable");
  set_errno_string(ErrorCode::ERROR_HTTP_PRECONDITION_FAILED, "precondition failed");
  set_errno_string(ErrorCode::ERROR_HTTP_SERVER_ERROR, "internal server error");
  set_errno_string(ErrorCode::ERROR_HTTP_SERVICE_UNAVAILABLE, "service unavailable");
  set_errno_string(ErrorCode::ERROR_HTTP_GATEWAY_TIMEOUT, "gateway timeout");
  set_errno_string(ErrorCode::ERROR_HTTP_CORRUPTED_JSON, "invalid JSON object");
  set_errno_string(ErrorCode::ERROR_HTTP_SUPERFLUOUS_SUFFICES, "superfluous URL suffices");
  set_errno_string(ErrorCode::BAD_QUERY, "bad query");
  set_errno_string(ErrorCode::CANNOT_LOAD_SCHEMA, "cant load schema");
  set_errno_string(ErrorCode::CANNOT_CONNECT_TO_DATABASE, "cant connect to database");
  set_errno_string(ErrorCode::PASSWORD_NOT_MATCH, "password not match");
  set_errno_string(ErrorCode::EXPIRED, "expired");
  set_errno_string(ErrorCode::UNIQUE_ERROR, "unique error");
  set_errno_string(ErrorCode::SCHEMA_ERROR, "schema error");
  set_errno_string(ErrorCode::ERROR_FORM_EMPTY, "");
  set_errno_string(ErrorCode::ERROR_FORM_FIELD_ERROR, "field error");
}
/// @brief initializes the error messages
inline void InitializeError() { InitializeErrorMessages(); }
inline std::ostream& operator<<(std::ostream& out, ErrorCode const e)
{
  out << static_cast<int>(e) << " " << errno_string(e);
  return out;
}
inline bool isEr(ErrorCode er) { return er != ErrorCode::ERROR_NO_ERROR; }
}  // namespace ok
