#ifndef NEXT_CONFIG_H
#define NEXT_CONFIG_H

#include <string>

#include "NextDefine.h"


#ifdef __cplusplus
extern "C" {
#endif
#include "libavutil/avutil.h"
#include "libavutil/dict.h"
#ifdef __cplusplus
}
#endif

#define CONFIG_OFFSET(x) offsetof(PlayerConfig, x)

#define CONFIG_INT(default__, min__, max__)                                    \
  OPTION_TYPE_INT32, {.i64 = default__}, min__, max__

#define CONFIG_INT64(default__, min__, max__)                                  \
  OPTION_TYPE_INT64, {.i64 = default__}, min__, max__

enum OptionType {
    OPTION_TYPE_UNKNOWN = 0,
    OPTION_TYPE_INT32   = 1,
    OPTION_TYPE_INT64   = 2,
    OPTION_TYPE_STRING  = 3
};

enum ConfigType {
    CONFIG_TYPE_FORMAT = 1,
    CONFIG_TYPE_CODEC  = 2,
    CONFIG_TYPE_PLAYER = 3,
    CONFIG_TYPE_SWS    = 4,
    CONFIG_TYPE_SWR    = 5
};

typedef struct DemuxCacheControl {
    int max_buffer_size;
    int high_water_mark_in_bytes;
    int first_high_water_mark_in_ms;
    int next_high_water_mark_in_ms;
    int last_high_water_mark_in_ms;
    int current_high_water_mark_in_ms;
} DemuxCacheControl;

struct PlayerConfig {
    int32_t audio_disable;
    int32_t video_disable;
    int32_t loop;
    int32_t framedrop;
    int64_t seek_at_start;
    int32_t max_fps;
    int32_t start_on_prepared;
    int32_t pictq_size;
    int32_t packet_buffering;
    int32_t enable_accurate_seek;
    int32_t accurate_seek_timeout;
    int32_t video_hdr_enable;
    int32_t enable_vtb;
    int32_t enable_ndkvdec;
    int32_t mediacodec_auto_rotate;
    int32_t vtb_max_error_count;
    DemuxCacheControl dcc;
};

struct AvConfig {
    const char *name;
    const char *desc;
    int offset;
    OptionType type;
    union {
        int64_t i64;
        const char *str;
    } defaultVal;
    double min;
    double max;
};

static const AvConfig AvConfigs[] = {
        {"an", "disable audio", CONFIG_OFFSET(audio_disable), CONFIG_INT(0, 0, 1)},
        {"vn", "disable video", CONFIG_OFFSET(video_disable), CONFIG_INT(0, 0, 1)},
        {"loop", "set number of times the playback shall be looped",
                CONFIG_OFFSET(loop), CONFIG_INT(1, INT_MIN, INT_MAX)},
        {"framedrop", "drop frames when cpu is too slow",
                CONFIG_OFFSET(framedrop), CONFIG_INT(0, -1, 120)},
        {"Seek-at-Start", "set offset of player should be seeked",
                CONFIG_OFFSET(seek_at_start), CONFIG_INT64(0, 0, INT_MAX)},
        {"max-fps", "drop frames when fps is greater than max-fps",
                CONFIG_OFFSET(max_fps), CONFIG_INT(61, -1, 121)},
        {"Start-on-prepared", "automatically Start playing on prepared",
                CONFIG_OFFSET(start_on_prepared), CONFIG_INT(1, 0, 1)},
        {"video-pictq-size", "max picture queue frame count",
                CONFIG_OFFSET(pictq_size),
                CONFIG_INT(VIDEO_PICTURE_QUEUE_SIZE_DEFAULT, VIDEO_PICTURE_QUEUE_SIZE_MIN,
                           VIDEO_PICTURE_QUEUE_SIZE_MAX)},
        {"max-buffer-size", "max buffer PacketCount should be pre-read",
                CONFIG_OFFSET(dcc.max_buffer_size),
                CONFIG_INT(MAX_QUEUE_SIZE, 0, MAX_QUEUE_SIZE)},
        {"first-high-water-mark-ms", "first chance to wakeup read_thread",
                CONFIG_OFFSET(dcc.first_high_water_mark_in_ms),
                CONFIG_INT(DEFAULT_FIRST_HIGH_WATER_MARK_IN_MS,
                           DEFAULT_FIRST_HIGH_WATER_MARK_IN_MS,
                           DEFAULT_LAST_HIGH_WATER_MARK_IN_MS)},
        {"next-high-water-mark-ms", "second chance to wakeup read_thread",
                CONFIG_OFFSET(dcc.next_high_water_mark_in_ms),
                CONFIG_INT(DEFAULT_NEXT_HIGH_WATER_MARK_IN_MS,
                           DEFAULT_FIRST_HIGH_WATER_MARK_IN_MS,
                           DEFAULT_LAST_HIGH_WATER_MARK_IN_MS)},
        {"last-high-water-mark-ms", "last chance to wakeup read_thread",
                CONFIG_OFFSET(dcc.last_high_water_mark_in_ms),
                CONFIG_INT(DEFAULT_LAST_HIGH_WATER_MARK_IN_MS,
                           DEFAULT_FIRST_HIGH_WATER_MARK_IN_MS,
                           DEFAULT_LAST_HIGH_WATER_MARK_IN_MS)},
        {"packet-buffering",
                "Pause output until enough packets have been read after stalling",
                CONFIG_OFFSET(packet_buffering), CONFIG_INT(1, 0, 1)},
        {"enable-accurate-seek", "enable accurate Seek",
                CONFIG_OFFSET(enable_accurate_seek), CONFIG_INT(1, 0, 1)},
        {"accurate-Seek-timeout", "accurate Seek timeout",
                CONFIG_OFFSET(accurate_seek_timeout),
                CONFIG_INT(MAX_ACCURATE_SEEK_TIMEOUT, 0, MAX_ACCURATE_SEEK_TIMEOUT)},
        {"enable-video-hdr", "enable video hdr", CONFIG_OFFSET(video_hdr_enable),
                CONFIG_INT(0, 0, 1)},
        // iOS options
        {"enable_vtb", "use VideoToolbox to decode video", CONFIG_OFFSET(enable_vtb),
                CONFIG_INT(0, 0, 1)},
        {"enable_vtb-max-error-count", "VideoToolbox: max error count",
                CONFIG_OFFSET(vtb_max_error_count), CONFIG_INT(5, 0, INT_MAX)},
        // Android options
        {"enable-mediacodec", "use MediaCodec to decode video", CONFIG_OFFSET(enable_ndkvdec),
                CONFIG_INT(0, 0, 1)}
};

class NextPlayerConfig {
public:
    NextPlayerConfig();
    ~NextPlayerConfig();
    void reset();
    PlayerConfig *operator->();
    PlayerConfig *get();

private:
    PlayerConfig *mConfig{nullptr};
};

struct GeneralConfig {
    AVDictionary *swsConfig{nullptr};
    AVDictionary *swrConfig{nullptr};
    AVDictionary *codecConfig{nullptr};
    AVDictionary *formatConfig{nullptr};
    std::shared_ptr<NextPlayerConfig> playerConfig;
};

#endif // NEXT_CONFIG_H
