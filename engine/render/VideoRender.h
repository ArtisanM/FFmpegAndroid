/**
 * Note: interface of video render
 * Date: 2025/12/13
 * Author: frank
 */

#ifndef VIDEO_RENDER_H
#define VIDEO_RENDER_H

#if defined(__ANDROID__)
#include <android/native_window.h>
#endif

#include "VideoRenderInfo.h"
#include "NextErrorCode.h"

class VideoRender {
public:
    VideoRender() = default;

    virtual ~VideoRender() = default;

#if defined(__ANDROID__)

    virtual int init() {return RESULT_OK;};

    virtual int setSurface(ANativeWindow *nativeWindow) {return RESULT_OK;};

#elif defined(__APPLE__)

    virtual int init(){return RESULT_OK;};

    virtual int initWithFrame(CGRect cgrect) {return RESULT_OK;};

    virtual UIView *getRedRenderView() {return nil;};

#endif

    virtual int attachFilter(VideoFilterType videoFilterType,
                             VideoFrameMetaData *inputFrameMetaData) {return RESULT_OK;};

    virtual int detachFilter(VideoFilterType videoFilterType) {return RESULT_OK;};

    virtual int detachAllFilter() {return RESULT_OK;};

    virtual int onInputFrame(VideoFrameMetaData *redRenderBuffer) {return RESULT_OK;};

    virtual int onRender() {return RESULT_OK;};

    virtual int onRender(VideoRenderBufferContext *bufferContext, bool render) {return RESULT_OK;};

    virtual int onRenderCacheFrame() {return RESULT_OK;};

    virtual int releaseContext() {return RESULT_OK;};

    virtual void close() {};

};

#endif
