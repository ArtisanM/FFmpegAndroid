
#include "NextPlayer.h"

#define TAG "NextPlayer"

static bool g_ffmpeg_global_inited{false};
static LogCallback g_log_callback;
static std::mutex *g_log_mutex = new std::mutex();
static std::atomic<int> g_alive_player_count{0};

static void logCallbackReport(void *ptr, int level, const char *fmt, va_list list) {
    std::unique_lock<std::mutex> lock(*g_log_mutex);

    if (!g_log_callback) {
        return;
    }

    va_list temp;
    char line[1024 + 256];
    static int print_prefix = 1;

    if (level > AV_LOG_INFO)
        return;

    va_copy(temp, list);
    av_log_format_line(ptr, level, fmt, temp, line, sizeof(line), &print_prefix);
    va_end(temp);

    g_log_callback(level, TAG, line);
}

static void logCallbackWrapper(void *ptr, int level, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    logCallbackReport(ptr, level, fmt, args);
    va_end(args);
}

static void logCallback(void *ptr, int level, const char *buf) {
    if (buf) {
        logCallbackWrapper(nullptr, level, "%s", buf);
    }
}

void globalInit() {
    if (g_ffmpeg_global_inited) {
        return;
    }
#if CONFIG_AVDEVICE
    avdevice_register_all();
#endif
#if CONFIG_AVFILTER
    avfilter_register_all();
#endif

    avformat_network_init();
    av_log_set_callback(logCallbackReport);
    g_ffmpeg_global_inited = true;
}

void globalUninit() {
    if (!g_ffmpeg_global_inited) {
        return;
    }
    avformat_network_deinit();
    g_ffmpeg_global_inited = false;
}

void setLogCallbackLevel(int level) {

}

void setLogCallback(LogCallback cb) {
    std::unique_lock<std::mutex> lock(*g_log_mutex);
    av_log_set_callback(logCallbackReport);
    setLogLevel(LOG_LEVEL_DEBUG);
    setLogCallback(logCallback, nullptr);
    g_log_callback = std::move(cb);
}

NextPlayer::NextPlayer(int id, MsgCallback msgCallback) : mMsgCb(std::move(msgCallback)) {
    g_alive_player_count++;
}

NextPlayer::~NextPlayer() {
    mPlayer.reset();
    g_alive_player_count--;
}

sp<NextPlayer> NextPlayer::create(int id, MsgCallback msg_cb) {
    sp<NextPlayer> ret = std::shared_ptr<NextPlayer>(new NextPlayer(id, std::move(msg_cb)));
    if (!ret) {
        return ret;
    }
    if (ret->init() != RESULT_OK) {
        ret.reset();
    }
    return ret;
}

int32_t NextPlayer::init() {
    mPlayer = std::make_shared<PlayerKernel>(std::bind(&NextPlayer::notifyListener, this,
              std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
              std::placeholders::_4, std::placeholders::_5));

    if (mPlayer->init() != RESULT_OK) {
        return ERROR_OTHER_OOM;
    }
    mMsgQueue.flush();
    return RESULT_OK;
}

int32_t NextPlayer::setDataSource(const std::string& url) {
    std::unique_lock<std::mutex> lock(mLock);
    if (url.empty()) {
        return ERROR_URL_INVALID;
    }
    if (mPlayerState != MP_STATE_IDLE) {
        return ERROR_PLAYER_STATE;
    }
    lock.unlock();
    handlerURL(url);
    mPlayer->setDataSource(mUrl);
    lock.lock();
    changeState(MP_STATE_INITIALIZED);
    return RESULT_OK;
}

int32_t NextPlayer::prepareAsync() {
    std::unique_lock<std::mutex> lock(mLock);
    if (mPlayerState != MP_STATE_INITIALIZED &&
        mPlayerState != MP_STATE_STOPPED) {
        return ERROR_PLAYER_STATE;
    }
    changeState(MP_STATE_ASYNC_PREPARING);
    mMsgQueue.start();
    int32_t ret = RESULT_OK;

    mMsgThread = std::thread(
            [](NextPlayer *mp) {
                if (mp) {
                    mp->mMsgCb(mp);
                }
            },
            this);
    lock.unlock();

    ret = mPlayer->prepareAsync();

    if (ret != RESULT_OK) {
        changeState(MP_STATE_ERROR);
    }
    return ret;
}

int32_t NextPlayer::start() {
    {
        std::lock_guard<std::mutex> lock(mLock);
        if (!isPlaybackState()) {
            return ERROR_PLAYER_STATE;
        }
    }
    mMsgQueue.remove(REQUEST_START);
    notifyListener(REQUEST_START);
    return RESULT_OK;
}

int32_t NextPlayer::pause() {
    {
        std::lock_guard<std::mutex> lock(mLock);
        if (!isPlaybackState()) {
            return ERROR_PLAYER_STATE;
        }
    }
    mMsgQueue.remove(REQUEST_PAUSE);
    notifyListener(REQUEST_PAUSE);
    return RESULT_OK;
}

