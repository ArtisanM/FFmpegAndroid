#ifndef FFMPEG_VIDEO_DECODER_H
#define FFMPEG_VIDEO_DECODER_H

#include "decode/common/VideoCodecInfo.h"
#include "decode/VideoDecoder.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "libavcodec/avcodec.h"
#ifdef __cplusplus
}
#endif

class FFmpegVideoDecoder : public VideoDecoder {
public:
    explicit FFmpegVideoDecoder(int codecId);

    ~FFmpegVideoDecoder() override;

    int init(const MetaData *metadata) override;

    int decode(const AVPacket *pkt) override;

    int flush() override;

    int setVideoFormat(const MetaData *metadata) override;

    int release() override;

private:
    bool bFlushState = false;
    AVCodecContext *mCodecContext = nullptr;

};

#endif
