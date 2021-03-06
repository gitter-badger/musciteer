// ----------------------------------------------------------------------------
//
//     Author     : Benny Bach <benny.bach@gmail.com>
//                  Copyright (C) 2016
//
// ----------------------------------------------------------------------------
#ifndef __player__message_h__
#define __player__message_h__

// ----------------------------------------------------------------------------
#include "audio_buffer.h"

// ----------------------------------------------------------------------------
#include "../dm/track.h"

// ----------------------------------------------------------------------------
#include <dripcore/channel.h>
#include <dripcore/task.h>

// ----------------------------------------------------------------------------
class message;

// ----------------------------------------------------------------------------
using message_channel = dripcore::channel<message>;

// ----------------------------------------------------------------------------
class audio_output_response
{
public:
  audio_output_response() : error_code(0)
  {
  }
  audio_output_response(audio_output_response&& other)
  {
    error_code = std::move(other.error_code);
    error_message = std::move(other.error_message);
  }
public:
  int error_code;
  std::string error_message;
};

// ----------------------------------------------------------------------------
class audio_output_notification_message
{
public:
  audio_output_notification_message()
  {
  }
  audio_output_notification_message(audio_output_notification_message&& other)
  {
    *this = std::move(other);
  }
  audio_output_notification_message& operator=(audio_output_notification_message&& rhs)
  {
    channel = std::move(rhs.channel);
    return *this;
  }
public:
  message_channel channel;
};

// ----------------------------------------------------------------------------
class audio_output_subscribe_message : public audio_output_notification_message
{
};

// ----------------------------------------------------------------------------
class audio_output_unsubscribe_message : public audio_output_notification_message
{
};

// ----------------------------------------------------------------------------
class audio_output_device_list_request
{
public:
  audio_output_device_list_request()
  {
  }
  audio_output_device_list_request(audio_output_device_list_request&& other)
  {
    *this = std::move(other);
  }
  audio_output_device_list_request& operator=(audio_output_device_list_request&& rhs)
  {
    current = std::move(rhs.current);
    reply = std::move(rhs.reply);
    return *this;
  }
public:
  std::string current;
  message_channel reply;
};

// ----------------------------------------------------------------------------
class audio_output_device_list_response
{
public:
  audio_output_device_list_response()
  {
  }
  audio_output_device_list_response(audio_output_device_list_response&& other)
  {
    device_names = std::move(other.device_names);
    current = std::move(other.current);
  }
public:
  std::vector<std::string> device_names;
  std::string current;
};

// ----------------------------------------------------------------------------
class settings_changed_message
{
public:
  settings_changed_message()
  {
  }
  settings_changed_message(settings_changed_message&& other)
  {
  }
};

// ----------------------------------------------------------------------------
class audio_output_open_request
{
public:
  audio_output_open_request() : device_name()
  {
  }
  audio_output_open_request(audio_output_open_request&& other)
  {
    device_name = std::move(other.device_name);
    reply = std::move(other.reply);
  }
public:
  std::string device_name;
  message_channel reply;
};

// ----------------------------------------------------------------------------
using audio_output_open_response = audio_output_response;

// ----------------------------------------------------------------------------
class audio_output_close_request
{
public:
  audio_output_close_request()
  {
  }
  audio_output_close_request(audio_output_close_request&& other)
  {
    reply = std::move(other.reply);
  }
public:
  message_channel reply;
};

// ----------------------------------------------------------------------------
using audio_output_close_response = audio_output_response;

// ----------------------------------------------------------------------------
class play_request
{
public:
  play_request() : id()
  {
  }
  play_request(play_request&& other)
  {
    id = std::move(other.id);
  }
public:
  std::string id;
};

// ----------------------------------------------------------------------------
class stop_request
{
public:
  stop_request()
  {
  }
  stop_request(stop_request&& other)
  {
  }
};

// ----------------------------------------------------------------------------
class skip_request
{
public:
  skip_request()
  {
  }
  skip_request(skip_request&& other)
  {
  }
};

// ----------------------------------------------------------------------------
class queue_request
{
public:
  queue_request() : id()
  {
  }
  queue_request(queue_request&& other)
  {
    id = std::move(other.id);
  }
public:
  std::string id;
};

