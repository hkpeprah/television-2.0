#include "swap_menu.h"

#include <pebble.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define SWAP_SELECT_ROW (0)

struct SwapMenuSection {
  const char *title;
  SwapMenuItem *items;
  uint8_t num_items;
  union {
    bool is_allocated:1;
    uint8_t flags;
  };
};

struct SwapMenu {
  Window *window;
  MenuLayer *menu_layer;

  uint8_t current_section;
  uint8_t num_sections;
  SwapMenuSection **sections;
};

// Menu Layer Callbacks
///////////////////////////
static uint16_t prv_get_num_rows(MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
  SwapMenu *menu = callback_context;
  return menu->sections[menu->current_section]->num_items + 1;
}

static int16_t prv_get_cell_height(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
  const int16_t cell_height = 44;
  return cell_height;
}

static void prv_draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
  SwapMenu *menu = callback_context;
  SwapMenuSection *section = menu->sections[menu->current_section];
  MenuIndex selected_index = menu_layer_get_selected_index(menu->menu_layer);

  const uint16_t selected_row = selected_index.row;
  const bool selected = (selected_row == cell_index->row);
  const GRect bounds = layer_get_bounds(cell_layer);

  if (cell_index->row == SWAP_SELECT_ROW) {
    if (selected) {
      graphics_context_set_fill_color(ctx, GColorDarkGray);
      graphics_context_set_stroke_color(ctx, GColorBlack);
    } else {
      graphics_context_set_fill_color(ctx, GColorLightGray);
      graphics_context_set_stroke_color(ctx, GColorDarkGray);
    }
    const uint16_t corner_radius = 0;
    graphics_fill_rect(ctx, bounds, corner_radius, GCornersAll);
    graphics_draw_rect(ctx, bounds);
    graphics_context_set_text_color(ctx, GColorWhite);
    menu_cell_basic_draw(ctx, cell_layer, section->title, NULL, NULL);
  } else {
    SwapMenuItem *item = &section->items[cell_index->row - 1];
    if (selected) {
      graphics_context_set_fill_color(ctx, GColorVividCerulean);
      graphics_context_set_text_color(ctx, GColorWhite);
    } else {
      graphics_context_set_fill_color(ctx, GColorWhite);
      graphics_context_set_text_color(ctx, GColorBlack);
    }
    graphics_fill_rect(ctx, bounds, 0, GCornersAll);
    menu_cell_basic_draw(ctx, cell_layer, item->title, item->subtitle, NULL);
  }
}

static void prv_select_click(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
  SwapMenu *menu = callback_context;
  SwapMenuSection *section = menu->sections[menu->current_section];
  if (cell_index->row == SWAP_SELECT_ROW) {
    menu->current_section++;
    menu->current_section %= menu->num_sections;
  } else {
    SwapMenuItem *item = &section->items[cell_index->row - 1];
    if (item->callback) {
      item->callback(item, item->callback_context);
    }
  }
  menu_layer_reload_data(menu->menu_layer);
}

// Private API
///////////////////////////
static void prv_window_load(Window *window) {
  SwapMenu *menu = window_get_user_data(window);
  Layer *window_layer = window_get_root_layer(window);
  GRect frame = layer_get_frame(window_layer);

  MenuLayer *menu_layer = menu_layer_create(frame);
  menu->menu_layer = menu_layer;
  menu_layer_set_click_config_onto_window(menu_layer, window);
  menu_layer_pad_bottom_enable(menu_layer, false /* no pad */);
  menu_layer_set_callbacks(menu_layer, menu, (MenuLayerCallbacks){
    .get_num_rows = prv_get_num_rows,
    .get_cell_height = prv_get_cell_height,
    .draw_row = prv_draw_row,
    .select_click = prv_select_click
  });

  layer_add_child(window_layer, menu_layer_get_layer(menu_layer));
}

static void prv_window_unload(Window *window) {
  SwapMenu *menu = window_get_user_data(window);
  for (uint8_t idx = 0; idx < menu->num_sections; idx++) {
    swap_menu_section_destroy(menu->sections[idx]);
  }

  if (menu->sections) {
    free(menu->sections);
  }

  if (menu->menu_layer) {
    menu_layer_destroy(menu->menu_layer);
  }

  memset(menu, 0, sizeof(SwapMenu));
  free(menu);

  window_destroy(window);
}

// Public API
///////////////////////////
SwapMenuSection *swap_menu_section_create(const char *title) {
  SwapMenuSection *section = malloc(sizeof(SwapMenuSection));
  memset(section, 0, sizeof(SwapMenuSection));
  section->is_allocated = true;
  section->title = title;
  return section;
}

void swap_menu_section_destroy(SwapMenuSection *section) {
  if (!section) {
    return;
  }
  if (section->items) {
    free(section->items);
  }
  if (section->is_allocated) {
    memset(section, 0, sizeof(SwapMenuSection));
    free(section);
  }
}

void swap_menu_section_add_item(SwapMenuSection *section,
                                const char *title, const char *subtitle,
                                SwapMenuItemCallback cb, void *callback_context) {
  if (!section) {
    return;
  }
  uint8_t num_items = section->num_items;
  section->num_items++;
  section->items = realloc(section->items, section->num_items * sizeof(SwapMenuItem));

  SwapMenuItem *item = &section->items[num_items];
  item->title = title;
  item->subtitle = subtitle;
  item->callback = cb;
  item->callback_context = callback_context;
}

SwapMenu *swap_menu_create(void) {
  SwapMenu *menu = malloc(sizeof(SwapMenu));
  Window *window = window_create();

  memset(menu, 0, sizeof(SwapMenu));

  menu->window = window;
  window_set_user_data(window, menu);
  window_set_window_handlers(window, (WindowHandlers){
    .load = prv_window_load,
    .unload = prv_window_unload
  });

  return menu;
}

Window *swap_menu_get_window(SwapMenu *menu) {
  if (!menu) {
    return NULL;
  }
  return menu->window;
}

void swap_menu_add_section(SwapMenu *menu, SwapMenuSection *section) {
  if (!(menu && section)) {
    return;
  }
  menu->num_sections++;
  menu->sections = realloc(menu->sections, menu->num_sections * sizeof(SwapMenuSection *));
  menu->sections[menu->num_sections - 1] = section;
}

void swap_menu_push(SwapMenu *menu) {
  if (!menu) {
    return;
  }
  Window *window = swap_menu_get_window(menu);
  const bool animated = true;
  window_stack_push(window, animated);
}

void swap_menu_pop(SwapMenu *menu) {
  if (!menu) {
    return;
  }
  Window *window = swap_menu_get_window(menu);
  const bool animated = true;
  window_stack_remove(window, animated);
}

void swap_menu_reload(SwapMenu *menu) {
  if (!menu) {
    return;
  }
  menu_layer_reload_data(menu->menu_layer);
}
