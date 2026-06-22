#ifndef NEXT_LOG_H
#define NEXT_LOG_H

#define LOG_LEVEL_FATAL 8
#define LOG_LEVEL_ERROR 16
#define LOG_LEVEL_WARN  24
#define LOG_LEVEL_INFO  32
#define LOG_LEVEL_DEBUG 48


int logPrint(int level, const char *tag, const char *fmt, ...);
void setLogLevel(int level);
void setLogCallback(void (*callback)(void *, int, const char *), void *userdata);

typedef struct LogContext {
  int level;
  void *userdata;
  void (*callback)(void *arg, int level, const char *buf);
} LogContext;

#define NEXT_LOGD(...) logPrint(LOG_LEVEL_DEBUG, __VA_ARGS__)
#define NEXT_LOGI(...) logPrint(LOG_LEVEL_INFO, __VA_ARGS__)
#define NEXT_LOGW(...) logPrint(LOG_LEVEL_WARN, __VA_ARGS__)
#define NEXT_LOGE(...) logPrint(LOG_LEVEL_ERROR, __VA_ARGS__)

#endif
