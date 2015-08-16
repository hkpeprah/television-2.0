#pragma once
#include "subscription_menu.h"

#include <pebble.h>

typedef struct SubscriptionItemWindow SubscriptionItemWindow;

typedef void(*SubscriptionItemWindowCallback)(const SubscriptionItem *item, void *callback_context);

typedef struct {
  SubscriptionItemWindowCallback unload;
  SubscriptionItemWindowCallback subscribe;
  SubscriptionItemWindowCallback unsubscribe;
} SubscriptionItemWindowCallbacks;

// Allocates a SubscriptionItemWindow on the heap and returns a pointer to it.
// @param item pointer to the SubscriptionItem to create the window for
// @returns pointer to a SubscriptionItemWindow
// @note It is assumed that the item is long-lived
SubscriptionItemWindow *subscription_item_window_create(const SubscriptionItem *item);

// Returns the underlying window of a SubscriptionItemWindow
// @param pointer to a SubscriptionItemWindow
// @returns pointer to a Window
Window *subscription_item_window_get_window(SubscriptionItemWindow *item_window);

// Sets the callbacks for a SubscriptionItemWindow
// @param item_window Pointer to a SubscriptionItemWindow
// @param callbacks The callbacks to set for the SubscriptionItemWindow
// @param callback_context The context to pass to the callbacks
void subscription_item_window_set_callbacks(SubscriptionItemWindow *item_window,
                                            SubscriptionItemWindowCallbacks callbacks,
                                            void *callback_context);

// Pushes a SubscriptionItemWindow onto the window stack
// @param item_window Pointer to a SubscriptionItemWindow
void subscription_item_window_push(SubscriptionItemWindow *item_window);

// Removes a SubscriptionItemWindow from the window stack
// @param item_window Pointer to a SubscriptionItemWindow
void subscription_item_window_pop(SubscriptionItemWindow *item_window);
