#include "constants.h"
#include "subscription_item_window.h"
#include "subscription_menu.h"
#include "utils.h"

#include "debug/logging.h"

#include <limits.h>
#include <pebble.h>
#include <stdlib.h>
#include <string.h>

// Drawing Routines
////////////////////////////////////
static void prv_draw_triangle(GContext *ctx, GPoint p1, GPoint p2, GPoint p3) {
  const GPathInfo info = {
    .num_points = 3,
    .points = (GPoint []){p1, p2, p3}
  };
  GPath *path = gpath_create(&info);
  gpath_draw_filled(ctx, path);
  gpath_destroy(path);
}

static void prv_item_layer_update_proc(Layer *layer, GContext *ctx) {
  SubscriptionMenu *menu = *((SubscriptionMenu **)layer_get_data(layer));
  const SubscriptionItem *item = NULL;
  const GRect bounds = layer_get_bounds(layer);
  const uint16_t padding = 5;

  uint16_t idx;
  for (idx = 0; idx < menu->num_items; idx++) {
    if (layer == menu->item_layers[idx]) {
      item = menu->items[idx];
      break;
    }
  }

  if (!item) {
    return;
  }

  GColor colour;
  if (item->crunchyroll) {
    colour = CRUNCHYROLL_COLOUR;
  } else if (item->funimation) {
    colour = FUNIMATION_COLOUR;
  } else {
    colour = DEFAULT_COLOUR;
  }

  graphics_context_set_fill_color(ctx, colour);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  const GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_24);
  const GSize size = graphics_text_layout_get_content_size(item->name, font,
    GRect(0, 0, bounds.size.w, SHRT_MAX), GTextOverflowModeWordWrap, GTextAlignmentLeft);
  const GRect text_bounds =
    GRect(bounds.origin.x + padding, bounds.size.h / 2 - size.h - 4, bounds.size.w - 2 * padding, bounds.size.h);

  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_text(ctx, item->name, font, text_bounds, GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);

  const uint16_t stroke_width = 2;
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, stroke_width /* pixels */);
  graphics_draw_line(ctx, GPoint(text_bounds.origin.x, text_bounds.origin.y + size.h + padding),
                     GPoint(text_bounds.size.w + padding, text_bounds.origin.y + size.h + padding));

  const GFont subtitle_font = fonts_get_system_font(FONT_KEY_GOTHIC_18);
  const GRect subtitle_rect = GRect(text_bounds.origin.x, text_bounds.origin.y + size.h + padding + stroke_width,
                                    text_bounds.size.w, text_bounds.size.h);
  graphics_draw_text(ctx, item->network.name, subtitle_font, subtitle_rect, GTextOverflowModeFill, GTextAlignmentLeft, NULL);

  const uint16_t side_length = 8;
  if (idx < menu->num_items - 1) {
    const GPoint p1 = GPoint((bounds.size.w - 2 * side_length) / 2, bounds.size.h - side_length - 2 * padding);
    const GPoint p2 = GPoint((bounds.size.w + 2 * side_length) / 2, bounds.size.h - side_length - 2 * padding);
    const GPoint p3 = GPoint(bounds.size.w / 2, bounds.size.h - 2 * padding);

    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_context_set_fill_color(ctx, GColorWhite);

    prv_draw_triangle(ctx, p1, p2, p3);
  }

  if (idx) {
    const GPoint p1 = GPoint((bounds.size.w - 2 * side_length) / 2, bounds.origin.y + side_length + 2 * padding);
    const GPoint p2 = GPoint((bounds.size.w + 2 * side_length) / 2, bounds.origin.y + side_length + 2 * padding);
    const GPoint p3 = GPoint(bounds.size.w / 2, bounds.origin.y + 2 * padding);

    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_context_set_fill_color(ctx, GColorWhite);

    prv_draw_triangle(ctx, p1, p2, p3);
  }
}

// Private API
////////////////////////////////////
static void prv_handle_configure_required(SubscriptionMenu *menu) {
  Layer *window_layer = window_get_root_layer(menu->window);
  GRect frame = layer_get_frame(window_layer);

  const int16_t padding = 10;
  GRect text_frame = GRect(frame.origin.x + padding, frame.origin.y,
                           frame.size.w - 2 * padding, frame.size.h);

  TextLayer *text_layer = text_layer_create(text_frame);
  menu->text_layer = text_layer;
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

static void prv_select_click_handler(ClickRecognizerRef recognizer, void *context) {
  SubscriptionMenu *menu = context;
  GPoint offset = scroll_layer_get_content_offset(menu->scroll_layer);
  for (uint16_t idx = 0; idx < menu->num_items; idx++) {
    Layer *item_layer = menu->item_layers[idx];
    const SubscriptionItem *item = menu->items[idx];
    const GRect frame = layer_get_frame(item_layer);
    if (frame.origin.y == ABS(offset.y)) {
      SubscriptionItemWindow *item_window = subscription_item_window_create(item);
      subscription_item_window_push(item_window);
      break;
    }
  }
}

static void prv_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  SubscriptionMenu *menu = context;
  GRect bounds = layer_get_bounds(menu->item_layers[0]);
  GPoint offset = scroll_layer_get_content_offset(menu->scroll_layer);
  offset.y += bounds.size.h;

  const bool animated = true;
  scroll_layer_set_content_offset(menu->scroll_layer, offset, animated);
}

