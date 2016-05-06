// ----------------------------------------------------------------------------
//
//     Filename   : dripcore/socket_streambuf.h
//
//     Author     : Benny Bach <benny.bach@gmail.com>
//                  Copyright (C) 2015
//
// --- Description: -----------------------------------------------------------
//
//
// ----------------------------------------------------------------------------
#ifndef __dripcore__socket_streambuf_h__
#define __dripcore__socket_streambuf_h__

// ----------------------------------------------------------------------------
#include <dripcore/task.h>

// ----------------------------------------------------------------------------
#include <streambuf>

// ----------------------------------------------------------------------------
namespace dripcore
{
  class end_of_stream : public std::exception
  {
  };

  class socket_streambuf : public std::basic_streambuf<char>
  {
  public:
    socket_streambuf(socket& socket, task& task, bool throw_on_eof = false)
      :
      socket_(socket),
      task_(task),
      throw_on_eof_(throw_on_eof)
    {
      init_put_area();
      init_get_area();
      task_.attach_eventable(socket_);
    }
  public:
    virtual ~socket_streambuf()
    {
      task_.detach_eventable(socket_);
    }
  private:
    void init_get_area()
    {
      setg(ibuf_.data(), ibuf_.data(), ibuf_.data());
    }
  private:
    void init_put_area()
    {
      setp(obuf_.data(), obuf_.data()+obuf_.size());
    }
  protected:
    virtual int_type underflow() override
    {
      if ( read() > 0 ) {
        return traits_type::not_eof(sgetc());
      }
      else {
        return traits_type::eof();
      }
    }
  protected:
  virtual int_type overflow(int_type ch = traits_type::eof()) override
  {
    sync();

    if ( ch != traits_type::eof() ) {
      sputc(ch);
    }

    return traits_type::not_eof(ch);
  }
  protected:
    virtual int sync() override
    {
      send();
      init_put_area();

      return 0;
    }
  private:
    ssize_t read()
    {
      auto res = ssize_t{0};

      do
      {
        res = socket_.recv(ibuf_.data(), ibuf_.size(), 0);

        if ( res < 0 )
        {
          if ( socket_.not_ready() ) {
            task_.wait_readable(socket_);
          }
          else {
            throw socket_error(errno);
          }
        }
        else if ( res > 0 )
        {
          setg(ibuf_.data(), ibuf_.data(), ibuf_.data()+res);
        }
        else
        {
          if ( throw_on_eof_ )
          {
            throw end_of_stream();
          }
          break;
        }
      }
      while ( res < 0 );

      return res;
    }
  private:
    ssize_t send()
    {
      auto ptr = pbase();
      auto len = pptr()-ptr;

      do
      {
        auto res = socket_.send(ptr, len, 0);

        if ( res < 0 )
        {
          if ( socket_.not_ready() ) {
            task_.wait_writable(socket_);
          }
          else {
            throw socket_error(errno);
          }
        }
        else
        {
          len -= res;
          ptr += res;
        }
      }
      while ( len > 0 );

      return len;
    }
  private:
    std::array<char, 4096> obuf_;
    std::array<char, 4096> ibuf_;
  private:
    socket& socket_;
    task& task_;
    bool throw_on_eof_;
  };
}

// ----------------------------------------------------------------------------
#endif
