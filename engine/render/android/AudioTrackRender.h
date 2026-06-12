#if defined(__ANDROID__)
#ifndef AUDIO_TRACK_RENDER_H
#define AUDIO_TRACK_RENDER_H

#include <thread>

#include "render/AudioRender.h"
#include "AudioTrackHelper.h"

#define AUDIO_TRACK_MODE_STREAM 1

#define AUDIO_TRACK_MAX_SPEED   2

#define AUDIO_STREAM_TYPE_MUSIC 3

enum AudioTrackFormat {
    ENCODING_INVALID   = 0,
    ENCODING_DEFAULT   = 1,
    ENCODING_PCM_16BIT = 2,
    ENCODING_PCM_8BIT  = 3,
    ENCODING_PCM_FLOAT = 4
};

enum class AudioTrackChannelConfig {
    CHANNEL_CONFIG_INVALID  = 0x0,
    CHANNEL_CONFIG_DEFAULT  = 0x1,
    CHANNEL_CONFIG_MONO     = 0x4,
    CHANNEL_CONFIG_STEREO   = 0xC,
    CHANNEL_CONFIG_LF       = 0x20,
    CHANNEL_CONFIG_QUAD     = 0xCC,
    CHANNEL_CONFIG_SURROUND = 0x41C,
    CHANNEL_CONFIG_5POINT1  = 0xFC,
    CHANNEL_CONFIG_7POINT1  = 0x3FC
};

class AudioTrackRender : public AudioRender {
public:
    AudioTrackRender();

    ~AudioTrackRender() override;

    int openAudio(const AudioRenderInfo &expect, AudioRenderInfo &actual,
                  std::unique_ptr<AudioCallback> &audioCallback) override;

    void pauseAudio(bool paused) override;

    void flushAudio() override;

    double getDelay() override;

    void setDefaultDelay(double latency) override;

    int getAudioCallBack() override;

    void setPlaybackRate(float playbackRate) override;

    void setPlaybackVolume(float volume) override;

    int getAudioSessionId() override;

    void closeAudio(bool waiting) override;

private:
    static int AudioLoop(AudioTrackRender *pRender);

    static void AdjustAudioInfo(const AudioRenderInfo &desired, AudioRenderInfo &obtained);

private:
    jobject mAudioTrack     = nullptr;
    jbyteArray mByteBuffer  = nullptr;
    int mByteBufferCapacity = 0;

    std::mutex mMutex;
    std::mutex mThreadMutex;
    std::thread *mAudioThread = nullptr;
    std::condition_variable mWakeupCond;

    int mBufferSize     = 0;
    uint8_t *mBuffer    = nullptr;
    double mMinLatency  = 0;
    int mAudioSessionId = -1;

    volatile bool bIsPaused      = true;
    volatile bool bNeedFlush     = false;
    volatile bool bSetVolume     = false;
    volatile bool bChangeSpeed   = false;
    volatile bool bAbortRequest  = false;
    volatile float mLeftVolume   = 0;
    volatile float mRightVolume  = 0;
    volatile float mCurrentSpeed = 0;

};

#endif // AUDIO_TRACK_RENDER_H
#endif // defined(__ANDROID__)
