// ----------------------------------------------------------------------------
//
//     Author     : Benny Bach <benny.bach@gmail.com>
//                  Copyright (C) 2015
//
// ----------------------------------------------------------------------------
#include "http_connection.h"
#include "tracks_handler.h"
#include "albums_handler.h"
#include "player_handler.h"
#include "sources_handler.h"
#include "static_file_handler.h"
#include "api.h"

// ----------------------------------------------------------------------------
#include "../player/player.h"

// ----------------------------------------------------------------------------
#include <http/base64.h>

// ----------------------------------------------------------------------------
#include <crypto++/sha.h>

// ----------------------------------------------------------------------------
#include <regex>

/////
// Websocket send task.

websocket_send_task::websocket_send_task(dripcore::task& connection, dripcore::socket& socket, message_channel channel)
  : connection_(connection), socket_(socket), ch_(channel)
{
}

websocket_send_task::~websocket_send_task()
{
  auto player = musciteer::player();
  player.unsubscribe(ch_);
}

void websocket_send_task::init()
{
  auto player = musciteer::player();
  player.subscribe(ch_);
}

void websocket_send_task::main()
{
  dripcore::streambuf sbuf(socket_, *this);
  std::ostream os(&sbuf);

  //std::cout << "websocket_send_task - start" << std::endl;
  while ( os )
  {
    auto msg = ch_.recv(this);

    if ( stopping() ) {
      break;
    }

    handle_message(msg, os);
  }
  connection_.resume();
  std::cout << "websocket_send_task - stopping state=" << unsigned(state()) << ", os.state=" << os.rdstate() << std::endl;
}

void websocket_send_task::shutdown()
{
  ch_.send(message{}, this);
}

void websocket_send_task::handle_message(const message& msg, std::ostream& os)
{
  switch ( msg.type )
  {
    case message::device_list_res_id:
      handle(msg.device_list_res, os);
      break;
    case message::stream_begin_notify_id:
      handle(msg.stream_begin_notify, os);
      break;
    case message::stream_end_notify_id:
      handle(msg.stream_begin_notify, os);
      break;
    case message::stream_progress_notify_id:
      handle(msg.stream_progress_notify, os);
      break;
    case message::queue_update_id:
      handle(msg.queue_update, os);
      break;
    case message::player_state_id:
      handle(msg.player_state, os);
      break;
    case message::source_notify_id:
      handle(msg.source_notify, os);
      break;
    case message::stream_data_res_id:
      handle(msg.stream_data_res, os);
      break;
  }
}

void websocket_send_task::handle(const audio_output_device_list_response& m, std::ostream& os)
{
  json event = {
    { "event", "audio_device_list"},
    { "data",  { m.current, m.device_names } }
  };

  send(os, event.dump());
}

void websocket_send_task::handle(const audio_output_stream_begin_notification& m, std::ostream& os)
{
  json event = {
    { "event", "stream_begin"},
    { "data",  {
      { "stream_id", m.stream_id } }
    }
  };

  send(os, event.dump());
}

void websocket_send_task::handle(const audio_output_stream_end_notification& m, std::ostream& os)
{
  json event = {
    { "event", "stream_end"},
    { "data",  {
      { "stream_id", m.stream_id } }
    }
  };

  send(os, event.dump());
}

void websocket_send_task::handle(const audio_output_stream_progress_notification& m, std::ostream& os)
{
  json event = {
    { "event", "stream_progress"},
    { "data", {
      { "stream_id", m.stream_id },
      { "duration", m.duration },
      { "length", m.length } }
    }
  };

  send(os, event.dump());
}

void websocket_send_task::handle(const queue_update_notification& m, std::ostream& os)
{
  json event = {
    { "event", "queue_update"},
    { "data",  {
      { "queue_size", m.queue_size },
      { "track", musciteer::to_json(*m.track) } }
    }
  };

  send(os, event.dump());
}

void websocket_send_task::handle(const player_state_notification& m, std::ostream& os)
{
  json event = {
    { "event", "player_state"},
    { "data",  {
      { "state", m.state } }
    }
  };

  send(os, event.dump());
}

void websocket_send_task::handle(const source_notification& m, std::ostream& os)
{
  json event = {
    { "event", "source_notification"},
    { "data",  {
      { "type", unsigned(m.type) },
      { "source_name", m.source_name },
      { "message", m.message } }
    }
  };

  send(os, event.dump());
}

