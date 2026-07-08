#ifndef MEDIACODEC_VIDEO_DECODER_H
#define MEDIACODEC_VIDEO_DECODER_H

#include "media/NdkMediaCodec.h"
#include "media/NdkMediaFormat.h"
#include "decode/common/VideoCodecInfo.h"
#include "decode/VideoDecoder.h"

struct AMediaFormatReleaser {
    void operator()(AMediaFormat *ptr) const { AMediaFormat_delete(ptr); }
};

struct AMediaCodecReleaser {
    void operator()(AMediaCodec *ptr) const { AMediaCodec_delete(ptr); }
};

class MediaCodecVideoDecoder : public VideoDecoder {
public:
    explicit MediaCodecVideoDecoder(int codecId);

    ~MediaCodecVideoDecoder() override;

    int init(const MetaData *metadata) override;

    int decode(const AVPacket *pkt) override;

    int flush() override;

    int setVideoFormat(const MetaData *metadata) override;

    int setHardwareContext(HardWareContext *context) override;

    int updateHardwareContext(HardWareContext *context) override;

    int release() override;

    int getSerial();

private:
    int initMediaCodec();

    int releaseMediaCodec();

    int initMediaFormat(const MetaData *metadata);

    int releaseMediaFormatDesc();

    int feedDecoder(const AVPacket *pkt);

    int drainDecoder();

    int mBufferIndex   = -1;
    bool bEofState     = false;
    bool bDrainState   = false;
    bool bFirstFrame   = false;
    bool bDecoderStart = false;

    std::atomic_int mSerial{};
    CodecContext mCodecContext{};
    ANativeWindow *mNativeWindow{};
    std::unique_ptr<AMediaCodec, AMediaCodecReleaser> mMediaCodec{};
    std::unique_ptr<AMediaFormat, AMediaFormatReleaser> mMediaFormat{};
};

#endif //MEDIACODEC_VIDEO_DECODER_H
