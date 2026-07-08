#include "MediacodecDecoder.h"

#include <android/log.h>
#include <android/native_window.h>

#include <memory>

#include "media/NdkMediaExtractor.h"
#include "NextLog.h"

extern "C" {
#include "decode/common/nal_convert.h"
#include <libavcodec/codec_id.h>
#include "libavutil/avutil.h"
}

#define MEDIACODEC_TAG "MediaCodec"

#define INPUT_TIMEOUT_30MS  (30 * 1000)
#define OUTPUT_TIMEOUT_10MS (10 * 1000)

static void mediacodec_buffer_release(MediaCodecBufferContext *context, bool render) {
    if (context && context->media_codec && context->decoder) {
        if (context->decoder_serial ==
                (reinterpret_cast<MediaCodecVideoDecoder *>(context->decoder))
                        ->getSerial()) {
            AMediaCodec_releaseOutputBuffer(
                    reinterpret_cast<AMediaCodec *>(context->media_codec),
                    context->buffer_index, render);
        } else {
            NEXT_LOGW(MEDIACODEC_TAG,
                    "release buffer serial is not equal, index:%d "
                    "serial:%d\n",
                    context->buffer_index, context->decoder_serial);
        }
    }
}

MediaCodecVideoDecoder::MediaCodecVideoDecoder(int codecId)
        : VideoDecoder(codecId) {
    std::atomic_init(&mSerial, 1);
}

MediaCodecVideoDecoder::~MediaCodecVideoDecoder() { release(); }

int MediaCodecVideoDecoder::init(const MetaData *metadata) {

    release();

//    if (metadata) {
//        SetHardwareContext(
//                metadata->getVideoFormatMetadata()->hardware_context);
//        if (metadata->getSize() > 0) {
//            initMediaFormat(metadata);
//            initMediaCodec();
//        }
//    }

    return RESULT_OK;
}

int MediaCodecVideoDecoder::release() {
    releaseMediaCodec();
    releaseMediaFormatDesc();
    NEXT_LOGD(MEDIACODEC_TAG, "%p release \n", this);
    return RESULT_OK;
}

int MediaCodecVideoDecoder::feedDecoder(const AVPacket *pkt) {
    ssize_t buf_idx = mBufferIndex;
    if (buf_idx < 0) {
        buf_idx = AMediaCodec_dequeueInputBuffer(mMediaCodec.get(),
                                                 INPUT_TIMEOUT_30MS);
        if (buf_idx < 0) {
            if (buf_idx == AMEDIACODEC_INFO_TRY_AGAIN_LATER) {
                return ERROR_PLAYER_TRY_AGAIN;
            }
            NEXT_LOGE(MEDIACODEC_TAG, "dequeue input buffer error: %zd\n", buf_idx);
            return ERROR_DECODE_VIDEO_DEC;
        }
    }
    mBufferIndex = -1;

    size_t buf_size;
    uint8_t *buf = AMediaCodec_getInputBuffer(mMediaCodec.get(), buf_idx, &buf_size);
    if (buf_size <= 0 || buf == nullptr) {
        NEXT_LOGE(MEDIACODEC_TAG, "get input buffer error: %d\n", static_cast<int>(buf_size));
        return ERROR_DECODE_VIDEO_DEC;
    }

    if (pkt->size == -1) {
        bDrainState = true;
    }

    memcpy(buf, pkt->data, pkt->size);

    int64_t pts = pkt->pts;
    if (pts == AV_NOPTS_VALUE && pkt->dts != AV_NOPTS_VALUE) {
        pts = pkt->dts;
    }
    if (pts >= 0) {
        pts = pts * 1000;
    } else {
        pts = 0;
    }

    auto status = AMediaCodec_queueInputBuffer(
            mMediaCodec.get(), buf_idx, 0, pkt->size, pts,
            bDrainState ? AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM : 0);
    if (status != AMEDIA_OK) {
        NEXT_LOGE(MEDIACODEC_TAG, "queue input buffer error: %d\n",
                static_cast<int>(status));
        return ERROR_DECODE_VIDEO_DEC;
    }

    return RESULT_OK;
}