// ----------------------------------------------------------------------------
class audio_output_stream_begin
{
public:
  audio_output_stream_begin() : stream_id(0)
  {
  }
  audio_output_stream_begin(audio_output_stream_begin&& other)
  {
    stream_id = std::move(other.stream_id);
    sample_rate = std::move(other.sample_rate);
    length = std::move(other.length);
    completed_buffer_ch = std::move(other.completed_buffer_ch);
  }
public:
  unsigned stream_id;
  unsigned sample_rate;
  std::chrono::milliseconds length;
  dripcore::channel<audio_buffer> completed_buffer_ch;
};

// ----------------------------------------------------------------------------
class audio_output_stream_end
{
public:
  audio_output_stream_end()
  {
  }
  audio_output_stream_end(audio_output_stream_end&& other)
  {
    reply = std::move(other.reply);
  }
public:
  message_channel reply;
};

// ----------------------------------------------------------------------------
class audio_output_stream_buffer
{
public:
  audio_output_stream_buffer() : buffer()
  {
  }
  audio_output_stream_buffer(audio_output_stream_buffer&& other)
  {
    buffer = std::move(other.buffer);
  }
public:
  audio_buffer buffer;
};

// ----------------------------------------------------------------------------
class audio_output_stream_begin_notification
{
public:
  audio_output_stream_begin_notification()
  {
  }
  audio_output_stream_begin_notification(audio_output_stream_begin_notification&& other)
  {
    stream_id = std::move(other.stream_id);
  }
public:
  unsigned stream_id;
};

// ----------------------------------------------------------------------------
class audio_output_stream_end_notification
{
public:
  audio_output_stream_end_notification()
  {
  }
  audio_output_stream_end_notification(audio_output_stream_end_notification&& other)
  {
    stream_id = std::move(other.stream_id);
  }
public:
  unsigned stream_id;
};

// ----------------------------------------------------------------------------
class audio_output_stream_progress_notification
{
public:
  audio_output_stream_progress_notification()
  {
  }
  audio_output_stream_progress_notification(audio_output_stream_progress_notification&& other)
  {
    stream_id = std::move(other.stream_id);
    duration = std::move(other.duration);
    length = std::move(other.length);
  }
public:
  unsigned stream_id;
  unsigned duration;
  unsigned length;
};

// ----------------------------------------------------------------------------
class queue_update_notification
{
public:
  queue_update_notification()
  {
  }
  queue_update_notification(queue_update_notification&& other)
  {
    queue_size = std::move(other.queue_size);
    track = std::move(other.track);
  }
public:
  unsigned queue_size;
  std::shared_ptr<musciteer::dm::track> track;
};

// ----------------------------------------------------------------------------
class player_state_notification
{
public:
  player_state_notification()
  {
  }
  player_state_notification(player_state_notification&& other)
  {
    state = std::move(other.state);
  }
public:
  unsigned state;
};

// ----------------------------------------------------------------------------
class source_notification
{
public:
  enum class id
  {
    status,
    error,
    track_unavailable
  };
public:
  source_notification()
  {
  }
  source_notification(source_notification&& other)
  {
    type = std::move(other.type);
    source_name = std::move(other.source_name);
    message = std::move(other.message);
  }
public:
  id type;
  std::string source_name;
  std::string message;
};

// ----------------------------------------------------------------------------
class stream_data_request
{
public:
  stream_data_request() : stream_id()
  {
  }
  stream_data_request(stream_data_request&& other)
  {
    stream_id = std::move(other.stream_id);
    reply = std::move(other.reply);
  }
public:
  unsigned stream_id;
  message_channel reply;
};

// ----------------------------------------------------------------------------
class stream_data_response
{
public:
  stream_data_response() : stream_id()
  {
  }
  stream_data_response(stream_data_response&& other)
  {
    stream_id = std::move(other.stream_id);
    track = std::move(other.track);
  }
public:
  int stream_id;
  std::shared_ptr<musciteer::dm::track> track;
};

