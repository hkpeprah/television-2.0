#include "subscription_item_window.h"
#include "subscription_menu.h"

#include <limits.h>
#include <pebble.h>
#include <stdlib.h>
#include <string.h>

struct SubscriptionItemWindow {
  Window *window;
  ScrollLayer *scroll_layer;

  Layer *title_layer;
  Layer *detail_layer;

  const SubscriptionItem *item;
  GColor colour;

  SubscriptionItemWindowCallbacks callbacks;
  void *callback_context;
};

// Private API
///////////////////////////////
static void prv_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  SubscriptionItemWindow *item_window = context;
  GRect bounds = layer_get_bounds(item_window->title_layer);
  GPoint offset = scroll_layer_get_content_offset(item_window->scroll_layer);
  offset.y += bounds.size.h;

  const bool animated = true;
  scroll_layer_set_content_offset(item_window->scroll_layer, offset, animated);
}

static void prv_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  SubscriptionItemWindow *item_window = context;
  GRect bounds = layer_get_bounds(item_window->title_layer);
  GPoint offset = scroll_layer_get_content_offset(item_window->scroll_layer);
  offset.y -= bounds.size.h;

  const bool animated = true;
  scroll_layer_set_content_offset(item_window->scroll_layer, offset, animated);
}

static void prv_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, prv_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, prv_down_click_handler);
}

static void prv_draw_triangle(GContext *ctx, GPoint p1, GPoint p2, GPoint p3) {
  const GPathInfo info = {
    .num_points = 3,
    .points = (GPoint []){p1, p2, p3}
  };
  GPath *path = gpath_create(&info);
  gpath_draw_filled(ctx, path);
  gpath_destroy(path);
}

static void prv_detail_layer_update_proc(Layer *layer, GContext *ctx) {
}

static void prv_title_layer_update_proc(Layer *layer, GContext *ctx) {
  SubscriptionItemWindow *item_window = *((SubscriptionItemWindow **)layer_get_data(layer));
  const SubscriptionItem *item = item_window->item;
  const GRect bounds = layer_get_bounds(layer);
  const uint16_t padding = 5;

  graphics_context_set_fill_color(ctx, item_window->colour);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  const GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_24);
  const GSize size = graphics_text_layout_get_content_size(item->name, font,
      GRect(0, 0, bounds.size.w, SHRT_MAX), GTextOverflowModeWordWrap, GTextAlignmentLeft);
  const GRect text_bounds = GRect(bounds.origin.x + padding, bounds.size.h / 2 - size.h,
                                  bounds.size.w - 2 * padding, bounds.size.h);

  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_text(ctx, item->name, font, text_bounds, GTextOverflowModeFill, GTextAlignmentRight, NULL);

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
  const GPoint p1 = GPoint((bounds.size.w - 2 * side_length) / 2, bounds.size.h - side_length - 2 * padding);
  const GPoint p2 = GPoint((bounds.size.w + 2 * side_length) / 2, bounds.size.h - side_length - 2 * padding);
  const GPoint p3 = GPoint(bounds.size.w / 2, bounds.size.h - 2 * padding);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_fill_color(ctx, GColorWhite);
  prv_draw_triangle(ctx, p1, p2, p3);
}

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect frame = layer_get_frame(window_layer);

  SubscriptionItemWindow *item_window = window_get_user_data(window);
  if (item_window->item->crunchyroll) {
    item_window->colour = GColorRajah;
  } else if (item_window->item->funimation) {
    item_window->colour = GColorFolly;
  } else {
    item_window->colour = GColorVividCerulean;
  }

  ScrollLayer *scroll_layer = scroll_layer_create(frame);
  item_window->scroll_layer = scroll_layer;

  Layer *title_layer = layer_create_with_data(frame, sizeof(SubscriptionItemWindow **));
  SubscriptionItemWindow **title_data = layer_get_data(title_layer);

  *title_data = item_window;
  item_window->title_layer = title_layer;

  layer_set_update_proc(title_layer, prv_title_layer_update_proc);
  scroll_layer_add_child(scroll_layer, title_layer);

  const GRect detail_rect = GRect(frame.origin.x, frame.origin.y + frame.size.h, frame.size.w, frame.size.h);
  Layer *detail_layer = layer_create_with_data(detail_rect, sizeof(SubscriptionItemWindow **));
  SubscriptionItemWindow **detail_data = layer_get_data(detail_layer);

  *detail_data = item_window;
  item_window->detail_layer = detail_layer;

  layer_set_update_proc(detail_layer, prv_detail_layer_update_proc);
  scroll_layer_add_child(scroll_layer, detail_layer);

  scroll_layer_set_shadow_hidden(scroll_layer, true /* no shadow */);
  scroll_layer_set_content_size(scroll_layer, GSize(frame.origin.x, detail_rect.origin.y + frame.size.h));

  layer_add_child(window_layer, scroll_layer_get_layer(scroll_layer));

  window_set_click_config_provider_with_context(window, prv_click_config_provider, item_window);
}

static void prv_window_unload(Window *window) {
  SubscriptionItemWindow *item_window = window_get_user_data(window);
  if (item_window) {
    if (item_window->callbacks.unload) {
      item_window->callbacks.unload(item_window->item, item_window->callback_context);
    }

    if (item_window->scroll_layer) {
      scroll_layer_destroy(item_window->scroll_layer);
    }

    if (item_window->title_layer) {
      layer_destroy(item_window->title_layer);
    }

    if (item_window->detail_layer) {
      layer_destroy(item_window->detail_layer);
    }

    free(item_window);
  }
  window_destroy(window);
}

// Public API
///////////////////////////////
SubscriptionItemWindow *subscription_item_window_create(const SubscriptionItem *item) {
  SubscriptionItemWindow *item_window = malloc(sizeof(SubscriptionItemWindow));
  memset(item_window, 0, sizeof(SubscriptionItemWindow));

  Window *window = window_create();
  item_window->window = window;

  window_set_background_color(window, GColorWhite);
  window_set_window_handlers(window, (WindowHandlers){
    .load = prv_window_load,
    .unload = prv_window_unload
  });
  window_set_user_data(window, item_window);

  item_window->item = item;

  return item_window;
}

Window *subscription_item_window_get_window(SubscriptionItemWindow *item_window) {
  if (!item_window) {
    return NULL;
  }
  return item_window->window;
}

void subscription_item_window_set_callbacks(SubscriptionItemWindow *item_window,
                                            SubscriptionItemWindowCallbacks callbacks,
                                            void *callback_context) {
  if (!item_window) {
    return;
  }
  item_window->callbacks = callbacks;
  item_window->callback_context = callback_context;
}

void subscription_item_window_push(SubscriptionItemWindow *item_window) {
  if (!item_window) {
    return;
  }
  const bool animated = true;
  window_set_user_data(item_window->window, item_window);
  window_stack_push(item_window->window, animated);
}

void subscription_item_window_pop(SubscriptionItemWindow *item_window) {
  if (!item_window) {
    return;
  }
  const bool animated = true;
  window_stack_remove(item_window->window, animated);
}
