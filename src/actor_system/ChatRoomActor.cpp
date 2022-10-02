#include "actor_system/ChatRoomActor.hpp"
#include "actor_system/TableActor.hpp"
#include "actor_system/MutateActors.hpp"
#include "utils/time_functions.hpp"
#include "mutate/Mutate.hpp"
#include "pystring.hpp"
namespace ok
{
namespace smart_actor::chat_room_actor
{
void unsubscribeTableActors(ChatRoomActorPtr self, WsEvent const &event, ws_connector_actor_int connectionActor)
{
  auto event1 = event;
  auto event2 = event;
  auto event3 = event;
  event1[0] = "unsubscribe";
  event2[0] = "unsubscribe";
  event3[0] = "unsubscribe";
  event1[1] = "conversation_list";
  event2[1] = "participant_list";
  event3[1] = "message_list";
  self->send(self->state.conversationListActor, caf::unsubscribe_atom_v, event1, connectionActor);
  self->send(self->state.participantListActor, caf::unsubscribe_atom_v, event2, connectionActor);
  self->send(self->state.messageListActor, caf::unsubscribe_atom_v, event3, connectionActor);
}
void saveListActorOrUpdateArgs(ChatRoomActorPtr self,
                               subscribed_config_multi_index &subscribedActors,
                               DocumentKey memberKey,
                               DocumentKey conversationKey,
                               DocumentKey participantKey,
                               WsEvent const &event,
                               WsArguments const &args,
                               ws_connector_actor_int connectionActor) noexcept
{
  auto it = subscribedActors.get<2>().find(std::make_tuple(connectionActor, event));
  if (it != subscribedActors.get<2>().end())
  {
    subscribedActors.get<2>().modify(it,
                                     [](SubscriberConfig &config)
                                     {
                                       // currently we do nothing here
                                     });
  }
  else
  {
    SubscriberConfig config{connectionActor, std::move(memberKey), event, args, jsoncons::getStringKey(args, "project"), participantKey};
    self->state.subscribedActors.insert(std::move(config));
  }
}
/*void getInitialDataAndSend(ChatRoomActorPtr self, chat_room_actor_state &state, DocumentKey memberKey, WsEvent const &event, WsArguments const &args, ws_connector_actor_int connectionActor)
{
  self->request(state.conversationListActor, caf::infinite, get_initial_data_atom_v, event, args, memberKey, self)
      .then(
          [=](jsoncons::ojson const &resultData)
          {
            if (!resultData.empty()) { self->send(connectionActor, caf::forward_atom_v, resultData); }
          });
  self->request(state.participantListActor, caf::infinite, get_initial_data_atom_v, event, args, memberKey, self)
      .then(
          [=](jsoncons::ojson const &resultData)
          {
            if (!resultData.empty()) { self->send(connectionActor, caf::forward_atom_v, resultData); }
          });
  self->request(state.messageListActor, caf::infinite, get_initial_data_atom_v, event, args, memberKey, self)
      .then(
          [=](jsoncons::ojson const &resultData)
          {
            if (!resultData.empty()) { self->send(connectionActor, caf::forward_atom_v, resultData); }
          });
}*/
void participantSaveError(WsEvent const &event, ok::ErrorCode &erDb, ChatRoomActorPtr self, ws_connector_actor_int connectionActor)
{
  jsoncons::ojson participantSaveErrorJson;
  participantSaveErrorJson["description"] = "error saving participant (" + ok::errno_string(erDb) + ")";
  participantSaveErrorJson["error"] = true;
  auto msg = ok::smart_actor::connection::wsMessageBase();
  ok::smart_actor::connection::addEventAndJson(msg, event, participantSaveErrorJson);
  self->send(connectionActor, caf::forward_atom_v, msg);
}
void sendNoParticipant(WsEvent const &event, ChatRoomActorPtr self, ws_connector_actor_int connectionActor)
{
  jsoncons::ojson participantJson;
  participantJson["participant"] = false;
  auto msg = ok::smart_actor::connection::wsMessageBase();
  ok::smart_actor::connection::addEventAndJson(msg, event, participantJson);
  self->send(connectionActor, caf::forward_atom_v, msg);
}
void sendParticipantKey(WsEvent const &event, ChatRoomActorPtr self, std::string &participantKey, ws_connector_actor_int connectionActor)
{
  jsoncons::ojson participantJson;
  participantJson["participant"] = participantKey;
  auto msg = ok::smart_actor::connection::wsMessageBase();
  ok::smart_actor::connection::addEventAndJson(msg, event, participantJson);
  self->send(connectionActor, caf::forward_atom_v, msg);
}
void updateConvesation(WsEvent const &event,
                       DocumentKey const &conversationKey,
                       DocumentKey const &convRevKey,
                       DocumentKey const &projectKey,
                       DocumentKey const &memberKey,
                       jsoncons::ojson const &conversationBackendValue,
                       ChatRoomActorPtr self,
                       ws_connector_actor_int connectionActor)
{
  auto mutateEvent = event;
  mutateEvent[0] = "update";
  mutateEvent[1] = "conversation";
  jsoncons::ojson filter;
  filter["_key"] = conversationKey;
  filter["_rev"] = convRevKey;
  jsoncons::ojson updateMutateArgs;
  updateMutateArgs["f"] = filter;
  updateMutateArgs["value"] = jsoncons::ojson{};
  self->request(self->state.conversionMutateActor, caf::infinite, insert_and_get_atom_v, memberKey, mutateEvent, updateMutateArgs, conversationBackendValue, false, true, connectionActor);
}
void conversationEvent(ChatRoomActorPtr self,
                       DocumentKey const &memberKey,
                       DocumentKey const &conversationKey,
                       DocumentKey const &participantKey,
                       jsoncons::ojson const &message,
                       ws_connector_actor_int connectionActor,
                       std::function<void()> afterEventMutate)
{
  jsoncons::ojson event(jsoncons::json_array_arg, {"get", "message_mutate", "1"});
  jsoncons::ojson args;
  args["value"] = jsoncons::ojson{};
  args["conversation"] = conversationKey;
  jsoncons::ojson messageBackendValue;
  messageBackendValue["from"] = std::string{participantKey};
  messageBackendValue["type"] = "event";
  messageBackendValue["message"] = message;
  self->request(self->state.messageMutateActor, caf::infinite, insert_and_get_atom_v, memberKey, event, args, messageBackendValue, true, true, connectionActor)
      .then(
          [=](ok::ErrorCode er, std::unordered_set<DocumentKey> changedKeys)
          {
            if (!ok::isEr(er)) { afterEventMutate(); }
          });
}
void createParticipant(WsEvent const &event,
                       DocumentKey const &conversationKey,
                       DocumentKey const &projectKey,
                       DocumentKey const &memberKey,
                       ChatRoomActorPtr self,
                       jsoncons::ojson &value,
                       jsoncons::ojson &backendValue,
                       ws_connector_actor_int connectionActor,
                       std::function<void(std::string)> sendParticipant)
{
  // If MemberKey present it means existing member else use Name Key to get the name.
  jsoncons::ojson newParticipantMutateArgs;
  newParticipantMutateArgs["project"] = projectKey;
  newParticipantMutateArgs["conversation"] = conversationKey;
  newParticipantMutateArgs["value"] = value;
  self->request(self->state.participantMutateActor, caf::infinite, insert_and_get_atom_v, memberKey, event, newParticipantMutateArgs, backendValue, true, false, connectionActor)
      .then(
          [=](ok::ErrorCode er, std::unordered_set<DocumentKey> changedKeys)
          {
            if (ok::isEr(er)) { participantSaveError(event, er, self, connectionActor); }
            if (changedKeys.size() > 0)
            {
              sendParticipant(*changedKeys.begin());
              // update conversation to show changes realtime
              jsoncons::ojson conversationBackendValue;
              updateConvesation(event, conversationKey, "", projectKey, memberKey, conversationBackendValue, self, connectionActor);
            }
          });
}
chat_room_actor_int::behavior_type ChatRoomActor(ChatRoomActorPtr self, Database database, DocumentKey projectKey, DocumentKey conversationKey)
{
  self->set_exception_handler(ok::smart_actor::supervisor::default_exception_handler("Chat Room Actor"));
  self->set_exit_handler(
      [](caf::scheduled_actor *, caf::exit_msg &msg)
      {
        LOG_ERROR << "Chat Room Actor exit_handler error";
        LOG_ERROR << "source: " << caf::deep_to_string(msg.source) << " reason: " << caf::deep_to_string(msg.reason);
      });
  // span table actors:
  self->state.conversationListActor = self->spawn(ok::smart_actor::table_actor::TableActor<ws_connector_actor_int>, database, "conversation", false, true);
  self->monitor(self->state.conversationListActor);
  self->state.participantListActor = self->spawn(ok::smart_actor::table_actor::TableActor<ws_connector_actor_int>, database, "participant", false, true);
  self->monitor(self->state.participantListActor);
  self->state.messageListActor = self->spawn(ok::smart_actor::table_actor::TableActor<ws_connector_actor_int>, database, "message", false, true);
  self->monitor(self->state.messageListActor);
  // spawn mutate actors:
  using LA = std::vector<table_actor_int<ws_connector_actor_int>>;
  self->request(ok::smart_actor::supervisor::globalActor, caf::infinite, global_list_atom_v, database, "conversation")
      .then(
          [=](table_actor_int<ws_connector_actor_int> conversationListActor)
          {
            self->state.conversionMutateActor = self->spawn(ok::smart_actor::user::BaseMutateActor<ws_connector_actor_int>,
                                                            LA{self->state.conversationListActor, conversationListActor},
                                                            database,
                                                            "conversation",
                                                            ok::smart_actor::user::emptyPreProcess,
                                                            ok::smart_actor::user::emptyPreProcess);
            self->monitor(self->state.conversionMutateActor);
          });
  self->state.participantMutateActor = self->spawn(ok::smart_actor::user::BaseMutateActor<ws_connector_actor_int>,
                                                   LA{self->state.participantListActor},
                                                   database,
                                                   "participant",
                                                   ok::smart_actor::user::emptyPreProcess,
                                                   ok::smart_actor::user::emptyPreProcess);
  self->monitor(self->state.participantMutateActor);
  self->state.messageMutateActor = self->spawn(ok::smart_actor::user::BaseMutateActor<ws_connector_actor_int>,
                                               LA{self->state.messageListActor},
                                               database,
                                               "message",
                                               ok::smart_actor::user::emptyPreProcess,
                                               ok::smart_actor::user::emptyPreProcess);
  self->monitor(self->state.messageMutateActor);
  // when join Chat Room Actor:
  // 1. if conversation row not exist create it by mutation actor
  // 2. create a participant by mutation actor
  //
  // add following properties to converstion row:
  /*"date_created": "2015-12-16T22:18:37Z",
  "date_updated": "2015-12-16T22:18:38Z",
  "state": "inactive",
  "timers": {
    "date_inactive": "2015-12-16T22:19:38Z",
    "date_closed": "2015-12-16T22:28:38Z"
  },*/
  //
  // If no message happen between 5 minutes mark conversation as inactive
  // If message is send and its inactive mark as active
  // messages can be sent to active or inactive conversation, if conversation is marked as closed no new messages can be sent.
  // write Guest Frontend:
  // 1. show chat button
  // 2. on click of it, 1. if not logged member show a form
  // 3. on submitting form show messageList and InputBox
  // 4. show close chat button.
  // write Admin Frontend
  //
  // implement video meeting and breakout rooms feature too.
  // First think of model.
  // how to handle images and videos at backend that are uploaded from frontend?
  // Use MultiPartParser
  return {
      [=](caf::subscribe_atom, WsEvent const &event, WsArguments const &args, DocumentKey const &memberKey, ws_connector_actor_int connectionActor)
      {
        if (!ok::smart_actor::table_actor::argsSchemaCheck(args)) return false;
        // subscribe to 3 list actors:
        auto event1 = event;
        auto event2 = event;
        auto event3 = event;
        event1[1] = "conversation_list";
        event2[1] = "participant_list";
        event3[1] = "message_list";
        jsoncons::ojson conversationArgs;
        jsoncons::ojson conversationFilter;
        conversationFilter["_key"] = "=\"" + conversationKey + "\"";
        conversationArgs["f"] = conversationFilter;
        conversationArgs["project"] = projectKey;
        conversationArgs["h"] = false;
        self->request(self->state.conversationListActor, caf::infinite, caf::subscribe_atom_v, event1, conversationArgs, memberKey, connectionActor);
        jsoncons::ojson participantArgs;
        participantArgs["conversation"] = conversationKey;
        participantArgs["h"] = false;
        self->request(self->state.participantListActor, caf::infinite, caf::subscribe_atom_v, event2, participantArgs, memberKey, connectionActor);
        jsoncons::ojson messageArgs;
        messageArgs["conversation"] = conversationKey;
        messageArgs["h"] = false;
        self->request(self->state.messageListActor, caf::infinite, caf::subscribe_atom_v, event3, messageArgs, memberKey, connectionActor);
        // create participant if no participant key given
        // send participant key to the frontend
        // FOR t IN 1..1000 OUTBOUND 'conversation/15842977' has_participant
        //  FILTER t._key == "" || t.memberKey == "1"
        jsoncons::ojson participantFindArgs;
        jsoncons::ojson participantMutateArgsFilter(jsoncons::json_array_arg);
        if (!memberKey.empty())
        {
          jsoncons::ojson participantMutateArgsFilter2;
          participantMutateArgsFilter2["memberKey"] = "=\"" + memberKey + "\"";
          participantMutateArgsFilter2["active"] = true;
          participantMutateArgsFilter.push_back(participantMutateArgsFilter2);
        }
        else
        {
          jsoncons::ojson participantMutateArgsFilter2;
          participantMutateArgsFilter2["memberKey"] = "=\"\"";
          participantMutateArgsFilter.push_back(participantMutateArgsFilter2);
        }
        // get participant = key || memberkey = memberkey to find the participant
        participantFindArgs["f"] = participantMutateArgsFilter;
        participantFindArgs["project"] = projectKey;
        participantFindArgs["conversation"] = conversationKey;
        participantFindArgs["h"] = false;
        auto sendParticipant = [=](std::string participantKey)
        {
          saveListActorOrUpdateArgs(self, self->state.subscribedActors, memberKey, conversationKey, participantKey, event, args, connectionActor);
          sendParticipantKey(event, self, participantKey, connectionActor);
        };
        self->request(self->state.participantListActor, caf::infinite, get_initial_data_atom_v, event, participantFindArgs, memberKey, connectionActor)
            .then(
                [=](jsoncons::ojson result)
                {
                  if (result["r"]["result"].size() == 0)
                  {
                    if (!jsoncons::ObjectMemberBoolVal(args, "isAdmin"))
                    {
                      jsoncons::ojson value;
                      value["_key"] = "";
                      value["name"] = memberKey.empty() ? "Visitor" : "";
                      value["email"] = memberKey.empty() ? "" : "";
                      value["contactNo"] = memberKey.empty() ? "" : "";
                      jsoncons::ojson backendValue;
                      backendValue["memberKey"] = memberKey;
                      backendValue["active"] = true;
                      backendValue["lastReadMessageIndex"] = 1;
                      backendValue["lastReadTimestamp"] = utils::time::getEpochMilliseconds();
                      createParticipant(event, conversationKey, projectKey, memberKey, self, value, backendValue, connectionActor, sendParticipant);
                    }
                    else
                      sendNoParticipant(event, self, connectionActor);
                  }
                  else
                    sendParticipant(result["r"]["result"][0]["_key"].as<std::string>());
                });
        auto it = self->state.subscribedActors.get<2>().find(std::make_tuple(connectionActor, event));
        if (it != self->state.subscribedActors.get<2>().end()) return true;
        else
          return false;
      },
      [=](caf::unsubscribe_atom,
          WsEvent const &event,
          WsArguments const &args,
          DocumentKey const &memberKey,
          DocumentKey const &convKey,
          DocumentKey const &convRevKey,
          ws_connector_actor_int connectionActor)
      {
        jsoncons::ojson newEvent{event};
        newEvent[0] = "subscribe";
        auto it = self->state.subscribedActors.get<2>().find(std::make_tuple(connectionActor, newEvent));
        if (it != self->state.subscribedActors.get<2>().end())
        {
          unsubscribeTableActors(self, event, connectionActor);
          self->state.subscribedActors.get<2>().erase(it);
        }
        // TODO: when no subscribers close all actors
      },
      [=](end_conversation_atom,
          WsEvent const &event,
          WsArguments const &args,
          DocumentKey const &memberKey,
          DocumentKey const &convKey,
          DocumentKey const &convRevKey,
          ws_connector_actor_int connectionActor)
      {
        jsoncons::ojson newEvent{event};
        newEvent[0] = "subscribe";
        newEvent[1] = "conversation";
        auto it = self->state.subscribedActors.get<2>().find(std::make_tuple(connectionActor, newEvent));
        if (it != self->state.subscribedActors.get<2>().end())
        {
          // TODO: when no subscribers close all actors
          // Todo: only participants of the chat can close the chat.
          conversationEvent(self,
                            memberKey,
                            convKey,
                            it->participantKey,
                            "conversation closed",
                            connectionActor,
                            [=]()
                            {
                              jsoncons::ojson conversationBackendValue;
                              conversationBackendValue["state"] = "closed";
                              updateConvesation(event, conversationKey, convRevKey, projectKey, memberKey, conversationBackendValue, self, connectionActor);
                              for (auto &config : self->state.subscribedActors)
                              {
                                unsubscribeTableActors(self, config.event, connectionActor);
                                jsoncons::ojson endChatJson;
                                endChatJson["error"] = false;
                                auto msg = ok::smart_actor::connection::wsMessageBase();
                                auto event = config.event;
                                event[0] = "update";
                                event[1] = "conversation_end";
                                ok::smart_actor::connection::addEventAndJson(msg, event, endChatJson);
                                self->send(config.actor, caf::forward_atom_v, msg);
                              }
                              self->state.subscribedActors.clear();
                            });
        }
        else
        {
          // only joined participan can close the chat.
        }
      },
      [=](join_participant_atom, WsEvent const &event, jsoncons::ojson const &args, DocumentKey const &memberKey, ws_connector_actor_int connectionActor)
      {
        if (jsoncons::ObjectMemberBoolVal(args, "isAdmin") && jsoncons::ObjectMemberIsString(args, "alias"))
        {
          auto alias = args["alias"].as_string();
          std::vector<std::string> result;
          pystring::split(alias, result, "/");
          if (result.size() == 2)
          {
            auto &key = result[0];
            auto &keyValue = result[1];
            if (key == "member")
            {
              jsoncons::ojson value;
              jsoncons::ojson backendValue;
              {
                jsoncons::ojson r;
                if (auto [er, myResp] = ok::db::getDocumentWithProjection(database, "member", keyValue, ""); ok::isEr(er))
                {
                  // error
                }
                else
                {
                  auto slice = myResp->slices().front().get("result");
                  if (slice.length() == 1)
                  {
                    backendValue["email"] = slice[0].get("email").copyString();
                    std::string name;
                    if (slice[0].hasKey("firstName")) name = slice[0].get("firstName").copyString();
                    if (slice[0].hasKey("firstName"))
                    {
                      name += " ";
                      name += slice[0].get("lastName").copyString();
                    }
                    backendValue["name"] = name;
                  }
                }
              }
              backendValue["_key"] = "";
              backendValue["contactNo"] = "";
              backendValue["memberKey"] = keyValue;
              backendValue["active"] = true;
              backendValue["lastReadMessageIndex"] = 1;
              backendValue["lastReadTimestamp"] = utils::time::getEpochMilliseconds();
              auto eventConversation = event;
              eventConversation[0] = "subscribe";
              eventConversation[1] = "conversation";
              createParticipant(event,
                                conversationKey,
                                projectKey,
                                memberKey,
                                self,
                                value,
                                backendValue,
                                connectionActor,
                                [=](std::string participantKey)
                                {
                                  saveListActorOrUpdateArgs(self, self->state.subscribedActors, memberKey, conversationKey, participantKey, eventConversation, args, connectionActor);
                                  sendParticipantKey(eventConversation, self, participantKey, connectionActor);
                                });
            }
            else if (key == "alias")
            {
              jsoncons::ojson value;
              jsoncons::ojson backendValue;
              {
                jsoncons::ojson r;
                if (auto [er, myResp] = ok::db::getDocumentWithProjection(database, "alias", keyValue, ""); ok::isEr(er))
                {
                  // error
                }
                else
                {
                  auto slice = myResp->slices().front().get("result");
                  if (slice.length() == 1)
                  {
                    if (slice[0].hasKey("name")) backendValue["name"] = slice[0].get("name").copyString();
                  }
                }
              }
              backendValue["_key"] = "";
              backendValue["email"] = "";
              backendValue["contactNo"] = "";
              backendValue["memberKey"] = memberKey;
              backendValue["active"] = true;
              backendValue["lastReadMessageIndex"] = 1;
              backendValue["lastReadTimestamp"] = utils::time::getEpochMilliseconds();
              auto eventConversation = event;
              eventConversation[0] = "subscribe";
              eventConversation[1] = "conversation";
              createParticipant(event,
                                conversationKey,
                                projectKey,
                                memberKey,
                                self,
                                value,
                                backendValue,
                                connectionActor,
                                [=](std::string participantKey)
                                {
                                  saveListActorOrUpdateArgs(self, self->state.subscribedActors, memberKey, conversationKey, participantKey, eventConversation, args, connectionActor);
                                  sendParticipantKey(eventConversation, self, participantKey, connectionActor);
                                });
            }
            else if (key == "participant")
            {
              jsoncons::ojson value;
              jsoncons::ojson backendValue;
              backendValue["_key"] = "";
              backendValue["name"] = "";
              backendValue["email"] = "";
              backendValue["contactNo"] = "";
              backendValue["memberKey"] = memberKey;
              backendValue["active"] = true;
              backendValue["joinAs"] = keyValue;
              backendValue["lastReadMessageIndex"] = 1;
              backendValue["lastReadTimestamp"] = utils::time::getEpochMilliseconds();
              auto eventConversation = event;
              eventConversation[0] = "subscribe";
              eventConversation[1] = "conversation";
              createParticipant(event,
                                conversationKey,
                                projectKey,
                                memberKey,
                                self,
                                value,
                                backendValue,
                                connectionActor,
                                [=](std::string participantKey)
                                {
                                  saveListActorOrUpdateArgs(self, self->state.subscribedActors, memberKey, conversationKey, participantKey, eventConversation, args, connectionActor);
                                  sendParticipantKey(eventConversation, self, participantKey, connectionActor);
                                });
            }
          }
        }
      },
      [=](create_message_atom,
          WsEvent const &event,
          jsoncons::ojson &args,
          DocumentKey const &memberKey,
          DocumentKey const &convKey,
          DocumentKey const &convRevKey,
          ws_connector_actor_int connectionActor)
      {
        auto it = self->state.subscribedActors.get<1>().find(std::make_tuple(connectionActor));
        if (it != self->state.subscribedActors.get<1>().end() && jsoncons::ObjectMemberIsObject(args, "value"))
        {
          jsoncons::ojson backendValue;
          backendValue["from"] = std::string{it->participantKey};
          self->request(self->state.messageMutateActor, caf::infinite, insert_and_get_atom_v, memberKey, event, args, backendValue, true, false, connectionActor)
              .then(
                  [=](ok::ErrorCode er, std::unordered_set<DocumentKey> changedKeys)
                  {
                    self->send(connectionActor, caf::forward_atom_v, ok::smart_actor::connection::addSuccess(ok::smart_actor::connection::wsMessageBase(), event));
                    jsoncons::ojson conversationBackendValue;
                    updateConvesation(event, conversationKey, convRevKey, projectKey, memberKey, conversationBackendValue, self, connectionActor);
                  });
        }
      },
      [=](typing_event_atom, WsEvent const &event, jsoncons::ojson &args, DocumentKey const &memberKey, ws_connector_actor_int connectionActor) {  // send typing event to all actor except current one
        auto it = self->state.subscribedActors.get<1>().find(std::make_tuple(connectionActor));
        if (it != self->state.subscribedActors.get<1>().end())
        {
          for (auto &subscriberConfig : self->state.subscribedActors)
          {
            if (subscriberConfig.actor == connectionActor) continue;
            jsoncons::ojson typingEventJson;
            typingEventJson["typing"] = true;
            typingEventJson["_key"] = it->participantKey;
            auto msg = ok::smart_actor::connection::wsMessageBase();
            ok::smart_actor::connection::addEventAndJson(msg, subscriberConfig.event, typingEventJson);
            self->send(subscriberConfig.actor, caf::forward_atom_v, msg);
          }
        }
      },
      [=](modify_participan_atom, WsEvent const &event, jsoncons::ojson &args, DocumentKey const &memberKey, ws_connector_actor_int connectionActor)
      {
        jsoncons::ojson backendValue;
        backendValue["memberKey"] = memberKey;
        self->send(self->state.participantMutateActor, insert_atom_v, memberKey, event, args, backendValue, true, false, connectionActor);
      },
      [=](get_conversation_list_actor_atom) { return self->state.conversationListActor; },
      [=](get_message_list_actor_atom) { return self->state.messageListActor; },
      [=](get_participant_list_actor_atom) { return self->state.participantListActor; },
      [=](get_conversation_muate_actor_atom) { return self->state.conversionMutateActor; },
      [=](get_message_muate_actor_atom) { return self->state.messageMutateActor; },
      [=](get_participant_muate_actor_atom) { return self->state.participantMutateActor; },
      [=](schema_changed_atom, std::string const &schemaKey)
      {
        if (schemaKey == "conversation")
        {
          self->send(self->state.conversationListActor, schema_changed_atom_v);
          self->send(self->state.conversionMutateActor, schema_changed_atom_v);
        }
        else if (schemaKey == "participant")
        {
          self->send(self->state.participantListActor, schema_changed_atom_v);
          self->send(self->state.participantMutateActor, schema_changed_atom_v);
        }
        else if (schemaKey == "message")
        {
          self->send(self->state.messageListActor, schema_changed_atom_v);
          self->send(self->state.messageMutateActor, schema_changed_atom_v);
        }
      },
      [=](caf::forward_atom, jsoncons::ojson const &result)
      {
        // Not used handler
        for (auto &subscriberConfig : self->state.subscribedActors) { self->send(subscriberConfig.actor, caf::forward_atom_v, result); }
      },
      [=](conn_exit_atom, ws_connector_actor_int connectionActor)
      {
        auto it = self->state.subscribedActors.get<1>().equal_range(std::make_tuple(connectionActor));
        for (auto Itr = it.first; Itr != it.second; ++Itr) { unsubscribeTableActors(self, Itr->event, connectionActor); }
        self->state.subscribedActors.get<1>().erase(it.first, it.second);
        // TODO: when no subscribers close all actors
      },
      CONN_EXIT};
}
ErrorCode conversationDeletePreProcess(DocumentKey const &database, WsArguments const &args, ok::smart_actor::connection::MutateEventType et, Fields &fields)
{
  if (et == ok::smart_actor::connection::MutateEventType::delete_)
  {
    if (auto [er, userSuppliedDocumentJson] = ok::mutate_actor::impl::get_merged_bind_vars::getKeyRev(args); ok::isEr(er)) return er;
    else
    {
      if (auto [er, state] = ok::db::chat::getConversationState(database, userSuppliedDocumentJson["_key"].as_string()); ok::isEr(er)) return er;
      else if (state == "closed")
        return ok::ErrorCode::ERROR_NO_ERROR;
      else
        return ok::ErrorCode::ERROR_FORBIDDEN;
    }
  }
  return ok::ErrorCode::ERROR_NO_ERROR;
}
ErrorCode conversationDeletePostProcess(DocumentKey const &database, WsArguments const &args, ok::smart_actor::connection::MutateEventType et, Fields &fields)
{
  if (et == ok::smart_actor::connection::MutateEventType::delete_)
  {
    if (auto [er, userSuppliedDocumentJson] = ok::mutate_actor::impl::get_merged_bind_vars::getKeyRev(args); ok::isEr(er)) return er;
    else
    {
      return ok::db::chat::deteteChatData(database, userSuppliedDocumentJson["_key"].as_string());
    }
  }
  return ok::ErrorCode::ERROR_NO_ERROR;
}
}  // namespace smart_actor::chat_room_actor
}  // namespace ok
