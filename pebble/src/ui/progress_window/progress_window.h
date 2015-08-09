#pragma once
#include <pebble.h>

typedef Window ProgressWindow;

// Creates and returns a ProgressWindow
// @returns Pointer to a ProgressWindow
ProgressWindow *progress_window_create(void);

// Gets the underlying Window of a ProgressWindow
// @param progress_window Pointer to a ProgressWindow
// @returns Window
Window *progress_window_get_window(ProgressWindow *progress_window);

// Pushes a ProgressWindow onto the window stack
// @param progress_window Pointer to a ProgressWindow
void progress_window_push(ProgressWindow *progress_window);

// Pops a ProgressWindow from the window stack
// @param progress_window Pointer to a ProgressWindow
void progress_window_pop(ProgressWindow *progress_window);

// Sets the text to be displayed on a ProgressWindow
// @param progress_window Pointer to a ProgressWindow
// @param text The text to set
void progress_window_set_text(ProgressWindow *progress_window, const char *text);
