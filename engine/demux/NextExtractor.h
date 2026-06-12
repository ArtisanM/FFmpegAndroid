#ifndef NEXT_EXTRACTOR_H
#define NEXT_EXTRACTOR_H

#include "ExtractorInterface.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "libavformat/avformat.h"
#ifdef __cplusplus
}
#endif

class NextExtractor : public ExtractorInterface {
public:

    explicit NextExtractor(NotifyCallback &notifyCb);

    ~NextExtractor() override;

    int open(const std::string &url, FFmpegOption &opt,
             std::shared_ptr<MetaData> &metadata) override;

    int readPacket(AVPacket *pkt) override;

    int seek(int64_t timestamp, int64_t rel, int seekFlags) override;

    int getError() override;

    int getStreamType(int streamIndex) override;

    void setInterrupt() override;

    void close() override;

private:
    static int interruptCallback(void *opaque);

    void notifyListener(int32_t what, int32_t arg1 = 0, int32_t arg2 = 0,
                        void *obj = nullptr, int len = 0);

private:
    NotifyCallback mNotifyCb;
    std::atomic_bool bAbort {false};
    AVFormatContext *mFormatCtx = nullptr;

};

#endif
