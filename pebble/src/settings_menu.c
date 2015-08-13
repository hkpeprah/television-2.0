#include "settings_menu.h"

#include "ui/swap_menu/swap_menu.h"

#include <pebble.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  swap_menu_section_add_item(sections[0], "Status", data->crunchyroll.premium ? "Premium" : "Free", NULL, NULL);

  // Add the items for the Funimation section
  swap_menu_section_add_item(sections[1], "Subscription", data->funimation.premium ? "Subscription" : "Free", NULL, NULL);

  // Add the items for the user settings section
  static char country_code[MAX_COUNTRY_LEN] = {0};
  strncpy(country_code, data->country, MAX_COUNTRY_LEN - 1);

  static char subscription_count[20] = {0};
  snprintf(subscription_count, 19, "%d", data->num_subscriptions);

  swap_menu_section_add_item(sections[2], "Country", "US", NULL, NULL);
  swap_menu_section_add_item(sections[2], "Subscriptions", subscription_count, NULL, NULL);

  swap_menu_push(menu);
}
