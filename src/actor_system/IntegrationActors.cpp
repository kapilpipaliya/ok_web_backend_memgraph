// https://www.vmime.org/public/documentation/book/vmime-book.pdf
#include "actor_system/IntegrationActors.hpp"
#include "utils/GlobalSettings.hpp"
namespace ok::smart_actor
{
namespace connection
{
backup_db_actor::behavior_type BackupActor(backup_db_actor::pointer self)
{
  self->set_exception_handler(ok::smart_actor::supervisor::default_exception_handler("Mutate FileNotify Actor"));
  return {[=](backup_atom)
          {
            // todo // If there is mutation done on database, dump it every 30 minutes.
          },
          CONN_EXIT};
}
db_health_check_actor_int::behavior_type DBHealthCheckActor(db_health_check_actor_int::pointer self)
{
  self->set_exception_handler(ok::smart_actor::supervisor::default_exception_handler("Database Health Check Actor"));
  return {[=](db_health_check_atom) { monitorDatabaseHealth(self); },
          [=](conn_exit_atom)
          {
            LOG_DEBUG << "exiting " << self->name();
            self->quit();
          }};
}
session_clean_actor_int::behavior_type SessionCleanActor(session_clean_actor_int::pointer self)
{
  self->set_exception_handler(ok::smart_actor::supervisor::default_exception_handler("Mutate SessionClean Actor"));
  return {[=](session_clean_atom) { cleanExpiredCookiesEveryHour(self); },
          [=](conn_exit_atom)
          {
            LOG_DEBUG << "exiting " << self->name();
            self->quit();
          }};
}
template <typename A>
void sendDatabaseDownEmail(A act)
{
  std::string body = "Email Confirmation Successful: \n";
  body += "Hello Database server is down. fix it.\n";
  act->send(ok::smart_actor::supervisor::emailMutateActor, send_email_atom_v, "", "admin@o-k.website", "Database down", body);
}
void monitorDatabaseHealth(db_health_check_actor_int::pointer act)
{
  constexpr auto fifteen_minute_milliSeconds = 15 * 60 * 1000;
  LOG_DEBUG << "database health check...";
  if (!ok::db::isDatabaseServerConnected())
  {
    sendDatabaseDownEmail(act);
    LOG_DEBUG << "Database is not connected. fix it..";
  }
  act->delayed_send(act, std::chrono::milliseconds(fifteen_minute_milliSeconds), db_health_check_atom_v);
}
void cleanExpiredCookiesEveryHour(session_clean_actor_int::pointer act)
{
  constexpr auto one_hour_milliSeconds = 1 * 60 * 60 * 1000;
  ok::db::cleanExpiredCookies();
  act->delayed_send(act, std::chrono::milliseconds(one_hour_milliSeconds), session_clean_atom_v);
}
}  // namespace connection
namespace integrations
{
namespace email
{
// Global session object
// static vmime::shared_ptr<vmime::net::session> g_session =
//    vmime::net::session::create();
email_actor_int::behavior_type EmailActor(email_actor_int::stateful_pointer<email_actor_state> self)
{
  self->set_exception_handler(ok::smart_actor::supervisor::default_exception_handler("Mutate Email Actor"));
  impl::connect(self->state.trans);
  return {[=](send_email_atom, std::string name, std::string const &toEmail, std::string const &subject, std::string const &message)
          { impl::sendMail(self->state.trans, name, toEmail, subject, message); },
          CONN_EXIT};
}
namespace impl
{
// TODO: Use backup email service when first server fails.
struct MyTH : vmime::net::timeoutHandler
{
  bool isTimeOut() override { return (getTime() >= m_last + 20); }
  void resetTimeOut() override { m_last = getTime(); }
  bool handleTimeOut() override
  {
    LOG_DEBUG << "TimeOut!" << getTime();
    return false;
  }
  unsigned long getTime() { return vmime::platform::getHandler()->getUnixTime(); }
  unsigned long m_last;
};
struct MyTHF : vmime::net::timeoutHandlerFactory
{
  vmime::shared_ptr<vmime::net::timeoutHandler> create() override { return vmime::make_shared<MyTH>(); }
};
struct MyT : vmime::net::tracer
{
public:
  MyT(vmime::string const &proto, int const cId) : m_p(proto), m_cId(cId) {}
  void traceSend(vmime::string const &l) override { traceReceive("(Client)" + l); }
  void traceReceive(vmime::string const &l) override { LOG_DEBUG << m_p << ":" << m_cId << "->" << l; }
  vmime::string const m_p;
  int const m_cId;
};
class MyTF : public vmime::net::tracerFactory
{
public:
  MyTF(){};
  vmime::shared_ptr<vmime::net::tracer> create(vmime::shared_ptr<vmime::net::service> const &serv, int const connectionId) { return vmime::make_shared<MyT>(serv->getProtocolName(), connectionId); }
};
vmime::shared_ptr<vmime::security::cert::X509Certificate> loadX509CertificateFromFile(std::string const &path)
{
  std::ifstream certFile;
  certFile.open(path, std::ios::in | std::ios::binary);
  if (!certFile)
  {
    LOG_DEBUG << "cant open certificate file";
    // ...handle error...
  }
  vmime::utility::inputStreamAdapter is(certFile);
  vmime::shared_ptr<vmime::security::cert::X509Certificate> cert;
  cert = vmime::security::cert::X509Certificate::import(is);
  return cert;
}
class MyFakeVarifier : public vmime::security::cert::defaultCertificateVerifier
{
public:
  void verify(const std::shared_ptr<vmime::security::cert::certificateChain> &chain, const std::string &hostname) {}
};
void connect(vmime::shared_ptr<vmime::net::transport> &trans)
{
  // send email:
  // Set the global C and C++ locale to the user-configured locale.
  // The locale should use UTF-8 encoding for these tests to run successfully.
  try
  {
    std::locale::global(std::locale(""));
  }
  catch (std::exception &)
  {
    // std::setlocale(LC_ALL, ""); // libc++ says no function
  }
  try
  {
    vmime::shared_ptr<vmime::net::session> sess = vmime::net::session::create();
    sess->getProperties().setProperty("transport.protocol", "smtp");  // "smtps" = classical SSL/TLS, but is now deprecated
    // vmime::shared_ptr<vmime::net::transport> trans = sess->getTransport();
    trans = sess->getTransport();
    if (auto server = ok::smart_actor::connection::getSmtpVal("server"); !server.isNone()) { trans->setProperty("server.address", server.copyString()); }
    else
    {
      LOG_DEBUG << "MAIL SERVER ADDRESS NOT FOUND IN SETTINGS";
      return;
    }
    if (auto port = ok::smart_actor::connection::getSmtpVal("port"); !port.isNone()) { trans->setProperty("server.port", port.getInt()); }
    else
    {
      LOG_DEBUG << "MAIL SERVER PORT NOT FOUND USING 587";
      trans->setProperty("server.port", 587);
    }
    trans->setProperty("options.sasl", true);
    trans->setProperty("connection.tls", true);
    trans->setProperty("options.need-authentication", true);  // this line is required, but not documented
    trans->setProperty("transport.smtp.options.need-authentication", true);
    if (auto username = ok::smart_actor::connection::getSmtpVal("username"); !username.isNone()) { trans->setProperty("auth.username", username.copyString()); }
    else
    {
      LOG_DEBUG << "MAIL SERVER username NOT FOUND";
      return;
    }
    if (auto password = ok::smart_actor::connection::getSmtpVal("password"); !password.isNone()) { trans->setProperty("auth.password", password.copyString()); }
    else
    {
      LOG_DEBUG << "MAIL SERVER password NOT FOUND";
      return;
    }
    trans->setProperty("options.chunking", false);
    trans->setTimeoutHandlerFactory(vmime::make_shared<MyTHF>());
    trans->setTracerFactory(vmime::make_shared<MyTF>());
    vmime::shared_ptr<MyFakeVarifier> vrf = vmime::make_shared<MyFakeVarifier>();
    std::vector<vmime::shared_ptr<vmime::security::cert::X509Certificate> > rootCAs;
    rootCAs.push_back(loadX509CertificateFromFile("/etc/ssl/certs/DST_Root_CA_X3.pem"));
    rootCAs.push_back(loadX509CertificateFromFile("/etc/ssl/certs/ISRG_Root_X1.pem"));
    rootCAs.push_back(loadX509CertificateFromFile("/etc/ssl/certs/IdenTrust_Commercial_Root_CA_1.pem"));
    rootCAs.push_back(loadX509CertificateFromFile("/etc/ssl/certs/IdenTrust_Public_Sector_Root_CA_1.pem"));
    rootCAs.push_back(loadX509CertificateFromFile("/etc/ssl/certs/DST_Root_CA_X3.pem"));
    vrf->setX509RootCAs(rootCAs);
    // Then, load certificates that the user explicitely chose to trust
    std::vector<vmime::shared_ptr<vmime::security::cert::X509Certificate> > trusted;
    trusted.push_back(loadX509CertificateFromFile("/etc/ssl/certs/DST_Root_CA_X3.pem"));
    trusted.push_back(loadX509CertificateFromFile("/etc/ssl/certs/ISRG_Root_X1.pem"));
    trusted.push_back(loadX509CertificateFromFile("/etc/ssl/certs/IdenTrust_Commercial_Root_CA_1.pem"));
    trusted.push_back(loadX509CertificateFromFile("/etc/ssl/certs/IdenTrust_Public_Sector_Root_CA_1.pem"));
    trusted.push_back(loadX509CertificateFromFile("/etc/ssl/certs/DST_Root_CA_X3.pem"));
    vrf->setX509TrustedCerts(trusted);
    trans->setCertificateVerifier(vrf);
    // connect() gives error when internet is down
    // ERROR: AddressSanitizer: heap-use-after-free on address
    trans->connect();
    // VMime exception
  }
  catch (vmime::exception &e)
  {
    LOG_DEBUG << "vmime::exception: " << e.what();
    // Standard exception
  }
  catch (std::exception &e)
  {
    LOG_DEBUG << "std::exception: " << e.what();
  }
}
void sendMail(vmime::shared_ptr<vmime::net::transport> &trans, std::string const &name, std::string const &toEmail, std::string const &subject, std::string const &message)
{
  try
  {
    if (!trans || !trans->isConnected())
    {
      connect(trans);
      if (!trans->isConnected()) { LOG_DEBUG << "server must be connected to send mail. giving up"; }
    }
    vmime::messageBuilder mb;
    // Fill in the basic fields
    if (auto from = ok::smart_actor::connection::getSmtpVal("from"); !from.isNone()) { mb.setExpeditor(vmime::mailbox(from.copyString())); }
    else
    {
      LOG_DEBUG << "MAIL SERVER from NOT FOUND";
      return;
    }
    vmime::addressList to;
    to.appendAddress(vmime::make_shared<vmime::mailbox>(toEmail));
    mb.setRecipients(to);
    // vmime::addressList bcc;
    // bcc.appendAddress(
    // vmime::make_shared<vmime::mailbox>("you-bcc@nowhere.com"));
    // mb.setBlindCopyRecipients(bcc);
    mb.setSubject(vmime::text(subject));
    // Message body
    mb.getTextPart()->setText(vmime::make_shared<vmime::stringContentHandler>(message));
    // Construction
    vmime::shared_ptr<vmime::message> msg = mb.construct();
    // Raw text generation
    LOG_DEBUG << "Generated message:";
    LOG_DEBUG << "==================";
    vmime::utility::outputStreamAdapter out(std::cout);
    msg->generate(out);
    // Send the message
    trans->send(msg);
    // VMime exception
  }
  catch (vmime::exception &e)
  {
    LOG_DEBUG << "vmime::exception: " << e.what();
    // throw; // don't throw
    // Standard exception
  }
  catch (std::exception &e)
  {
    LOG_DEBUG << "std::exception: " << e.what();
    // throw; // don't throw
  }
}
}  // namespace impl
}  // namespace email
}  // namespace integrations
}  // namespace ok::smart_actor
