#include "settings_menu.h"
#include "subscription_menu.h"
#include "utils.h"

#include "debug/logging.h"
#include "message/message.h"
#include "ui/config_window/config_window.h"
#include "ui/progress_window/progress_window.h"

#include <pebble.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef enum {
  MainMenuSubscriptions = 0,
  MainMenuSettings,
  NumMainMenuItems
} MainMenuIems;

typedef struct {
  MenuLayer *menu_layer;
  GBitmap **menu_icons;
  GBitmap **menu_icons_inverted;

  ProgressWindow *progress_window;
  ConfigWindow *config_window;

  SubscriptionMenu *subscription_menu;
} WindowData;

static bool s_error_window_present = false;

// Error Window
//////////////////////////
static void prv_error_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect frame = layer_get_frame(window_layer);

  const int16_t padding = 10;
  GRect text_frame = GRect(frame.origin.x + padding, frame.origin.y,
                           frame.size.w - 2 * padding, frame.size.h);

  TextLayer *text_layer = text_layer_create(text_frame);
  window_set_user_data(window, text_layer);
  text_layer_set_text_color(text_layer, GColorBlack);
  text_layer_set_background_color(text_layer, GColorWhite);
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(text_layer, GTextAlignmentCenter);
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text(text_layer, "Something went wrong!  Double check your internet"
      " connection as well as your Bluetooth connection to your phone.");

  GSize content_size = text_layer_get_content_size(text_layer);
  text_frame.origin.y += (frame.size.h - content_size.h) / 2;
  layer_set_frame(text_layer_get_layer(text_layer), text_frame);

  layer_add_child(window_layer, text_layer_get_layer(text_layer));

  s_error_window_present = true;
}

static void prv_error_window_unload(Window *window) {
  TextLayer *text_layer = window_get_user_data(window);
  if (text_layer) {
    text_layer_destroy(text_layer);
  }
  s_error_window_present = false;
  window_destroy(window);
}

static void prv_show_error_window(void) {
  Window *window = window_create();
  window_set_background_color(window, GColorWhite);
  window_set_window_handlers(window, (WindowHandlers){
    .load = prv_error_window_load,
    .unload = prv_error_window_unload
  });

  const bool animated = true;
  window_stack_push(window, animated);
}

// Main Menu
//////////////////////////
static uint16_t prv_menu_layer_get_num_rows_cb(MenuLayer *menu_layer, uint16_t section_index,
                                               void *callback_context) {
  return NumMainMenuItems;
}

static int16_t prv_menu_layer_get_cell_height_cb(MenuLayer *menu_layer, MenuIndex *cell_index,
                                                 void *callback_context) {
  Layer *layer = menu_layer_get_layer(menu_layer);
  GRect bounds = layer_get_bounds(layer);
  return bounds.size.h / NumMainMenuItems;
}

static void prv_menu_layer_select_click_cb(MenuLayer *menu_layer, MenuIndex *cell_index,
                                           void *callback_context) {
  WindowData *data = callback_context;
  switch (cell_index->row) {
    case MainMenuSubscriptions: {
      Tuplet tuplets[] = { TupletInteger(AppKeyRequest, RequestKeySubscriptions) };
      if (app_message_send(tuplets, ARRAY_LENGTH(tuplets))) {
        // Show the progress window while we fetch the information
        data->progress_window = progress_window_create();
        data->subscription_menu = subscription_menu_create();
        subscription_menu_push(data->subscription_menu);
        progress_window_push(data->progress_window);
      }
      break;
    }
    case MainMenuSettings: {
      Tuplet tuplets[] = { TupletInteger(AppKeyRequest, RequestKeySettings) };
      if (app_message_send(tuplets, ARRAY_LENGTH(tuplets))) {
        // Show the progress window while we fetch the settings information
        data->progress_window = progress_window_create();
        progress_window_push(data->progress_window);
      }
      break;
    }
    default:
      ERROR("NYI");
  }
}

