#pragma once
#include <stdbool.h>
#include <stdint.h>

#define MAX_COUNTRY_LEN (10)

typedef struct {
  struct {
    bool premium:1;
  } crunchyroll;
  struct {
    bool premium:1;
  } funimation;
  char country[MAX_COUNTRY_LEN];
  uint8_t num_subscriptions;
} SettingsMenuData;

// Shows the User Settings Menu via a SwapMenu
void show_settings_menu(SettingsMenuData *data);
