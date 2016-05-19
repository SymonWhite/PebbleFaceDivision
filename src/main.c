#include <pebble.h>

const double C_PI = 3.1415926535897932384626433832795;

static Window *s_main_window;
static TextLayer *s_time_layer;
static Layer *s_circle_layer;
static Layer *s_analog_layer;
//static Layer *s_digital_layer;
static TextLayer *s_date_layer;
static Layer *s_hour_line_layer;
static Layer *s_minute_line_layer;
static Layer *s_clockhand_layer;

static void init();
static void deinit();
static void main_window_load(Window*);
static void main_window_unload(Window*);
static void tick_handler(struct tm*, TimeUnits);
static void update_time();
static void circle_layer_update_proc(Layer*, GContext*);
static void analog_layer_update_proc(Layer*, GContext*);
//static void digital_layer_update_proc(Layer*, GContext*);
static void hour_line_layer_update_proc(Layer*, GContext*);
static void minute_line_layer_update_proc(Layer*, GContext*);
static void clockhand_layer_update_proc(Layer*, GContext*);

static GPoint* getLinePoints (int32_t, int, int);
static void getCirleLinePoints(int, int, int);

int main(void) {
    init();
    app_event_loop();
    deinit();
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "i=%d", i); 
}

static void init() {
    // Create main Window element and assign to pointer
    s_main_window = window_create();

    // Set handlers to manage the elements inside the Window
    window_set_window_handlers(
        s_main_window, 
        (WindowHandlers) {
            .load = main_window_load,
            .unload = main_window_unload
        });

    // Show the Window on the watch, with animated=true
    bool animated = true;
    window_stack_push(s_main_window, animated);

    // Make sure the time is displayed from the start
    update_time();

    // Register with TickTimerService
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
    //Destroy Window
    window_destroy(s_main_window);
}

static void main_window_load(Window *window) {
    window_set_background_color(window, GColorBlack);
    
    // Get information about the Window
    Layer *window_layer = window_get_root_layer(window);
    GRect window_layer_bounds = layer_get_bounds(window_layer);

    // Create layer
    s_circle_layer  = layer_create(window_layer_bounds);
    s_analog_layer  = layer_create(window_layer_bounds);
    //s_digital_layer = layer_create(window_layer_bounds);
    s_date_layer  = text_layer_create(
        GRect(0, PBL_IF_ROUND_ELSE(100, 100), window_layer_bounds.size.w, 50)
    );

    // Improve the layout to be more like a watchface
    text_layer_set_background_color(s_date_layer, GColorClear);
    text_layer_set_text_color(s_date_layer, GColorWhite);
    text_layer_set_text(s_date_layer, "00.00 KW00");
    text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
    text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
    
    // Create the TextLayer with specific bounds
    s_time_layer = text_layer_create(
        GRect(0, PBL_IF_ROUND_ELSE(30, 30), window_layer_bounds.size.w, 50)
    );

    // Improve the layout to be more like a watchface
    text_layer_set_background_color(s_time_layer, GColorClear);
    text_layer_set_text_color(s_time_layer, GColorWhite);
    text_layer_set_text(s_time_layer, "00:00");
    text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

    // Assign the custom drawing procedure
    layer_set_update_proc(s_circle_layer, circle_layer_update_proc);
    layer_set_update_proc(s_analog_layer, analog_layer_update_proc);
    //layer_set_update_proc(s_digital_layer, digital_layer_update_proc);
    //layer_set_update_proc(s_date_layer, date_layer_update_proc);

    // Add it as a child layer to the Window's root layer
    layer_add_child(window_layer, s_circle_layer);
    layer_add_child(window_layer, s_analog_layer);
    //layer_add_child(window_layer, s_date_layer);
    //layer_add_child(window_layer, s_digital_layer);
    
    layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
    layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

    // Redraw this as soon as possible
    layer_mark_dirty(s_circle_layer);
    layer_mark_dirty(s_analog_layer);
    //layer_mark_dirty(s_date_layer);
    //layer_mark_dirty(s_digital_layer);
    
    //update_time();
}

