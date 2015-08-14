#pragma once
#include <pebble.h>

typedef struct SwapMenu SwapMenu;

typedef struct SwapMenuItem SwapMenuItem;

typedef struct SwapMenuSection SwapMenuSection;

typedef void(*SwapMenuItemCallback)(SwapMenuItem *item, void *callback_context);

struct SwapMenuItem {
  const char *title;
  const char *subtitle;
  SwapMenuItemCallback callback;
  void *callback_context;
};

// Creates a SwapMenuSection on the heap and returns a pointer to it
// @param title The text to display for this section in the SwapMenu
// @note If added to a SwapMenu, it takes care of deleting this item itself
//     given that it was allocated on the heap.
SwapMenuSection *swap_menu_section_create(const char *title);

// Destroys a SwapMenuSection and the associated items.
// @param section Pointer to the SwapMenuSection to destry
void swap_menu_section_destroy(SwapMenuSection *section);

// Adds an item to a SwapMenuSection
// @param section The SwapMenuSection to add the item to
// @param title The text to display for the title of the item
// @param subtitle The text to display for the subtitle of the item
// @param cb The SwapMenuItemCallback to call when the item is selected
// @param callback_context Context to pass to the callback
void swap_menu_section_add_item(SwapMenuSection *section,
                                const char *title, const char *subtitle,
                                SwapMenuItemCallback cb, void *callback_context);

// Creates a SwapMenu on the heap and returns a pointer to it
// @returns pointer to a SwapMenu
SwapMenu *swap_menu_create(void);

// Returns the underlying window of the SwapMenu
// @param menu Pointer to the SwapMenu whose Window to grab
// @returns Pointer to a window
Window *swap_menu_get_window(SwapMenu *menu);

// Adds a SwapMenuSection to a SwapMenu
// @param menu Pointer to the SwapMenu to add the section to
// @param section Pointer to the section to add to the SwapMenu
void swap_menu_add_section(SwapMenu *menu, SwapMenuSection *section);

// Reloads the data in a SwapMenu
// @param menu Pointer to a SwapMenu to reload
void swap_menu_reload(SwapMenu *menu);

// Pushes a SwapMenu onto the window stack
// @param menu Pointer to the SwapMenu to push
void swap_menu_push(SwapMenu *menu);

// Removes a SwapMenu from the window stack
// @param menu Pointer to the SwapMenu to remove
void swap_menu_pop(SwapMenu *menu);