static void prv_menu_layer_draw_row_cb(GContext *ctx, const Layer *cell_layer,
                                       MenuIndex *cell_index, void *callback_context) {
  GRect bounds = layer_get_bounds(cell_layer);
  GRect box = bounds;
  char *text = NULL;

  switch (cell_index->row) {
    case MainMenuSubscriptions:
      text = "My Series";
      break;
    case MainMenuSettings:
      text = "Settings";
      break;
    default:
      ERROR("NYI");
      return;
  }

  WindowData *data = callback_context;
  GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  int16_t font_height = 32;

  const int16_t offset_x = 12;
  const int16_t bmp_width = 34;
  const int16_t bmp_height = 30;
  GBitmap *bmp = NULL;

  MenuIndex selected_index = menu_layer_get_selected_index(data->menu_layer);
  if (selected_index.row == cell_index->row) {
    bmp = data->menu_icons_inverted[cell_index->row];
  } else {
    bmp = data->menu_icons[cell_index->row];
  }

  box.origin.x += offset_x;
  box.origin.y = box.origin.y + (box.size.h - font_height) / 2;
  box.size.w = bmp_width;
  box.size.h = bmp_height;

  graphics_context_set_compositing_mode(ctx, GCompOpSet);
  graphics_draw_bitmap_in_rect(ctx, bmp, box);

  box.origin.x += bmp_width + 5;
  box.size.w = bounds.size.w - box.origin.x;
  box.size.h = font_height;

  graphics_draw_text(ctx, (const char *)text, font, box, GTextOverflowModeFill,
      GTextAlignmentLeft, NULL);
}

static void prv_window_load(Window *window) {
  WindowData *data = window_get_user_data(window);
  Layer *window_layer = window_get_root_layer(window);
  GRect frame = layer_get_frame(window_layer);

  uint16_t resource_ids[] = {
    RESOURCE_ID_SUBSCRIPTIONS_MENU_ICON,
    RESOURCE_ID_SETTINGS_MENU_ICON
  };

  uint16_t resource_ids_inverted[] = {
    RESOURCE_ID_SUBSCRIPTIONS_MENU_ICON_INVERTED,
    RESOURCE_ID_SETTINGS_MENU_ICON_INVERTED
  };

  data->menu_icons = malloc(sizeof(GBitmap *) * NumMainMenuItems);
  data->menu_icons_inverted = malloc(sizeof(GBitmap *) * NumMainMenuItems);

  for (uint8_t idx = 0; idx < NumMainMenuItems; idx++) {
    data->menu_icons[idx] = gbitmap_create_with_resource(resource_ids[idx]);
    data->menu_icons_inverted[idx] = gbitmap_create_with_resource(resource_ids_inverted[idx]);
  }

  MenuLayer *menu_layer = menu_layer_create(frame);
  data->menu_layer = menu_layer;

  menu_layer_set_normal_colors(menu_layer, GColorWhite, GColorBlack);
  menu_layer_set_highlight_colors(menu_layer, GColorVividCerulean, GColorWhite);
  menu_layer_pad_bottom_enable(menu_layer, false /* disabled */);
  menu_layer_set_callbacks(menu_layer, data, (MenuLayerCallbacks){
    .get_num_rows = prv_menu_layer_get_num_rows_cb,
    .get_cell_height = prv_menu_layer_get_cell_height_cb,
    .select_click = prv_menu_layer_select_click_cb,
    .draw_row = prv_menu_layer_draw_row_cb
  });
  menu_layer_set_click_config_onto_window(menu_layer, window);

  layer_add_child(window_layer, menu_layer_get_layer(menu_layer));
}

static void prv_window_unload(Window *window) {
  WindowData *data = window_get_user_data(window);

  if (data->menu_layer) {
    menu_layer_destroy(data->menu_layer);
  }

  if (data->menu_icons) {
    for (uint8_t idx = 0; idx < NumMainMenuItems; idx++) {
      gbitmap_destroy(data->menu_icons[idx]);
    }
    free(data->menu_icons);
  }

  if (data->menu_icons_inverted) {
    for (uint8_t idx = 0; idx < NumMainMenuItems; idx++) {
      gbitmap_destroy(data->menu_icons_inverted[idx]);
    }
    free(data->menu_icons_inverted);
  }

  progress_window_pop(data->progress_window);

  memset(data, 0, sizeof(WindowData));

  free(data);

  window_destroy(window);
}

