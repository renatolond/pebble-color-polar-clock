#include <pebble.h>

#ifdef PBL_COLOR
	#include "gcolor_definitions.h"
#endif

#include <time.h>

static Window *s_main_window;

const GPathInfo SECOND_SEGMENT_PATH_POINTS = {
	.num_points = 3,
	.points = (GPoint []) {
		{0, 0},
		{-7, -70}, // 70 = radius + fudge; 7 = 70*tan(6 degrees); 6 degrees per second;
		{7,  -70},
	}
};

const GPathInfo MINUTE_SEGMENT_PATH_POINTS = {
	.num_points = 3,
	.points = (GPoint []) {
		{0, 0},
		{-7, -70}, // 70 = radius + fudge; 7 = 70*tan(6 degrees); 6 degrees per minute;
		{7,  -70},
	}
};

const GPathInfo HOUR_SEGMENT_PATH_POINTS = {
	3,
	(GPoint []) {
		{0, 0},
		{-5, -48}, // 48 = radius + fudge; 5 = 48*tan(6 degrees); 30 degrees per hour;
		{5,  -48},
	}
};

const GPathInfo BATTERY_SEGMENT_PATH_POINTS = {
	3,
	(GPoint []) {
		{0, 0},
		{-9, -70}, // 70 = radius + fudge; 9 = 70*tan(3 degrees); 3 degrees per point;
		{9,  -70},
	}
};
//GPath *second_segment_path;
//Layer *second_display_layer;

GPath *minute_segment_path;
Layer *minute_display_layer;

GPath *hour_segment_path;
Layer *hour_display_layer;

GPath *battery_segment_path;
Layer *battery_display_layer;

//#define SEE_VISUAL_MARKS 1

#ifdef SEE_VISUAL_MARKS
int window_x, window_y;
#endif

// Languages
#define LANG_DUTCH 0
#define LANG_ENGLISH 1
#define LANG_FRENCH 2
#define LANG_GERMAN 3
#define LANG_SPANISH 4
#define LANG_PORTUGUESE 5
#define LANG_SWEDISH 6
#define LANG_MAX 7

#define HOUR_X 26
#define HOUR_Y 56
#define HOUR_W 60
#define HOUR_H 50
TextLayer *s_hour_layer;

#define MINUTE_X 88
#define MINUTE_Y 66
#define MINUTE_W 55
#define MINUTE_H 50
TextLayer *s_minute_layer;

#define WEEKDAY_X 45
#define WEEKDAY_Y 36
#define WEEKDAY_W 55
#define WEEKDAY_H 50
TextLayer *s_weekday_layer;

#define DAY_AND_MONTH_X 22
#define DAY_AND_MONTH_Y 93
#define DAY_AND_MONTH_W 105
#define DAY_AND_MONTH_H 50
TextLayer *s_day_and_month_layer;

#define BATTERY_CHARGING_Y 0
#define BATTERY_CHARGING_W 16
#define BATTERY_CHARGING_H 16
static GBitmap* s_bitmap;
static BitmapLayer *s_bitmap_layer;

static int angle_90 = TRIG_MAX_ANGLE / 4;
static int angle_180 = TRIG_MAX_ANGLE / 2;
static int angle_270 = 3 * TRIG_MAX_ANGLE / 4;

static int battery_circle_outer_radius = 71, battery_circle_inner_radius,
			//seconds_circle_outer_radius = 71, seconds_circle_inner_radius,
			minutes_circle_outer_radius, minutes_circle_inner_radius,
			hours_circle_outer_radius, hours_circle_inner_radius;
#define BATTERY_CIRCLE_THICKNESS 2
#define SECONDS_CIRCLE_THICKNESS 4
#define MINUTES_CIRCLE_THICKNESS 1
#define HOURS_CIRCLE_THICKNESS 2
#define CIRCLE_SPACE 5

