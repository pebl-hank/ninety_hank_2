#include "pebble.h"
#include "stdlib.h"
#include "string.h"
#include "time.h"

#include "config.h"
#include "my_math.h"
#include "suncalc.h"

static Window *window;

static GBitmap *background_image;
static BitmapLayer *background_layer;

// TODO: Handle 12/24 mode preference when it's exposed.
static GBitmap *time_format_image;
static BitmapLayer *time_format_layer;

static TextLayer *DayOfWeekLayer; 
static TextLayer *moonLayer; 
static TextLayer *cwLayer; 
static TextLayer *text_addTimeZone1_layer; 
static TextLayer *text_addTimeZone2_layer; 
static TextLayer *text_sunrise_layer; 
static TextLayer *text_sunset_layer; 
static TextLayer *battery_layer;
static TextLayer *connection_layer;
static TextLayer *second_layer1;
static TextLayer *second_layer2;




const int DAY_NAME_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_DAY_NAME_SUN,
  RESOURCE_ID_IMAGE_DAY_NAME_MON,
  RESOURCE_ID_IMAGE_DAY_NAME_TUE,
  RESOURCE_ID_IMAGE_DAY_NAME_WED,
  RESOURCE_ID_IMAGE_DAY_NAME_THU,
  RESOURCE_ID_IMAGE_DAY_NAME_FRI,
  RESOURCE_ID_IMAGE_DAY_NAME_SAT
};

static GBitmap *day_name_image;
static BitmapLayer *day_name_layer;


const int DATENUM_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_DATENUM_0,
  RESOURCE_ID_IMAGE_DATENUM_1,
  RESOURCE_ID_IMAGE_DATENUM_2,
  RESOURCE_ID_IMAGE_DATENUM_3,
  RESOURCE_ID_IMAGE_DATENUM_4,
  RESOURCE_ID_IMAGE_DATENUM_5,
  RESOURCE_ID_IMAGE_DATENUM_6,
  RESOURCE_ID_IMAGE_DATENUM_7,
  RESOURCE_ID_IMAGE_DATENUM_8,
  RESOURCE_ID_IMAGE_DATENUM_9
};

#define TOTAL_MOON_DIGITS 1
static GBitmap *moon_digits_images[TOTAL_MOON_DIGITS];
static BitmapLayer *moon_digits_layers[TOTAL_MOON_DIGITS];

const int MOON_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_MOON_0,
  RESOURCE_ID_IMAGE_MOON_1,
  RESOURCE_ID_IMAGE_MOON_2,
  RESOURCE_ID_IMAGE_MOON_3,
  RESOURCE_ID_IMAGE_MOON_4,
  RESOURCE_ID_IMAGE_MOON_5,
  RESOURCE_ID_IMAGE_MOON_6,
  RESOURCE_ID_IMAGE_MOON_7
};


#define TOTAL_DATE_DIGITS 12
static GBitmap *date_digits_images[TOTAL_DATE_DIGITS];
static BitmapLayer *date_digits_layers[TOTAL_DATE_DIGITS];


const int BIG_DIGIT_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_NUM_0,
  RESOURCE_ID_IMAGE_NUM_1,
  RESOURCE_ID_IMAGE_NUM_2,
  RESOURCE_ID_IMAGE_NUM_3,
  RESOURCE_ID_IMAGE_NUM_4,
  RESOURCE_ID_IMAGE_NUM_5,
  RESOURCE_ID_IMAGE_NUM_6,
  RESOURCE_ID_IMAGE_NUM_7,
  RESOURCE_ID_IMAGE_NUM_8,
  RESOURCE_ID_IMAGE_NUM_9
};


#define TOTAL_TIME_DIGITS 4
static GBitmap *time_digits_images[TOTAL_TIME_DIGITS];
static BitmapLayer *time_digits_layers[TOTAL_TIME_DIGITS];

