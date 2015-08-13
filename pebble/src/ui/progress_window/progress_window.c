#include "progress_window.h"

#include "../../debug/logging.h"

#include <pebble.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  Layer *progress_layer;

  const char *text;
  TextLayer *text_layer;

  AppTimer *progress_timer;

  uint8_t current_circle;
} ProgressWindowData;

// Private API
/////////////////////////////////
static void prv_progress_layer_timer_cb(void *data) {
  layer_mark_dirty((Layer *)data);
}

static void prv_progress_layer_update_proc(Layer *layer, GContext *ctx) {
  ProgressWindowData *data = *((ProgressWindowData **)layer_get_data(layer));
  GRect box = layer_get_bounds(layer);

  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, box, 0, GCornerNone);

  // We start in the center top to draw the animation and iterate
  // clockwise to draw the circles.
  const int16_t radius = 6;
  const int16_t num_circles = 5;

  const GRect text_box = layer_get_frame(text_layer_get_layer(data->text_layer));
  const int16_t text_padding = 12;

  const int16_t x_offset = box.origin.x + box.size.w / num_circles;
  const int16_t y_offset = text_box.size.h + text_box.origin.y + text_padding;

  GPoint p = (GPoint){0, y_offset};
  for (int8_t circle_idx = 0; circle_idx < num_circles; circle_idx++) {
    graphics_context_set_stroke_color(ctx, GColorBlack);
    if (circle_idx == data->current_circle) {
      graphics_context_set_fill_color(ctx, GColorBlack);
    } else {
      graphics_context_set_fill_color(ctx, GColorLightGray);
    }
    p.x = x_offset * circle_idx + x_offset / 2 + 1;
    graphics_fill_circle(ctx, p, radius);
    graphics_draw_circle(ctx, p, radius);
  }

  data->current_circle += 1;
  data->current_circle %= num_circles;

  const uint16_t timeout_ms = 400;
  app_timer_cancel(data->progress_timer);
  data->progress_timer = app_timer_register(timeout_ms, prv_progress_layer_timer_cb, data->progress_layer);
}

static void prv_progress_window_load(Window *window) {
  ProgressWindowData *data = window_get_user_data(window);
  Layer *window_layer = window_get_root_layer(window);
  GRect frame = layer_get_frame(window_layer);

  Layer *progress_layer = layer_create_with_data(frame, sizeof(ProgressWindowData **));
  ProgressWindowData **layer_data = layer_get_data(progress_layer);

  GRect text_frame = frame;
  const int16_t font_height = 28;
  text_frame.origin.y += (text_frame.size.h / 2) - font_height;
  text_frame.size.h = font_height;

  TextLayer *text_layer = text_layer_create(text_frame);
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  text_layer_set_text_color(text_layer, GColorBlack);
  text_layer_set_background_color(text_layer, GColorClear);
  text_layer_set_text(text_layer, data->text);
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));

  data->progress_layer = progress_layer;
  data->text_layer = text_layer;
  *layer_data = data;

  layer_set_update_proc(progress_layer, prv_progress_layer_update_proc);

  layer_add_child(progress_layer, text_layer_get_layer(text_layer));
  layer_add_child(window_layer, progress_layer);

  layer_mark_dirty(progress_layer);
}

static void prv_progress_window_unload(Window *window) {
  ProgressWindowData *data = window_get_user_data(window);

  if (data->progress_layer) {
    layer_destroy(data->progress_layer);
  }

  if (data->text_layer) {
    text_layer_destroy(data->text_layer);
  }

  app_timer_cancel(data->progress_timer);

  memset(data, 0, sizeof(ProgressWindowData));

  free(data);

  window_destroy(window);
}

static void prv_null_click_handler(ClickRecognizerRef recognizer, void *context) {
  return;
}

static void prv_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_BACK, prv_null_click_handler);
}

// Public API
/////////////////////////////////
ProgressWindow *progress_window_create(void) {
  Window *window = window_create();
  window_set_window_handlers(window, (WindowHandlers){
    .unload = prv_progress_window_unload,
    .load = prv_progress_window_load
  });

  ProgressWindowData *data = malloc(sizeof(ProgressWindowData));
  memset(data, 0, sizeof(ProgressWindowData));

  data->text = "Loading";

  window_set_click_config_provider_with_context(window, prv_click_config_provider, window);
  window_set_background_color(window, GColorWhite);
  window_set_user_data(window, data);

  return (ProgressWindow *)window;
}

Window *progress_window_get_window(ProgressWindow *progress_window) {
  return (Window *)progress_window;
}

void progress_window_push(ProgressWindow *progress_window) {
  Window *window = progress_window_get_window(progress_window);
  const bool animated = true;
  window_stack_push(window, animated);
}

void progress_window_pop(ProgressWindow *progress_window) {
  Window *window = progress_window_get_window(progress_window);
  const bool animated = true;
  window_stack_remove(window, animated);
}

void progress_window_set_text(ProgressWindow *progress_window, const char *text) {
  if (!progress_window) {
    return;
  }

  Window *window = progress_window_get_window(progress_window);
  ProgressWindowData *data = window_get_user_data(window);
  data->text = text;
}
