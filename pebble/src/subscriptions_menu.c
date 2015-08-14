#include "subscriptions_menu.h"

#include <pebble.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  TextLayer *text_layer;
} SubscriptionsWindowData;

// Private API
////////////////////////////////////
static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect frame = layer_get_frame(window_layer);

  SubscriptionsWindowData *data = malloc(sizeof(SubscriptionsWindowData));
  memset(data, 0, sizeof(SubscriptionsWindowData));

  window_set_user_data(window, data);

  const int16_t padding = 10;
  GRect text_frame = GRect(frame.origin.x + padding, frame.origin.y, frame.size.w - 2 * padding, frame.size.h);

  TextLayer *text_layer = text_layer_create(text_frame);
  data->text_layer = text_layer;
  text_layer_set_text_color(text_layer, GColorBlack);
  text_layer_set_background_color(text_layer, GColorWhite);
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(text_layer, GTextAlignmentCenter);
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text(text_layer, "It appears you haven't subscribed to anything."
      "  Open the configuration page on your phone to start.");

  GSize content_size = text_layer_get_content_size(text_layer);
  text_frame.origin.y += (frame.size.h - content_size.h) / 2;
  layer_set_frame(text_layer_get_layer(text_layer), text_frame);

  layer_add_child(window_layer, text_layer_get_layer(text_layer));
}

static void prv_window_unload(Window *window) {
  SubscriptionsWindowData *data = window_get_user_data(window);

  if (data->text_layer) {
    text_layer_destroy(data->text_layer);
  }

  memset(data, 0, sizeof(SubscriptionsWindowData));
  free(data);

  window_destroy(window);
}

// Public API
////////////////////////////////////
void subscriptions_menu_show(void) {
  Window *window = window_create();

  window_set_background_color(window, GColorWhite);
  window_set_window_handlers(window, (WindowHandlers){
    .load = prv_window_load,
    .unload = prv_window_unload
  });

  const bool animated = true;
  window_stack_push(window, animated);
}
