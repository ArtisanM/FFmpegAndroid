#ifndef FFMPEG_AUDIO_DECODER_H
#define FFMPEG_AUDIO_DECODER_H

#include "AudioDecoder.h"

extern "C" {
#include "libavcodec/avcodec.h"
}

class FFmpegAudioDecoder : public AudioDecoder {
public:
    FFmpegAudioDecoder();

    ~FFmpegAudioDecoder() override;

    int init(AudioCodecConfig &config) override;

    int decode(const AVPacket *pkt) override;

    int flush() override;

    int release() override;

    void setDecodeCallback(AudioDecodeCallback *callback) override;

private:
    AVCodecContext *mCodecContext;

};

#endif
