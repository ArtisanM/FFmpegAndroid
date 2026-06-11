#ifndef VIDEO_DECODER_H
#define VIDEO_DECODER_H

#include <map>
#include <memory>
#include <mutex>

#include "decode/common/MixedBuffer.h"
#include "decode/common/VideoCodecInfo.h"
#include "NextErrorCode.h"
#include "NextStructDefine.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "libavcodec/packet.h"
#ifdef __cplusplus
}
#endif

struct CodecContext {
    int width         = 0;
    int height        = 0;
    int sar_num       = 0;
    int sar_den       = 1;
    bool is_hdr       = false;
    int nal_size      = 4;
    bool is_annexb    = false;
    int rotate_degree = 0;
    AVColorRange color_range = AVCOL_RANGE_MPEG;
};

class VideoDecodeCallback {
public:
    virtual int onDecodedFrame(std::unique_ptr<MixedBuffer> frame) = 0;

    virtual void onDecodeError(int error, int errorCode) = 0;

    virtual ~VideoDecodeCallback() = default;
};

class VideoDecoder {
public:
    explicit VideoDecoder(int codecId);

    virtual ~VideoDecoder();

    virtual int init(const MetaData *metadata) = 0;

    virtual int decode(const AVPacket *pkt) = 0;

    void setDecodeCallback(VideoDecodeCallback *callback);

    virtual int setVideoFormat(const MetaData *metadata) = 0;

    virtual int flush() = 0;

    virtual int setHardwareContext(HardWareContext *context) {
        return ERROR_PLAYER_UNSUPPORTED;
    }

    virtual int updateHardwareContext(HardWareContext *context) {
        return ERROR_PLAYER_UNSUPPORTED;
    }

    virtual int release() = 0;

protected:
    int mCodecId = -1;
    VideoDecodeCallback *mVideoDecodeCallback = nullptr;
};

#endif
