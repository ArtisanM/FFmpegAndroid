#ifndef MEDIA_PARSE_HANDLER_H
#define MEDIA_PARSE_HANDLER_H

#include <condition_variable>
#include <mutex>
#include <unordered_map>

#include "common/BaseThread.h"
#include "common/NextConfig.h"
#include "common/NextPacket.h"
#include "common/NextPacketQueue.h"
#include "common/NextPlayerDefine.h"
#include "NextExtractor.h"


using PrepareCallBack = std::function<void(sp<MetaData> &)>;

class MediaParseHandler : public BaseThread {
public:
    MediaParseHandler(const sp<PlayerLink> &pLink, NotifyCallback notifyCb, const char *threadName);

    ~MediaParseHandler() override;

    int Open(std::string &url);

    void SetPrepareCb(PrepareCallBack callBack);

    void SetConfig(const sp<GeneralConfig> &config);

    int PrepareAsync();

    int Seek(int64_t msec);

    bool FrontIsFlush(int streamType);

    void ToggleBuffering(bool buffering);

    int GetSerial();

    int GetPacket(std::unique_ptr<NextPacket> &pkt, int streamType, bool block);

    void executeTask() override;

    int Stop();

    void Release();

private:

    int SetMetaData();

    int PutPacketByType(PacketOpType type); // 1=flush 2=eof

    bool IsBufferFinish();

    static int GetErrorType(int errorCode);

    void NotifyListener(int what, int arg1 = 0, int arg2 = 0);

    int PerformFlush();

    sp<NextPacketQueue> GetQueueByStreamType(int streamType);

    void CheckBuffering();

    void UpdateCacheStatistic();

    bool CheckDropNonRefFrame(AVPacket *pkt);

private:
    int mMaxBufferSize{MAX_QUEUE_SIZE};
    int mBufferingPercent{0};

    bool bEOF{false};
    bool bBuffering{false};
    std::atomic_bool bReleased{false};

    sp<MetaData> mMetaData;
    sp<PlayerLink> mPlayerLink;
    sp<NextExtractor> mExtractor;
    sp<GeneralConfig> mGeneralConfig;

    std::string mUrl;
    std::mutex mLock;
    std::condition_variable mCond;
    NotifyCallback mNotifyCb;
    PrepareCallBack mPrepareCb;
    std::unordered_map<int, sp<NextPacketQueue>> mPktQueueMap;
};

#endif //MEDIA_PARSE_HANDLER_H
