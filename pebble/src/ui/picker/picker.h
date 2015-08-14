#pragma once
#include <pebble.h>
#include <stdint.h>

typedef struct Picker Picker;

typedef void(*PickerCallback)(Picker *picker, int idx, void *callback_context);

// Allocates a Picker on the heap and returns a pointer to it
// @param text The text to display for the Picker
// @returns Pointer to a Picker
Picker *picker_create(const char *text);

// Returns the underlying Window of a Picker
// @param picker Pointer to the picker whom's window to grab
// @return Pointer to a Window
Window *picker_get_window(Picker *picker);

// Adds an option to a Picker
// @param picker Pointer to a picker
// @param text The text to display for the option
// @param cb The PickerCallback to call if the item is clicked
// @param callback_context The callback context to pass when the option is clicked
void picker_add_option(Picker *picker, const char *text, PickerCallback cb, void *callback_context);

// Sets the selected item in the Picker
// @param picker Pointer to the Picker to set
// @param idx Index of the element to make visible
void picker_set_selected_index(Picker *picker, uint8_t idx);

// Pushes a Picker onto the window stack
// @param picker Pointer to a picker
void picker_push(Picker *picker);

// Destroys a Picker, removing it from the window stack if it has been pushed
// @param picker Pointer to a picker
void picker_destroy(Picker *picker);