static void main_window_unload(Window *window) {
    // Destroy TextLayer
    text_layer_destroy(s_time_layer);
    text_layer_destroy(s_date_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    update_time();
}

static void update_time() {
    // Get a tm structure
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    // Write the currenz hours and minutes info a buffer
    static char s_date_buffer[14];
    strftime(s_date_buffer, sizeof(s_date_buffer), "%d.%m KW%U", tick_time);

    //Display this time on the TextLayer
    text_layer_set_text(s_date_layer, s_date_buffer);
    
    // Write the currenz hours and minutes info a buffer
    static char s_time_buffer[8];
    strftime(
        s_time_buffer, 
        sizeof(s_time_buffer), 
        clock_is_24h_style() ? "%H:%M" : "%I:%M", 
        tick_time
    );

    //Display this time on the TextLayer
    text_layer_set_text(s_time_layer, s_time_buffer);
}

static void circle_layer_update_proc(Layer *layer, GContext *ctx) {
    GPoint center = GPoint(90, 90);

    //outterCircle
    uint16_t outter_radius = 86;
    uint8_t outter_stroke_width = 7;
    // Set the line color
    //graphics_context_set_stroke_color(ctx, GColorOrange);
    graphics_context_set_stroke_color(ctx, GColorOrange);
    // Set the line width
    graphics_context_set_stroke_width(ctx, outter_stroke_width);
    //Draw a circle
    graphics_draw_circle(ctx, center, outter_radius);

    //innerCircle
    /*
    uint16_t inner_radius = 75;
    uint8_t inner_stroke_width = 1;
    graphics_context_set_stroke_color(ctx, GColorDarkCandyAppleRed);
    graphics_context_set_stroke_width(ctx, inner_stroke_width);
    graphics_draw_circle(ctx, center, inner_radius);
    */
}

static GPoint* getLinePoints (int32_t angle, int start_radius, int end_radius) {
    static GPoint pointArray[2];
    pointArray[0] = GPoint(
        (int16_t)(sin_lookup(angle) * (int32_t)start_radius / TRIG_MAX_RATIO) + 90,
        (int16_t)(-cos_lookup(angle) * (int32_t)start_radius / TRIG_MAX_RATIO) + 90
    );
    pointArray[1] = GPoint(
        (int16_t)(sin_lookup(angle) * (int32_t)end_radius / TRIG_MAX_RATIO) + 90,
        (int16_t)(-cos_lookup(angle) * (int32_t)end_radius / TRIG_MAX_RATIO) + 90
    );
    return pointArray;
}

static void getCirleLinePoints(int lineCount, int start_radius, int end_radius) {
    for (int i = 0; i < lineCount; i++) {
        int32_t angle = TRIG_MAX_ANGLE * i / lineCount;
        GPoint *point_array = getLinePoints(angle, start_radius, end_radius);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "graphics_draw_line(ctx, GPoint(%d,%d), GPoint(%d,%d));", (int)point_array[0].x, (int)point_array[0].y, (int)point_array[1].x, (int)point_array[1].y);
    }
}

