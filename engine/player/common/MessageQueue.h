#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include "NextDefine.h"
#include "NextStructDefine.h"

#include <condition_variable>
#include <mutex>
#include <queue>

class AVMessage {
public:
    AVMessage() = default;

    AVMessage(int what, int arg1 = 0, int arg2 = 0, void *obj = nullptr, int len = 0);

    ~AVMessage();

    void clear();

public:
    int mWhat     = 0;
    int mArg1     = 0;
    int mArg2     = 0;
    void *mObj    = nullptr;
    int64_t mTime = 0;
};

class MessageQueue {
public:
    MessageQueue();

    ~MessageQueue();

    int32_t start();

    int32_t push(int what, int arg1 = 0, int arg2 = 0, void *obj = nullptr, int len = 0);

    sp<AVMessage> pop(bool block);

    int32_t flush();

    int32_t remove(int what);

    int32_t recycle(sp<AVMessage> &msg);

    int32_t abort();

private:
    bool mMsgAbort;
    std::mutex mMsgMutex;
    std::condition_variable mMsgCondition;
    std::queue<sp<AVMessage>> mMessageQueue;
    std::queue<sp<AVMessage>> mRecycledQueue;
};

#endif
