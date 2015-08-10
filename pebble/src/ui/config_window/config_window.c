#include "config_window.h"

#include <pebble.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  TextLayer *text_layer;
} ConfigWindowData;

// Private API
////////////////////////////////////
static void prv_config_window_load(Window *window) {
  ConfigWindowData *data = window_get_user_data(window);
  Layer *window_layer = window_get_root_layer(window);
  GRect frame = layer_get_frame(window_layer);

  GRect text_frame = frame;
  text_frame.size.h /= 2;
  text_frame.origin.y += text_frame.size.h;

  TextLayer *text_layer = text_layer_create(text_frame);
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_overflow_mode(text_layer, GTextOverflowModeWordWrap);
  text_layer_set_text_color(text_layer, GColorBlack);
  text_layer_set_background_color(text_layer, GColorWhite);
  text_layer_set_text(text_layer, "Configure the app on your phone.");

  data->text_layer = text_layer;

  layer_add_child(window_layer, text_layer_get_layer(text_layer));
}

static void prv_config_window_unload(Window *window) {
  ConfigWindowData *data = window_get_user_data(window);
  if (data->text_layer) {
    text_layer_destroy(data->text_layer);
  }

  memset(data, 0, sizeof(ConfigWindowData));

  free(data);

  window_destroy(window);
}

static void prv_click_handler(ClickRecognizerRef recognizer, void *context) {
  Window *window = context;
  ConfigWindowData *data = window_get_user_data(window);
  (void)data;
}

static void prv_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, prv_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, prv_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, prv_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, prv_click_handler);
}

// Public API
////////////////////////////////////
ConfigWindow *config_window_create(void) {
  Window *window = window_create();
  ConfigWindowData *data = malloc(sizeof(ConfigWindowData));
  memset(data, 0, sizeof(ConfigWindowData));

  window_set_window_handlers(window, (WindowHandlers){
    .load = prv_config_window_load,
    .unload = prv_config_window_unload
  });

  window_set_click_config_provider_with_context(window, prv_click_config_provider, window);
  window_set_background_color(window, GColorWhite);
  window_set_user_data(window, data);

  return (ConfigWindow *)window;
}

Window *config_window_get_window(ConfigWindow *config_window) {
  return (Window *)config_window;
}

void config_window_push(ConfigWindow *config_window) {
  if (!config_window) {
    return;
  }
  const bool animated = true;
  window_stack_push(config_window_get_window(config_window), animated);
}

void config_window_pop(ConfigWindow *config_window) {
  if (!config_window) {
    return;
  }
  const bool animated = true;
  window_stack_remove(config_window_get_window(config_window), animated);
}