int MediaCodecVideoDecoder::drainDecoder() {
    if (!bEofState) {
        AMediaCodecBufferInfo info;
        int64_t drain_timeout_us = OUTPUT_TIMEOUT_10MS;
        if (!bFirstFrame) {
            drain_timeout_us = 0;
        }
        auto index = AMediaCodec_dequeueOutputBuffer(mMediaCodec.get(), &info,
                                                     drain_timeout_us);
        if (index >= 0) {
            std::unique_ptr<MixedBuffer> buffer =
                    std::make_unique<MixedBuffer>(BufferType::BUFFER_VIDEO_FRAME, 0);

            VideoFrameMetadata *meta = buffer->getVideoFrameMetadata();
            if (mCodecContext.rotate_degree == 90 || mCodecContext.rotate_degree == 270) {
                meta->height = mCodecContext.width;
                meta->width = mCodecContext.height;
            } else {
                meta->height = mCodecContext.height;
                meta->width = mCodecContext.width;
            }
            meta->pixel_format = VideoPixelFormat::PIXEL_FORMAT_MEDIACODEC;
            meta->pts = info.presentationTimeUs / 1000;
            if (meta->pts < 0) {
                meta->pts = AV_NOPTS_VALUE;
            }
            meta->buffer_context =
                    reinterpret_cast<void *>(new MediaCodecBufferContext{
                            .decoder = this,
                            .buffer_index = static_cast<int>(index),
                            .media_codec = mMediaCodec.get(),
                            .decoder_serial = mSerial.load(),
                            .release_output_buffer = mediacodec_buffer_release,
                    });

            if (info.flags & AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM) {
                NEXT_LOGI(MEDIACODEC_TAG, "output end of stream\n");
                bEofState = true;
                return ERROR_PLAYER_EOF;
            }

            if (!bFirstFrame) {
                bFirstFrame = true;
            }

            mVideoDecodeCallback->onDecodedFrame(std::move(buffer));

        } else if (index == AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED) {
            NEXT_LOGI(MEDIACODEC_TAG, "output buffer changed\n");
        } else if (index == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED) {
            auto format = AMediaCodec_getOutputFormat(mMediaCodec.get());
            NEXT_LOGI(MEDIACODEC_TAG, "output format changed to %s\n",
                    AMediaFormat_toString(format));
            AMediaFormat_delete(format);
        } else {
            NEXT_LOGE(MEDIACODEC_TAG, "unexpected info code: %zd\n", index);
            if (bDrainState) {
                NEXT_LOGE(MEDIACODEC_TAG, "return eof due to drain process error");
                return ERROR_PLAYER_EOF;
            }
        }
    } else {
        return ERROR_PLAYER_EOF;
    }
    return RESULT_OK;
}

int MediaCodecVideoDecoder::decode(const AVPacket *pkt) {
    if (!pkt) {
        return ERROR_DECODE_INVALID;
    }
// TODO
//    if (buffer->getVideoPacketMetadata()->format == VideoPacketFormat::PKT_FORMAT_AVCC ||
//        (buffer->getVideoPacketMetadata()->format ==
//         VideoPacketFormat::PKT_FORMAT_EXTRADATA &&
//         !mCodecContext.is_annexb)) {
//        H2645ConvertState state = {0, 0};
//        convert_h2645_to_annexb(pkt->data, pkt->size,
//                                mCodecContext.nal_size, &state);
//    }

    if (!mMediaFormat || !mMediaCodec) {
        return ERROR_DECODE_NOT_INIT;
    }

    int ret = drainDecoder();
    if (ret == ERROR_PLAYER_EOF) {
        return ERROR_PLAYER_EOF;
    }
    ret = feedDecoder(pkt);
    return ret;
}