static void set_container_image(GBitmap **bmp_image, BitmapLayer *bmp_layer, const int resource_id, GPoint origin) {
  GBitmap *old_image = *bmp_image;

  *bmp_image = gbitmap_create_with_resource(resource_id);
  GRect frame = (GRect) {
    .origin = origin,
    .size = (*bmp_image)->bounds.size
  };
  bitmap_layer_set_bitmap(bmp_layer, *bmp_image);
  layer_set_frame(bitmap_layer_get_layer(bmp_layer), frame);

  if (old_image != NULL) {
  	gbitmap_destroy(old_image);
  }
}


static unsigned short get_display_hour(unsigned short hour) {
  if (clock_is_24h_style()) {
    return hour;
  }

  unsigned short display_hour = hour % 12;

  // Converts "0" to "12"
  return display_hour ? display_hour : 12;
}


int moon_phase(int y, int m, int d)
{
    /*
      calculates the moon phase (0-7), accurate to 1 segment.
      0 = > new moon.
      4 => full moon.
      */
    int c,e;
    double jd;
    int b;

    if (m < 3) {
        y--;
        m += 12;
    }
    ++m;
    c = 365.25*y;
    e = 30.6*m;
    jd = c+e+d-694039.09;  	/* jd is total days elapsed */
    jd /= 29.53;        	/* divide by the moon cycle (29.53 days) */
    b = jd;		   			/* int(jd) -> b, take integer part of jd */
    jd -= b;		   		/* subtract integer part to leave fractional part of original jd */
    b = jd*8 + 0.5;	   		/* scale fraction from 0-8 and round by adding 0.5 */
    b = b & 7;		   		/* 0 and 8 are the same so turn 8 into 0 */
    return b;
}


void adjustTimezone(float* time) 
{
  *time += TIMEZONE;
  *time = *time - 1; // Winter Time - quick&dirty fix
  if (*time > 24) *time -= 24;
  if (*time < 0) *time += 24;
}

void updateSunsetSunrise()
{
	// Calculating Sunrise/sunset with courtesy of Michael Ehrmann
	// https://github.com/mehrmann/pebble-sunclock
	static char sunrise_text[] = "00:00";
	static char sunset_text[]  = "00:00";
	
	//PblTm pblTime;
	//get_time(&pblTime);
  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);

	char *time_format;

	if (clock_is_24h_style()) 
	{
	  time_format = "%R";
	} 
	else 
	{
	  time_format = "%I:%M";
	}

	float sunriseTime = calcSunRise(current_time->tm_year, current_time->tm_mon+1, current_time->tm_mday, LATITUDE, LONGITUDE, 91.0f);
	float sunsetTime = calcSunSet(current_time->tm_year, current_time->tm_mon+1, current_time->tm_mday, LATITUDE, LONGITUDE, 91.0f);
	adjustTimezone(&sunriseTime);
	adjustTimezone(&sunsetTime);

	if (!current_time->tm_isdst) 
	{
	  sunriseTime+=1;
	  sunsetTime+=1;
	} 

	current_time->tm_min = (int)(60*(sunriseTime-((int)(sunriseTime))));
	current_time->tm_hour = (int)sunriseTime;
	strftime(sunrise_text, sizeof(sunrise_text), time_format, current_time);
	text_layer_set_text(text_sunrise_layer, sunrise_text);

	current_time->tm_min = (int)(60*(sunsetTime-((int)(sunsetTime))));
	current_time->tm_hour = (int)sunsetTime;
	strftime(sunset_text, sizeof(sunset_text), time_format, current_time);
	text_layer_set_text(text_sunset_layer, sunset_text);
}

static void handle_battery(BatteryChargeState charge_state) {
  static char battery_text[] = "100%";

  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "c");
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d%%", charge_state.charge_percent + 10);
  }
  text_layer_set_text(battery_layer, battery_text);
}



static void handle_bluetooth(bool connected) {
  if( !connected )
  {
    vibes_short_pulse();
  }
  text_layer_set_text(connection_layer, connected ? "BT" : "no BT");
}



unsigned short the_last_hour = 25;
unsigned short the_last_minute = 61;

