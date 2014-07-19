#include "pebble.h"

// keys for app message and storage
#define BATTERY_MODE   2
#define DATE_MODE_UK          0
#define DATE_MODE_US          1
	
#define BATTERY_MODE_NEVER    0
#define BATTERY_MODE_IF_LOW   1
#define BATTERY_MODE_ALWAYS   2
#define GRAPHICS_MODE_NORMAL  0
#define GRAPHICS_MODE_INVERT  1

#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))
	
static int mInvert = GRAPHICS_MODE_NORMAL;			// Invert colours (0/1)
static int mVibeMinutes = 0;	// Vibrate every X minutes

static int battery_mode   = BATTERY_MODE_ALWAYS;
static int date_mode      = DATE_MODE_UK;
static int show_date = 0;

static GBitmap *battery_images[22];
static BitmapLayer *battery_layer;

Layer *simple_bg_layer;

Window *window;
static Layer *layer;
TextLayer *text_date_layer;
TextLayer *text_time_layer;

static GBitmap *image;


static void layer_update_callback(Layer *me, GContext* ctx) {

  GRect bounds = image->bounds;
 
  graphics_draw_bitmap_in_rect(ctx, image, (GRect) { .origin = { ((144 - bounds.size.w) / 2) + 1.5, 0 }, .size = bounds.size});
}

void displayDate(struct tm* pbltime)
{

    static char dateString[] = "99/99";
  
	//  check for empty time
     time_t now;
     if (pbltime == NULL) {
       now = time(NULL);
       pbltime = localtime(&now);
    }
	
	if (date_mode == DATE_MODE_UK){	

	   snprintf(dateString,sizeof(dateString),   "%d/%d",pbltime->tm_mday,pbltime->tm_mon + 1);

	}
	if (date_mode == DATE_MODE_US) {
	   snprintf(dateString,sizeof(dateString),   "%d/%d",pbltime->tm_mon + 1,pbltime->tm_mday);


    }

     APP_LOG(APP_LOG_LEVEL_DEBUG, "datestring! %s ", dateString);
  text_layer_set_text(text_time_layer, dateString);

}
void display_time(struct tm* pbltime) {

  //  check for empty time

  time_t now;
  if (pbltime == NULL) {
    now = time(NULL);
    pbltime = localtime(&now);
  }

  static char time_text[] = "00:00";
  //static char date_text[] = "Xxxxxxxxx 00";

  char *time_format;


  // TODO: Only update the date when it's changed.
  //trftime(date_text, sizeof(date_text), "%e %B", pbltime);
  //text_layer_set_text(text_date_layer, date_text);


  if (clock_is_24h_style()) {
    time_format = "%R";
  } else {
    time_format = "%I:%M";
  }

  strftime(time_text, sizeof(time_text), time_format, pbltime);

  // Kludge to handle lack of non-padded hour format string
  // for twelve hour clock.
  if (!clock_is_24h_style() && (time_text[0] == '0')) {
    memmove(time_text, &time_text[1], sizeof(time_text) - 1);
  }

  text_layer_set_text(text_time_layer, time_text);
}

void handle_battery(BatteryChargeState charge_state) {
//#if DEBUG
  //strftime(debug_buffer, DEBUG_BUFFER_BYTES, "%d.%m.%Y %H:%M:%S", now);
//  snprintf(debug_buffer, DEBUG_BUFFER_BYTES, "%s%d%%",  charge_state.is_charging ? "+" : "", charge_state.charge_percent);
//  text_layer_set_text(debug_layer, debug_buffer);
//#endif
//#if SCREENSHOT
//  bitmap_layer_set_bitmap(battery_layer, battery_images[1]);
//  bool showSeconds = seconds_mode != SECONDS_MODE_NEVER;
//  bool showBattery = battery_mode != BATTERY_MODE_NEVER;
//  bool showDate = date_mode != DATE_MODE_OFF;
//#else
  bitmap_layer_set_bitmap(battery_layer, battery_images[
    (charge_state.is_charging ? 11 : 0) + min(charge_state.charge_percent / 10, 10)]);
  bool battery_is_low = charge_state.charge_percent <= 10;
  bool showBattery = battery_mode == BATTERY_MODE_ALWAYS
    || charge_state.is_charging;
//#endif
  layer_set_hidden(bitmap_layer_get_layer(battery_layer), !showBattery);

}