#ifdef PBL_COLOR
#define NBR_COLORS 60
#define NBR_COLORS_HOURS 24
#define NBR_COLORS_WEEKDAYS 7
const uint8_t colors[NBR_COLORS] = {GColorTiffanyBlueARGB8, GColorTiffanyBlueARGB8, GColorCobaltBlueARGB8,
	GColorCobaltBlueARGB8, GColorCobaltBlueARGB8, GColorVividCeruleanARGB8,
	GColorBlueMoonARGB8, GColorBlueMoonARGB8, GColorBlueMoonARGB8,
	GColorBlueARGB8, GColorBlueARGB8, GColorBlueARGB8,
	GColorElectricUltramarineARGB8, GColorElectricUltramarineARGB8, GColorElectricUltramarineARGB8,
	GColorVividVioletARGB8, GColorIndigoARGB8, GColorIndigoARGB8,
	GColorIndigoARGB8, GColorPurpleARGB8, GColorPurpleARGB8,
	GColorPurpleARGB8, GColorJazzberryJamARGB8, GColorJazzberryJamARGB8,
	GColorJazzberryJamARGB8, GColorFashionMagentaARGB8, GColorFollyARGB8,
	GColorFollyARGB8, GColorFollyARGB8, GColorRedARGB8,
	GColorRedARGB8, GColorRedARGB8, GColorOrangeARGB8,
	GColorOrangeARGB8, GColorOrangeARGB8, GColorChromeYellowARGB8,
	GColorWindsorTanARGB8, GColorWindsorTanARGB8, GColorWindsorTanARGB8,
	GColorLimerickARGB8, GColorLimerickARGB8, GColorLimerickARGB8,
	GColorKellyGreenARGB8, GColorKellyGreenARGB8, GColorKellyGreenARGB8,
	GColorSpringBudARGB8, GColorBrightGreenARGB8, GColorBrightGreenARGB8,
	GColorBrightGreenARGB8, GColorGreenARGB8, GColorGreenARGB8,
	GColorGreenARGB8, GColorMalachiteARGB8, GColorMalachiteARGB8,
	GColorMalachiteARGB8, GColorMediumSpringGreenARGB8, GColorJaegerGreenARGB8,
	GColorJaegerGreenARGB8, GColorJaegerGreenARGB8, GColorTiffanyBlueARGB8,
};
const uint8_t colors_hours[NBR_COLORS_HOURS] = {GColorTiffanyBlueARGB8, GColorCobaltBlueARGB8, GColorVividCeruleanARGB8,
	GColorBlueMoonARGB8, GColorBlueARGB8, GColorElectricUltramarineARGB8,
	GColorVividVioletARGB8, GColorIndigoARGB8, GColorPurpleARGB8,
	GColorJazzberryJamARGB8, GColorFashionMagentaARGB8, GColorFollyARGB8,
	GColorRedARGB8, GColorOrangeARGB8, GColorChromeYellowARGB8,
	GColorWindsorTanARGB8, GColorLimerickARGB8, GColorKellyGreenARGB8,
	GColorSpringBudARGB8, GColorBrightGreenARGB8, GColorGreenARGB8,
	GColorMalachiteARGB8, GColorMediumSpringGreenARGB8, GColorJaegerGreenARGB8,
};
const uint8_t colors_weekdays[NBR_COLORS_WEEKDAYS] = {GColorTiffanyBlueARGB8, GColorBlueARGB8, GColorPurpleARGB8,
	GColorRedARGB8, GColorOrangeARGB8, GColorChromeYellowARGB8,
	GColorGreenARGB8,};
#endif

const char weekdays[LANG_MAX][7][6] = {
	{ "zon", "maa", "din", "woe", "don", "vri", "zat" },// Dutch
	{ "sun", "mon", "tue", "wed", "thu", "fri", "sat" },// English
	{ "dim", "lun", "mar", "mer", "jeu", "ven", "sam" },// French
	{ "son", "mon", "die", "mit", "don", "fre", "sam" },// German
	{ "dom", "lun", "mar", "mie", "jue", "vie", "sab" },// Spanish
	{ "dom", "seg", "ter", "qua", "qui", "sex", "sab" },// Portuguese
	{ "sön", "mån", "Tis", "ons", "tor", "fre", "lör" } // Swedish
};
static int current_language = LANG_ENGLISH;

