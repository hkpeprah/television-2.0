#include "constants.h"
#include "subscription_item_window.h"
#include "subscription_menu.h"

#include "debug/logging.h"

#include <limits.h>
#include <pebble.h>
#include <stdlib.h>
#include <string.h>

struct SubscriptionItemWindow {
  Window *window;
  ScrollLayer *scroll_layer;

  Layer *detail_layer;

  const SubscriptionItem *item;
  GColor colour;

  SubscriptionItemWindowCallbacks callbacks;
  void *callback_context;
};

// Private API
///////////////////////////////
static void prv_draw_head_and_body(GContext *ctx, GRect *bounds, const char *header, const char *body) {
  const GFont header_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  const GFont body_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);

  bounds->size.h += 14;

  graphics_draw_text(ctx, header, header_font, *bounds, GTextOverflowModeFill, GTextAlignmentLeft, NULL);

  bounds->origin.y += 14;

  GSize text_size = graphics_text_layout_get_content_size(body, body_font,
    GRect(0, 0, bounds->size.w, SHRT_MAX), GTextOverflowModeWordWrap, GTextAlignmentLeft);

  bounds->size.h += text_size.h + 12;

  graphics_draw_text(ctx, body, body_font, *bounds, GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);

  bounds->origin.y += text_size.h + 12;
}

static void prv_detail_layer_update_proc(Layer *layer, GContext *ctx) {
  const SubscriptionItemWindow *item_window = *((SubscriptionItemWindow **)layer_get_data(layer));
  const SubscriptionItem *item = item_window->item;

  const uint16_t padding = 4;
  const uint16_t text_height = 10;
  const GFont title_font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  const GFont normal_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);

  GRect bounds = layer_get_bounds(layer);
  bounds.origin.x += padding;
  bounds.size.w -= (2 * padding);

  GSize text_size = graphics_text_layout_get_content_size(item->name, title_font,
      GRect(0, 0, bounds.size.w, SHRT_MAX), GTextOverflowModeWordWrap, GTextAlignmentLeft);

  const GRect name_box = GRect(bounds.origin.x, bounds.origin.y, bounds.size.w, text_size.h + 4);
  graphics_context_set_text_color(ctx, GColorBlack);

  bounds.size.h += text_size.h + 4;

  graphics_draw_text(ctx, item->name, title_font, name_box, GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);

  bounds.origin.y += text_size.h + 4;
  bounds.size.h += padding;

  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_draw_line(ctx, GPoint(bounds.origin.x, bounds.origin.y), GPoint(bounds.size.w, bounds.origin.y));

  bounds.origin.y += padding;
  bounds.size.h += text_height + 4;

  const GFont subtitle_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  graphics_draw_text(ctx, "Network", subtitle_font, bounds, GTextOverflowModeFill, GTextAlignmentLeft, NULL);

  bounds.origin.y += text_height + 4;
  bounds.size.h += text_size.h + 12;

  text_size = graphics_text_layout_get_content_size(item->network.name, normal_font,
      GRect(0, 0, bounds.size.w, SHRT_MAX), GTextOverflowModeWordWrap, GTextAlignmentLeft);

  graphics_draw_text(ctx, item->network.name, normal_font, bounds, GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);

  bounds.origin.y += text_size.h + 12;

  if (item->has_latest == false) {
    // If there is no latest episode, then we're done here and can simply display that message to the user
    // before returning.
    const char *msg = "There is no latest episode for this series.  Either we have yet to "
      "update it, the series season has ended or it is on hiatus.";
    graphics_draw_text(ctx, msg, subtitle_font, bounds, GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
    return;
  }

  if (strlen(item->latest.name) > 0) {
    prv_draw_head_and_body(ctx, &bounds, "Episode Name", item->latest.name);
  }

  if (item->latest.season) {
    char season_text[MAX_INT_LEN + 1] = {0};
    snprintf(season_text, MAX_INT_LEN, "%lu", item->latest.season);
    prv_draw_head_and_body(ctx, &bounds, "Season", season_text);
  }

  char number_text[MAX_INT_LEN + 1] = {0};
  snprintf(number_text, MAX_INT_LEN, "%lu", item->latest.number);
  prv_draw_head_and_body(ctx, &bounds, "Number", number_text);

  if (strlen(item->latest.summary) > 0) {
    prv_draw_head_and_body(ctx, &bounds, "Summary", item->latest.summary);
  }

  if (item->latest.runtime) {
    const uint16_t len = strlen(" minutes") + MAX_INT_LEN + 1;
    char runtime_text[len];
    memset(runtime_text, 0, len);
    snprintf(runtime_text, len - 1, "%lu minutes", item->latest.runtime);
    prv_draw_head_and_body(ctx, &bounds, "Runtime", runtime_text);
  }

  if (item->latest.timestamp) {
    char timestamp_text[31] = {0};
    strftime(timestamp_text, 30, "%I:%M %p - %b %d %Y", localtime(&item->latest.timestamp));
    prv_draw_head_and_body(ctx, &bounds, "Air Date", timestamp_text);
  }

  const GSize new_size = GSize(bounds.size.w + 2 * padding, bounds.origin.y);

  GRect frame = layer_get_frame(layer);
  layer_set_frame(layer, GRect(frame.origin.x, frame.origin.y, frame.size.w, new_size.h));
  layer_set_bounds(layer, (GRect){{0, 0}, new_size});

  scroll_layer_set_content_size(item_window->scroll_layer, new_size);
}

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect frame = layer_get_frame(window_layer);

  SubscriptionItemWindow *item_window = window_get_user_data(window);
  if (item_window->item->crunchyroll) {
    item_window->colour = CRUNCHYROLL_COLOUR;
  } else if (item_window->item->funimation) {
    item_window->colour = FUNIMATION_COLOUR;
  } else {
    item_window->colour = DEFAULT_COLOUR;
  }

  ScrollLayer *scroll_layer = scroll_layer_create(frame);
  item_window->scroll_layer = scroll_layer;

  const GRect detail_rect = frame;
  Layer *detail_layer = layer_create_with_data(detail_rect, sizeof(SubscriptionItemWindow **));
  SubscriptionItemWindow **detail_data = layer_get_data(detail_layer);

  *detail_data = item_window;
  item_window->detail_layer = detail_layer;

  layer_set_update_proc(detail_layer, prv_detail_layer_update_proc);

  scroll_layer_add_child(scroll_layer, detail_layer);

  scroll_layer_set_shadow_hidden(scroll_layer, true /* no shadow */);
  scroll_layer_set_click_config_onto_window(scroll_layer, item_window->window);
  scroll_layer_set_content_size(scroll_layer, GSize(frame.size.w, detail_rect.origin.y + frame.size.h));

  layer_add_child(window_layer, scroll_layer_get_layer(scroll_layer));
}

static void prv_window_unload(Window *window) {
  SubscriptionItemWindow *item_window = window_get_user_data(window);
  if (item_window->callbacks.unload) {
    item_window->callbacks.unload(item_window->item, item_window->callback_context);
  }

  if (item_window->scroll_layer) {
    scroll_layer_destroy(item_window->scroll_layer);
  }

  if (item_window->detail_layer) {
    layer_destroy(item_window->detail_layer);
  }

  memset(item_window, 0, sizeof(SubscriptionItemWindow));
  free(item_window);

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
