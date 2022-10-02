#pragma once
#include <jsoncons/json.hpp>
#include "caf/all.hpp"
#include "alias.hpp"
#include "CAF.hpp"
#include "WsConnectionActor.hpp"
#include "utils/ErrorConstants.hpp"
#include <fuerte/connection.h>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include "mutate/Field.hpp"
namespace ok
{
enum class ErrorCode;
namespace smart_actor::chat_room_actor
{
struct SubscriberConfig
{
  SubscriberConfig(ws_connector_actor_int actor_, DocumentKey memberKey_, WsEvent event_, WsArguments args_, DocumentKey projectKey_, DocumentKey participantkey_)
      : actor(actor_), memberKey(memberKey_), event(event_), args(args_), projectKey(projectKey_), participantKey(participantkey_)
  {
  }
  ws_connector_actor_int actor;
  DocumentKey memberKey;
  WsEvent event;
  WsArguments args;
  DocumentKey projectKey;
  DocumentKey participantKey;
};
struct actor_key : boost::multi_index::composite_key<SubscriberConfig, BOOST_MULTI_INDEX_MEMBER(SubscriberConfig, ws_connector_actor_int, actor)>
{
};
struct actor_event_key
    : boost::multi_index::composite_key<SubscriberConfig, BOOST_MULTI_INDEX_MEMBER(SubscriberConfig, ws_connector_actor_int, actor), BOOST_MULTI_INDEX_MEMBER(SubscriberConfig, WsEvent, event)>
{
};
struct actor_participant_key : boost::multi_index::composite_key<SubscriberConfig,
                                                                 BOOST_MULTI_INDEX_MEMBER(SubscriberConfig, ws_connector_actor_int, actor),
                                                                 BOOST_MULTI_INDEX_MEMBER(SubscriberConfig, DocumentKey, participantKey)>
{
};
struct chat_subscriber_all_key : boost::multi_index::composite_key<SubscriberConfig,
                                                                   BOOST_MULTI_INDEX_MEMBER(SubscriberConfig, ws_connector_actor_int, actor),
                                                                   BOOST_MULTI_INDEX_MEMBER(SubscriberConfig, DocumentKey, memberKey),
                                                                   BOOST_MULTI_INDEX_MEMBER(SubscriberConfig, WsEvent, event),
                                                                   BOOST_MULTI_INDEX_MEMBER(SubscriberConfig, WsArguments, args),
                                                                   BOOST_MULTI_INDEX_MEMBER(SubscriberConfig, DocumentKey, projectKey),
                                                                   BOOST_MULTI_INDEX_MEMBER(SubscriberConfig, DocumentKey, participantKey)>
{
};
using subscribed_config_multi_index = boost::multi_index::multi_index_container<SubscriberConfig,
                                                                                boost::multi_index::indexed_by<boost::multi_index::ordered_non_unique<chat_subscriber_all_key>,
                                                                                                               boost::multi_index::ordered_non_unique<actor_key>,
                                                                                                               boost::multi_index::ordered_non_unique<actor_event_key>,
                                                                                                               boost::multi_index::ordered_non_unique<actor_participant_key>>>;
struct chat_room_actor_state
{
  static inline constexpr char const *name = "chat-room-actor";
  Database database;
  SchemaKey projectKey;
  subscribed_config_multi_index subscribedActors;
  table_actor_int<ws_connector_actor_int> conversationListActor;
  table_actor_int<ws_connector_actor_int> participantListActor;
  table_actor_int<ws_connector_actor_int> messageListActor;
  base_mutate_actor_int<ws_connector_actor_int> conversionMutateActor;
  base_mutate_actor_int<ws_connector_actor_int> participantMutateActor;
  base_mutate_actor_int<ws_connector_actor_int> messageMutateActor;
};
using ChatRoomActorPtr = chat_room_actor_int::stateful_pointer<chat_room_actor_state>;
chat_room_actor_int::behavior_type ChatRoomActor(ChatRoomActorPtr self, Database database, DocumentKey projectKey, DocumentKey conversationKey);
using Fields = tsl::ordered_map<ID, ok::mutate_schema::Field>;
ErrorCode conversationDeletePreProcess(DocumentKey const &database, WsArguments const &args, ok::smart_actor::connection::MutateEventType et, Fields &fields);
ErrorCode conversationDeletePostProcess(DocumentKey const &database, WsArguments const &args, ok::smart_actor::connection::MutateEventType et, Fields &fields);
}  // namespace smart_actor::chat_room_actor
}  // namespace ok
