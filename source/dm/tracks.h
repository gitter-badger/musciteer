// ----------------------------------------------------------------------------
//
//     Author     : Benny Bach <benny.bach@gmail.com>
//                  Copyright (C) 2015
//
// ----------------------------------------------------------------------------
#ifndef __musicbox__dm__tracks_h__
#define __musicbox__dm__tracks_h__

// ----------------------------------------------------------------------------
#include "track.h"

// ----------------------------------------------------------------------------
#include "../storage/kvstore.h"
#include "../storage/base62.h"

// ----------------------------------------------------------------------------
namespace musicbox
{
  class tracks
  {
  public:
    tracks() : kvstore_(musicbox::kvstore())
    {
    }
  public:
    track find_by_id(const std::string& id)
    {
      return kvstore_.get<musicbox::track>(id);
    }
  public:
    void save(const musicbox::track& track)
    {
      kvstore_.set(track.id(), track);
    }
  public:
    void each(std::function<bool(musicbox::track& track)> value_cb)
    {
      kvstore_.each(
        [](const std::string& key) -> bool
        {
          if ( key.length() == 6 && key[0] == 't' ) {
            return true;
          }
          else {
            return false;
          }
        },
        [&](msgpack::istream& is) -> bool
        {
          musicbox::track track;

          if ( is >> track )
          {
            value_cb(track);
          }
          else
          {
            // ERROR!
          }
          return true;
        }
      );
    }
  public:
    track create_track()
    {
      musicbox::track track;

      auto id = kvstore_.increment("__track_next__", 1, key_min);

      if ( id > key_max )
      {
        throw std::runtime_error("track key space exhausted!");
      }

      track.id(base62_encode(id));
      return track;
    }
  private:
    kvstore kvstore_;
  private:
    static constexpr const uint64_t key_min = 50387305760;
    static constexpr const uint64_t key_max = 51303438591;
  };
}

// ----------------------------------------------------------------------------
#endif