void websocket_send_task::handle(const stream_data_response& m, std::ostream& os)
{
  json data;

  if ( m.stream_id != -1 )
  {
    data = {
      { "stream_id", m.stream_id },
      { "track", musciteer::to_json(*m.track) }
    };
  }

  json event = {
    { "event", "stream_data" }, { "data", data }
  };

  send(os, event.dump());
}

void websocket_send_task::send(std::ostream& os, const std::string& message)
{
  http::websocket::header header;

  header.fin(true);
  header.rsv(0);
  header.opcode(1);
  header.mask(false);
  header.payload_length(message.length());

  os << header << message << std::flush;
}

/////
// Websocket recv task.

websocket_recv_task::websocket_recv_task(dripcore::task& connection, dripcore::socket& socket, message_channel channel)
  : connection_(connection), socket_(socket), ch_(channel)
{
}

websocket_recv_task::~websocket_recv_task()
{
}

void websocket_recv_task::init()
{
}

void websocket_recv_task::main()
{
  //std::cout << "websocket_recv_task - start" << std::endl;

  dripcore::streambuf sbuf(socket_, *this);
  std::istream is(&sbuf);

  while ( is.good() )
  {
    http::websocket::header header;

    if ( is >> header )
    {
      if ( header.rsv() != 0 )
      {
        // ERROR!
      }

#if 0
      std::cout << "websocket:" << std::endl
        << "  fin : " << header.fin() << std::endl
        << "  rsv : " << header.rsv() << std::endl
        << "  opc : " << header.opcode() << std::endl
        << "  msk : " << header.mask() << std::endl
        << "  len : " << header.payload_length() << std::endl;
#endif

      dispatch(header, is);
    }
    else
    {
      std::cout << "websocket streaming error!" << std::endl;
    }
  }
  connection_.resume();
  std::cout << "websocket_recv_task - stopping state=" << unsigned(state()) << ", is.state=" << is.rdstate() << std::endl;
}

void websocket_recv_task::dispatch(http::websocket::header& header, std::istream& is)
{
  switch ( header.opcode() )
  {
    case 0:
      // Not implemented yet.
      break;
    case 1:
    {
      std::string message;

      // TODO: Check fin.
      // TODO: Check masking

      message.reserve(header.payload_length());
      for ( size_t i=0; i<header.payload_length(); ++i )
      {
        message.push_back(is.get() ^ header.masking_key(i));
      }
      on_message(message);
      break;
    }
    case 2:
      // Not implemented yet.
      break;
    case 8:
      is.setstate(std::ios_base::eofbit);
      return;
  }
}

void websocket_recv_task::on_message(const std::string& message)
{
  auto j = json::parse(message);

  if ( j.count("event") )
  {
    auto player = musciteer::player();
    auto event = j["event"];

    if ( event == "stream_data_sync" )
    {
      player.stream_data(j["data"].get<unsigned>(), ch_);
    }
    else if ( event == "play" )
    {
      auto data = j["data"];

      if ( data.is_null() ) {
        player.play();
      }
      else {
        player.play(data.get<std::string>());
      }
    }
    else if ( event == "stop" )
    {
      player.stop();
    }
    else if ( event == "skip" )
    {
      player.skip();
    }
    else if ( event == "queue" )
    {
      player.queue(j["data"].get<std::string>());
    }
    else
    {
      std::cerr << "unhandled websocket message=\"" << j << "\"" << std::endl;
    }
  }
  else
  {
    std::cerr << "unhandled websocket message=\"" << j << "\"" << std::endl;
  }
}

// ----------------------------------------------------------------------------
http_connection::http_connection(dripcore::socket socket)
  : socket_(std::move(socket))
{
  std::cout << "connection " << size_t(this) << std::endl;
}

// ----------------------------------------------------------------------------
http_connection::~http_connection()
{
  std::cout << "~connection " << size_t(this) << std::endl;
  detach_eventable(socket_);
}

void http_connection::init()
{
  attach_eventable(socket_);
}

// ----------------------------------------------------------------------------
void http_connection::main()
{
  streambuf_.reset(new dripcore::streambuf(socket_, *this));

  try
  {
    loop(streambuf_.get());
  }
  catch(const dripcore::end_of_stream& e)
  {
    std::cout << "client disconnected" << std::endl;
  }
  catch(const std::ios_base::failure& e)
  {
    //std::cout << "connection " << size_t(this) << " streaming error code=" << e.code() << ", message=" << e.what() << std::endl;
  }
  catch(const std::exception& e)
  {
    std::cout << "connection error " << e.what() << std::endl;
  }
}