int MediaCodecVideoDecoder::initMediaFormat(const MetaData *metadata) {
    if (!metadata || metadata->video_index < 0) {
        return ERROR_DECODE_INVALID;
    }

    auto trackInfo = metadata->track_info[metadata->video_index];
    if (trackInfo.width <= 0 || trackInfo.height <= 0) {
        return ERROR_DECODE_INVALID;
    }

    uint8_t *extradata    = trackInfo.extra_data;
    size_t extradata_size = trackInfo.extra_data_size;

    if (extradata_size < 7 || !extradata) {
        return ERROR_DECODE_INVALID;
    }

    mCodecContext.is_annexb = false;

    releaseMediaFormatDesc();
    mMediaFormat.reset(AMediaFormat_new());

    size_t sps_pps_size = 0;
    size_t convert_size = extradata_size + 20;
    uint8_t *convert_buffer =
            reinterpret_cast<uint8_t *>(calloc(1, convert_size));
    if (!convert_buffer) {
        return ERROR_OTHER_OOM;
    }

    if (extradata[0] == 1 || extradata[1] == 1) { // avcc format
        size_t nal_size = 0;
        if (mCodecId == AV_CODEC_ID_H264) {
            if (0 != convert_sps_pps(extradata, extradata_size,
                                     convert_buffer, convert_size,
                                     &sps_pps_size, &nal_size)) {
                return ERROR_PARSE_METADATA;
            }
        } else if (mCodecId == AV_CODEC_ID_HEVC) {
            if (0 != convert_hevc_nal_units(extradata, extradata_size,
                                            convert_buffer, convert_size,
                                            &sps_pps_size, &nal_size)) {
                return ERROR_PARSE_METADATA;
            }
        }
        mCodecContext.nal_size = nal_size;
        AMediaFormat_setBuffer(mMediaFormat.get(), "csd-0", convert_buffer,
                               sps_pps_size);
    } else {
        mCodecContext.is_annexb = true;
    }

    if (mCodecId == AV_CODEC_ID_H264) {
        AMediaFormat_setString(mMediaFormat.get(), AMEDIAFORMAT_KEY_MIME, "video/avc");
    } else if (mCodecId == AV_CODEC_ID_HEVC) {
        AMediaFormat_setString(mMediaFormat.get(), AMEDIAFORMAT_KEY_MIME, "video/hevc");
    }

    mCodecContext.width  = trackInfo.width;
    mCodecContext.height = trackInfo.height;

    if (mCodecContext.rotate_degree != 0) {
        AMediaFormat_setInt32(mMediaFormat.get(), "rotation-degrees",
                              mCodecContext.rotate_degree);
    }

    AMediaFormat_setInt32(mMediaFormat.get(), AMEDIAFORMAT_KEY_WIDTH,
                          mCodecContext.width);
    AMediaFormat_setInt32(mMediaFormat.get(), AMEDIAFORMAT_KEY_HEIGHT,
                          mCodecContext.height);
    AMediaFormat_setInt32(mMediaFormat.get(), AMEDIAFORMAT_KEY_MAX_INPUT_SIZE,
                          0);
    free(convert_buffer);
    return RESULT_OK;
}

int MediaCodecVideoDecoder::releaseMediaFormatDesc() {
    mMediaFormat.reset();
    return RESULT_OK;
}

int MediaCodecVideoDecoder::initMediaCodec() {
    releaseMediaCodec();

    if (!mMediaFormat) {
        return ERROR_DECODE_NOT_INIT;
    }
    mBufferIndex = -1;

    if (mCodecId == AV_CODEC_ID_H264) {
        mMediaCodec.reset(AMediaCodec_createDecoderByType("video/avc"));
    } else if (mCodecId == AV_CODEC_ID_HEVC) {
        mMediaCodec.reset(AMediaCodec_createDecoderByType("video/hevc"));
    }

    if (!mMediaCodec) {
        NEXT_LOGE(MEDIACODEC_TAG, "%p initInternal error\n", this);
        return ERROR_DECODE_VIDEO_OPEN;
    }

    media_status_t status = AMediaCodec_configure(
            mMediaCodec.get(), mMediaFormat.get(), mNativeWindow, nullptr, 0);

    if (status != AMEDIA_OK) {
        return ERROR_DECODE_VIDEO_OPEN;
    }

    status = AMediaCodec_start(mMediaCodec.get());

    if (status != AMEDIA_OK) {
        return ERROR_DECODE_VIDEO_OPEN;
    }

    bDecoderStart = true;

    return RESULT_OK;
}

