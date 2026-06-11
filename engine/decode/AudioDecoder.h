/**
 * Note: interface of Audio decoder
 * Date: 2025/12/7
 * Author: frank
 */

#ifndef AUDIO_DECODER_H
#define AUDIO_DECODER_H

#include <memory>

#ifdef __cplusplus
extern "C" {
#endif
#include "libavcodec/packet.h"
#include "libavutil/frame.h"
#ifdef __cplusplus
}
#endif

struct AudioCodecConfig {
    int profile        = -1;
    int codec_id       = -1;
    int channels       = 0;
    int sample_rate    = 0;
    int extradata_size = 0;
    uint8_t *extradata = nullptr;
    char *codec_name   = nullptr;
};

class AudioDecodeCallback {
public:
    virtual int onDecodedFrame(AVFrame *frame) = 0;

    virtual void onDecodeError(int error) = 0;

    virtual ~AudioDecodeCallback() = default;
};

class AudioDecoder {
public:

    virtual ~AudioDecoder() = default;

    virtual int init(AudioCodecConfig &config) = 0;

    virtual int decode(const AVPacket *pkt) = 0;

    virtual int flush() = 0;

    virtual int release() = 0;

    virtual void setDecodeCallback(AudioDecodeCallback *callback) = 0;

protected:
    AudioDecodeCallback *mAudioDecodedCallback = nullptr;
};

#endif
