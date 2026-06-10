/**
 * Note: Factory of audio render
 * Date: 2026/6/10
 * Author: frank
 */

#ifndef AUDIO_RENDER_FACTORY_H
#define AUDIO_RENDER_FACTORY_H

#include "AudioRender.h"

#if defined(__ANDROID__)
#include "render/android/AudioTrackRender.h"
#endif
#if defined(__APPLE__)
#include "ios/AudioQueueRender.h"
#endif

class AudioRenderFactory {
public:
    static std::unique_ptr<AudioRender> CreateAudioRender() {
#if defined(__ANDROID__)
        return std::unique_ptr<AudioRender>(new AudioTrackRender());
#endif
#if defined(__APPLE__)
        return std::unique_ptr<AudioRender>(new AudioQueueRender());
#endif
        return nullptr;
    }
};

#endif