int32_t NextPlayer::seekTo(int64_t msec) {
    {
        std::lock_guard<std::mutex> lock(mLock);
        if (!isPlaybackState()) {
            return ERROR_PLAYER_STATE;
        }
    }
    mCurSeekPos.store(msec);
    mSeeking.store(true);
    mMsgQueue.remove(REQUEST_SEEK);
    notifyListener(REQUEST_SEEK, (int)msec);
    return RESULT_OK;
}

int32_t NextPlayer::stop() {
    std::unique_lock<std::mutex> lock(mLock);
    if (mPlayerState != MP_STATE_ASYNC_PREPARING &&
        mPlayerState != MP_STATE_PREPARED && mPlayerState != MP_STATE_STARTED &&
        mPlayerState != MP_STATE_PAUSED && mPlayerState != MP_STATE_COMPLETED &&
        mPlayerState != MP_STATE_STOPPED) {
        return ERROR_PLAYER_STATE;
    }
    mMsgQueue.remove(REQUEST_START);
    mMsgQueue.remove(REQUEST_PAUSE);
    mMsgQueue.abort();
    int32_t ret = RESULT_OK;
    lock.unlock();
    ret = mPlayer->stop();
    lock.lock();
    if (RESULT_OK == ret) {
        changeState(MP_STATE_STOPPED);
    }
    return ret;
}

void NextPlayer::release() {
    NEXT_LOGD(TAG, "%s\n", __func__);
    std::unique_lock<std::mutex> lock(mLock);
    changeState(MP_STATE_RELEASE);
    lock.unlock();
    mMsgQueue.abort();
    mMsgQueue.flush();
    mPlayer->setNotifyCb(nullptr);
    mPlayer->release();

    if (mMsgThread.joinable()) {
        mMsgThread.join();
    }
    NEXT_LOGD(TAG, "%s end\n", __func__);
}

int32_t NextPlayer::getCurrentPosition(int64_t &position) {
    if (mSeeking.load()) {
        position = mCurSeekPos;
        return RESULT_OK;
    }
    if (!mPlayer) {
        position = 0;
        return ERROR_PLAYER_NOT_INIT;
    }
    return mPlayer->getCurrentPosition(position);
}

int32_t NextPlayer::getDuration(int64_t &duration) {
    return mPlayer->getDuration(duration);
}

bool NextPlayer::isPlaying() { return (mPlayerState == MP_STATE_STARTED); }

void NextPlayer::setVolume(const float left, const float right) {
    mPlayer->setVolume(left, right);
}

void NextPlayer::notifyListener(int what, int arg1, int arg2, void *obj, int len) {
    mMsgQueue.push(what, arg1, arg2, obj, len);
}

int32_t NextPlayer::setConfig(int type, const std::string &name, const std::string &value) {
    return mPlayer->setConfig(type, name, value);
}

int32_t NextPlayer::setConfig(int type, const std::string &name, int64_t value) {
    return mPlayer->setConfig(type, name, value);
}

int32_t NextPlayer::setOption(int key, int64_t value) {
    return mPlayer->setOption(key, value);
}

int32_t NextPlayer::setOption(int key, const float value) {
    return mPlayer->setOption(key, value);
}

int64_t NextPlayer::getOption(int key, int64_t defaultValue) {
    return mPlayer->getOption(key, defaultValue);
}

float NextPlayer::getOption(int key, const float defaultValue) {
    return mPlayer->getOption(key, defaultValue);
}

int32_t NextPlayer::getVideoCodecInfo(std::string &videoInfo) {
    return mPlayer->getVideoCodecInfo(videoInfo);
}

int32_t NextPlayer::getAudioCodecInfo(std::string &audioInfo) {
    return mPlayer->getAudioCodecInfo(audioInfo);
}

int32_t NextPlayer::getPlayUrl(std::string &url) {
    if (!mPlayer->mVideoState->play_url.empty()) {
        url = mPlayer->mVideoState->play_url;
    }
    return RESULT_OK;
}

int NextPlayer::getPlayerState() {
    std::lock_guard<std::mutex> lock(mLock);
    return static_cast<int>(mPlayerState);
}

#if defined(__ANDROID__)

int32_t NextPlayer::setVideoSurface(JNIEnv *env, jobject surface) {
    int32_t ret = RESULT_OK;
    jobject oldSurface = mSurface;
    if (oldSurface == surface ||
        (oldSurface && surface && env->IsSameObject(oldSurface, surface))) {
        NEXT_LOGI(TAG, "%s same surface\n", __func__);
        return ret;
    }

    if (surface) {
        mSurface = env->NewGlobalRef(surface);
    } else {
        mSurface = nullptr;
    }

    ANativeWindow *nativeWindow = nullptr;
    if (surface) {
        nativeWindow = ANativeWindow_fromSurface(env, surface);
        if (!nativeWindow) {
            NEXT_LOGE(TAG, "%s %p null nativeWindow\n", __func__, surface);
        }
    }

//    ANativeWindow_acquire(nativeWindow);
    ret = mPlayer->setVideoSurface(nativeWindow);

    if (oldSurface) {
        JniDeleteGlobalRefP(env, &oldSurface);
    }

    return ret;
}

