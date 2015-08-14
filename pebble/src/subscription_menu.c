#include "subscription_menu.h"

#include <pebble.h>
#include <stdlib.h>
#include <string.h>

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

static void prv_window_load(Window *window) {
  SubscriptionMenu *menu = window_get_user_data(window);
  if (menu->num_items == 0) {
    prv_handle_configure_required(menu);
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
  if (!(menu && item)) {
    if (item) {
      subscription_item_destroy(item);
    }
    return;
  }
  uint16_t num_items = menu->num_items;
  menu->num_items++;
  menu->items = realloc(menu->items, sizeof(SubscriptionItem *) * menu->num_items);
  menu->items[num_items] = item;
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

  if (menu->window) {
    window_destroy(menu->window);
  }

  memset(menu, 0, sizeof(SubscriptionMenu));
  free(menu);
}

void subscription_item_destroy(SubscriptionItem *item) {
  if (!item) {
    return;
  }
  if (item->latest.name) {
    free(item->latest.name);
  }
  if (item->latest.summary) {
    free(item->latest.summary);
  }
  if (item->network.name) {
    free(item->network.name);
  }
  if (item->name) {
    free(item->name);
  }
  memset(item, 0, sizeof(SubscriptionItem));
  free(item);
}
