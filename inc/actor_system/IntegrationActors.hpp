#pragma once
#include "caf/all.hpp"
#include "CAF.hpp"
#include "mutate/MutateSchema.hpp"
#include "actor_system/MutateActors.hpp"
#include "vmime/vmime.hpp"
namespace ok::smart_actor
{
namespace connection
{
backup_db_actor::behavior_type BackupActor(backup_db_actor::pointer self);
db_health_check_actor_int::behavior_type DBHealthCheckActor(db_health_check_actor_int::pointer self);
void monitorDatabaseHealth(db_health_check_actor_int::pointer act);
session_clean_actor_int::behavior_type SessionCleanActor(session_clean_actor_int::pointer self);
void cleanExpiredCookiesEveryHour(session_clean_actor_int::pointer act);
}  // namespace connection
namespace integrations
{
namespace email
{
struct email_actor_state
{
  static inline constexpr char const *name = "email-actor";
  vmime::shared_ptr<vmime::net::transport> trans;
  ~email_actor_state() { trans->disconnect(); }
};
email_actor_int::behavior_type EmailActor(email_actor_int::stateful_pointer<email_actor_state> self);
namespace impl
{
void connect(vmime::shared_ptr<vmime::net::transport> &trans);
void sendMail(vmime::shared_ptr<vmime::net::transport> &trans, std::string const &name, std::string const &toEmail, std::string const &subject, std::string const &message);
}  // namespace impl
}  // namespace email
}  // namespace integrations
}  // namespace ok::smart_actor
