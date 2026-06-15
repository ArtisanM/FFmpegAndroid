#ifndef AUDIO_DECODE_HANDLER_H
#define AUDIO_DECODE_HANDLER_H

#include <condition_variable>
#include <mutex>
#include <queue>

#include "common/NextFrameBuffer.h"
#include "common/NextFrameQueue.h"
#include "decode/AudioDecoder.h"
#include "MediaParseHandler.h"


class AudioDecodeHandler : public BaseThread, AudioDecodeCallback {
public:
    AudioDecodeHandler(sp<MediaParseHandler> &mediaParseHandler, const sp<PlayerLink> &pLink,
                       NotifyCallback notifyCb, const char *threadName);

    ~AudioDecodeHandler() override;

    void executeTask() override;

    void setConfig(const sp<GeneralConfig> &config);

    int prepare(const sp<MetaData> &metadata);

    int getFrame(std::unique_ptr<FrameBuffer> &buffer);

    int getSerial();

    int onDecodedFrame(AVFrame *frame) override;

    void onDecodeError(int error) override;

    void resetEof();

    int stop();

    void release();

private:

    int init();

    int performDecode(AVPacket *pkt);

    int readPacketOrBuffering(std::unique_ptr<NextPacket> &pkt);

    int performFlush();

    int resetDecoderFormat();

    void notifyListener(int what, int arg1 = 0, int arg2 = 0);

    bool checkAccurateSeek(const std::unique_ptr<FrameBuffer> &buffer);

private:

    bool bEOF{false};
    bool bReleased{false};

    std::mutex mLock;
    std::condition_variable mCond;
    std::unique_ptr<FrameQueue> mFrameQueue;
    std::unique_ptr<AudioDecoder> mAudioDecoder;

    sp<MetaData> mMetaData;
    sp<PlayerLink> mPlayerLink;
    sp<GeneralConfig> mGeneralConfig;
    sp<MediaParseHandler> mRedSourceController;

    NotifyCallback mNotifyCb;
};

#endif //AUDIO_DECODE_HANDLER_H