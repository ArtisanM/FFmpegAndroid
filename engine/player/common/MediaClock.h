#ifndef MEDIA_CLOCK_H
#define MEDIA_CLOCK_H

#include <mutex>

enum AVCLockType {
    CLOCK_AUDIO    = 0,
    CLOCK_VIDEO    = 1,
    CLOCK_EXTERNAL = 2
};

class MediaClock {
public:
    MediaClock();

    void setClock(double pts);

    double getClock();

    void setClockSerial(int serial);

    int getClockSerial();

    void setSpeed(double speed);

    void setPause(bool paused);

private:
    static double getCurrentTime();

private:
    std::mutex mLock;
    int mSerial;
    bool bPause;
    double mSpeed;
    double mPtsDrift;
    double mLastUpdateTime;

};

#endif // MEDIA_CLOCK_H
