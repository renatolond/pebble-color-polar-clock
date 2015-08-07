#include <pebble.h>

#ifdef PBL_COLOR
	#include "gcolor_definitions.h"
#endif

#include <time.h>

static Window *s_main_window;
static TextLayer *s_time_layer;

const GPathInfo SECOND_SEGMENT_PATH_POINTS = {
  .num_points = 3,
  .points = (GPoint []) {
    {0, 0},
    {-7, -70}, // 70 = radius + fudge; 7 = 70*tan(6 degrees); 6 degrees per minute;
    {7,  -70},
  }
};

GPath *second_segment_path;
Layer *second_display_layer;

static int angle_90 = TRIG_MAX_ANGLE / 4;
static int angle_180 = TRIG_MAX_ANGLE / 2;
static int angle_270 = 3 * TRIG_MAX_ANGLE / 4;

static int outerCircleOuterRadius = 71, outerCircleInnerRadius;
#define OUTER_CIRCLE_THICKNESS 4


#ifdef PBL_COLOR
#define NBR_COLORS 60
static uint8_t colors[NBR_COLORS] = {GColorVividCeruleanARGB8, GColorVividCeruleanARGB8, GColorVividCeruleanARGB8,
	GColorVividCeruleanARGB8, GColorVividCeruleanARGB8, GColorVividCeruleanARGB8,
	GColorVividCeruleanARGB8, GColorVividCeruleanARGB8, GColorVividCeruleanARGB8,
	GColorVividCeruleanARGB8, GColorVividCeruleanARGB8, GColorVividCeruleanARGB8,
	GColorVividCeruleanARGB8, GColorVividCeruleanARGB8, GColorVividCeruleanARGB8,
	GColorVividCeruleanARGB8, GColorVividCeruleanARGB8, GColorVividCeruleanARGB8,
	GColorVividCeruleanARGB8, GColorVividCeruleanARGB8, GColorVividCeruleanARGB8,
	GColorVividCeruleanARGB8, GColorVividCeruleanARGB8, GColorVividCeruleanARGB8,
	GColorVividCeruleanARGB8, GColorVividCeruleanARGB8, GColorVividCeruleanARGB8,
	GColorVividCeruleanARGB8, GColorVividCeruleanARGB8, GColorVividCeruleanARGB8,
	GColorRedARGB8, GColorRedARGB8, GColorRedARGB8,
	GColorRedARGB8, GColorRedARGB8, GColorRedARGB8,
	GColorRedARGB8, GColorRedARGB8, GColorRedARGB8,
	GColorRedARGB8, GColorRedARGB8, GColorRedARGB8,
	GColorRedARGB8, GColorRedARGB8, GColorRedARGB8,
	GColorRedARGB8, GColorRedARGB8, GColorRedARGB8,
	GColorRedARGB8, GColorRedARGB8, GColorRedARGB8,
	GColorRedARGB8, GColorRedARGB8, GColorRedARGB8,
	GColorRedARGB8, GColorRedARGB8, GColorRedARGB8,
	GColorRedARGB8, GColorRedARGB8, GColorRedARGB8,
};
#endif

static int32_t sec_a, sec_a1, sec_a2, hour_a, hour_a1, hour_a2;
static int32_t minutesWidth = TRIG_MAX_ANGLE / 60;
static int32_t hourWidth = TRIG_MAX_ANGLE / 24;

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
static void calcAngles(struct tm *t) {
	if(t->tm_sec == 59) {
		sec_a1 = 0;
		sec_a2 = TRIG_MAX_ANGLE;
	} else {
		sec_a = TRIG_MAX_ANGLE * t->tm_sec / 60 - angle_90;
		sec_a2 = -angle_90;
		sec_a1 = sec_a + minutesWidth;
	}

	hour_a = TRIG_MAX_ANGLE * (60*(t->tm_hour%12)+t->tm_min) / 720 - angle_90;
	hour_a1 = hour_a - hourWidth;
	hour_a2 = hour_a + hourWidth;
}

void second_display_layer_update_callback(Layer *me, GContext* ctx) {
	time_t temp = time(NULL);

	struct tm *t = localtime(&temp);

	GRect rect = layer_get_frame(me);
	GPoint center = grect_center_point(&rect);

	calcAngles(t);

#ifdef PBL_COLOR
	GColor front = (GColor8){.argb=colors[t->tm_sec]};
#else
	GColor front = GColorWhite;
#endif

	graphics_context_set_fill_color(ctx, front);

	graphics_fill_circle(ctx, center, outerCircleOuterRadius);

	graphics_context_set_fill_color(ctx, GColorBlack);
	graphics_fill_circle(ctx, center, outerCircleInnerRadius);
	graphics_draw_arc(ctx, center, outerCircleOuterRadius+1, OUTER_CIRCLE_THICKNESS+2, sec_a1, sec_a2, GColorBlack);
}

static void main_window_load(Window *window) {
	outerCircleInnerRadius = outerCircleOuterRadius - OUTER_CIRCLE_THICKNESS;

	// Init the layer for the second display
	second_display_layer = layer_create(layer_get_frame(window_get_root_layer(window)));
	GRect temp = layer_get_frame(second_display_layer);
	gpath_move_to(second_segment_path, grect_center_point(&temp));
	layer_set_update_proc(second_display_layer, &second_display_layer_update_callback);
	layer_add_child(window_get_root_layer(window), second_display_layer);
}

static void main_window_unload(Window *window) {
	// Destroy TextLayer
	text_layer_destroy(s_time_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	layer_mark_dirty(second_display_layer);
}

static void init() {
	// Create main Window element and assign to pointer
	s_main_window = window_create();

	second_segment_path = gpath_create(&SECOND_SEGMENT_PATH_POINTS);

	// Set handlers to manage the elements inside the Window
	window_set_window_handlers(s_main_window, (WindowHandlers) {
			.load = main_window_load,
			.unload = main_window_unload
	});

	// Show the Window on the watch, with animated=true
	window_stack_push(s_main_window, true);
	// Register with TickTimerService
	tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
}

static void deinit() {
	// Destroy Window
	window_destroy(s_main_window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}