// Called once per second
static void handle_second_tick(struct tm* current_time, TimeUnits units_changed) {
   static char time_text[] = "00";
  static char time_text1[] = "0";
  static char time_text2[] = "0";
  
  strftime(time_text, sizeof(time_text), "%S", current_time);
  time_text1[0] = time_text[0];
  time_text2[0] = time_text[1];
  text_layer_set_text(second_layer1, time_text1);
  text_layer_set_text(second_layer2, time_text2);

  handle_battery(battery_state_service_peek());

  unsigned short display_minute = current_time->tm_min;
  if (the_last_minute != display_minute){  //check only every minute
	  the_last_minute = display_minute;
	  unsigned short display_hour = get_display_hour(current_time->tm_hour);


	  // Hour
	  set_container_image(&time_digits_images[0], time_digits_layers[0], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour/10], GPoint(4, 94));
	  set_container_image(&time_digits_images[1], time_digits_layers[1], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour%10], GPoint(37, 94));

	  //Minute
	  set_container_image(&time_digits_images[2], time_digits_layers[2], BIG_DIGIT_IMAGE_RESOURCE_IDS[current_time->tm_min/10], GPoint(80, 94));
	  set_container_image(&time_digits_images[3], time_digits_layers[3], BIG_DIGIT_IMAGE_RESOURCE_IDS[current_time->tm_min%10], GPoint(111, 94));
	  
	  
	  // ======== Time Zone 1  
	  text_layer_set_text(text_addTimeZone1_layer, AdditionalTimezone_1_Description); 
	  short  display_hour_tz1 = display_hour AdditionalTimezone_1;
	  if (display_hour_tz1 > 24) display_hour_tz1 -= 24;
	  if (display_hour_tz1 < 0) display_hour_tz1 += 24;
	  set_container_image(&date_digits_images[4], date_digits_layers[4], DATENUM_IMAGE_RESOURCE_IDS[display_hour_tz1/10], GPoint(75, 5));
	  set_container_image(&date_digits_images[5], date_digits_layers[5], DATENUM_IMAGE_RESOURCE_IDS[display_hour_tz1%10], GPoint(88, 5));  
	  set_container_image(&date_digits_images[6], date_digits_layers[6], DATENUM_IMAGE_RESOURCE_IDS[current_time->tm_min/10], GPoint(108, 5));
	  set_container_image(&date_digits_images[7], date_digits_layers[7], DATENUM_IMAGE_RESOURCE_IDS[current_time->tm_min%10], GPoint(121, 5));  
	  // ======== Time Zone 1   
	  
	  // ======== Time Zone 2  
	  text_layer_set_text(text_addTimeZone2_layer, AdditionalTimezone_2_Description); 
	  short  display_hour_tz2 = display_hour AdditionalTimezone_2;
	  if (display_hour_tz2 > 24) display_hour_tz2 -= 24;
	  if (display_hour_tz2 < 0) display_hour_tz2 += 24;
	  set_container_image(&date_digits_images[8], date_digits_layers[8], DATENUM_IMAGE_RESOURCE_IDS[display_hour_tz2/10], GPoint(75, 26));
	  set_container_image(&date_digits_images[9], date_digits_layers[9], DATENUM_IMAGE_RESOURCE_IDS[display_hour_tz2%10], GPoint(88, 26));  
	  set_container_image(&date_digits_images[10], date_digits_layers[10], DATENUM_IMAGE_RESOURCE_IDS[current_time->tm_min/10], GPoint(108, 26));
	  set_container_image(&date_digits_images[11], date_digits_layers[11], DATENUM_IMAGE_RESOURCE_IDS[current_time->tm_min%10], GPoint(121, 26));  
	  // ======== Time Zone 2 	  

	  
	  if (the_last_hour != display_hour){  //check only every hour
	  
		//vibes_short_pulse(); //Vibrates once per hour
		
		 // set_container_image(&day_name_image, day_name_layer, DAY_NAME_IMAGE_RESOURCE_IDS[current_time->tm_wday], GPoint(69, 61));
		  text_layer_set_text(DayOfWeekLayer, DAY_NAME_LANGUAGE[current_time->tm_wday]); 
		 
		 //Day
		  set_container_image(&date_digits_images[0], date_digits_layers[0], DATENUM_IMAGE_RESOURCE_IDS[current_time->tm_mday/10], GPoint(day_month_x[0], 71));
		  set_container_image(&date_digits_images[1], date_digits_layers[1], DATENUM_IMAGE_RESOURCE_IDS[current_time->tm_mday%10], GPoint(day_month_x[0] + 13, 71));

		 // Month
		 
		  set_container_image(&date_digits_images[2], date_digits_layers[2], DATENUM_IMAGE_RESOURCE_IDS[(current_time->tm_mon+1)/10], GPoint(day_month_x[1], 71));
		  set_container_image(&date_digits_images[3], date_digits_layers[3], DATENUM_IMAGE_RESOURCE_IDS[(current_time->tm_mon+1)%10], GPoint(day_month_x[1] + 13, 71));
		 
	  
		  if (!clock_is_24h_style()) {
			if (current_time->tm_hour >= 12) {
				layer_set_hidden(bitmap_layer_get_layer(time_format_layer), false);
			  set_container_image(&time_format_image, time_format_layer, RESOURCE_ID_IMAGE_PM_MODE, GPoint(10, 78));
			} else {
				layer_set_hidden(bitmap_layer_get_layer(time_format_layer), true);


			}

			if (display_hour/10 == 0) {
				layer_set_hidden(bitmap_layer_get_layer(time_digits_layers[0]), true);
			} else {
				layer_set_hidden(bitmap_layer_get_layer(time_digits_layers[0]), false);


			}
		  }
		  
		// -------------------- Moon_phase
			int moonphase_number;
			moonphase_number = moon_phase(current_time->tm_year+1900,current_time->tm_mon,current_time->tm_mday);

			set_container_image(&moon_digits_images[0], moon_digits_layers[0], MOON_IMAGE_RESOURCE_IDS[moonphase_number], GPoint(1, 1));
		
			text_layer_set_text(moonLayer, MOONPHASE_NAME_LANGUAGE[moonphase_number]); 
		// -------------------- Moon_phase	  
		  
		// -------------------- Calendar week  
		  static char cw_text[] = "XX00";
		  strftime(cw_text, sizeof(cw_text), TRANSLATION_CW , current_time);
		  text_layer_set_text(cwLayer, cw_text); 
		// ------------------- Calendar week  
		
		  
		} //check only every hour
	  
	  
	  the_last_hour = display_hour;
	  updateSunsetSunrise();
	}

}

