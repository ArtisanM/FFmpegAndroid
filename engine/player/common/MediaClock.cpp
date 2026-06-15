/**
 * Note: media clock of player
 * Date: 2026/3/17
 * Author: frank
 */

#include "MediaClock.h"

#include <chrono>
#include <cmath>

using namespace std::chrono;

MediaClock::MediaClock()
        : mSerial(0),
          bPause(true),
          mSpeed(1.0f) {
    mLastUpdateTime = getCurrentTime();
    mPtsDrift = 0 - mLastUpdateTime;
}

void MediaClock::setClock(double pts) {
    std::lock_guard<std::mutex> lock(mLock);
    double now = getCurrentTime();
    mPtsDrift = pts - now;
    mLastUpdateTime = now;
}

double MediaClock::getClock() {
    std::lock_guard<std::mutex> lock(mLock);
    double now = getCurrentTime();
    if (bPause)
        now = mLastUpdateTime;
    return mPtsDrift + now - (now - mLastUpdateTime) * (1.0f - mSpeed);
}

void MediaClock::setClockSerial(int serial) {
    std::lock_guard<std::mutex> lck(mLock);
    mSerial = serial;
}

int MediaClock::getClockSerial() {
    std::lock_guard<std::mutex> lock(mLock);
    return mSerial;
}

void MediaClock::setSpeed(double speed) {
    std::lock_guard<std::mutex> lock(mLock);
    mSpeed = speed;
}

void MediaClock::setPause(bool paused) {
    std::lock_guard<std::mutex> lock(mLock);
    bPause = paused;
}

double MediaClock::getCurrentTime() {
    return static_cast<double>(duration_cast<milliseconds >(
            system_clock::now().time_since_epoch()).count()) / 1000.0;
}