// ----------------------------------------------------------------------------
void http_connection::loop(std::streambuf* sbuf)
{
  while ( true )
  {
    http::request request(sbuf);
    http::response response(sbuf);

    if ( request >> request )
    {
      std::string upgrade;

      if ( request.get_header("Upgrade", upgrade) )
      {
        if ( upgrade == "websocket" )
        {
          std::string sec_websocket_key;

          if ( request.get_header("Sec-WebSocket-Key", sec_websocket_key) )
          {
            std::string sec_websocket_accept = sec_websocket_key+guid;

            byte digest[CryptoPP::SHA1::DIGESTSIZE];

            CryptoPP::SHA1().CalculateDigest(
              digest,
              reinterpret_cast<const unsigned char*>(sec_websocket_accept.data()),
              sec_websocket_accept.length()
            );

            response << "HTTP/1.1 101 Switching Protocols" << crlf
              << "Upgrade: websocket" << crlf
              << "Connection: Upgrade" << crlf
              << "Sec-WebSocket-Accept: " << base64::encode(digest, sizeof(digest)) << crlf
              << crlf
              << std::flush;

            // Protect against dead connections.
            socket_.keepalive();

            message_channel channel;

            auto recv_task = spawn<websocket_recv_task>(*this, socket_, channel).lock();
            auto send_task = spawn<websocket_send_task>(*this, socket_, channel).lock();

            std::cout << "http_connection " << size_t(this) << " - switched to websocket protocol" << std::endl;

            while ( !recv_task->done() || !send_task->done() )
            {
              yield();

              if ( !recv_task->done() ) {
                recv_task->stop();
              }

              if ( !send_task->done() ) {
                send_task->stop();
              }

            }
          }
          else
          {
            // Error!
          }
          return;
        }
        else
        {
          // Error!
        }
      }
      else
      {
        dispatch(request, response);
      }
    }
    else
    {
      std::cout << "failed to read request!" << std::endl;
      break;
    }
  }
}

// ----------------------------------------------------------------------------
void http_connection::dispatch(http::request& request, http::response& response)
{
  auto uri = request.uri();

  std::regex track_re("^/api/tracks(/.*)?");
  std::regex albums_re("^/api/albums(/.*)?");
  std::regex player_re("^/api/player(/.*)?");
  std::regex sources_re("^/api/sources(/.*)?");
  std::regex assets_re("^/(assets/.+)");

  std::smatch match;

  //std::cout << "dispatch uri " << uri << std::endl;

  if ( uri == "/" || uri == "/albums" || uri == "/tracks" )
  {
    static_file_handler handler(request, response);
    handler.call("index.html");
  }
  else if ( uri == "/android")
  {
    static_file_handler handler(request, response);
    handler.call("android.html");
  }
  else if ( std::regex_match(uri, match, track_re) )
  {
    tracks_handler handler(request, response, this);

    if ( match.size() == 2 )
    {
      handler.call(match[1]);
    }
    else
    {
      // ERROR!
    }
  }
  else if ( std::regex_match(uri, match, albums_re) )
  {
    albums_handler handler(request, response);

    if ( match.size() == 2 )
    {
      handler.call(match[1]);
    }
    else
    {
      // ERROR!
    }
  }
  else if ( std::regex_match(uri, match, player_re) )
  {
    player_handler handler(request, response, this);

    if ( match.size() == 2 )
    {
      handler.call(match[1]);
    }
    else
    {
      // ERROR!
    }
  }
  else if ( std::regex_match(uri, match, sources_re) )
  {
    sources_handler handler(request, response, this);

    if ( match.size() == 2 )
    {
      handler.call(match[1]);
    }
    else
    {
      // ERROR!
    }
  }
  else if ( std::regex_match(uri, match, assets_re) )
  {
    if ( match.size() == 2 )
    {
      static_file_handler handler(request, response);
      handler.call(match[1]);
    }
    else
    {
      // ERROR!
    }
  }
  else
  {
    not_found(response);
  }
}

// ----------------------------------------------------------------------------
void http_connection::not_found(http::response& response)
{
  response << "HTTP/1.1 404 Not Found" << crlf
    << "Content-Length: 0" << crlf
    << crlf;
}
