#include "db/Permission.hpp"
#include <trantor/utils/Logger.h>
#include <iostream>
#include "utils/GlobalSettings.hpp"
#include <absl/strings/str_format.h>
#include "db/DatabaseApi.hpp"
#include "utils/ErrorConstants.hpp"
#include "actor_system/Routes.hpp"
namespace ok
{
namespace permission
{
ErrorCode checkPermission(Database const &database,
                          DocumentKey const &memberKey,
                          DocumentKey const &projectKey,
                          const std::string &category,
                          ok::smart_actor::connection::MutateEventType eventType,
                          bool childPermissionCheck,
                          bool adminPermissionCheck) noexcept
{
  switch (eventType)
  {
    case ok::smart_actor::connection::MutateEventType::insert: return checkPermission(database, memberKey, projectKey, category, "create", childPermissionCheck, adminPermissionCheck); break;
    case ok::smart_actor::connection::MutateEventType::replace: return checkPermission(database, memberKey, projectKey, category, "edit", childPermissionCheck, adminPermissionCheck); break;
    case ok::smart_actor::connection::MutateEventType::update: return checkPermission(database, memberKey, projectKey, category, "edit", childPermissionCheck, adminPermissionCheck); break;
    case ok::smart_actor::connection::MutateEventType::batchUpdate: return checkPermission(database, memberKey, projectKey, category, "edit", childPermissionCheck, adminPermissionCheck); break;
    case ok::smart_actor::connection::MutateEventType::changePosition: return checkPermission(database, memberKey, projectKey, category, "edit", childPermissionCheck, adminPermissionCheck); break;
    case ok::smart_actor::connection::MutateEventType::delete_: return checkPermission(database, memberKey, projectKey, category, "delete", childPermissionCheck, adminPermissionCheck); break;
    case ok::smart_actor::connection::MutateEventType::batchDelete: return checkPermission(database, memberKey, projectKey, category, "delete", childPermissionCheck, adminPermissionCheck); break;
    default: return ok::ErrorCode::ERROR_INTERNAL; break;
  }
}
ErrorCode checkPermission(Database const &database,
                          DocumentKey memberKey,
                          DocumentKey const &projectKey,
                          std::string const &category,
                          std::string const &property,
                          bool childPermissionCheck,
                          bool adminPermissionCheck) noexcept
{
  if (database == "global") { return ok::ErrorCode::ERROR_NO_ERROR; }
  if (database == "user") { return ok::ErrorCode::ERROR_NO_ERROR; }
  if (database == "_system") { return ok::ErrorCode::ERROR_NO_ERROR; }
  if (memberKey.empty()) { memberKey = "guest"; }
  if (projectKey.empty()) { return impl::checkPermissionTopLevel(database, memberKey, category, property); }
  bool result = false;  // temporary
  if (childPermissionCheck)
  {
    // Find has member is in:
    // Find has_group of each group
    // Find has_roles in each group
    // Find roles
    // FILTER hg._from == CONCAT('project/', 'organization')
    constexpr absl::string_view projectQuery = R"aql(FOR m in member
FILTER m._key == '%s'
FOR hm IN has_member
  FILTER hm._to == m._id
  FOR hg in has_group
    FILTER hg._to == hm._from
    let project_id = CONCAT('project/', '%s')
    // for each group current member is in
    // filter group: groups' project whose any parent contains selected project (find project is in group or project's parent is in group)
    // parent_project <- project_childof <- child_project
    // it means each project must be in some group
    FILTER LENGTH(FOR v, e, p IN 0..9999 INBOUND hg._from project_childof
            FILTER e._to == project_id   || v._id == project_id
            LIMIT 1 RETURN true) == 1
    //FOR v IN 0..9999 INBOUND hg._from project_childof
      //FOR hg2 in has_group
        //FILTER hg2.from == v._to
        FOR gr in has_role
          //FILTER gr._from == hg2._to
          FILTER gr._from == hg._to
          FOR hp in has_permission
            FILTER hp._from == gr._to AND hp._to == CONCAT("permission/", "%s")
            FILTER hp.properties['%s'] == true
            LIMIT 1
            RETURN true)aql";
    // a@a.com, organization, update
    auto permissionQuery = absl::StrFormat(projectQuery, memberKey, projectKey, category, property);
    auto [erDb, response] = Api::Cursor::PostCursor::request(database, permissionQuery, {});
    if (isEr(erDb))
    {
      if (response)
      {
        auto slice = response->slices().front();
        LOG_FATAL << "Error: " << slice.toJson();
      }
      LOG_FATAL << "Error: " << errno_string(erDb) << "\n| database is: " << database << " | executed query: " << permissionQuery;
      return ok::ErrorCode::BAD_QUERY;
    }
    {
      auto slice = response->slices().front();
      if (ok::smart_actor::connection::getPrintVal("permission")) { LOG_DEBUG << "executed query: \n" << permissionQuery << "\n | database is: " << database << " | Result: " << slice.toJson(); }
      if (auto r = slice.get("result"); r.isArray() && r.length() == 1)
      {
        if (r[0].isBool()) { result = r[0].getBool(); }
      }
    }
  }
  if (!result && adminPermissionCheck) { return impl::checkPermissionTopLevel(database, memberKey, category, property); }
  return ok::ErrorCode::ERROR_HTTP_UNAUTHORIZED;
}
namespace impl
{
ErrorCode checkPermissionTopLevel(Database const &database, DocumentKey const &memberKey, std::string const &category, std::string const &property) noexcept
{
  if (database == "global") { return ok::ErrorCode::ERROR_NO_ERROR; }
  // Find groups member is in:
  // Find group_roles in each group
  // Find roles
  constexpr absl::string_view adminQuery = R"aql(FOR m in member
FILTER m._key == '%s'
FOR hm IN has_member
  FILTER hm._to == m._id
  // filter group: only top level group which has no parent project
  FILTER LENGTH(FOR v IN 1..1 INBOUND hm._from has_group RETURN v) == 0
  FOR hr in has_role
    FILTER hr._from == hm._from
    FOR hp in has_permission
      FILTER hp._from == hr._to AND hp._to == CONCAT("permission/", "%s")
      FILTER hp.properties['%s'] == true
      LIMIT 1
      RETURN true)aql";
  // a@a.com, organization, update
  auto permissionQuery = absl::StrFormat(adminQuery, memberKey, category, property);
  auto [erDb, response] = Api::Cursor::PostCursor::request(database, permissionQuery, {});
  if (isEr(erDb))
  {
    if (response)
    {
      auto slice = response->slices().front();
      LOG_FATAL << "Error: " << slice.toJson();
    }
    LOG_FATAL << "Error: " << errno_string(erDb) << "\n| database is: " << database << " | executed query: " << permissionQuery;
    return ok::ErrorCode::BAD_QUERY;
  }
  auto slice = response->slices().front();
  if (ok::smart_actor::connection::getPrintVal("permission")) { LOG_DEBUG << "executed query: \n" << permissionQuery << "\n | database is: " << database << " | Result: " << slice.toJson(); }
  if (auto r = slice.get("result"); r.isArray() && r.length() == 1)
  {
    if (r[0].isBool())
    {
      if (r[0].getBool()) { return ok::ErrorCode::ERROR_NO_ERROR; }
      else
      {
        return ok::ErrorCode::ERROR_HTTP_UNAUTHORIZED;
      }
    }
    else
    {
      return ok::ErrorCode::ERROR_HTTP_UNAUTHORIZED;
    }
  }
  else
  {
    return ok::ErrorCode::ERROR_HTTP_UNAUTHORIZED;
  }
}
}  // namespace impl
}  // namespace permission
}  // namespace ok
