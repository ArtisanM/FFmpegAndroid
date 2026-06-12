/**
 * Note: interface of audio render
 * Date: 2025/12/10
 * Author: frank
 */

#ifndef AUDIO_RENDER_H
#define AUDIO_RENDER_H

#include <memory>

#include "AudioRenderInfo.h"

class AudioRender {
public:
    AudioRender() = default;

    virtual ~AudioRender() = default;

    virtual int openAudio(const AudioRenderInfo &expect, AudioRenderInfo &actual,
                          std::unique_ptr<AudioCallback> &audioCallback) = 0;

    virtual void pauseAudio(bool paused) = 0;

    virtual void flushAudio() = 0;

    virtual double getDelay() = 0;

    virtual void setDefaultDelay(double latency) = 0;

    virtual int getAudioCallBack() = 0;

    virtual void setPlaybackRate(float playbackRate) = 0;

    virtual void setPlaybackVolume(float volume) = 0;

    virtual int getAudioSessionId() = 0;

    virtual void closeAudio(bool waiting) = 0;

protected:
    std::unique_ptr<AudioCallback> mAudioCallback = nullptr;
};

#endif
