#ifndef VIDEO_RENDER_HANDLER_H
#define VIDEO_RENDER_HANDLER_H

#include <atomic>
#include <condition_variable>
#include <mutex>

#include "common/NextFrameBuffer.h"
#include "common/NextFrameQueue.h"
#include "common/MediaClock.h"
#include "kernel/handler/VideoDecodeHandler.h"
#include "render/VideoRenderFactory.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "libswscale/swscale.h"
#ifdef __cplusplus
}
#endif

class VideoRenderHandler : public BaseThread {
public:
    VideoRenderHandler(sp<VideoDecodeHandler> &videoDecodeHandler,
                       const sp<PlayerLink> &pLink, NotifyCallback notifyCb,
                       const char *threadName);

    ~VideoRenderHandler() override;

    void setConfig(const sp<GeneralConfig> &config);

    int prepare(sp<MetaData> &metadata);

#if defined(__ANDROID__)
    int setVideoSurface(ANativeWindow *surface);
#endif

#if defined(__APPLE__)
    UIView *initWithFrame(int type, CGRect cgrect);
#endif

    int startRender();

    int pauseRender();

    void executeTask() override;

    int flush();

    int stop();

    void release();

private:

    int init();

    double computeDelay(double delay);

    double computeDuration(std::unique_ptr<FrameBuffer> &buffer) const;

    int readFrame(std::unique_ptr<FrameBuffer> &buffer);

    int renderFrame(std::unique_ptr<FrameBuffer> &buffer);

    int convertPixelFormat(std::unique_ptr<FrameBuffer> &buffer);

    int updateVideoMetaData();

    void notifyListener(int what, int arg1 = 0, int arg2 = 0);

private:
    struct FrameTick {
        int serial{0};
        double time{0.0};
        double pts{0.0};
        double duration{0.0};
    };

    std::mutex mLock;
    std::condition_variable mCond;

    bool bPaused{false};
    bool bForceRefresh{true};

    sp<MetaData> mMetaData;
    sp<PlayerLink> mPlayerLink;
    sp<GeneralConfig> mGeneralConfig;
    sp<VideoDecodeHandler> mVideoDecodeHandler;

    FrameTick mFrameTick;
    NotifyCallback mNotifyCb;
    VideoSpeedMeter mSpeedMeter;
    SwsContext *mSwsContext{nullptr};
    VideoFrameMetaData mVideoFrameMetaData{};
    std::unique_ptr<VideoRender> mVideoRender{nullptr};
    VideoRenderType mRenderType{VIDEO_RENDER_UNKNOWN};
    VideoRenderBufferContext mVideoRenderBufferContext{};

#if defined(__ANDROID__)
    ANativeWindow *mNativeWindow{};
    std::atomic_bool mSurfaceUpdate{false};
#endif

};

#endif //VIDEO_RENDER_HANDLER_H