//static int32_t seconds_a1, seconds_a2;
static int32_t minutes_a1, minutes_a2;
static int32_t hour_a1, hour_a2;
static int32_t battery_a1, battery_a2, battery_level;

/*\
|*| DrawArc function thanks to Cameron MacFarland (http://forums.getpebble.com/profile/12561/Cameron%20MacFarland)
\*/
static void graphics_draw_arc(GContext *ctx, GPoint center, int radius, int thickness, int start_angle, int end_angle, GColor c) {
	int32_t xmin = 65535000, xmax = -65535000, ymin = 65535000, ymax = -65535000;
	int32_t cosStart, sinStart, cosEnd, sinEnd;
	int32_t r, t;

	while (start_angle < 0) start_angle += TRIG_MAX_ANGLE;
	while (end_angle < 0) end_angle += TRIG_MAX_ANGLE;

	start_angle %= TRIG_MAX_ANGLE;
	end_angle %= TRIG_MAX_ANGLE;

	if (end_angle == 0) end_angle = TRIG_MAX_ANGLE;

	if (start_angle > end_angle) {
		graphics_draw_arc(ctx, center, radius, thickness, start_angle, TRIG_MAX_ANGLE, c);
		graphics_draw_arc(ctx, center, radius, thickness, 0, end_angle, c);
	} else {
		// Calculate bounding box for the arc to be drawn
		cosStart = cos_lookup(start_angle);
		sinStart = sin_lookup(start_angle);
		cosEnd = cos_lookup(end_angle);
		sinEnd = sin_lookup(end_angle);

		r = radius;
		// Point 1: radius & start_angle
		t = r * cosStart;
		if (t < xmin) xmin = t;
		if (t > xmax) xmax = t;
		t = r * sinStart;
		if (t < ymin) ymin = t;
		if (t > ymax) ymax = t;

		// Point 2: radius & end_angle
		t = r * cosEnd;
		if (t < xmin) xmin = t;
		if (t > xmax) xmax = t;
		t = r * sinEnd;
		if (t < ymin) ymin = t;
		if (t > ymax) ymax = t;

		r = radius - thickness;
		// Point 3: radius-thickness & start_angle
		t = r * cosStart;
		if (t < xmin) xmin = t;
		if (t > xmax) xmax = t;
		t = r * sinStart;
		if (t < ymin) ymin = t;
		if (t > ymax) ymax = t;

		// Point 4: radius-thickness & end_angle
		t = r * cosEnd;
		if (t < xmin) xmin = t;
		if (t > xmax) xmax = t;
		t = r * sinEnd;
		if (t < ymin) ymin = t;
		if (t > ymax) ymax = t;

		// Normalization
		xmin /= TRIG_MAX_RATIO;
		xmax /= TRIG_MAX_RATIO;
		ymin /= TRIG_MAX_RATIO;
		ymax /= TRIG_MAX_RATIO;

		// Corrections if arc crosses X or Y axis
		if ((start_angle < angle_90) && (end_angle > angle_90)) {
			ymax = radius;
		}

		if ((start_angle < angle_180) && (end_angle > angle_180)) {
			xmin = -radius;
		}

		if ((start_angle < angle_270) && (end_angle > angle_270)) {
			ymin = -radius;
		}

		// Slopes for the two sides of the arc
		float sslope = (float)cosStart/ (float)sinStart;
		float eslope = (float)cosEnd / (float)sinEnd;

		if (end_angle == TRIG_MAX_ANGLE) eslope = -1000000;

		int ir2 = (radius - thickness) * (radius - thickness);
		int or2 = radius * radius;

		graphics_context_set_stroke_color(ctx, c);

		for (int x = xmin; x <= xmax; x++) {
			for (int y = ymin; y <= ymax; y++)
			{
				int x2 = x * x;
				int y2 = y * y;

				if (
						(x2 + y2 < or2 && x2 + y2 >= ir2) && (
							(y > 0 && start_angle < angle_180 && x <= y * sslope) ||
							(y < 0 && start_angle > angle_180 && x >= y * sslope) ||
							(y < 0 && start_angle <= angle_180) ||
							(y == 0 && start_angle <= angle_180 && x < 0) ||
							(y == 0 && start_angle == 0 && x > 0)
							) && (
								(y > 0 && end_angle < angle_180 && x >= y * eslope) ||
								(y < 0 && end_angle > angle_180 && x <= y * eslope) ||
								(y > 0 && end_angle >= angle_180) ||
								(y == 0 && end_angle >= angle_180 && x < 0) ||
								(y == 0 && start_angle == 0 && x > 0)
								)
				   )
					graphics_draw_pixel(ctx, GPoint(center.x+x, center.y+y));
			}
		}
	}
}
static void calc_angles(struct tm *t) {
//	seconds_a1 = TRIG_MAX_ANGLE * t->tm_sec / 60 - angle_90;
//	seconds_a2 = -angle_90;

	minutes_a1 = TRIG_MAX_ANGLE * t->tm_min / 60 - angle_90;
	minutes_a2 = -angle_90;

	if(!clock_is_24h_style()) {
		hour_a1 = (TRIG_MAX_ANGLE * ((60*(t->tm_hour%12)) + t->tm_min) / (60*12)) - angle_90;
	} else {
		hour_a1 = (TRIG_MAX_ANGLE * ((60*t->tm_hour) + t->tm_min) / (60*24)) - angle_90;
	}
	hour_a2 = -angle_90;
}

