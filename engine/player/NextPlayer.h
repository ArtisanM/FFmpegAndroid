#ifndef NEXT_PLAYER_H
#define NEXT_PLAYER_H

#if defined(__ANDROID__)

#include <android/native_window_jni.h>

#endif

#include "common/NextConfig.h"
#include "common/MessageQueue.h"
#include "CommonUtil.h"
#include "kernel/PlayerKernel.h"
#include "NextDefine.h"
#include "NextErrorCode.h"
#include "NextMessage.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
}

void globalInit();

void globalUninit();

void setLogLevel(int level);

void setLogCallback(LogCallback cb);

void setLogCallbackLevel(int level);

class NextPlayer {
    using MsgCallback = std::function<int32_t(NextPlayer *)>;

public:
    ~NextPlayer();

    static sp<NextPlayer> create(int id, MsgCallback callback);

    int32_t setDataSource(const std::string &url);

    int32_t prepareAsync();

    int32_t start();

    int32_t pause();

    int32_t seekTo(int64_t msec);

    int32_t stop();

    void release();

    int32_t getCurrentPosition(int64_t &position);

    int32_t getDuration(int64_t &duration);

    bool isPlaying();

    void setVolume(float left, float right);

    void notifyListener(int what, int arg1 = 0, int arg2 = 0,
                        void *obj = nullptr, int len = 0);

    int32_t setConfig(int type, const std::string &name, const std::string &value);

    int32_t setConfig(int type, const std::string &name, int64_t value);

    int32_t setOption(int key, int64_t value);

    int32_t setOption(int key, float value);

    int64_t getOption(int key, int64_t defaultValue);

    float getOption(int key, float defaultValue);

    int32_t getVideoCodecInfo(std::string &videoInfo);

    int32_t getAudioCodecInfo(std::string &audioInfo);

    int32_t getPlayUrl(std::string &url);

    int getPlayerState();

#if defined(__ANDROID__)

    int32_t setVideoSurface(JNIEnv *env, jobject surface);

#endif

#if defined(__APPLE__)
    UIView *initWithFrame(int type, CGRect cgrect);
#endif

    void *setInjectOpaque(void *opaque);

    void *setWeakThiz(void *weak);

    void *getWeakThiz();

    sp<AVMessage> getMessage(bool block = false);

    int32_t recycleMessage(sp<AVMessage> &msg);

private:
    NextPlayer() = default;

    NextPlayer(int id, MsgCallback callback);

    int32_t init();

    void changeState(PlayerState state);

    bool isPlaybackState();

    void handlerURL(const std::string &url);

private:

    std::string mUrl;
    std::mutex mLock;
    MsgCallback mMsgCb;
    std::thread mMsgThread;
    MessageQueue mMsgQueue;
    sp<PlayerKernel> mPlayer;
    std::atomic<void *> mWeakThiz{};
    std::atomic_bool mSeeking{false};
    std::atomic<int64_t> mCurSeekPos{-1};
    PlayerState mPlayerState{MP_STATE_IDLE};
#if defined(__ANDROID__)
    jobject mSurface{nullptr};
#endif
#if defined(__HARMONY__)
    OHNativeWindow *mWindow{nullptr};
#endif
};

#endif //NEXT_PLAYER_H