void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  // Need to be static because they're used by the system later.

	display_time(tick_time);

}

void handle_tap(AccelAxisType axis, int32_t direction) {
	if (show_date == 0){
	   displayDate(NULL);
		show_date = 1;
	}
	else {
		display_time(NULL);
		show_date = 0;
	}

}

void handle_deinit(void) {
	accel_tap_service_unsubscribe();
  battery_state_service_unsubscribe();
  gbitmap_destroy(image);
  for (int i = 0; i < 22; i++)
     gbitmap_destroy(battery_images[i]);
  tick_timer_service_unsubscribe();
}

void handle_init(void) {
  window = window_create();
  window_stack_push(window, true /* Animated */);
  window_set_background_color(window, GColorBlack);

  Layer *window_layer = window_get_root_layer(window);

  text_date_layer = text_layer_create(GRect(8, 74, 144-8, 168-74));
  text_layer_set_text_color(text_date_layer, GColorWhite);
  text_layer_set_background_color(text_date_layer, GColorClear);
  text_layer_set_font(text_date_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_CONDENSED_21)));
  layer_add_child(window_layer, text_layer_get_layer(text_date_layer));
	
  GRect bounds = layer_get_frame(window_layer);
 layer=layer_create(bounds);
 layer_set_update_proc(layer, layer_update_callback);
 layer_add_child(window_layer, layer);
 image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
	
	  // Push the window onto the stack
  text_time_layer = text_layer_create(GRect(10, 128, 144-20, 168-128));
  text_layer_set_text_color(text_time_layer, GColorWhite);
  text_layer_set_background_color(text_time_layer, GColorClear);
	text_layer_set_text_alignment(text_time_layer, GTextAlignmentCenter);
  text_layer_set_font(text_time_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_SUBSET_35)));
  layer_add_child(window_layer, text_layer_get_layer(text_time_layer));
	

     battery_images[0] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_0 );  
     battery_images[1] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_10);  
     battery_images[2] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_20 );  
     battery_images[3] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_30);  
	 battery_images[4] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_40 );  
     battery_images[5] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_50);  
	 battery_images[6] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_60 );  
     battery_images[7] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_70);  
	 battery_images[8] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_80 );  
     battery_images[9] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_90);  
	 battery_images[10] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_100 );  
     battery_images[11] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CHARGING_0);  
     battery_images[12] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CHARGING_10);  
     battery_images[13] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CHARGING_20);  
     battery_images[14] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CHARGING_30);  
     battery_images[15] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CHARGING_40);  
     battery_images[16] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CHARGING_50);  
     battery_images[17] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CHARGING_60);  
     battery_images[18] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CHARGING_70);  
     battery_images[19] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CHARGING_80);  
     battery_images[20] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CHARGING_90);  
	 battery_images[21] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CHARGING_100);  

	battery_layer = bitmap_layer_create(GRect(130, 168 - 24, 10, 20));
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(battery_layer));
	//layer_add_child(window_get_root_layer(window), text_layer_get_layer(text_time_layer));

	tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
  // TODO: Update display here to avoid blank display on launch?
	
	battery_state_service_subscribe(&handle_battery);
    handle_battery(battery_state_service_peek());
	
	accel_tap_service_subscribe(handle_tap);
	  //  display time (immediately before first tick event)
    display_time(NULL);
	// use the accel data so that if the watch hasn't moved we dont buzz
	//	accel_data_service_subscribe(0, handleAccel);
	
}


int main(void) {
  handle_init();

  app_event_loop();
  
  handle_deinit();
}
