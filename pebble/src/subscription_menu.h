#pragma once
#include <pebble.h>
#include <stdbool.h>
#include <stdint.h>

#define MAX_NAME_LEN (40)

typedef struct {
  // The name of the episode
  char name[MAX_NAME_LEN + 1];
  // The summary of the episode
  char *summary;
  // The season the latest episode belongs to
  uint32_t season;
  // The number of this episode
  uint32_t number;
  // The timestamp for when this episode airs; seconds
  // since epoch.
  uint32_t timestamp;
  // The runtime of the episode
  uint32_t runtime;
} Episode;

typedef struct {
  // The identifier of the network this item belongs to
  uint32_t id;
  // The name of the network (or producer) of this item
  char name[MAX_NAME_LEN + 1];
} Network;

typedef struct {
  // True if this is a Crunchyroll item
  bool crunchyroll;
  // True if this is a Television item
  bool television;
  // True if this is a Funimation item
  bool funimation;
  // True if the user is subscribed to this item
  bool subscribed;
  // True if this item has a latest episode
  bool has_latest;
  // Latest episode
  Episode latest;
  // Network the series belongs to
  Network network;
  // The identifier of the series/movie
  uint32_t id;
  // The name of the series/movie.
  char name[MAX_NAME_LEN + 1];
  // The runtime of this item in minutes
  uint32_t runtime;
} SubscriptionItem;

typedef struct {
  Window *window;

  TextLayer *text_layer;
  MenuLayer *menu_layer;

  SubscriptionItem **items;
  uint16_t num_items;
} SubscriptionMenu;

// Creates and returns a pointer to a SubscriptionMenu
// @returns Pointer to a SubscriptionMenu
SubscriptionMenu *subscription_menu_create(void);

// Adds an item to a SubscriptionMenu
// @param menu Pointer to the SubscriptionMenu to which to add
// @param item Pointer to the SubscriptionItem to add
void subscription_menu_add_item(SubscriptionMenu *menu, SubscriptionItem *item);

// Pushes a SubscriptionMenu onto the window stack.
// @param menu Pointer to a SubscriptionMenu
void subscription_menu_push(SubscriptionMenu *menu);

// Removes a SubscriptionMenu from the window stack.
// @param menu Pointer to the SubscriptionMenu to remove
void subscription_menu_pop(SubscriptionMenu *menu);

// Destroys a SubscriptionMenu, freeing the associated memory.
// @param menu Pointer to the SubscriptionMenu to destroy
void subscription_menu_destroy(SubscriptionMenu *menu);

// Destroys a SubscriptionItem, freeing the associated memory.
// @param item Pointer to a SubscriptionItem
void subscription_item_destroy(SubscriptionItem *item);
