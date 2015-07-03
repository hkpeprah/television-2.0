#ifndef __MESSAGE_H__
#define __MESSAGE_H__

typedef enum {
  NUM_APP_KEYS
} AppKeys;

// Note: On a failure, the handler will be called with success being false, when
// this is the case, the iterator will be NULL.
typedef void(*AppMessageHandler)(DictionaryIterator *iterator, bool success, void *context);

bool app_message_send(Tuplet*, uint8_t);
void app_message_register_handler(AppMessageHandler, void*);
void app_message_init(void);
void app_message_deinit(void);

#endif /* __MESSAGE_H__ */
