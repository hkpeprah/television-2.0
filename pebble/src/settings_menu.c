#include "settings_menu.h"

#include "debug/logging.h"
#include "message/message.h"
#include "ui/picker/picker.h"
#include "ui/swap_menu/swap_menu.h"

#include <pebble.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static SwapMenu *s_settings_menu = NULL;

static const char *COUNTRY_CODES[] = {
  "US",
  "CA",
  "GB",
  "AU",
  "CN",
  "FR",
  "DE",
  "JP",
  "KP",
  "KR",
  "MX"
};

static const char *COUNTRY_NAMES[] = {
  "United States",
  "Canada",
  "Great Britain",
  "Australia",
  "China",
  "France",
  "Germany",
  "Japan",
  "Democratic People's Republic of Korea",
  "Republic of Korea",
  "Mexico"
};

static void prv_change_funimation_status(SwapMenuItem *item, void *callback_context) {
  bool premium = !(bool)callback_context;
  Tuplet tuplets[] = {
    TupletInteger(AppKeyRequest, RequestKeyUpdate),
    TupletInteger(AppKeyFunimationStatus, premium)
  };
  if (app_message_send(tuplets, ARRAY_LENGTH(tuplets))) {
    item->subtitle = premium ? "Subscription" : "Free";
    item->callback_context = (void *)premium;
  }
}

static void prv_change_crunchyroll_status(SwapMenuItem *item, void *callback_context) {
  bool premium = !(bool)callback_context;
  Tuplet tuplets[] = {
    TupletInteger(AppKeyRequest, RequestKeyUpdate),
    TupletInteger(AppKeyCrunchyrollStatus, premium)
  };
  if (app_message_send(tuplets, ARRAY_LENGTH(tuplets))) {
    item->subtitle = premium ? "Premium" : "Free";
    item->callback_context = (void *)premium;
  }
}

static void prv_country_picker_callback(Picker *picker, int idx, void *callback_context) {
  Tuplet tuplets[] = {
    TupletInteger(AppKeyRequest, RequestKeyUpdate),
    TupletCString(AppKeyCountryCode, COUNTRY_CODES[idx]),
  };

  // Remove the Picker from the Window Stack
  picker_destroy(picker);

  if (app_message_send(tuplets, ARRAY_LENGTH(tuplets))) {
    const char *country_name = COUNTRY_NAMES[idx];
    SwapMenuItem *item = callback_context;
    item->subtitle = country_name;
    item->callback_context = (void *)idx;
  }
  swap_menu_reload(s_settings_menu);
}

static void prv_change_country(SwapMenuItem *item, void *callback_context) {
  int selected_index = (int)callback_context;
  Picker *picker = picker_create("Choose Country:");
  for (uint8_t idx = 0; idx < ARRAY_LENGTH(COUNTRY_NAMES); idx++) {
    picker_add_option(picker, COUNTRY_NAMES[idx], prv_country_picker_callback, item);
  }
  picker_push(picker);
  picker_set_selected_index(picker, selected_index);
}

void show_settings_menu(SettingsMenuData *data) {
  const char *titles[] = {"Crunchyroll", "Funimation", "User Settings"};
  uint8_t num_sections = ARRAY_LENGTH(titles);

  SwapMenu *menu = swap_menu_create();
  SwapMenuSection *sections[num_sections];

  for (uint8_t idx = 0; idx < num_sections; idx++) {
    sections[idx] = swap_menu_section_create(titles[idx]);
    swap_menu_add_section(menu, sections[idx]);
  }

  // Add the items for the Crunchyroll section
  swap_menu_section_add_item(sections[0], "Status", data->crunchyroll.premium ? "Premium" : "Free",
                             prv_change_crunchyroll_status, (void *)data->crunchyroll.premium);

  // Add the items for the Funimation section
  swap_menu_section_add_item(sections[1], "Subscription", data->funimation.premium ? "Subscription" : "Free",
                             prv_change_funimation_status, (void *)data->funimation.premium);

  // Add the items for the user settings section
  static const char *country_name = "United States";
  int selected_index = 0;
  for (uint8_t idx = 0; idx < ARRAY_LENGTH(COUNTRY_CODES); idx++) {
    if (strcmp(COUNTRY_CODES[idx], data->country) == 0) {
      country_name = COUNTRY_NAMES[idx];
      selected_index = idx;
      break;
    }
  }

  static char subscription_count[20] = {0};
  snprintf(subscription_count, 19, "%d", data->num_subscriptions);

  swap_menu_section_add_item(sections[2], "Country", country_name, prv_change_country, (void *)selected_index);
  swap_menu_section_add_item(sections[2], "Subscriptions", subscription_count, NULL, NULL);

  s_settings_menu = menu;
  swap_menu_push(menu);
}
