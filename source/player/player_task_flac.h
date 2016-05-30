// ----------------------------------------------------------------------------
//
//     Author     : Benny Bach <benny.bach@gmail.com>
//                  Copyright (C) 2015
//
// ----------------------------------------------------------------------------
#ifndef __player__player_task_flac_h__
#define __player__player_task_flac_h__

// ----------------------------------------------------------------------------
#include "message.h"
#include "audio_buffer.h"
#include "audio_output.h"

// ----------------------------------------------------------------------------
#include <dripcore/task.h>

// ----------------------------------------------------------------------------
#include <FLAC++/decoder.h>

// ----------------------------------------------------------------------------
namespace musicbox
{
  class flac_decoder : public FLAC::Decoder::File
  {
    using milliseconds = audio_output_alsa::milliseconds;
  public:
    flac_decoder(const std::string& filename, unsigned stream_id, std::shared_ptr<audio_output_alsa> audio_output)
      : FLAC::Decoder::File(), stream_id_(stream_id), audio_output_(audio_output)
    {
      auto res = FLAC::Decoder::File::init(filename.c_str());

      if ( FLAC__STREAM_DECODER_INIT_STATUS_OK != res ) {
        std::cout << "flac decoder error" << std::endl;
      }
    }
  public:
    void stream_buffer_channel_init(size_t num_buffers)
    {
      for ( size_t i=0; i<num_buffers; ++i ) {
        buffer_ch_.send(audio_buffer{}, task_);
      }
    }
  public:
    void play(dripcore::task* task)
    {
      task_ = task;

      stream_buffer_channel_init(5);
      process_until_end_of_stream();
      stream_end();
    }
  private:
    void stream_end()
    {
      message_channel done;
      message m(message::stream_end_id);

      m.stream_end.reply = done;
      audio_output_->send(std::move(m));

      done.recv(task_);
    }
  protected:
    FLAC__StreamDecoderReadStatus read_callback(FLAC__byte buffer[], size_t *bytes)
    {
      return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
    }
  protected:
    FLAC__StreamDecoderWriteStatus write_callback(const FLAC__Frame* frame, const FLAC__int32* const buffer[])
    {
      auto buf = buffer_ch_.recv(task_);

      buf.clear();
      buf.writen(buffer, frame->header.blocksize, bits_per_sample_);

      message m(message::stream_buffer_id);

      m.stream_buffer.buffer = std::move(buf);
      audio_output_->send(std::move(m));

      if ( task_->stopping() || task_->done() ) {
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
      }
      else {
        return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
      }
    }
  protected:
    void metadata_callback(const ::FLAC__StreamMetadata *metadata)
    {
      bits_per_sample_ = metadata->data.stream_info.bits_per_sample;

      auto sample_rate = metadata->data.stream_info.sample_rate;
      auto total_samples = metadata->data.stream_info.total_samples;

      std::chrono::duration<float> length{float(total_samples)/float(sample_rate)};

      message m(message::stream_begin_id);

      m.stream_begin.stream_id = stream_id_;
      m.stream_begin.sample_rate = sample_rate;
      m.stream_begin.length = std::chrono::duration_cast<milliseconds>(length);
      m.stream_begin.completed_buffer_ch = buffer_ch_;
      audio_output_->send(std::move(m));
    }
  protected:
    void error_callback (::FLAC__StreamDecoderErrorStatus status)
    {
      std::cout << "decoder error" << std::endl;
    }
  private:
    dripcore::task* task_;
    std::shared_ptr<audio_output_alsa> audio_output_;
  private:
    unsigned stream_id_;
    unsigned bits_per_sample_;
    dripcore::channel<audio_buffer> buffer_ch_;
  };

  class player_task_flac : public dripcore::task
  {
  public:
    player_task_flac(std::shared_ptr<player_session> session)
      :
      decoder_(session->track()->sources()[0].uri(), session->id(), session->audio_output())
    {
    }
  private:
    void main()
    {
      decoder_.play(this);
    }
  private:
    flac_decoder decoder_;
  };
}

// ----------------------------------------------------------------------------
#endif