#endif

#if defined(__APPLE__)
UIView *RsPlayer::initWithFrame(int type, CGRect cgrect) {
  Autolock lock(mLock);
  return mPlayer->initWithFrame(type, cgrect);
}
#endif

void *NextPlayer::setInjectOpaque(void *opaque) {
    void *prevWeak = nullptr;
    prevWeak = mPlayer->setInjectOpaque(opaque);
    return prevWeak;
}

void *NextPlayer::setWeakThiz(void *weak) {
    void *prevWeak = mWeakThiz.load();
    mWeakThiz.store(weak);
    return prevWeak;
}

void *NextPlayer::getWeakThiz() { return mWeakThiz.load(); }

sp<AVMessage> NextPlayer::getMessage(bool block) {
    while (true) {
        if (mPlayerState == MP_STATE_RELEASE) {
            NEXT_LOGE(TAG, "player state interrupt, return null\n");
            return nullptr;
        }
        bool waitNext = false;
        sp<AVMessage> msg = mMsgQueue.pop(block);
        if (!msg) {
            NEXT_LOGE(TAG, "%s: null msg\n", __func__);
            return nullptr;
        }

        switch (msg->mWhat) {
            case MSG_ON_PREPARED: {
                std::unique_lock<std::mutex> lock(mLock);
                if (mPlayerState == MP_STATE_ASYNC_PREPARING) {
                    changeState(MP_STATE_PREPARED);
                } else {
                    NEXT_LOGE(TAG,"MSG_ON_PREPARED: expecting state: MP_STATE_ASYNC_PREPARING\n");
                }
                lock.unlock();
                sp<NextDictionary> player_config =
                        mPlayer ? mPlayer->getConfig(CONFIG_TYPE_PLAYER) : nullptr;
                lock.lock();
                if (!player_config->getInt64("Start-on-prepared", 1)) {
                    changeState(MP_STATE_PAUSED);
                } else {
                    changeState(MP_STATE_STARTED);
                }
                break;
            }
            case MSG_ON_COMPLETED: {
                {
                    std::unique_lock<std::mutex> lock(mLock);
                    changeState(MP_STATE_COMPLETED);
                }
                mPlayer->pause();
                break;
            }
            case MSG_SEEK_COMPLETE: {
                mSeeking.store(false);
                mCurSeekPos.store(-1);
                break;
            }
            case REQUEST_START: {
                waitNext = true;
                int32_t ret = RESULT_OK;
                std::unique_lock<std::mutex> lock(mLock);
                if (isPlaybackState() && mPlayer) {
                    lock.unlock();
                    ret = mPlayer->start();
                    lock.lock();
                    if (ret == RESULT_OK) {
                        changeState(MP_STATE_STARTED);
                    }
                }
                break;
            }
            case REQUEST_PAUSE: {
                waitNext = true;
                std::unique_lock<std::mutex> lock(mLock);
                if (isPlaybackState()) {
                    lock.unlock();
                    int pause_ret = mPlayer->pause();
                    lock.lock();
                    if (pause_ret == RESULT_OK) {
                        changeState(MP_STATE_PAUSED);
                    }
                }
                break;
            }
            case REQUEST_SEEK: {
                waitNext = true;
                std::unique_lock<std::mutex> lock(mLock);
                if (isPlaybackState()) {
                    int pos = msg->mArg1;
                    lock.unlock();
                    if (mPlayer->seekTo(pos) == RESULT_OK) {

                    }
                }
                break;
            }
        }

        if (waitNext) {
            recycleMessage(msg);
            continue;
        }

        return msg;
    }

    return nullptr;
}

int32_t NextPlayer::recycleMessage(sp<AVMessage> &msg) {
    return mMsgQueue.recycle(msg);
}

void NextPlayer::changeState(PlayerState state) {
    mPlayerState = state;
    notifyListener(MSG_PLAY_STATE_CHANGED);
}

bool NextPlayer::isPlaybackState() {
    if (mPlayerState != MP_STATE_PREPARED && mPlayerState != MP_STATE_STARTED &&
        mPlayerState != MP_STATE_PAUSED && mPlayerState != MP_STATE_COMPLETED) {
        return false;
    }
    return true;
}

void NextPlayer::handlerURL(const std::string &url) {
    if (mUrl.empty()) {
        mUrl = url;
    }

    if (mPlayer->mVideoState->play_url.empty()) {
        mPlayer->mVideoState->play_url = mUrl;
    }
}
