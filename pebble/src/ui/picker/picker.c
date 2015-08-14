#include "picker.h"

#include <pebble.h>
#include <stdlib.h>
#include <string.h>

#define PICKER_HEIGHT  (36)

typedef struct {
  const char *text;
  PickerCallback callback;
  void *callback_context;
} PickerOption;

struct Picker {
  Window *window;
  MenuLayer *menu_layer;
  PickerOption *options;
  uint8_t num_options;
};

// Private API
///////////////////////////////
static uint16_t prv_get_num_rows(MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
  Picker *picker = callback_context;
  return picker->num_options;
}

static int16_t prv_get_cell_height(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
  return PICKER_HEIGHT;
}

static void prv_draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
  const GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  const GRect bounds = layer_get_bounds(cell_layer);
  const GRect text_bounds = GRect(bounds.origin.x + 2, bounds.origin.y, bounds.size.w - 4, bounds.size.h);
  const uint8_t corner_radius = 0;
  Picker *picker = callback_context;
  PickerOption *option = &picker->options[cell_index->row];
  graphics_fill_rect(ctx, bounds, corner_radius, GCornersAll);
  graphics_draw_text(ctx, option->text, font, text_bounds, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
}

static void prv_select_click(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
  Picker *picker = callback_context;
  PickerOption *option = &picker->options[cell_index->row];
  if (option->callback) {
    option->callback(picker, cell_index->row, callback_context);
  }
}

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect frame = layer_get_frame(window_layer);

  const uint8_t padding = 10;
  const GRect picker_frame = GRect(frame.origin.x + padding, frame.origin.y + (frame.size.h - PICKER_HEIGHT) / 2 - 2,
                                   frame.size.w - 2 * padding, PICKER_HEIGHT);

  Picker *picker = window_get_user_data(window);
  MenuLayer *menu_layer = menu_layer_create(picker_frame);
  picker->menu_layer = menu_layer;

  menu_layer_pad_bottom_enable(menu_layer, false /* no pad */);
  menu_layer_set_click_config_onto_window(menu_layer, window);
  menu_layer_set_normal_colors(menu_layer, GColorVividCerulean, GColorWhite);
  menu_layer_set_highlight_colors(menu_layer, GColorVividCerulean, GColorWhite);
  menu_layer_set_callbacks(menu_layer, picker, (MenuLayerCallbacks){
    .get_num_rows = prv_get_num_rows,
    .get_cell_height = prv_get_cell_height,
    .draw_row = prv_draw_row,
    .select_click = prv_select_click
  });

  layer_add_child(window_layer, menu_layer_get_layer(menu_layer));
}

static void prv_window_unload(Window *window) {
  Picker *picker = window_get_user_data(window);

  if (picker->menu_layer) {
    menu_layer_destroy(picker->menu_layer);
  }

  if (picker->options) {
    free(picker->options);
  }

  memset(picker, 0, sizeof(Picker));
  free(picker);

  window_destroy(window);
}

// Public API
///////////////////////////////
Picker *picker_create(const char *title) {
  Picker *picker = malloc(sizeof(Picker));
  Window *window = window_create();

  window_set_background_color(window, GColorWhite);
  window_set_user_data(window, picker);
  window_set_window_handlers(window, (WindowHandlers){
    .load = prv_window_load,
    .unload = prv_window_unload
  });

  memset(picker, 0, sizeof(Picker));
  picker->window = window;

  return picker;
}

Window *picker_get_window(Picker *picker) {
  if (!picker) {
    return NULL;
  }
  return picker->window;
}

void picker_add_option(Picker *picker, const char *text, PickerCallback cb, void *callback_context) {
  if (!picker) {
    return;
  }
  uint8_t num_options = picker->num_options;
  picker->num_options++;
  picker->options = realloc(picker->options, sizeof(PickerOption) * picker->num_options);
  picker->options[num_options] = (PickerOption){
    .text = text,
    .callback = cb,
    .callback_context = callback_context
  };
}

void picker_push(Picker *picker) {
  if (!picker) {
    return;
  }
  const bool animated = true;
  window_set_user_data(picker->window, picker);
  window_stack_push(picker->window, animated);
}

void picker_destroy(Picker *picker) {
  if (!picker) {
    return;
  }
  const bool animated = true;
  window_stack_remove(picker->window, animated);
}
