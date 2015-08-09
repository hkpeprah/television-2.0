#include <pebble.h>

#include "debug/logging.h"
#include "message/message.h"
#include "ui/progress_window/progress_window.h"

#include <stdbool.h>
#include <stdint.h>

// Private API
//////////////////////////

static void prv_init(void) {
  bool has_media_id = false;
  uint32_t media_id = 0;

  switch (launch_reason()) {
    case APP_LAUNCH_TIMELINE_ACTION: {
      has_media_id = true;
      media_id = launch_get_args();
      break;
    }
    case APP_LAUNCH_SYSTEM:
    case APP_LAUNCH_PHONE:
    case APP_LAUNCH_USER:
      break;
    default:
      ERROR("NYI");
  }

  (void)media_id;
  (void)has_media_id;

  app_message_init();
}

static void prv_deinit(void) {
  app_message_deinit();
}

// App Boilerplate
//////////////////////////

int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
  return 0;
}
