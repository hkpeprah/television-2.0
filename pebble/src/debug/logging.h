#ifndef __LOGGING_H__
#define __LOGGING_H__
#include <pebble.h>

#define __APP_NAME__  "Digital Monsters"

#define DEBUG(fmt, ...)    APP_LOG(APP_LOG_LEVEL_DEBUG, fmt, ## __VA_ARGS__)
#define WARNING(fmt, ...)  APP_LOG(APP_LOG_LEVEL_WARNING, fmt, ## __VA_ARGS__)
#define ERROR(fmt, ...)    APP_LOG(APP_LOG_LEVEL_ERROR, fmt, ## __VA_ARGS__)
#define INFO(fmt, ...)     APP_LOG(APP_LOG_LEVEL_INFO, fmt, ## __VA_ARGS__)

#endif /* __LOGGING_H__ */
