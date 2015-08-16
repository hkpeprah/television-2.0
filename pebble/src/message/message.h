#pragma once
#include <pebble.h>

typedef enum {
  // We receive this key when we are to either show or hide the configuration
  // window for the application.
  AppKeyConfig = 0,
  AppKeyRequest,           // uint8
  // Settings Keys
  AppKeyCrunchyrollStatus, // boolean
  AppKeyFunimationStatus,  // boolean
  AppKeyCountryCode,       // string
  AppKeySubscriptionCount, // uint32
  // Item Keys
  AppKeyId,                // uint32
  AppKeyName,              // cstring
  AppKeySubscribed,        // boolean
  AppKeyNetworkId,         // uint32
  AppKeyNetworkName,       // cstring
  AppKeyRuntime,           // uint32
  AppKeyLatestName,        // cstring
  AppKeyLatestSummary,     // cstring
  AppKeyLatestSeason,      // uint32
  AppKeyLatestNumber,      // uint32
  AppKeyLatestTimestamp,   // uint32
  AppKeyLatestRuntime,     // uint32
  AppKeyError,             // uint16
  // This key is used when we don't have anything to return, but we want to dismiss
  // a progress window which may be visible
  AppKeyDismiss,
  // Number of keys in the keys section of our appinfo.json
  NumAppKeys
} AppKeys;

typedef enum {
  RequestKeySettings = 0,
  RequestKeyUpdate,
  RequestKeySubscribe,
  RequestKeyUnsubscribe,
  RequestKeySubscriptions,
  NumRequestKeys
} RequestKeys;

// Note: On a failure, the handler will be called with success being false, when
// this is the case, the iterator will be NULL.
typedef void(*AppMessageHandler)(DictionaryIterator *iterator, bool success, void *context);

// Called to send a series of Tuples to the javascript.
// @param data An array of Tuplets to send
// @param len Length of the Tuples array
bool app_message_send(Tuplet *data, uint8_t len);

// Called to register a handler for when an app message is received.
// @param handler The handler to register
// @param callback_context The context to pass to the callback
void app_message_register_handler(AppMessageHandler handler, void *callback_context);

// Initializes the app message state and ata.
void app_message_init(void);

// De-initializes the app message state and data.
void app_message_deinit(void);
