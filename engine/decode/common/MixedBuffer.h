#ifndef MIXED_BUFFER_H
#define MIXED_BUFFER_H

#include <cstdint>
#include <vector>

#include "VideoCodecInfo.h"
#include "NextDefine.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "libavutil/pixfmt.h"
#include "libavutil/channel_layout.h"
#ifdef __cplusplus
}
#endif

struct MediaCodecBufferContext {
    void *opaque;
    void *decoder;
    int buffer_index;
    void *media_codec;
    int decoder_serial;

    void (*release_output_buffer)(MediaCodecBufferContext *context, bool render);
};

struct FFmpegBufferContext {
    void *opaque;
    void *av_frame;

    void (*release_frame)(FFmpegBufferContext *context);
};

struct VideoToolBufferContext {
    void *buffer = nullptr;
};

enum class BufferType {
    BUFFER_VIDEO_FRAME  = 1,
    BUFFER_VIDEO_PACKET = 2,
    BUFFER_VIDEO_FORMAT = 3
};

struct VideoFrameMetadata {
    int width;
    int height;

    int64_t pts; // ms
    int64_t dts; // ms

    int stride_y;
    int stride_u;
    int stride_v;
    uint8_t *buffer_y = nullptr;
    uint8_t *buffer_u = nullptr;
    uint8_t *buffer_v = nullptr;

    VideoPixelFormat pixel_format;
    void *buffer_context = nullptr;  // release buffer

};

struct VideoPacketMetadata {
    int offset = 0;
    int64_t pts; // ms
    int64_t dts; // ms
    uint32_t decode_flags;
};

struct VideoFormatMetadata {

    ~VideoFormatMetadata() {
        if (hardware_context) {
            delete hardware_context;
            hardware_context = nullptr;
        }
    }

    int width;
    int height;
    bool is_hdr = false;

    HardWareContext *hardware_context = nullptr;
    std::vector<int> items;

    AVColorSpace colorspace;
    AVColorRange color_range;
    AVColorPrimaries color_primaries;
    AVColorTransferCharacteristic color_trc;

    int sar_num = 0;
    int sar_den = 1;
};

class MixedBuffer {
public:
    MixedBuffer(BufferType type, uint8_t *data, int size, bool own_data);

    MixedBuffer(BufferType type, int capacity);

    ~MixedBuffer();

    int getSize() const;

    uint8_t *getData() const;

    BufferType getType() const;

    uint8_t *obtainData();

    VideoFrameMetadata *getVideoFrameMetadata() const;

    VideoPacketMetadata *getVideoPacketMetadata() const;

    VideoFormatMetadata *getVideoFormatMetadata() const;

private:
    void initType(BufferType type);

    int        mSize;
    uint8_t   *mData;
    bool       bOwnData;
    BufferType mBufferType;

    std::unique_ptr<VideoFrameMetadata>  mVideoFrameMetadata;
    std::unique_ptr<VideoPacketMetadata> mVideoPacketMetadata;
    std::unique_ptr<VideoFormatMetadata> mVideoFormatMetadata;
};

#endif
