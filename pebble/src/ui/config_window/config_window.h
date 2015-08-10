#pragma once
#include <pebble.h>

typedef Window ConfigWindow;

// Allocates a ConfigWindow on the heap, initializes it and returns it.
// @returns Pointer to a ConfigWindow
ConfigWindow *config_window_create(void);

// Returns the Window for the ConfigWindow
// @param config_window Pointer to a ConfigWindow
// @returns Underlying window of the ConfigWindow
Window *config_window_get_window(ConfigWindow *config_window);

// Pushes a ConfigWindow onto the window stack.
// @param config_window The ConfigWindow to add to the window stack
void config_window_push(ConfigWindow *config_window);

// Removes a ConfigWindow from the window stack.
// @param config_window The ConfigWindow to remove from the window stack
void config_window_pop(ConfigWindow *config_window);
