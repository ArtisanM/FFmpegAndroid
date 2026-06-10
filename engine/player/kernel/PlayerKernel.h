#ifndef RS_PLAYER_KERNEL_H
#define RS_PLAYER_KERNEL_H

#include "common/NextConfig.h"
#include "CommonUtil.h"
#include "kernel/handler/AudioDecodeHandler.h"
#include "kernel/handler/VideoDecodeHandler.h"
#include "kernel/handler/AudioRenderHandler.h"
#include "kernel/handler/VideoRenderHandler.h"
#include "kernel/handler/MediaParseHandler.h"
#include "NextDictionary.h"
#include "NextErrorCode.h"

extern "C" {
#include "libavutil/dict.h"
#include "libavutil/pixdesc.h"
}

struct ApplicationContext {
    void *opaque; // user data
    int (*app_func_event)(ApplicationContext *h, int event_type, void *obj);
};

void globalSetInjectCallback(InjectCallback cb);

class RsPlayer;

class PlayerKernel {
public:
    PlayerKernel() = default;

    PlayerKernel(NotifyCallback notifyCb);

    ~PlayerKernel();

    int init();

    void setDataSource(const std::string &url);

    void setDataSourceFd(int64_t fd);

    int prepareAsync();

    int start();

    int pause();

    int seekTo(int64_t msec, bool flush = true);

    int stop();

    void release();

    int getCurrentPosition(int64_t &position);

    int getDuration(int64_t &duration);

    void setVolume(float left, float right);

    void setLoop(int count);

    int getLoop();

    int setConfig(int type, const std::string &name, const std::string &value);

    int setConfig(int type, const std::string &name, int64_t value);

    int setOption(int key, int64_t value);

    int setOption(int key, float value);

    int64_t getOption(int key, int64_t defaultVal);

    float getOption(int key, float defaultVal);

    void setNotifyCb(NotifyCallback cb);

    void *setInjectOpaque(void *opaque);

    void *getInjectOpaque();

    sp<NextDictionary> getConfig(int type);

    int getVideoCodecInfo(std::string &videoInfo) const;

    int getAudioCodecInfo(std::string &audioInfo) const;

#if defined(__ANDROID__)

    int setVideoSurface(ANativeWindow *surface);

#endif
#if defined(__APPLE__)
    UIView *initWithFrame(int type, CGRect cgrect);
#endif

    void notifyListener(int what, int arg1 = 0, int arg2 = 0, void *obj = nullptr, int len = 0);

private:
    int configInternal();

    int performConfigs();

    int resetConfigs();

    int performStart();

    int performPrepare();

    int performSeek(int64_t msec);

    int performStop();

    int performPause();

    int performFlush();

    void parseExtraData(TrackInfo &info) const;

    void prepareStream(sp<MetaData> &metadata);

    void preparedCallback(sp<MetaData> &metadata);

    void setPlaybackRate(float rate);

public:
    sp<PlayerLink> mVideoState;

private:
    std::string mUrl;
    std::mutex mLock;
    std::mutex mSurfaceLock;
    std::mutex mNotifyCbLock;
    std::mutex mAudioStreamLock;
    std::mutex mVideoStreamLock;
    std::atomic<void *> mInjectOpaque{};

    int64_t mFd{-1};
    bool bAbort{false};
    bool bPausedByUser{false};
    std::atomic_bool mSeeking{false};
    std::atomic_bool mCompleted{false};

    sp<MediaParseHandler>  mParseHandler;
    sp<VideoDecodeHandler> mVideoDecHandler;
    sp<AudioDecodeHandler> mAudioDecHandler;
    sp<VideoRenderHandler> mVideoRenderHandler;
    sp<AudioRenderHandler> mAudioRenderHandler;

    sp<MetaData> mMetaData;
    NotifyCallback mNotifyCb;
    ApplicationContext *mAppCtx{nullptr};
    std::atomic<int64_t> mCurSeekPos{-1};

    sp<NextDictionary> mSwsConfig;
    sp<NextDictionary> mSwrConfig;
    sp<NextDictionary> mCodecConfig;
    sp<NextDictionary> mPlayerConfig;
    sp<NextDictionary> mFormatConfig;
    sp<GeneralConfig> mGeneralConfig;
};

#endif //RS_PLAYER_KERNEL_H
