#ifndef NEXT_LOG_H
#define NEXT_LOG_H

#define AV_LEVEL_FATAL 128
#define AV_LEVEL_ERROR 64
#define AV_LEVEL_WARN  32
#define AV_LEVEL_INFO  16
#define AV_LEVEL_DEBUG 8


int logPrint(int level, const char *tag, const char *fmt, ...);
void setLogLevel(int level);
void setLogCallback(void (*callback)(void *, int, const char *), void *userdata);

typedef struct LogContext {
  int level;
  void *userdata;
  void (*callback)(void *arg, int level, const char *buf);
} LogContext;

#define NEXT_LOGD(...) logPrint(AV_LEVEL_DEBUG, __VA_ARGS__)
#define NEXT_LOGI(...) logPrint(AV_LEVEL_INFO, __VA_ARGS__)
#define NEXT_LOGW(...) logPrint(AV_LEVEL_WARN, __VA_ARGS__)
#define NEXT_LOGE(...) logPrint(AV_LEVEL_ERROR, __VA_ARGS__)

#endif
