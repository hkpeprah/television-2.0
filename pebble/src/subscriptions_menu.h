#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  struct {
    bool crunchyroll:1;
    bool television:1;
    bool funimation:1;
    bool subscribed:1;
    uint8_t flags;
  } flags;
  struct {
    char *title;
    char *description;
    uint32_t timestamp;
  } latest;
  char *title;
  char *description;
} SubscriptionItem;

// Shows the Subscriptions Window on the screen
void subscriptions_menu_show(void);