void handle_battery(BatteryChargeState charge_state) {
	if (charge_state.is_charging) {
		battery_a1 = 1-angle_90;
		battery_a2 = -angle_90;
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Battery charging...");
		layer_set_hidden(bitmap_layer_get_layer(s_bitmap_layer), false);
	} else {
		battery_a1 = TRIG_MAX_ANGLE * charge_state.charge_percent / 100 - angle_90;
		battery_a2 = -angle_90;
		if(charge_state.charge_percent == 0) {
			battery_a1 = 1-angle_90;
		}
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Battery_a1: %d", (int)battery_a1);
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Charge_percent: %d", charge_state.charge_percent);
		battery_level = charge_state.charge_percent;
		layer_set_hidden(bitmap_layer_get_layer(s_bitmap_layer), true);
		layer_mark_dirty(battery_display_layer);
	}
}

//void second_display_layer_update_callback(Layer *me, GContext* ctx) {
//	GRect rect = layer_get_frame(me);
//	GPoint center = grect_center_point(&rect);
//
//#ifdef PBL_COLOR
//	time_t temp = time(NULL);
//	struct tm *t = localtime(&temp);
//	GColor front = (GColor8){.argb=colors[t->tm_sec]};
//#else
//	GColor front = GColorWhite;
//#endif
//
//	graphics_context_set_fill_color(ctx, front);
//
//	graphics_fill_circle(ctx, center, seconds_circle_outer_radius);
//
//	graphics_context_set_fill_color(ctx, GColorBlack);
//	graphics_fill_circle(ctx, center, seconds_circle_inner_radius);
//	graphics_draw_arc(ctx, center, seconds_circle_outer_radius+1, SECONDS_CIRCLE_THICKNESS+2, seconds_a1, seconds_a2, GColorBlack);
//}

void battery_display_layer_update_callback(Layer *me, GContext* ctx) {
	GRect rect = layer_get_frame(me);
	GPoint center = grect_center_point(&rect);

#ifdef PBL_COLOR
	GColor front;
	if(battery_level < 10) {
		front = GColorRed;
	} else if (battery_level < 20) {
		front = GColorYellow;
	} else {
		front = GColorGreen;
	}
#else
	GColor front = GColorWhite;
#endif

	graphics_context_set_fill_color(ctx, front);

	graphics_fill_circle(ctx, center, battery_circle_outer_radius);

	graphics_context_set_fill_color(ctx, GColorBlack);
	graphics_fill_circle(ctx, center, battery_circle_inner_radius);
	graphics_draw_arc(ctx, center, battery_circle_outer_radius+1, BATTERY_CIRCLE_THICKNESS+2, battery_a1, battery_a2, GColorBlack);
}

