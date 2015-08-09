#include <pebble.h>
#include <stdlib.h>

#include "message.h"
#include "../debug/logging.h"

typedef struct {
  AppMessageHandler message_handler;
  void *callback_context;
} AppMessageData;


// Private message handlers
////////////////////////////

static char *prv_translate_error(AppMessageResult result) {
  switch (result) {
    case APP_MSG_OK:
      return "APP_MSG_OK";
    case APP_MSG_SEND_TIMEOUT:
      return "APP_MSG_SEND_TIMEOUT";
    case APP_MSG_SEND_REJECTED:
      return "APP_MSG_SEND_REJECTED";
    case APP_MSG_NOT_CONNECTED:
      return "APP_MSG_NOT_CONNECTED";
    case APP_MSG_APP_NOT_RUNNING:
      return "APP_MSG_APP_NOT_RUNNING";
    case APP_MSG_INVALID_ARGS:
      return "APP_MSG_INVALID_ARGS";
    case APP_MSG_BUSY:
      return "APP_MSG_BUSY";
    case APP_MSG_BUFFER_OVERFLOW:
      return "APP_MSG_BUFFER_OVERFLOW";
    case APP_MSG_ALREADY_RELEASED:
      return "APP_MSG_ALREADY_RELEASED";
    case APP_MSG_CALLBACK_ALREADY_REGISTERED:
      return "APP_MSG_CALLBACK_ALREADY_REGISTERED";
    case APP_MSG_CALLBACK_NOT_REGISTERED:
      return "APP_MSG_CALLBACK_NOT_REGISTERED";
    case APP_MSG_OUT_OF_MEMORY:
      return "APP_MSG_OUT_OF_MEMORY";
    case APP_MSG_CLOSED:
      return "APP_MSG_CLOSED";
    case APP_MSG_INTERNAL_ERROR:
      return "APP_MSG_INTERNAL_ERROR";
    default:
      return "UNKNOWN ERROR";
  }
}

static void prv_in_recv_handler(DictionaryIterator *iterator, void *context) {
  AppMessageData *data = app_message_get_context();
  if (data->message_handler) {
    data->message_handler(iterator, true /* success */, data->callback_context);
  }
}

static void prv_in_dropped_handler(AppMessageResult reason, void *context) {
  ERROR(prv_translate_error(reason));
  AppMessageData *data = app_message_get_context();
  if (data->message_handler) {
    data->message_handler(NULL, false /* failure */, data->callback_context);
  }
}

// Public API
/////////////////////////////

bool send_app_message(Tuplet *data, uint8_t len) {
  DictionaryIterator *iter;
  uint8_t i;

  app_message_outbox_begin(&iter);
  if (iter == NULL) {
    // Outbox may be busy, or there was an error, either way
    // return to indicate this.
    return false;
  }

  for (i = 0; i < len; i++) {
    dict_write_tuplet(iter, &data[i]);
  }

  dict_write_end(iter);
  app_message_outbox_send();
  return true;
}

void app_message_register_handler(AppMessageHandler handler, void *callback_context) {
  AppMessageData *data = app_message_get_context();
  if (!data) {
    WARN("app message not initialized before registering");
    app_message_init();
    app_message_register_handler(handler, callback_context);
    return;
  }
  data->message_handler = handler;
  data->callback_context = callback_context;
}

void app_message_init(void) {
  AppMessageData *data = malloc(sizeof(AppMessageData));
  memset(data, 0, sizeof(AppMessageData));

  app_message_register_inbox_received(prv_in_recv_handler);
  app_message_register_inbox_dropped(prv_in_dropped_handler);
  app_message_open(app_message_inbox_size_maximum(),
    app_message_outbox_size_maximum());
  app_message_set_context((void *)data);

  app_message_open(app_message_inbox_size_maximum(),
    app_message_outbox_size_maximum());
}

void app_message_deinit(void) {
  AppMessageData *data = app_message_get_context();
  if (!data) {
    WARN("app message deinit called without being initialized first");
    return;
  }
  free(data);
}
