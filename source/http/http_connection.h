// ----------------------------------------------------------------------------
//
//     Author     : Benny Bach <benny.bach@gmail.com>
//                  Copyright (C) 2015
//
// --- Description: -----------------------------------------------------------
//
//
// ----------------------------------------------------------------------------
#ifndef __musciteer__http_connection_h__
#define __musciteer__http_connection_h__

// ----------------------------------------------------------------------------
#include <dripcore/task.h>
#include <dripcore/socket.h>

// ----------------------------------------------------------------------------
#include <http/request.h>
#include <http/response.h>
#include <http/handlers/websocket_handler_base.h>

// ----------------------------------------------------------------------------
#include <json.h>

// ----------------------------------------------------------------------------
class websocket_handler : public http::websocket_handler_base
{
  using websocket_handler_base::websocket_handler_base;

  void on_connect() override
  {
    std::cout << "websocket connected" << std::endl;
    send_message("{\"event\":\"hello\"}");
  }

  void on_close() override
  {
    std::cout << "websocket closed" << std::endl;
  }

  void on_message(const std::string& message) override
  {
    auto j = json::parse(message);

    if ( j.count("event") )
    {
      if ( j["event"] == "audio_device_list_sync" )
      {
        std::cout << "audio_device_list_sync" << std::endl;

        json audio_device_list = {
          { "event", "audio_device_list"},
          { "data", { "default", "pulse" } }
        };

        send_message(audio_device_list.dump());
      }
    }

    std::cout << "websocket message=\"" << j << "\"" << std::endl;
  }
};

// ----------------------------------------------------------------------------
class http_connection : public dripcore::task
{
public:
  http_connection(dripcore::socket socket);
  ~http_connection();
public:
  void main() override;
protected:
  void loop(std::streambuf* sbuf);
protected:
  void dispatch(http::request& request, http::response& response);
protected:
  void not_found(http::response& response);
private:
  dripcore::socket socket_;
private:
  static constexpr const char* crlf = "\r\n";
};

// ----------------------------------------------------------------------------
#endif