// ----------------------------------------------------------------------------
class message
{
public:
  enum id
  {
    undefined_id,
    device_list_req_id,
    device_list_res_id,
    settings_changed_id,
    open_req_id,
    open_res_id,
    close_req_id,
    close_res_id,
    play_req_id,
    stop_req_id,
    skip_req_id,
    queue_req_id,
    stream_begin_id,
    stream_end_id,
    stream_buffer_id,
    stream_begin_notify_id,
    stream_end_notify_id,
    stream_progress_notify_id,
    subscribe_id,
    unsubscribe_id,
    queue_update_id,
    player_state_id,
    source_notify_id,
    stream_data_req_id,
    stream_data_res_id,
  };
public:
  message() : type(undefined_id), ref(0)
  {
  }
  message(unsigned type, long long ref=0) : type(type), ref(ref)
  {
    switch ( type )
    {
      case device_list_req_id:
        new (&device_list_req) audio_output_device_list_request();
        break;
      case device_list_res_id:
        new (&device_list_res) audio_output_device_list_response();
        break;
      case settings_changed_id:
        new (&settings_changed) settings_changed_message();
        break;
      case open_req_id:
        new (&open_req) audio_output_open_request();
        break;
      case open_res_id:
        new (&open_res) audio_output_open_response();
        break;
      case close_req_id:
        new (&close_req) audio_output_close_request();
        break;
      case close_res_id:
        new (&close_res) audio_output_close_response();
        break;
      case play_req_id:
        new (&play_req) play_request();
        break;
      case stop_req_id:
        new (&stop_req) stop_request();
        break;
      case skip_req_id:
        new (&skip_req) skip_request();
        break;
      case queue_req_id:
        new (&queue_req) queue_request();
        break;
      case stream_begin_id:
        new (&stream_begin) audio_output_stream_begin();
        break;
      case stream_end_id:
        new (&stream_end) audio_output_stream_end();
        break;
      case stream_buffer_id:
        new (&stream_buffer) audio_output_stream_buffer();
        break;
      case stream_begin_notify_id:
        new (&stream_begin_notify) audio_output_stream_begin_notification();
        break;
      case stream_end_notify_id:
        new (&stream_end_notify) audio_output_stream_end_notification();
        break;
      case stream_progress_notify_id:
        new (&stream_progress_notify) audio_output_stream_progress_notification();
        break;
      case subscribe_id:
        new (&subscribe) audio_output_subscribe_message();
        break;
      case unsubscribe_id:
        new (&unsubscribe) audio_output_unsubscribe_message();
        break;
      case queue_update_id:
        new (&queue_update) queue_update_notification();
        break;
      case player_state_id:
        new (&player_state) player_state_notification();
        break;
      case source_notify_id:
        new (&source_notify) source_notification();
        break;
      case stream_data_req_id:
        new (&stream_data_req) stream_data_request();
        break;
      case stream_data_res_id:
        new (&stream_data_res) stream_data_response();
        break;
    }
  }
public:
  message(message&& other)
  {
    switch( other.type )
    {
      case device_list_req_id:
        new (&device_list_req) audio_output_device_list_request(std::move(other.device_list_req));
        break;
      case device_list_res_id:
        new (&device_list_res) audio_output_device_list_response(std::move(other.device_list_res));
        break;
      case settings_changed_id:
        new (&settings_changed) settings_changed_message(std::move(other.settings_changed));
        break;
      case open_req_id:
        new (&open_req) audio_output_open_request(std::move(other.open_req));
        break;
      case open_res_id:
        new (&open_res) audio_output_open_response(std::move(other.open_res));
        break;
      case close_req_id:
        new (&close_req) audio_output_close_request(std::move(other.close_req));
        break;
      case close_res_id:
        new (&close_res) audio_output_close_response(std::move(other.close_res));
        break;
      case play_req_id:
        new (&play_req) play_request(std::move(other.play_req));
        break;
      case stop_req_id:
        new (&stop_req) stop_request(std::move(other.stop_req));
        break;
      case skip_req_id:
        new (&skip_req) skip_request(std::move(other.skip_req));
        break;
      case queue_req_id:
        new (&queue_req) queue_request(std::move(other.queue_req));
        break;
      case stream_begin_id:
        new (&stream_begin) audio_output_stream_begin(std::move(other.stream_begin));
        break;
      case stream_end_id:
        new (&stream_end) audio_output_stream_end(std::move(other.stream_end));
        break;
      case stream_buffer_id:
        new (&stream_buffer) audio_output_stream_buffer(std::move(other.stream_buffer));
        break;
      case stream_begin_notify_id:
        new (&stream_begin_notify) audio_output_stream_begin_notification(std::move(other.stream_begin_notify));
        break;
      case stream_end_notify_id:
        new (&stream_end_notify) audio_output_stream_end_notification(std::move(other.stream_end_notify));
        break;
      case stream_progress_notify_id:
        new (&stream_progress_notify) audio_output_stream_progress_notification(std::move(other.stream_progress_notify));
        break;
      case subscribe_id:
        new (&subscribe) audio_output_subscribe_message(std::move(other.subscribe));
        break;
      case unsubscribe_id:
        new (&unsubscribe) audio_output_unsubscribe_message(std::move(other.unsubscribe));
        break;
      case queue_update_id:
        new (&queue_update) queue_update_notification(std::move(other.queue_update));
        break;
      case player_state_id:
        new (&player_state) player_state_notification(std::move(other.player_state));
        break;
      case source_notify_id:
        new (&source_notify) source_notification(std::move(other.source_notify));
        break;
      case stream_data_req_id:
        new (&stream_data_req) stream_data_request(std::move(other.stream_data_req));
        break;
      case stream_data_res_id:
        new (&stream_data_res) stream_data_response(std::move(other.stream_data_res));
        break;
    }
    type = other.type;
    ref = other.ref;
  }
public:
  ~message()
  {
    switch ( type )
    {
      case device_list_req_id:
        device_list_req.~audio_output_device_list_request();
        break;
      case device_list_res_id:
        device_list_res.~audio_output_device_list_response();
        break;
      case settings_changed_id:
        settings_changed.~settings_changed_message();
        break;
      case open_req_id:
        open_req.~audio_output_open_request();
        break;
      case open_res_id:
        open_res.~audio_output_open_response();
        break;
      case close_req_id:
        close_req.~audio_output_close_request();
        break;
      case close_res_id:
        close_res.~audio_output_close_response();
        break;
      case play_req_id:
        play_req.~play_request();
        break;
      case stop_req_id:
        stop_req.~stop_request();
        break;
      case skip_req_id:
        skip_req.~skip_request();
        break;
      case queue_req_id:
        queue_req.~queue_request();
        break;
      case stream_begin_id:
        stream_begin.~audio_output_stream_begin();
        break;
      case stream_end_id:
        stream_end.~audio_output_stream_end();
        break;
      case stream_buffer_id:
        stream_buffer.~audio_output_stream_buffer();
        break;
      case stream_begin_notify_id:
        stream_begin_notify.~audio_output_stream_begin_notification();
        break;
      case stream_end_notify_id:
        stream_end_notify.~audio_output_stream_end_notification();
        break;
      case stream_progress_notify_id:
        stream_progress_notify.~audio_output_stream_progress_notification();
        break;
      case subscribe_id:
        subscribe.~audio_output_subscribe_message();
        break;
      case unsubscribe_id:
        unsubscribe.~audio_output_unsubscribe_message();
        break;
      case queue_update_id:
        queue_update.~queue_update_notification();
        break;
      case player_state_id:
        player_state.~player_state_notification();
        break;
      case source_notify_id:
        source_notify.~source_notification();
        break;
      case stream_data_req_id:
        stream_data_req.~stream_data_request();
        break;
      case stream_data_res_id:
        stream_data_res.~stream_data_response();
        break;
    }
  }
public:
  unsigned type;
  union
  {
    audio_output_device_list_request device_list_req;
    audio_output_device_list_response device_list_res;
    settings_changed_message settings_changed;
    audio_output_open_request open_req;
    audio_output_open_response open_res;
    audio_output_close_request close_req;
    audio_output_close_response close_res;
    play_request play_req;
    stop_request stop_req;
    skip_request skip_req;
    queue_request queue_req;
    audio_output_stream_begin stream_begin;
    audio_output_stream_end stream_end;
    audio_output_stream_buffer stream_buffer;
    audio_output_stream_begin_notification stream_begin_notify;
    audio_output_stream_end_notification stream_end_notify;
    audio_output_stream_progress_notification stream_progress_notify;
    audio_output_subscribe_message subscribe;
    audio_output_unsubscribe_message unsubscribe;
    queue_update_notification queue_update;
    player_state_notification player_state;
    source_notification source_notify;
    stream_data_request stream_data_req;
    stream_data_response stream_data_res;
  };
  //long long ref;
  unsigned ref;
};

// ----------------------------------------------------------------------------
#endif