static void init(void) {
  memset(&time_digits_layers, 0, sizeof(time_digits_layers));
  memset(&time_digits_images, 0, sizeof(time_digits_images));
  memset(&date_digits_layers, 0, sizeof(date_digits_layers));
  memset(&date_digits_images, 0, sizeof(date_digits_images));
  memset(&moon_digits_layers, 0, sizeof(moon_digits_layers));
  memset(&moon_digits_images, 0, sizeof(moon_digits_images));
  

  window = window_create();
  if (window == NULL) {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "OOM: couldn't allocate window");
      return;
  }
  window_stack_push(window, true /* Animated */);
  Layer *window_layer = window_get_root_layer(window);

  background_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  background_layer = bitmap_layer_create(layer_get_frame(window_layer));
  bitmap_layer_set_bitmap(background_layer, background_image);
  layer_add_child(window_layer, bitmap_layer_get_layer(background_layer));

   // Day of week text
  DayOfWeekLayer = text_layer_create(GRect(35, 62, 40 /* width */, 30 /* height */));
  text_layer_set_text_color(DayOfWeekLayer, GColorWhite);
  text_layer_set_background_color(DayOfWeekLayer, GColorClear );
  text_layer_set_font(DayOfWeekLayer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(DayOfWeekLayer));
  
   // Moon Text
  moonLayer = text_layer_create(GRect(2, 25, 50 /* width */, 30 /* height */)); 
  text_layer_set_text_color(moonLayer, GColorWhite);
  text_layer_set_background_color(moonLayer, GColorClear );
  text_layer_set_font(moonLayer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  layer_add_child(window_layer, text_layer_get_layer(moonLayer));  
  
   // Calendar Week
  cwLayer = text_layer_create(GRect(2, 40, 80 /* width */, 30 /* height */)); 
  text_layer_set_text_color(cwLayer, GColorWhite);
  text_layer_set_background_color(cwLayer, GColorClear );
  text_layer_set_font(cwLayer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  layer_add_child(window_layer, text_layer_get_layer(cwLayer));  
  
  // Text for Additional Time Zone 1
  text_addTimeZone1_layer = text_layer_create(GRect(51, 6, 100 /* width */, 30 /* height */)); 
  text_layer_set_text_color(text_addTimeZone1_layer, GColorWhite);
  text_layer_set_background_color(text_addTimeZone1_layer, GColorClear );
  text_layer_set_font(text_addTimeZone1_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  layer_add_child(window_layer, text_layer_get_layer(text_addTimeZone1_layer));   
  
  // Text for Additional Time Zone 2
  text_addTimeZone2_layer = text_layer_create(GRect(51, 26, 100 /* width */, 30 /* height */)); 
  text_layer_set_text_color(text_addTimeZone2_layer, GColorWhite);
  text_layer_set_background_color(text_addTimeZone2_layer, GColorClear );
  text_layer_set_font(text_addTimeZone2_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  layer_add_child(window_layer, text_layer_get_layer(text_addTimeZone2_layer));  
  
  // Sunrise Text
  text_sunrise_layer = text_layer_create(GRect(7, 152, 50 /* width */, 30 /* height */)); 
  text_layer_set_text_color(text_sunrise_layer, GColorWhite);
  text_layer_set_background_color(text_sunrise_layer, GColorClear );
  text_layer_set_font(text_sunrise_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  layer_add_child(window_layer, text_layer_get_layer(text_sunrise_layer));     
  
  // Sunset Text
  text_sunset_layer = text_layer_create(GRect(110, 152, 50 /* width */, 30 /* height */)); 
  text_layer_set_text_color(text_sunset_layer, GColorWhite);
  text_layer_set_background_color(text_sunset_layer, GColorClear );
  text_layer_set_font(text_sunset_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  layer_add_child(window_layer, text_layer_get_layer(text_sunset_layer));      
  
  // Connection
  connection_layer = text_layer_create(GRect(60, 152, /* width */ 50, 34 /* height */));
  text_layer_set_text_color(connection_layer, GColorWhite);
  text_layer_set_background_color(connection_layer, GColorClear);
  text_layer_set_font(connection_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(connection_layer, GTextAlignmentCenter);
  text_layer_set_text(connection_layer, "BT");
  layer_add_child(window_layer, text_layer_get_layer(connection_layer));  

  // Battery state
  battery_layer = text_layer_create(GRect(40, 152, /* width */ 50, 34 /* height */));
  text_layer_set_text_color(battery_layer, GColorWhite);
  text_layer_set_background_color(battery_layer, GColorClear);
  text_layer_set_font(battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(battery_layer, GTextAlignmentLeft);
  text_layer_set_text(battery_layer, "100%");  
  layer_add_child(window_layer, text_layer_get_layer(battery_layer));  
  
  
    
  
  
  
  if (!clock_is_24h_style()) {
    time_format_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_24_HOUR_MODE);
    GRect frame = (GRect) {
      .origin = { .x = 17, .y = 68 },
      .size = time_format_image->bounds.size
    };
    time_format_layer = bitmap_layer_create(frame);
    bitmap_layer_set_bitmap(time_format_layer, time_format_image);
    layer_add_child(window_layer, bitmap_layer_get_layer(time_format_layer));
  }

 
   // Create time and date layers
  GRect dummy_frame = { {0, 0}, {0, 0} };
  
  for (int i = 0; i < TOTAL_TIME_DIGITS; ++i) {
    time_digits_layers[i] = bitmap_layer_create(dummy_frame);
	   bitmap_layer_set_background_color(time_digits_layers[i], GColorClear );
    layer_add_child(window_layer, bitmap_layer_get_layer(time_digits_layers[i]));
  }
  for (int i = 0; i < TOTAL_DATE_DIGITS; ++i) {
    date_digits_layers[i] = bitmap_layer_create(dummy_frame);
	   bitmap_layer_set_background_color(date_digits_layers[i], GColorClear );
    layer_add_child(window_layer, bitmap_layer_get_layer(date_digits_layers[i]));
  }
  
  for (int i = 0; i < TOTAL_MOON_DIGITS; ++i) {
    moon_digits_layers[i] = bitmap_layer_create(dummy_frame);
    layer_add_child(window_layer, bitmap_layer_get_layer(moon_digits_layers[i]));
  }
  
  // Second Layers
  second_layer1 = text_layer_create(GRect(89, 114, 50 /* width */, 30 /* height */)); 
  text_layer_set_text_color(second_layer1, GColorWhite);
  text_layer_set_background_color(second_layer1, GColorClear );
  text_layer_set_font(second_layer1, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  layer_add_child(window_layer, text_layer_get_layer(second_layer1));      
  second_layer2 = text_layer_create(GRect(121, 114, 50 /* width */, 30 /* height */)); 
  text_layer_set_text_color(second_layer2, GColorWhite);
  text_layer_set_background_color(second_layer2, GColorClear );
  text_layer_set_font(second_layer2, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  layer_add_child(window_layer, text_layer_get_layer(second_layer2));  
	
	

  // Avoids a blank screen on watch start.
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);

 // update_display(tick_time); // causes wrong time displayed initially


  
  // Second tick
  handle_second_tick(tick_time, SECOND_UNIT);
  tick_timer_service_subscribe(SECOND_UNIT, &handle_second_tick);
  
  battery_state_service_subscribe(&handle_battery);
  bluetooth_connection_service_subscribe(&handle_bluetooth);

  // Second tick

}


static void deinit(void) {
  layer_remove_from_parent(bitmap_layer_get_layer(background_layer));
  bitmap_layer_destroy(background_layer);
  gbitmap_destroy(background_image);

  layer_remove_from_parent(bitmap_layer_get_layer(time_format_layer));
  bitmap_layer_destroy(time_format_layer);
  gbitmap_destroy(time_format_image);

  layer_remove_from_parent(bitmap_layer_get_layer(day_name_layer));
  bitmap_layer_destroy(day_name_layer);
  gbitmap_destroy(day_name_image);
  
  text_layer_destroy(DayOfWeekLayer);
  text_layer_destroy(moonLayer);
  text_layer_destroy(cwLayer);
  text_layer_destroy(text_addTimeZone1_layer);
  text_layer_destroy(text_addTimeZone2_layer);  
  text_layer_destroy(text_sunrise_layer);
  text_layer_destroy(text_sunset_layer);  
  text_layer_destroy(connection_layer);
  text_layer_destroy(battery_layer);  
  text_layer_destroy(second_layer1);  
  text_layer_destroy(second_layer2);    
  
 
  for (int i = 0; i < TOTAL_DATE_DIGITS; i++) {
    layer_remove_from_parent(bitmap_layer_get_layer(date_digits_layers[i]));
    gbitmap_destroy(date_digits_images[i]);
    bitmap_layer_destroy(date_digits_layers[i]);
  }
  
   for (int i = 0; i < TOTAL_MOON_DIGITS; i++) {
    layer_remove_from_parent(bitmap_layer_get_layer(moon_digits_layers[i]));
    gbitmap_destroy(moon_digits_images[i]);
    bitmap_layer_destroy(moon_digits_layers[i]);
  } 
  

  

  for (int i = 0; i < TOTAL_TIME_DIGITS; i++) {
    layer_remove_from_parent(bitmap_layer_get_layer(time_digits_layers[i]));
    gbitmap_destroy(time_digits_images[i]);
    bitmap_layer_destroy(time_digits_layers[i]);
  }
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  tick_timer_service_unsubscribe();
  window_destroy( window );
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