void minute_display_layer_update_callback(Layer *me, GContext* ctx) {
	GRect rect = layer_get_frame(me);
	GPoint center = grect_center_point(&rect);

#ifdef PBL_COLOR
	time_t temp = time(NULL);
	struct tm *t = localtime(&temp);
	GColor front = (GColor8){.argb=colors[t->tm_min]};
#else
	GColor front = GColorWhite;
#endif

	graphics_context_set_fill_color(ctx, front);

	graphics_fill_circle(ctx, center, minutes_circle_outer_radius);

	graphics_context_set_fill_color(ctx, GColorBlack);
	graphics_fill_circle(ctx, center, minutes_circle_inner_radius);
	graphics_draw_arc(ctx, center, minutes_circle_outer_radius+1, MINUTES_CIRCLE_THICKNESS+2, minutes_a1, minutes_a2, GColorBlack);
}

void hour_display_layer_update_callback(Layer *me, GContext* ctx) {
	GRect rect = layer_get_frame(me);
	GPoint center = grect_center_point(&rect);

#ifdef PBL_COLOR
	time_t temp = time(NULL);
	struct tm *t = localtime(&temp);
	int index = t->tm_hour;
	if(!clock_is_24h_style()) {
		index = index * 2;
	}

	GColor front = (GColor8){.argb=colors_hours[t->tm_hour]};
#else
	GColor front = GColorWhite;
#endif

	graphics_context_set_fill_color(ctx, front);

	graphics_fill_circle(ctx, center, hours_circle_outer_radius);

	graphics_context_set_fill_color(ctx, GColorBlack);
	graphics_fill_circle(ctx, center, hours_circle_inner_radius);
	graphics_draw_arc(ctx, center, hours_circle_outer_radius+1, HOURS_CIRCLE_THICKNESS+2, hour_a1, hour_a2, GColorBlack);

#ifdef SEE_VISUAL_MARKS
	graphics_context_set_stroke_color(ctx, GColorWhite);
	graphics_draw_line(ctx, GPoint((window_x/2), 0), GPoint((window_x/2),window_y));
	graphics_draw_line(ctx, GPoint((window_x/2)+15, 0), GPoint((window_x/2)+15,window_y));
	graphics_draw_line(ctx, GPoint(0, window_y/2), GPoint(window_x,window_y/2));
	graphics_draw_line(ctx, GPoint(0, (window_y/2)-25), GPoint(window_x,(window_y/2)-25));
	graphics_draw_line(ctx, GPoint(0, (window_y/2)+25), GPoint(window_x,(window_y/2)+25));
#endif
}

static void main_window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);

#ifdef SEE_VISUAL_MARKS
	window_x = (layer_get_frame(window_layer).size.w);
	window_y = layer_get_frame(window_layer).size.h;
