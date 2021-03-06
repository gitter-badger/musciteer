// ----------------------------------------------------------------------------
//
//     Author     : Benny Bach <benny.bach@gmail.com>
//                  Copyright (C) 2015
//
// ----------------------------------------------------------------------------
#ifndef __musciteer__dm__artists_h__
#define __musciteer__dm__artists_h__

// ----------------------------------------------------------------------------
#include "artist.h"

// ----------------------------------------------------------------------------
#include "../storage/kvstore.h"
#include "../storage/base62.h"

// ----------------------------------------------------------------------------
namespace musciteer
{
  namespace dm
  {
    class artists
    {
    public:
      artists() : kvstore_(musciteer::kvstore())
      {
      }
    public:
      artist find_by_id(const std::string& id)
      {
        auto artist = dm::artist();

        kvstore_.get(id, artist);

        return artist;
      }
    public:
      void save(const dm::artist& artist)
      {
        kvstore_.set(artist.id(), artist);
      }
    public:
      void each(std::function<bool(dm::artist& artist)> value_cb)
      {
        kvstore_.each(
          [](const std::string& key) -> bool
          {
            if ( key.length() == 6 && key[0] == 'a' && key[1] == 'r' ) {
              return true;
            }
            else {
              return false;
            }
          },
          [&](msgpack::istream& is) -> bool
          {
            dm::artist artist;

            if ( is >> artist )
            {
              value_cb(artist);
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
      artist create_artist(const std::string& name)
      {
        dm::artist artist;

        auto id = kvstore_.increment("__artist_next__", 1, key_min);

        if ( id > key_max )
        {
          throw std::runtime_error("artist key space exhausted!");
        }

        artist.id(base62_encode(id));
        artist.name(name);

        return artist;
      }
    private:
      kvstore kvstore_;
    private:
      static constexpr const int64_t key_min = 33763927760;
      static constexpr const int64_t key_max = 33778704095;
    };
  }
}

// ----------------------------------------------------------------------------
#endif
