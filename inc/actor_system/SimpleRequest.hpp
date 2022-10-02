#pragma once
//#include "CAF.hpp" // can't include because It's used in arangod
#include "Rest/HttpRequest.h"
#include <velocypack/Builder.h>
#include <velocypack/Options.h>
#include <velocypack/Parser.h>
#include <velocypack/Validator.h>
#include <velocypack/velocypack-aliases.h>

struct SimpleRequestHelper;
namespace drogon
{
class WebSocketConnection;
class HttpResponse;
}
namespace ok
{
namespace smart_actor::connection
{
struct V8ActorState;
}
bool resolveRequestContext(arangodb::GeneralRequest& req);
// can't inherit HttpRequest because its final
class SimpleRequest : public arangodb::HttpRequest
{
public:
  std::unique_ptr<SimpleRequestHelper> helper;
  std::shared_ptr<drogon::WebSocketConnection> wsConnPtr{nullptr};
  ok::smart_actor::connection::V8ActorState* v8ActorState{nullptr};
  SimpleRequest(arangodb::ConnectionInfo const& info,
                std::shared_ptr<drogon::WebSocketConnection> wsConnPtr,
                std::unique_ptr<SimpleRequestHelper> helper,
                uint64_t mid,
                std::string path,
                std::string type,
                std::shared_ptr<VPackBuilder> payLoad);
  arangodb::Endpoint::TransportType transportType() override { return arangodb::Endpoint::TransportType::VST; }
  // size_t contentLength() const override {return _payload.size();}
  // arangodb::velocypack::StringRef rawPayload() const override {return VPackStringRef(reinterpret_cast<const char*>(_payload.data()), _payload.size());}
  arangodb::velocypack::Slice payload(bool strictValidation = true) override;
  // void setPayload(arangodb::velocypack::Buffer<uint8_t> buffer) override {_payload = std::move(buffer);}
  // void setDefaultContentType() override {_contentType = arangodb::rest::ContentType::JSON;}
};
class SimpleRestRequest : public arangodb::HttpRequest
{
public:
  std::unique_ptr<SimpleRequestHelper> helper;
  std::function<void(std::shared_ptr<drogon::HttpResponse> const&)> callback;
  //std::shared_ptr<ok::smart_actor::connection::V8ActorState> v8ActorState{nullptr};
  SimpleRestRequest(arangodb::ConnectionInfo const& info,
                    std::function<void(std::shared_ptr<drogon::HttpResponse> const&)> callback,
                    std::unique_ptr<SimpleRequestHelper> helper,
                    uint64_t mid,
                    std::string path,
                    std::string type,
                    std::shared_ptr<VPackBuilder> payLoad);
  arangodb::Endpoint::TransportType transportType() override { return arangodb::Endpoint::TransportType::HTTP; }
  // size_t contentLength() const override {return _payload.size();}
  // arangodb::velocypack::StringRef rawPayload() const override {return VPackStringRef(reinterpret_cast<const char*>(_payload.data()), _payload.size());}
  arangodb::velocypack::Slice payload(bool strictValidation = true) override;
  // void setPayload(arangodb::velocypack::Buffer<uint8_t> buffer) override {_payload = std::move(buffer);}
  // void setDefaultContentType() override {_contentType = arangodb::rest::ContentType::JSON;}
};
}  // namespace ok
