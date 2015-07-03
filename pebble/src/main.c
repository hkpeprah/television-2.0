#include <pebble.h>
#include "message/message.h"
#include "debug/logging.h"

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