static void analog_layer_update_proc(Layer *layer, GContext *ctx) {
    GRect layer_bounds = layer_get_bounds(layer);
    
    // Create layer
    s_hour_line_layer = layer_create(layer_bounds);
    s_minute_line_layer = layer_create(layer_bounds);
    s_clockhand_layer = layer_create(layer_bounds);
    
    // Assign the custom drawing procedure
    layer_set_update_proc(s_hour_line_layer, hour_line_layer_update_proc);
    layer_set_update_proc(s_minute_line_layer, minute_line_layer_update_proc);
    layer_set_update_proc(s_clockhand_layer, clockhand_layer_update_proc);
    
    // Add it as a child layer to the Window's root layer
    layer_add_child(layer, s_hour_line_layer);
    layer_add_child(layer, s_minute_line_layer);
    layer_add_child(layer, s_clockhand_layer);
    
    // Redraw this as soon as possible
    layer_mark_dirty(s_hour_line_layer);
    layer_mark_dirty(s_minute_line_layer);
    layer_mark_dirty(s_clockhand_layer);
}
/*
static void digital_layer_update_proc(Layer* layer, GContext* ctx) {
    GRect layer_bounds = layer_get_bounds(layer);
    
    // Create the TextLayer with specific bounds
    s_time_layer = text_layer_create(
        GRect(0, PBL_IF_ROUND_ELSE(58, 52), layer_bounds.size.w, 50)
    );
    // Create layer
    
    // Improve the layout to be more like a watchface
    text_layer_set_background_color(s_time_layer, GColorClear);
    text_layer_set_text_color(s_time_layer, GColorOrange);
    text_layer_set_text(s_time_layer, "00:00");
    text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
    
    // Assign the custom drawing procedure
    
    // Add it as a child layer to the Window's root layer
    layer_add_child(layer, text_layer_get_layer(s_time_layer));
    
    // Redraw this as soon as possible
    layer_mark_dirty(text_layer_get_layer(s_time_layer));
}
*/
static void hour_line_layer_update_proc(Layer *layer, GContext *ctx) {
    uint8_t stroke_width_3 = 3;
    /** @see getCirleLinePoints(60, 90, 77); */
    graphics_context_set_stroke_color(ctx, GColorOrange); // Set the line color
    graphics_context_set_stroke_width(ctx, stroke_width_3); // Set the line width
    
    graphics_draw_line(ctx, GPoint(90,0), GPoint(90,13)); // Draw a line
    graphics_draw_line(ctx, GPoint(134,13), GPoint(128,24));
    graphics_draw_line(ctx, GPoint(167,45), GPoint(156,52));
    graphics_draw_line(ctx, GPoint(180,90), GPoint(167,90));
    graphics_draw_line(ctx, GPoint(167,134), GPoint(156,128));
    graphics_draw_line(ctx, GPoint(135,167), GPoint(128,156));
    graphics_draw_line(ctx, GPoint(90,180), GPoint(90,167));
    graphics_draw_line(ctx, GPoint(46,167), GPoint(52,156));
    graphics_draw_line(ctx, GPoint(13,135), GPoint(24,128));
    graphics_draw_line(ctx, GPoint(0,90), GPoint(13,90));
    graphics_draw_line(ctx, GPoint(13,46), GPoint(24,52));
    graphics_draw_line(ctx, GPoint(45,13), GPoint(52,24));
}