int MediaCodecVideoDecoder::releaseMediaCodec() {
    mSerial++;
    if (mMediaCodec && bDecoderStart) {
        media_status_t ret = AMediaCodec_stop(mMediaCodec.get());
        if (ret < 0) {
            NEXT_LOGE(MEDIACODEC_TAG, "stop error %d\n", static_cast<int>(ret));
        }
    }
    mMediaCodec.reset();
    bDecoderStart = false;
    return RESULT_OK;
}

int MediaCodecVideoDecoder::setVideoFormat(const MetaData *metadata) {
    if (!metadata || metadata->video_index < 0) {
        NEXT_LOGE(MEDIACODEC_TAG, "metadata is invalid");
        return ERROR_DECODE_INVALID;
    }

//    AndroidHardWareContext *ctx =
//            reinterpret_cast<AndroidHardWareContext *>(
//            buffer->getVideoFormatMetadata()->hardware_context);
//    if (ctx) {
//        mNativeWindow = reinterpret_cast<ANativeWindow *>(ctx->native_window);
//        RS_LOGI(MEDIACODEC_TAG, "set native window %p\n", mNativeWindow);
//    }

    int ret = initMediaFormat(metadata);
    if (ret != RESULT_OK) {
        return ret;
    }
    ret = initMediaCodec();
    if (ret != RESULT_OK) {
        return ret;
    }

    return RESULT_OK;
}

int MediaCodecVideoDecoder::flush() {
    mSerial++;
    bDrainState = false;
    bEofState = false;
    bFirstFrame = false;
    if (mMediaCodec) {
        auto status = AMediaCodec_flush(mMediaCodec.get());
        if (status < 0) {
            NEXT_LOGE(MEDIACODEC_TAG, "flush error: %d\n", status);
            return ERROR_DECODE_VIDEO_DEC;
        }
    }
    return RESULT_OK;
}

//int MediaCodecVideoDecoder::getDelayedFrames() {
//// TODO
////    MixedBuffer buffer(BufferType::BUFFER_VIDEO_PACKET, 0);
////    while (Decode(&buffer) == RESULT_OK) {
////    }
//    bDrainState = false;
//    bEofState = false;
//    bFirstFrame = false;
//    return RESULT_OK;
//}

//int MediaCodecVideoDecoder::getDelayedFrame() {
//// TODO
////    MixedBuffer buffer(BufferType::BUFFER_VIDEO_PACKET, 0);
////    return Decode(&buffer);
//    return RESULT_OK;
//}

int MediaCodecVideoDecoder::setHardwareContext(HardWareContext *context) {
    auto *ctx =
            reinterpret_cast<AndroidHardWareContext *>(context);
    if (!ctx || !ctx->native_window) {
        return ERROR_RENDER_VIDEO_INIT;
    }
    if (mNativeWindow) {
        ANativeWindow_release(mNativeWindow);
    }
    ANativeWindow_acquire(reinterpret_cast<ANativeWindow *>(ctx->native_window));
    mNativeWindow = reinterpret_cast<ANativeWindow *>(ctx->native_window);

    initMediaCodec();
    return RESULT_OK;
}

int MediaCodecVideoDecoder::updateHardwareContext(HardWareContext *context) {
    auto *ctx = reinterpret_cast<AndroidHardWareContext *>(context);
    if (!ctx) {
        return ERROR_DECODE_NOT_INIT;
    }

    releaseMediaCodec();

    if (mNativeWindow) {
        NEXT_LOGI(MEDIACODEC_TAG, "release old native_window\n");
        ANativeWindow_release(mNativeWindow);
    }

    if (ctx->native_window) {
        NEXT_LOGI(MEDIACODEC_TAG, "acquire new native_window\n");
        ANativeWindow_acquire(reinterpret_cast<ANativeWindow *>(ctx->native_window));
    }

    mNativeWindow = reinterpret_cast<ANativeWindow *>(ctx->native_window);

    return RESULT_OK;
}

int MediaCodecVideoDecoder::getSerial() {
    int ret = mSerial.load();
    return ret;
}
