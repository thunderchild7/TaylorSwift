#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

#define MY_UUID { 0xC9, 0x60, 0x4D, 0xC9, 0xD2, 0x06, 0x46, 0xEF, 0xA0, 0xBA, 0xB1, 0x6F, 0xA1, 0x6D, 0x35, 0x54 }

PBL_APP_INFO(MY_UUID,
             “TS”WatchFace, “Oliver” Bowe,
             1, 1, /* App version */
             RESOURCE_ID_IMAGE_MENU_ICON,
             APP_INFO_WATCH_FACE);

Window window;

BmpContainer background_image;

void handle_init(AppContextRef ctx) {
  (void)ctx;
  static char text[] = "NOW";

  window_init(&window, "Now");
  window_stack_push(&window, true /* Animated */);

  resource_init_current_app(&APP_RESOURCES);

  bmp_init_container(RESOURCE_ID_IMAGE_BACKGROUND, &background_image);
  layer_add_child(&window.layer, &background_image.layer.layer);
}

void handle_deinit(AppContextRef ctx) {
  (void)ctx;

  bmp_deinit_container(&background_image);
}

void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .deinit_handler = &handle_deinit
  };
  app_event_loop(params, &handlers);
}