static void minute_line_layer_update_proc(Layer* layer, GContext* ctx) {
    uint8_t stroke_width_1 = 1;
    /** @see getCirleLinePoints(12, 90, 77); */
    graphics_context_set_stroke_color(ctx, GColorOrange); // Set the line color
    graphics_context_set_stroke_width(ctx, stroke_width_1); // Set the line width
    
    graphics_draw_line(ctx, GPoint(99,1), GPoint(98,14)); // Draw a line
    graphics_draw_line(ctx, GPoint(99,1), GPoint(98,14));
    graphics_draw_line(ctx, GPoint(108,3), GPoint(106,15));
    graphics_draw_line(ctx, GPoint(117,5), GPoint(113,17));
    graphics_draw_line(ctx, GPoint(126,8), GPoint(121,20));
    graphics_draw_line(ctx, GPoint(142,18), GPoint(135,28));
    graphics_draw_line(ctx, GPoint(150,24), GPoint(141,33));
    graphics_draw_line(ctx, GPoint(156,30), GPoint(147,39));
    graphics_draw_line(ctx, GPoint(162,38), GPoint(152,45));
    graphics_draw_line(ctx, GPoint(172,54), GPoint(160,59));
    graphics_draw_line(ctx, GPoint(175,63), GPoint(163,67));
    graphics_draw_line(ctx, GPoint(177,72), GPoint(165,74));
    graphics_draw_line(ctx, GPoint(179,81), GPoint(166,82));
    graphics_draw_line(ctx, GPoint(179,99), GPoint(166,98));
    graphics_draw_line(ctx, GPoint(177,108), GPoint(165,106));
    graphics_draw_line(ctx, GPoint(175,117), GPoint(163,113));
    graphics_draw_line(ctx, GPoint(172,126), GPoint(160,121));
    graphics_draw_line(ctx, GPoint(162,142), GPoint(152,135));
    graphics_draw_line(ctx, GPoint(156,150), GPoint(147,141));
    graphics_draw_line(ctx, GPoint(150,156), GPoint(141,147));
    graphics_draw_line(ctx, GPoint(142,162), GPoint(135,152));
    graphics_draw_line(ctx, GPoint(126,172), GPoint(121,160));
    graphics_draw_line(ctx, GPoint(117,175), GPoint(113,163));
    graphics_draw_line(ctx, GPoint(108,177), GPoint(106,165));
    graphics_draw_line(ctx, GPoint(99,179), GPoint(98,166));
    graphics_draw_line(ctx, GPoint(81,179), GPoint(82,166));
    graphics_draw_line(ctx, GPoint(72,177), GPoint(74,165));
    graphics_draw_line(ctx, GPoint(63,175), GPoint(67,163));
    graphics_draw_line(ctx, GPoint(54,172), GPoint(59,160));
    graphics_draw_line(ctx, GPoint(38,162), GPoint(45,152));
    graphics_draw_line(ctx, GPoint(30,156), GPoint(39,147));
    graphics_draw_line(ctx, GPoint(24,150), GPoint(33,141));
    graphics_draw_line(ctx, GPoint(18,142), GPoint(28,135));
    graphics_draw_line(ctx, GPoint(8,126), GPoint(20,121));
    graphics_draw_line(ctx, GPoint(5,117), GPoint(17,113));
    graphics_draw_line(ctx, GPoint(3,108), GPoint(15,106));
    graphics_draw_line(ctx, GPoint(1,99), GPoint(14,98));
    graphics_draw_line(ctx, GPoint(1,81), GPoint(14,82));
    graphics_draw_line(ctx, GPoint(3,72), GPoint(15,74));
    graphics_draw_line(ctx, GPoint(5,63), GPoint(17,67));
    graphics_draw_line(ctx, GPoint(8,54), GPoint(20,59));
    graphics_draw_line(ctx, GPoint(18,38), GPoint(28,45));
    graphics_draw_line(ctx, GPoint(24,30), GPoint(33,39));
    graphics_draw_line(ctx, GPoint(30,24), GPoint(39,33));
    graphics_draw_line(ctx, GPoint(38,18), GPoint(45,28));
    graphics_draw_line(ctx, GPoint(54,8), GPoint(59,20));
    graphics_draw_line(ctx, GPoint(63,5), GPoint(67,17));
    graphics_draw_line(ctx, GPoint(72,3), GPoint(74,15));
    graphics_draw_line(ctx, GPoint(81,1), GPoint(82,14));
}

static void clockhand_layer_update_proc(Layer* layer, GContext* ctx) {
    // Get a tm structure
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);
    uint8_t stroke_width_3 = 3;
    
    int32_t minute_angle = TRIG_MAX_ANGLE * tick_time->tm_min / 60;
    GPoint *minute_point_array = getLinePoints(minute_angle, 0, 73);
    // Set the line color
    graphics_context_set_stroke_color(ctx, GColorOrange);
    // Set the line width
    graphics_context_set_stroke_width(ctx, stroke_width_3);
    // Draw a line
    graphics_draw_line(ctx, minute_point_array[0] , minute_point_array[1]);
    
    int hour = tick_time->tm_hour;
    if (hour > 12) {
        hour = hour - 12;
    }
    int32_t angle_hour = (TRIG_MAX_ANGLE * hour / 12) + ((TRIG_MAX_ANGLE/12) * tick_time->tm_min /60);
    //int32_t angle_hour = (TRIG_MAX_ANGLE * hour / 12);
    GPoint *hour_point_array = getLinePoints(angle_hour, 0, 60);
    // Set the line color
    graphics_context_set_stroke_color(ctx, GColorOrange);
    // Set the line width
    graphics_context_set_stroke_width(ctx, stroke_width_3);
    // Draw a line
    graphics_draw_line(ctx, hour_point_array[0] , hour_point_array[1]);
}

