#include <pebble.h>

#include "debug/logging.h"
#include "message/message.h"

static void prv_init(void) {
  app_message_init();
  switch (launch_reason()) {
    case APP_LAUNCH_TIMELINE_ACTION:
      DEBUG("Timeline NYI");
      break;
    case APP_LAUNCH_SYSTEM:
    case APP_LAUNCH_PHONE:
      // Launched by the system / configuration window.
      DEBUG("Application launched by system.");
      break;
    case APP_LAUNCH_USER:
      DEBUG("Application launched by user.");
      break;
    default:
      WARN("NYI");
  }
  Window *window = window_create();
  window_stack_push(window, false);
}

static void prv_deinit(void) {
  app_message_deinit();
}

int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
  return 0;
}