// Private API
//////////////////////////
static void prv_handle_app_message(DictionaryIterator *iter, bool success, void *context) {
  if (!success || s_error_window_present) {
    return;
  }

  WindowData *data = context;
  Tuple *t = dict_read_first(iter);
  if (t == NULL) {
    return;
  }

  SubscriptionItem *item = malloc(sizeof(SubscriptionItem));
  memset(item, 0, sizeof(SubscriptionItem));

  bool should_show_settings_menu = false;
  SettingsMenuData *settings_data = malloc(sizeof(SettingsMenuData));
  memset(settings_data, 0, sizeof(SettingsMenuData));

  while (t != NULL) {
    switch (t->key) {
      case AppKeyConfig: {
        bool show = (bool)t->value->uint8;
        if (show && !data->config_window) {
          // If we get a message telling us to show the config window, and we haven't
          // yet showed it, then push it onto the window stack.
          data->config_window = config_window_create();
          config_window_push(data->config_window);
          if (data->subscription_menu) {
            subscription_menu_pop(data->subscription_menu);
            data->subscription_menu = NULL;
          }
        } else if (!show && data->config_window) {
          // If we have a message telling us to hide the config window, and we haven't
          // yet hidden it, then pop it from the window stack.
          config_window_pop(data->config_window);
          data->config_window = NULL;
        }
        break;
      }
      case AppKeyCrunchyrollStatus: {
        should_show_settings_menu = true;
        settings_data->crunchyroll.premium = (bool)t->value->uint8;
        break;
      }
      case AppKeyFunimationStatus: {
        should_show_settings_menu = true;
        settings_data->funimation.premium = (bool)t->value->uint8;
        break;
      }
      case AppKeyCountryCode: {
        should_show_settings_menu = true;
        strcpy(settings_data->country, t->value->cstring);
        break;
      }
      case AppKeySubscriptionCount: {
        should_show_settings_menu = true;
        settings_data->num_subscriptions = t->value->uint32;
        break;
      }
      case AppKeyId: {
        item->id = t->value->uint32;
        break;
      }
      case AppKeyName: {
        uint16_t len = strlen(t->value->cstring);
        strncpy(item->name, t->value->cstring, MIN(len, MAX_NAME_LEN));
        break;
      }
      case AppKeySubscribed: {
        item->subscribed = (bool)t->value->uint8;
        break;
      }
      case AppKeyNetworkId: {
        item->network.id = t->value->uint32;
        break;
      }
      case AppKeyNetworkName: {
        uint16_t len = strlen(t->value->cstring);
        strncpy(item->network.name, t->value->cstring, MIN(len, MAX_NAME_LEN));
        break;
      }
      case AppKeyRuntime: {
        item->runtime = t->value->uint32;
        break;
      }
      case AppKeyLatestName: {
        uint16_t len = strlen(t->value->cstring);
        item->has_latest = true;
        strncpy(item->latest.name, t->value->cstring, MIN(len, MAX_NAME_LEN));
        break;
      }
      case AppKeyLatestSummary: {
        item->has_latest = true;
        strncpy(item->latest.summary, t->value->cstring, MIN(strlen(t->value->cstring), MAX_SUMMARY_LEN));
        break;
      }
      case AppKeyLatestSeason: {
        item->has_latest = true;
        item->latest.season = t->value->uint32;
        break;
      }
      case AppKeyLatestNumber: {
        item->has_latest = true;
        item->latest.number = t->value->uint32;
        break;
      }
      case AppKeyLatestTimestamp: {
        item->has_latest = true;
        item->latest.timestamp = t->value->uint32;
        break;
      }
      case AppKeyLatestRuntime: {
        item->has_latest = true;
        item->latest.runtime = t->value->uint32;
        break;
      }
      case AppKeyError: {
        prv_show_error_window();
        subscription_menu_pop(data->subscription_menu);
        data->subscription_menu = NULL;
        break;
      }
      case AppKeyDismiss: {
        break;
      }
      default:
        ERROR("NYI");
        return;
    }
    t = dict_read_next(iter);
  }

  if (should_show_settings_menu) {
    if (data->subscription_menu) {
      subscription_menu_pop(data->subscription_menu);
    }
    data->subscription_menu = NULL;
    show_settings_menu(settings_data);
  }

  // We only free the item if we shouldn't show the SubscriptionItem as
  // the contract specifies that we will free the memory in the handler ourselves.
  subscription_menu_add_item(data->subscription_menu, item);

  // On communication we want to remove the progress window
  progress_window_pop(data->progress_window);
  data->progress_window = NULL;

  free(settings_data);
}

static void prv_handle_app_start(uint32_t media_id, bool open_media) {
  Window *window = window_create();
  window_set_background_color(window, GColorWhite);
  window_set_window_handlers(window, (WindowHandlers){
    .unload = prv_window_unload,
    .load = prv_window_load
  });

  WindowData *data = malloc(sizeof(WindowData));
  memset(data, 0, sizeof(WindowData));

  window_set_user_data(window, data);

  app_message_init();
  app_message_register_handler(prv_handle_app_message, data);

  const bool animated = false;
  window_stack_push(window, animated);
}

static void prv_init(void) {
  bool has_media_id = false;
  uint32_t media_id = 0;

  switch (launch_reason()) {
    case APP_LAUNCH_TIMELINE_ACTION: {
      has_media_id = true;
      media_id = launch_get_args();
      break;
    }
    case APP_LAUNCH_SYSTEM:
    case APP_LAUNCH_PHONE:
    case APP_LAUNCH_USER:
      break;
    default:
      ERROR("NYI");
  }

  prv_handle_app_start(media_id, has_media_id);
}

static void prv_deinit(void) {
  app_message_deinit();
}

// App Boilerplate
//////////////////////////
int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
  return 0;
}