#endif

	int battery_charging_x = layer_get_frame(window_layer).size.w - BATTERY_CHARGING_W;
	s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CHARGING);
	s_bitmap_layer = bitmap_layer_create(GRect(battery_charging_x, BATTERY_CHARGING_Y, BATTERY_CHARGING_W, BATTERY_CHARGING_H));
	bitmap_layer_set_bitmap(s_bitmap_layer, s_bitmap);
	layer_set_hidden(bitmap_layer_get_layer(s_bitmap_layer), true);
	layer_add_child(window_layer, bitmap_layer_get_layer(s_bitmap_layer));

	GRect temp;
	// Init the layer for the battery display
	battery_display_layer = layer_create(layer_get_frame(window_layer));
	temp = layer_get_frame(battery_display_layer);
	gpath_move_to(battery_segment_path, grect_center_point(&temp));
	layer_set_update_proc(battery_display_layer, &battery_display_layer_update_callback);
	layer_add_child(window_layer, battery_display_layer);

	// Init the layer for the second display
	//second_display_layer = layer_create(layer_get_frame(window_layer));
	//temp = layer_get_frame(second_display_layer);
	//gpath_move_to(second_segment_path, grect_center_point(&temp));
	//layer_set_update_proc(second_display_layer, &second_display_layer_update_callback);
	//layer_add_child(window_layer, second_display_layer);

	// Init the layer for the minute display
	minute_display_layer = layer_create(layer_get_frame(window_layer));
	temp = layer_get_frame(minute_display_layer);
	gpath_move_to(minute_segment_path, grect_center_point(&temp));
	layer_set_update_proc(minute_display_layer, &minute_display_layer_update_callback);
	layer_add_child(window_layer, minute_display_layer);

	// Init the layer for the hour display
	hour_display_layer = layer_create(layer_get_frame(window_layer));
	temp = layer_get_frame(hour_display_layer);
	gpath_move_to(hour_segment_path, grect_center_point(&temp));
	layer_set_update_proc(hour_display_layer, &hour_display_layer_update_callback);
	layer_add_child(window_layer, hour_display_layer);

	s_hour_layer = text_layer_create(GRect(HOUR_X, HOUR_Y, HOUR_W, HOUR_H));
	text_layer_set_background_color(s_hour_layer, GColorClear);
	text_layer_set_text_color(s_hour_layer, GColorWhite);
	text_layer_set_font(s_hour_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
	text_layer_set_text_alignment(s_hour_layer, GTextAlignmentRight);
	layer_add_child(window_layer, text_layer_get_layer(s_hour_layer));

	s_minute_layer = text_layer_create(GRect(MINUTE_X, MINUTE_Y, MINUTE_W, MINUTE_H));
	text_layer_set_background_color(s_minute_layer, GColorClear);
	text_layer_set_text_color(s_minute_layer, GColorWhite);
	text_layer_set_font(s_minute_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	text_layer_set_text_alignment(s_minute_layer, GTextAlignmentLeft);
	layer_add_child(window_layer, text_layer_get_layer(s_minute_layer));

	s_weekday_layer = text_layer_create(GRect(WEEKDAY_X, WEEKDAY_Y, WEEKDAY_W, WEEKDAY_H));
	text_layer_set_background_color(s_weekday_layer, GColorClear);
	text_layer_set_text_color(s_weekday_layer, GColorWhite);
	text_layer_set_font(s_weekday_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	text_layer_set_text_alignment(s_weekday_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(s_weekday_layer));

	s_day_and_month_layer = text_layer_create(GRect(DAY_AND_MONTH_X, DAY_AND_MONTH_Y, DAY_AND_MONTH_W, DAY_AND_MONTH_H));
	text_layer_set_background_color(s_day_and_month_layer, GColorClear);
	text_layer_set_text_color(s_day_and_month_layer, GColorWhite);
	text_layer_set_font(s_day_and_month_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
	text_layer_set_text_alignment(s_day_and_month_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(s_day_and_month_layer));

	battery_state_service_subscribe(handle_battery);
}

static void main_window_unload(Window *window) {
	// Destroy dinamically created stuff
	//layer_destroy(second_display_layer);
	layer_destroy(minute_display_layer);
	layer_destroy(hour_display_layer);
	layer_destroy(battery_display_layer);
	text_layer_destroy(s_minute_layer);
	text_layer_destroy(s_hour_layer);
	text_layer_destroy(s_weekday_layer);
	text_layer_destroy(s_day_and_month_layer);
	bitmap_layer_destroy(s_bitmap_layer);
	gbitmap_destroy(s_bitmap);
}

void set_day_and_month(struct tm *t) {
	static char day_and_month_buffer[] = "24 Mar";
	strftime(day_and_month_buffer, sizeof(day_and_month_buffer), "%d %b", t);
#ifdef PBL_COLOR
	GColor front = (GColor8){.argb=colors_hours[t->tm_mon*2]};
	text_layer_set_text_color(s_day_and_month_layer, front);
#else
	text_layer_set_text_color(s_day_and_month_layer, GColorWhite);
#endif
	text_layer_set_text(s_day_and_month_layer, day_and_month_buffer);
}

void set_weekday(struct tm *t) {
	static char weekday_buffer[] = "fri";
	snprintf(weekday_buffer, sizeof(weekday_buffer), "%s", weekdays[current_language][t->tm_wday]);
#ifdef PBL_COLOR
	GColor front = (GColor8){.argb=colors_weekdays[t->tm_wday]};
	text_layer_set_text_color(s_weekday_layer, front);
#else
	text_layer_set_text_color(s_weekday_layer, GColorWhite);
#endif
	text_layer_set_text(s_weekday_layer, weekday_buffer);
}

void set_hour_and_minutes(struct tm *t) {
	static char buffer[] = "00";
	if(!clock_is_24h_style()) {
		strftime(buffer, sizeof("00"), "%I", t);
	} else {
		strftime(buffer, sizeof("00"), "%H", t);
	}
#ifdef PBL_COLOR
	int index = t->tm_hour;
	if(!clock_is_24h_style()) {
		index = index * 2;
	}

	GColor front_hour = (GColor8){.argb=colors_hours[t->tm_hour]};
	text_layer_set_text_color(s_hour_layer, front_hour);
#else
	text_layer_set_text_color(s_hour_layer, GColorWhite);
#endif
	text_layer_set_text(s_hour_layer, buffer);

	static char buffer_mins[] = "00";
	//static int dd = 0;
	strftime(buffer_mins, sizeof("00"), "%M", t);
	//snprintf(buffer_mins,sizeof("00"), "%0d", dd);
	//dd++;
	//if(dd > 59) dd = 0;
#ifdef PBL_COLOR
	GColor front = (GColor8){.argb=colors[t->tm_min]};
	text_layer_set_text_color(s_minute_layer, front);
#else
	text_layer_set_text_color(s_minute_layer, GColorWhite);
#endif
	text_layer_set_text(s_minute_layer, buffer_mins);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	time_t temp = time(NULL);

	struct tm *t = localtime(&temp);
	calc_angles(t);

//	layer_mark_dirty(second_display_layer);
	layer_mark_dirty(minute_display_layer);
	layer_mark_dirty(hour_display_layer);
	layer_mark_dirty(battery_display_layer);

	set_hour_and_minutes(t);
	set_weekday(t);
	set_day_and_month(t);
	handle_battery(battery_state_service_peek());
}

static void init_radii(void) {
	battery_circle_inner_radius = battery_circle_outer_radius - BATTERY_CIRCLE_THICKNESS;

	minutes_circle_outer_radius = battery_circle_inner_radius - CIRCLE_SPACE;
	minutes_circle_inner_radius = minutes_circle_outer_radius - MINUTES_CIRCLE_THICKNESS;

	hours_circle_outer_radius = minutes_circle_inner_radius - CIRCLE_SPACE;
	hours_circle_inner_radius = hours_circle_outer_radius - HOURS_CIRCLE_THICKNESS;
}

static void init() {
	// Create main Window element and assign to pointer
	s_main_window = window_create();
	window_set_background_color(s_main_window, GColorBlack);

//	second_segment_path = gpath_create(&SECOND_SEGMENT_PATH_POINTS);
	minute_segment_path = gpath_create(&MINUTE_SEGMENT_PATH_POINTS);
	hour_segment_path = gpath_create(&HOUR_SEGMENT_PATH_POINTS);
	battery_segment_path = gpath_create(&BATTERY_SEGMENT_PATH_POINTS);

	init_radii();

	// Set handlers to manage the elements inside the Window
	window_set_window_handlers(s_main_window, (WindowHandlers) {
			.load = main_window_load,
			.unload = main_window_unload
	});

	// Show the Window on the watch, with animated=true
	window_stack_push(s_main_window, true);
	// Register with TickTimerService
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
	// Destroy Window
	window_destroy(s_main_window);
	gpath_destroy(minute_segment_path);
	gpath_destroy(hour_segment_path);
	gpath_destroy(battery_segment_path);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}
