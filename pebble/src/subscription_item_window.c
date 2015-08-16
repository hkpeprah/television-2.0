#include "subscription_item_window.h"
#include "subscription_menu.h"

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
static void prv_detail_layer_update_proc(Layer *layer, GContext *ctx) {
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
  if (item_window) {
    if (item_window->callbacks.unload) {
      item_window->callbacks.unload(item_window->item, item_window->callback_context);
    }

    if (item_window->scroll_layer) {
      scroll_layer_destroy(item_window->scroll_layer);
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
