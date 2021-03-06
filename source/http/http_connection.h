// ----------------------------------------------------------------------------
//
//     Author     : Benny Bach <benny.bach@gmail.com>
//                  Copyright (C) 2015
//
// ----------------------------------------------------------------------------
#ifndef __musciteer__http_connection_h__
#define __musciteer__http_connection_h__

// ----------------------------------------------------------------------------
#include "../player/message.h"

// ----------------------------------------------------------------------------
#include <dripcore/task.h>
#include <dripcore/socket.h>
#include <dripcore/streambuf.h>

// ----------------------------------------------------------------------------
#include <http/request.h>
#include <http/response.h>
#include <http/websocket.h>

// ----------------------------------------------------------------------------
class websocket_send_task : public dripcore::task
{
public:
  websocket_send_task(dripcore::task& connection, dripcore::socket& socket, message_channel channel);
public:
  ~websocket_send_task();
private:
  void init() override;
  void main() override;
  void shutdown() override;
private:
  void handle_message(const message& msg, std::ostream& os);
  void handle(const audio_output_device_list_response&, std::ostream&);
  void handle(const audio_output_stream_begin_notification&, std::ostream&);
  void handle(const audio_output_stream_end_notification&, std::ostream&);
  void handle(const audio_output_stream_progress_notification&, std::ostream&);
  void handle(const queue_update_notification&, std::ostream&);
  void handle(const player_state_notification&, std::ostream&);
  void handle(const source_notification&, std::ostream&);
  void handle(const stream_data_response&, std::ostream&);
  void send(std::ostream& os, const std::string& message);
private:
  dripcore::task& connection_;
  dripcore::socket& socket_;
  message_channel ch_;
};

// ----------------------------------------------------------------------------
class websocket_recv_task : public dripcore::task
{
public:
  websocket_recv_task(dripcore::task& connection, dripcore::socket& socket, message_channel channel);
  ~websocket_recv_task();
private:
  void init() override;
  void main() override;
private:
  void dispatch(http::websocket::header& header, std::istream& is);
  void on_message(const std::string& message);
private:
  dripcore::task& connection_;
  dripcore::socket& socket_;
  message_channel ch_;
};

// ----------------------------------------------------------------------------
class http_connection : public dripcore::task
{
public:
  http_connection(dripcore::socket socket);
  ~http_connection();
public:
  void init() override;
  void main() override;
protected:
  void loop(std::streambuf* sbuf);
protected:
  void dispatch(http::request& request, http::response& response);
protected:
  void not_found(http::response& response);
private:
  dripcore::socket socket_;
  std::unique_ptr<dripcore::streambuf> streambuf_;
private:
  static constexpr const char* crlf = "\r\n";
  static constexpr const char* guid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
};

// ----------------------------------------------------------------------------
#endif