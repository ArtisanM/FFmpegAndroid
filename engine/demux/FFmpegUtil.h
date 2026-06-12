#ifndef FFMPEG_UTIL_H
#define FFMPEG_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avstring.h"
#include "libavutil/dict.h"
#include "libavutil/display.h"
#include "libavutil/eval.h"
#include "libavutil/opt.h"
#ifdef __cplusplus
}
#endif

int64_t getBitrate(AVCodecParameters *codecpar);

double getRotation(AVStream *st);

AVDictionary **findStreamInfoOpts(AVFormatContext *s,
                                  AVDictionary *codec_opts);

AVDictionary *filterCodecOpts(AVDictionary *opts, enum AVCodecID codec_id,
                              AVFormatContext *s, AVStream *st, AVCodec *codec);
#endif
