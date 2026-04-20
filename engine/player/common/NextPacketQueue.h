#ifndef NEXT_PACKET_QUEUE_H
#define NEXT_PACKET_QUEUE_H

#include <condition_variable>
#include <mutex>
#include <queue>

#include "NextPacket.h"

#define MIN_PKT_DURATION 16

class NextPacketQueue {
public:
    NextPacketQueue() = default;

    explicit NextPacketQueue(int type);

    ~NextPacketQueue() = default;

    int PutPacket(std::unique_ptr<NextPacket> &pkt);

    int GetPacket(std::unique_ptr<NextPacket> &pkt, bool block);

    bool IsFlushPacket();

    int PacketCount();

    int64_t ByteCount();

    int64_t Duration();

    void Flush();

    void Release();

private:
    int64_t mDuration  = 0;
    int64_t mByteCount = 0;

    std::mutex mLock;
    std::condition_variable mCond;
    std::queue<std::unique_ptr<NextPacket>> mPktQueue;
};


#endif //Next_PACKET_QUEUE_H
