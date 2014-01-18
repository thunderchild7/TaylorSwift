#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

#define MY_UUID { 0xC9, 0x60, 0x4D, 0xC9, 0xD2, 0x06, 0x46, 0xEF, 0xA0, 0xBA, 0xB1, 0x6F, 0xA1, 0x6D, 0x35, 0x54 }

PBL_APP_INFO(MY_UUID,
             "TS WatchFace", "Oliver Bowe",
             1, 1, 
             RESOURCE_ID_IMAGE_MENU_ICON,
             APP_INFO_WATCH_FACE);

Window window;

BmpContainer background_image;
TextLayer text_time_layer;

void handle_init(AppContextRef ctx) {
  (void)ctx;

  window_init(&window, "TS");
  window_stack_push(&window, true /* Animated */);
  window_set_background_color(&window, GColorBlack);
	
  resource_init_current_app(&APP_RESOURCES);

  bmp_init_container(RESOURCE_ID_IMAGE_BACKGROUND, &background_image);
	  layer_set_frame(&background_image.layer.layer, GRect(4, 0, 135, 131));
  layer_add_child(&window.layer, &background_image.layer.layer);
	
	text_layer_init(&text_time_layer, window.layer.frame);
  text_layer_set_text_color(&text_time_layer, GColorWhite);
  text_layer_set_background_color(&text_time_layer, GColorBlack);
	
	text_layer_set_text_alignment(&text_time_layer, GTextAlignmentCenter);

  layer_set_frame(&text_time_layer.layer, GRect(0, 126, 144, 168));
	GFont custom_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_SUBSET_38));

  text_layer_set_font(&text_time_layer, custom_font);
  layer_add_child(&window.layer, &text_time_layer.layer);
}

void handle_deinit(AppContextRef ctx) {
  (void)ctx;

  bmp_deinit_container(&background_image);

}

void handle_minute_tick(AppContextRef ctx, PebbleTickEvent *t) {
	(void)t;
  (void)ctx;

  // Need to be static because they're used by the system later.
  static char time_text[] = "00:00";

  char *time_format;

  if (clock_is_24h_style()) {
    time_format = "%R";
  } else {
    time_format = "%I:%M";
  }

  string_format_time(time_text, sizeof(time_text), time_format, t->tick_time);

  // Kludge to handle lack of non-padded hour format string
  // for twelve hour clock.
  if (!clock_is_24h_style() && (time_text[0] == '0')) {
    memmove(time_text, &time_text[1], sizeof(time_text) - 1);
  }

  text_layer_set_text(&text_time_layer, time_text);
	
	// Vibrate Every Five Minutes
  //#if HOUR_VIBRATION
         
  //  if ((t->tick_time->tm_min % 5 == 0 ) && (t->tick_time->tm_sec==0))
  //  {
        //vibes_enqueue_custom_pattern(hour_pattern);
//		vibes_double_pulse();	
  //  }

  //#endif
}

void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .deinit_handler = &handle_deinit,

    .tick_info = {
      .tick_handler = &handle_minute_tick,
      .tick_units = MINUTE_UNIT
	}
  };
  app_event_loop(params, &handlers);
}