static void prv_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  SubscriptionMenu *menu = context;
  GRect bounds = layer_get_bounds(menu->item_layers[0]);
  GPoint offset = scroll_layer_get_content_offset(menu->scroll_layer);
  offset.y -= bounds.size.h;

  const bool animated = true;
  scroll_layer_set_content_offset(menu->scroll_layer, offset, animated);
}

static void prv_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, prv_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, prv_down_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, prv_select_click_handler);
}

static void prv_add_scroll_layer(SubscriptionMenu *menu) {
  Layer *window_layer = window_get_root_layer(menu->window);
  GRect frame = layer_get_frame(window_layer);
  ScrollLayer *scroll_layer = scroll_layer_create(frame);

  menu->scroll_layer = scroll_layer;

  scroll_layer_set_shadow_hidden(scroll_layer, true /* no shadow */);

  layer_add_child(window_layer, scroll_layer_get_layer(scroll_layer));
  window_set_click_config_provider_with_context(menu->window, prv_click_config_provider, menu);
}

static void prv_window_load(Window *window) {
  SubscriptionMenu *menu = window_get_user_data(window);
  if (menu->num_items == 0) {
    prv_handle_configure_required(menu);
  } else {
    prv_add_scroll_layer(menu);
  }
}

static void prv_window_unload(Window *window) {
  SubscriptionMenu *menu = window_get_user_data(window);
  subscription_menu_destroy(menu);
}

// Public API
////////////////////////////////////
SubscriptionMenu *subscription_menu_create(void) {
  SubscriptionMenu *menu = malloc(sizeof(SubscriptionMenu));
  memset(menu, 0, sizeof(SubscriptionMenu));

  Window *window = window_create();
  menu->window = window;
  window_set_background_color(window, GColorWhite);
  window_set_window_handlers(window, (WindowHandlers){
    .load = prv_window_load,
    .unload = prv_window_unload
  });

  window_set_user_data(window, menu);

  return menu;
}

void subscription_menu_add_item(SubscriptionMenu *menu, SubscriptionItem *item) {
  // Sanity Check
  // We only add valid items, that is items with the attributes we need to display them
  if (!(menu && item && item->name)) {
    if (item) {
      subscription_item_destroy(item);
    }
    return;
  }

  if (strcmp(item->network.name, "Funimation") == 0) {
    item->funimation = true;
  } else if (strcmp(item->network.name, "Crunchyroll") == 0) {
    item->crunchyroll = true;
  }

  uint16_t num_items = menu->num_items;
  menu->num_items++;
  menu->items = realloc(menu->items, sizeof(SubscriptionItem *) * menu->num_items);
  menu->items[num_items] = item;

  if (menu->text_layer) {
    text_layer_destroy(menu->text_layer);
    menu->text_layer = NULL;
  }

  if (menu->scroll_layer == NULL) {
    prv_add_scroll_layer(menu);
  }

  const uint16_t height = layer_get_bounds(window_get_root_layer(menu->window)).size.h;
  const GRect frame = layer_get_frame(scroll_layer_get_layer(menu->scroll_layer));
  const GRect item_frame = GRect(frame.origin.x, height * num_items, frame.size.w, height);

  Layer *item_layer = layer_create_with_data(item_frame, sizeof(const SubscriptionMenu **));
  menu->item_layers = realloc(menu->item_layers, sizeof(item_layer) * menu->num_items);
  menu->item_layers[num_items] = item_layer;

  const SubscriptionMenu **data = layer_get_data(item_layer);
  *data = menu;

  scroll_layer_add_child(menu->scroll_layer, item_layer);
  scroll_layer_set_content_size(menu->scroll_layer, GSize(frame.size.w, height * menu->num_items));

  layer_set_update_proc(item_layer, prv_item_layer_update_proc);

  if (num_items) {
    layer_mark_dirty(menu->item_layers[num_items - 1]);
  }
  layer_mark_dirty(item_layer);
}

void subscription_menu_push(SubscriptionMenu *menu) {
  if (!menu) {
    return;
  }
  const bool animated = true;
  window_set_user_data(menu->window, menu);
  window_stack_push(menu->window, animated);
}

void subscription_menu_pop(SubscriptionMenu *menu) {
  if (!menu) {
    return;
  }
  const bool animated = true;
  window_stack_remove(menu->window, animated);
}

void subscription_menu_destroy(SubscriptionMenu *menu) {
  if (!menu) {
    return;
  }

  if (menu->items) {
    for (uint16_t idx = 0; idx < menu->num_items; idx++) {
      subscription_item_destroy(menu->items[idx]);
    }
    free(menu->items);
  }

  if (menu->text_layer) {
    text_layer_destroy(menu->text_layer);
  }

  if (menu->scroll_layer) {
    scroll_layer_destroy(menu->scroll_layer);
  }

  if (menu->window) {
    window_destroy(menu->window);
  }

  if (menu->item_layers) {
    for (uint16_t idx = 0; idx < menu->num_items; idx++) {
      layer_destroy(menu->item_layers[idx]);
    }
    free(menu->item_layers);
  }

  memset(menu, 0, sizeof(SubscriptionMenu));
  free(menu);
}

void subscription_item_destroy(SubscriptionItem *item) {
  if (!item) {
    return;
  }

  memset(item, 0, sizeof(SubscriptionItem));
  free(item);
}
