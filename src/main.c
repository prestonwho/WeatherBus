#include <pebble.h>


#define KEY_TEMPERATURE 1
#define KEY_SUN 2
#define KEY_FORECAST_1 3
#define KEY_FORECAST_2 4
#define KEY_FORECAST_3 5
#define KEY_CONDITIONS_0 6
#define KEY_MOON 7


static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_tz1_layer;
static TextLayer *s_tz2_layer;
//static TextLayer *s_date_layer;
//static TextLayer *s_temp_layer;
static TextLayer *s_weather_layer;
static TextLayer *s_weathershadow_layer;
static TextLayer *s_forecast1_layer;
static TextLayer *s_forecast2_layer;
static TextLayer *s_forecast3_layer;
static TextLayer *s_sun_layer;
static TextLayer *s_moon_layer;
//static TextLayer *s_nextbus_layer;
//static TextLayer *s_bus0123_layer;
//static TextLayer *s_battery_layer;
//static TextLayer *s_connection_layer;


static Layer *s_path_layer;

// GPath describes the shape
static GPath *s_path;
//GColor backgroundColor = GColorLightGray;
static GPathInfo PATH_INFO = {
    .num_points = 4,
    .points = (GPoint[]) { {-1, 10}, {145, 10}, {145, 40}, {-1, 40 } }
};


//static TextLayer *s_bus456_layer;

#define MINSCOUNT 22
    
static int mins[] = { 304,
                      340,
                      359,
                      371,
                      382,
                      391,
                      401,
                      411,
                      421,
                      436,
                      456,
                      475,
                      525,

                      904,
                      940,
                      959,
                      973,
                      983,
                      1008,
                      1023,
                      1042,
                      1061 };





//var homeAM = [ '5:05', '5:41', '6:01', '6:13', '6:24', '6:34', '6:44', '6:54', '7:04', '7:19', '7:39', '7:58', '8:48' ];
//var workPM = [ '15:04', '15:40', '15:59', '16:13', '16:23', '16:48', '17:03', '17:22', '17:41' ];"


//static Layer *s_graph_layer;

static GFont s_time_font;
static GFont s_weather_font;

//static BitmapLayer *s_background_layer;
//static GBitmap *s_background_bitmap;

// Create a long-lived buffer
static char buffer[] = "00 00";
static char tz1buffer[] = "00";
static char tz2buffer[] = "00";

static char datebuffer[] = "Sat 2015-05-23";
static char nextbus[] = "99999";
static char bus0123[] = "88:88 (99999)\n88:88 (99999)\n88:88 (99999)\n88:88 (99999)";
static char battery_text[] = "100";
static char bluetooth_buffer[] = " ";


// WEATHER TEXTS
static char temperature_buffer[8];
static char weather_layer_buffer[128];
static char forecast1_layer_buffer[128];
static char forecast2_layer_buffer[128];
static char forecast3_layer_buffer[128];
static char sun_buffer[32];
static char moon_buffer[32];



//static char tempTime[] = "00:00";

//static char bus456[] = "99999\n99999\n99999";

static int etas[MINSCOUNT] = { 0 };


static void update_time() {

    
    
    //int etaIndex = 0;
    //int tempEta = 0;
    
    // temp variables
    int i, j, k, a;
    //int nbHours, nbMinutes;
    
    // Get a tm structure
    time_t temp = time(NULL); 
    struct tm *tick_time = localtime(&temp);
    struct tm *gmt_time = gmtime(&temp);
    
    
    // how many minutes into "today" am I?
    int minOfDay = ((tick_time->tm_hour * 60) + tick_time->tm_min);

    // find the diff between each bus time (min) and the current min
    for(i = 0; i < MINSCOUNT; i ++) {
        etas[i] = mins[i] - minOfDay;
        
        if(etas[i] < 0) { etas[i] += (60 * 24); }
    }
    

    // sort the array of etas
    for (j = 0; j < MINSCOUNT; j ++) {
        for (k = j + 1; k < MINSCOUNT; ++k) {
            if (etas[j] > etas[k]) {
                a =  etas[j];
                etas[j] = etas[k];
                etas[k] = a;
            }
        }
    }

    
    // text-format bus info. I really don't like the bus0123 part... I shouldn't be doing the eta+minOfDay calc
    // so many redundant times. Can easily be solved with another array for arrival times. Not sure if I can just
    // use the original mins[] array, now that I've sorted the etas. Maybe don't sort them, but find the 1st
    // non-negative and start the counter from there and %MINSCOUNT each time it increments?
    
    //if(etas[0] <= 99) {
        //text_layer_set_font(s_nextbus_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
        snprintf(nextbus, sizeof(nextbus), "%d", etas[0]);
    //} else {
        //text_layer_set_font(s_nextbus_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
    //    nbHours = etas[0] / 60;               // integer math; should be rounded down.
    //    nbMinutes = etas[0] - (nbHours * 60); // remove the whole hours leaving just the minutes
    //    snprintf(nextbus, sizeof(nextbus), "%d:%02d", nbHours, nbMinutes);
    //}
    
    snprintf(bus0123, sizeof(bus0123), "%02d:%02d (%d)\n%02d:%02d (%d)\n%02d:%02d (%d)", //\n%02d:%02d (%d)", 
             ((etas[0] + minOfDay - ((etas[0] + minOfDay) % 60)) / 60) % 24, ((etas[0] + minOfDay) % 60), etas[0], 
             ((etas[1] + minOfDay - ((etas[1] + minOfDay) % 60)) / 60) % 24, ((etas[1] + minOfDay) % 60), etas[1],
             ((etas[2] + minOfDay - ((etas[2] + minOfDay) % 60)) / 60) % 24, ((etas[2] + minOfDay) % 60), etas[2]); //,
             //((etas[3] + minOfDay - ((etas[3] + minOfDay) % 60)) / 60) % 24, ((etas[3] + minOfDay) % 60), etas[3]);
    
    //snprintf(bus456, sizeof(bus456), "%d\n%d\n%d\n%d", etas[4], etas[5], etas[6]);


    // Write the current hours and minutes into the buffer
    if(clock_is_24h_style() == true) {
        //Use 2h hour format
        strftime(buffer, sizeof("00 00"), "%H %M", tick_time);
    } else {
        //Use 12 hour format
        strftime(buffer, sizeof("00 00"), "%I %M", tick_time);
    }
    
    strftime(datebuffer, sizeof("Sat 2015-05-23"), "%a %F", tick_time);
    snprintf(tz1buffer, sizeof(tz1buffer), "%02d", (gmt_time->tm_hour));
    snprintf(tz2buffer, sizeof(tz2buffer), "%02d", (tick_time->tm_hour + 3) % 24);
    
    //s_tz1_layer
    
    
    // Display this time on the TextLayer
    text_layer_set_text(s_time_layer, buffer);
    text_layer_set_text(s_tz1_layer, tz1buffer);
    text_layer_set_text(s_tz2_layer, tz2buffer);
    
    //text_layer_set_text(s_date_layer, datebuffer);
    
    
    //text_layer_set_text(s_nextbus_layer, nextbus);
    
    //text_layer_set_text(s_bus0123_layer, bus0123);
    //text_layer_set_text(s_bus456_layer, bus456);



}



static void inbox_received_callback(DictionaryIterator *iterator, void *context) {




    // Read first item
    Tuple *t = dict_read_first(iterator);

    // For all items
    while(t != NULL) {
        // Which key was received?
        switch(t->key) {
            case KEY_TEMPERATURE:
                snprintf(temperature_buffer, sizeof(temperature_buffer), "%d", (int)t->value->int32);
                break;
            
            case KEY_FORECAST_1:
                snprintf(forecast1_layer_buffer, sizeof(forecast1_layer_buffer), "%s", t->value->cstring);
                break;
            
            case KEY_FORECAST_2:
                snprintf(forecast2_layer_buffer, sizeof(forecast2_layer_buffer), "%s", t->value->cstring);
                break;
            
            case KEY_FORECAST_3:
                snprintf(forecast3_layer_buffer, sizeof(forecast3_layer_buffer), "%s", t->value->cstring);
                break;
            
            case KEY_CONDITIONS_0:
                snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s", t->value->cstring);
                break;
            
            case KEY_SUN:
                snprintf(sun_buffer, sizeof(sun_buffer), "%s", t->value->cstring);
                break;
            
            case KEY_MOON:
                snprintf(moon_buffer, sizeof(moon_buffer), "%s", t->value->cstring);
                break;
            
            default:
                APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
                break;
        }

        // Look for next item
        t = dict_read_next(iterator);
    }

    // Assemble full strings and display
    //text_layer_set_text(s_temp_layer, temperature_buffer);

    
    layer_mark_dirty(s_path_layer);

    
    text_layer_set_text(s_weather_layer, weather_layer_buffer);
    text_layer_set_text(s_weathershadow_layer, weather_layer_buffer);

    text_layer_set_text(s_forecast1_layer, forecast1_layer_buffer);
    text_layer_set_text(s_forecast2_layer, forecast2_layer_buffer);
    text_layer_set_text(s_forecast3_layer, forecast3_layer_buffer);

    text_layer_set_text(s_sun_layer, sun_buffer);
    text_layer_set_text(s_moon_layer, moon_buffer);

}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}


static void handle_battery(BatteryChargeState charge_state) {
    

    if (charge_state.is_charging) {
        snprintf(battery_text, sizeof(battery_text), "%02d+", charge_state.charge_percent);
    } else {
        snprintf(battery_text, sizeof(battery_text), "%02d", charge_state.charge_percent);
    }
    
    layer_mark_dirty(s_path_layer);


    //text_layer_set_text(s_battery_layer, battery_text);
}


static void handle_bluetooth(bool connected) {
    //text_layer_set_text(s_connection_layer, 
    snprintf(bluetooth_buffer, sizeof(bluetooth_buffer), (connected ? "*" : ""));
    
    layer_mark_dirty(s_path_layer);

}


static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    
    update_time();

    // Get weather update every 30 minutes
    if(tick_time->tm_min % 30 == 0) {
        
        // Begin dictionary
        DictionaryIterator *iter;
        app_message_outbox_begin(&iter);

        // Add a key-value pair
        dict_write_uint8(iter, 0, 0);

        // Send the message!
        app_message_outbox_send();
    }
}

static void draw_text_shadowed(GContext * ctx, const char * text, GFont const font,
        const int x, const int y, const int w, const int h, const GTextOverflowMode overflow_mode,
        const GTextAlignment alignment, const GTextLayoutCacheRef layout,
        //const int fuzzN, const int fuzzE, const int fuzzS, const int fuzzW,
        const int fuzzNE, const int fuzzSE, const int fuzzSW, const int fuzzNW,
        GColor fuzzColor, GColor textColor) {
    
    graphics_context_set_text_color(ctx, fuzzColor);
    
    //if(fuzzW)  graphics_draw_text(ctx, text, font, GRect(x-fuzzW,y,w,h), overflow_mode, alignment, layout);
    //if(fuzzE)  graphics_draw_text(ctx, text, font, GRect(x+fuzzE,y,w,h), overflow_mode, alignment, layout);
    //if(fuzzN)  graphics_draw_text(ctx, text, font, GRect(x,y-fuzzN,w,h), overflow_mode, alignment, layout);
    //if(fuzzS)  graphics_draw_text(ctx, text, font, GRect(x,y+fuzzS,w,h), overflow_mode, alignment, layout);
    
    if(fuzzNW) graphics_draw_text(ctx, text, font, GRect(x-fuzzNW,y-fuzzNW,w,h), overflow_mode, alignment, layout);
    if(fuzzNE) graphics_draw_text(ctx, text, font, GRect(x+fuzzNE,y-fuzzNE,w,h), overflow_mode, alignment, layout);
    if(fuzzSE) graphics_draw_text(ctx, text, font, GRect(x+fuzzSE,y+fuzzSE,w,h), overflow_mode, alignment, layout);
    if(fuzzSW) graphics_draw_text(ctx, text, font, GRect(x-fuzzSW,y+fuzzSW,w,h), overflow_mode, alignment, layout);
    
    graphics_context_set_text_color(ctx, textColor);
    graphics_draw_text(ctx, text, font, GRect(x,y,w,h), overflow_mode, alignment, layout);
}


static void layer_update_proc(Layer *layer, GContext *ctx) {
    
    // TOP ROW
    
    
    // Draw the date
    draw_text_shadowed(ctx, datebuffer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
                       0,-5,144,17,
                       GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL,
                       0,1,0,0,
                       GColorBlack, GColorWhite);
    
    // Watch Battery
    draw_text_shadowed(ctx, battery_text, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
                       5,-5,139,15,
                       GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL,
                       0,1,0,0,
                       GColorBlack, GColorWhite);
    
    // Bluetooth Connection
    draw_text_shadowed(ctx, bluetooth_buffer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
                       0,-5,139,17,
                       GTextOverflowModeWordWrap, GTextAlignmentRight, NULL,
                       0,1,0,0,
                       GColorBlack, GColorWhite);
    
    graphics_context_set_stroke_color(ctx, GColorBlack); // GColorFromRGB(255, 0, 0));
    graphics_draw_line(ctx, GPoint(0,11), GPoint(144,11));
    graphics_context_set_stroke_color(ctx, GColorWhite); // GColorFromRGB(255, 0, 0));
    graphics_draw_line(ctx, GPoint(0,10), GPoint(144,10));
    
    
    
    // BUS ROW
    
    
    // Three buses following the next one
    draw_text_shadowed(ctx, bus0123, s_weather_font,
                       1,11,68,47,
                       GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL,
                       0,1,0,0,
                       GColorBlack, GColorWhite);
    
    
    // Next Bus Time
    draw_text_shadowed(ctx, nextbus, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DEJAVU_SANS_COND_47)),
                       0,1,144,41,
                       GTextOverflowModeWordWrap, GTextAlignmentRight, NULL,
                       0,2,0,0,
                       GColorBlack, GColorWhite);
    
    // "erase" the bottom portion of the next bus time.
    graphics_context_set_fill_color(ctx, GColorOxfordBlue);
    graphics_fill_rect(ctx, GRect(0, 41, 144, 10), 0, GCornerNone);
    
    graphics_context_set_stroke_color(ctx, GColorBlack); // GColorFromRGB(255, 0, 0));
    graphics_draw_line(ctx, GPoint(0,41), GPoint(144,41));
    graphics_context_set_stroke_color(ctx, GColorWhite); // GColorFromRGB(255, 0, 0));
    graphics_draw_line(ctx, GPoint(0,40), GPoint(144,40));
    
    
    
    // TIME ROW
    
    
    // Draw the time's background rectangle
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(3, 45, 140, 43), 0, GCornerNone);
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, GRect(2, 46, 140, 43), 0, GCornerNone);
    
    
    graphics_context_set_stroke_color(ctx, GColorBlack); // GColorFromRGB(255, 0, 0));
    graphics_draw_line(ctx, GPoint(0,95), GPoint(144,95));
    graphics_context_set_stroke_color(ctx, GColorWhite); // GColorFromRGB(255, 0, 0));
    graphics_draw_line(ctx, GPoint(0,94), GPoint(144,94));
    
    
    
    // CURRENT WEATHER ROW
    
    
    draw_text_shadowed(ctx, temperature_buffer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DEJAVU_SANS_COND_47)),
                       0,86,144,40,
                       GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL,
                       1,1,1,1,
                       GColorBlack, GColorWhite);
    
    // "erase" the bottom portion of the next bus time.
    graphics_context_set_fill_color(ctx, GColorOxfordBlue);
    graphics_fill_rect(ctx, GRect(0, 126, 144, 10), 0, GCornerNone);
    
    
    graphics_context_set_stroke_color(ctx, GColorBlack); // GColorFromRGB(255, 0, 0));
    graphics_draw_line(ctx, GPoint(0,125), GPoint(144,125));
    graphics_context_set_stroke_color(ctx, GColorWhite); // GColorFromRGB(255, 0, 0));
    graphics_draw_line(ctx, GPoint(0,124), GPoint(144,124));
    
    
    
    // WEATHER FORECAST ROW
    
    
    graphics_context_set_stroke_color(ctx, GColorBlack); // GColorFromRGB(255, 0, 0));
    graphics_draw_line(ctx, GPoint(0,155), GPoint(144,155));
    graphics_context_set_stroke_color(ctx, GColorWhite); // GColorFromRGB(255, 0, 0));
    graphics_draw_line(ctx, GPoint(0,154), GPoint(144,154));

    
    
    // SUN/MOON ROW
    
    
}

static void main_window_load(Window *window) {

    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
    
    // Create second custom font, apply it and add to Window
    s_weather_font = //fonts_get_system_font(FONT_KEY_GOTHIC_14);
        fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DEJAVU_SANS_9));
    
    //Create GBitmap, then set to created BitmapLayer
    //s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
    //s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
    //bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
    //layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));
    
    
    //s_graph_layer = layer_create(GRect(5, 5, 139, 45));
    //layer_set_background_color(s_graph_layer, GColorYellow);

    
    //s_rect1_layer = text_layer_create(GRect(0,0,144,9));
    //text_layer_set_background_color(s_rect1_layer, GColor)
    
    // Create GPath object
    s_path = gpath_create(&PATH_INFO);

    // Create Layer that the path will be drawn on
    s_path_layer = layer_create(bounds);
    layer_set_update_proc(s_path_layer, layer_update_proc);
    layer_add_child(window_layer, s_path_layer);
    
    
    
    
    window_set_background_color(s_main_window, GColorOxfordBlue); //GColorOxfordBlue);
    
    
    // Create time TextLayer
    s_time_layer = text_layer_create(GRect(0, 37, 144, 53));
    text_layer_set_background_color(s_time_layer, GColorClear);
    text_layer_set_text_color(s_time_layer, GColorBlack);
    text_layer_set_overflow_mode(s_time_layer, GTextOverflowModeWordWrap);
    //text_layer_set_text(s_time_layer, "00  00");
    text_layer_set_font(s_time_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DEJAVU_SANS_COND_47))); //fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));  //s_time_font);
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));

    
    s_tz1_layer = text_layer_create(GRect(58, 45, 29, 21));
    text_layer_set_background_color(s_tz1_layer, GColorClear);
    text_layer_set_text_color(s_tz1_layer, GColorBlack);
    //text_layer_set_text(s_tz1_layer, "23");
    text_layer_set_font(s_tz1_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text_alignment(s_tz1_layer, GTextAlignmentCenter);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_tz1_layer));
    
    s_tz2_layer = text_layer_create(GRect(58, 64, 29, 21));
    text_layer_set_background_color(s_tz2_layer, GColorClear);
    text_layer_set_text_color(s_tz2_layer, GColorBlack);
    //text_layer_set_text(s_tz2_layer, "23");
    text_layer_set_font(s_tz2_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text_alignment(s_tz2_layer, GTextAlignmentCenter);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_tz2_layer));
    
    

    handle_battery(battery_state_service_peek());
    handle_bluetooth(bluetooth_connection_service_peek());

    
    //Create GFont
    //s_time_font = fonts_get_system_font(FONT_KEY_LECO_38_BOLD_NUMBERS);
    //    fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_48));
    
/*
    // Next Bus
    s_nextbus_layer = text_layer_create(GRect(0,100,144,50));
    text_layer_set_overflow_mode(s_nextbus_layer, GTextOverflowModeWordWrap);
    text_layer_set_background_color(s_nextbus_layer, GColorClear);
    text_layer_set_text_color(s_nextbus_layer, GColorWhite);
    text_layer_set_text_alignment(s_nextbus_layer, GTextAlignmentRight);
    //text_layer_set_text(s_nextbus_layer, "59");
    text_layer_set_font(s_nextbus_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DEJAVU_SANS_COND_47)));
    //text_layer_set_font(s_nextbus_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_nextbus_layer));
*/

/* 
    // Buses index 1-3
    s_bus0123_layer = text_layer_create(GRect(1,12,68,47));
    text_layer_set_overflow_mode(s_bus0123_layer, GTextOverflowModeWordWrap);
    text_layer_set_background_color(s_bus0123_layer, GColorClear);
    text_layer_set_text_color(s_bus0123_layer, GColorWhite);
    text_layer_set_text_alignment(s_bus0123_layer, GTextAlignmentLeft);
    //text_layer_set_text(s_bus0123_layer, "59");
    text_layer_set_font(s_bus0123_layer, s_weather_font);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_bus0123_layer));
*/
    
    // Buses index 4-6
    //s_bus456_layer = text_layer_create(GRect(86,20,68,47));
    //text_layer_set_overflow_mode(s_bus456_layer, GTextOverflowModeWordWrap);
    //text_layer_set_background_color(s_bus456_layer, GColorClear);
    //text_layer_set_text_color(s_bus456_layer, GColorWhite);
    //text_layer_set_text_alignment(s_bus456_layer, GTextAlignmentCenter);
    //text_layer_set_text(s_nextbus_layer, "59");
    //text_layer_set_font(s_bus456_layer, s_weather_font);
    //layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_bus456_layer));
    
    //Apply to TextLayer
    

    //Apply to TextLayer
    
    
    // Add it as a child layer to the Window's root layer

    
    
    
    // Create current weather shadow Layer
    s_weathershadow_layer = text_layer_create(GRect(1, 94, 144, 30));
    text_layer_set_background_color(s_weathershadow_layer, GColorClear);
    text_layer_set_text_color(s_weathershadow_layer, GColorBlack);
    text_layer_set_text_alignment(s_weathershadow_layer, GTextAlignmentRight);
    //text_layer_set_text(s_weathershadow_layer, "Loading...");
    text_layer_set_font(s_weathershadow_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weathershadow_layer));

    
    // Create current weather Layer
    s_weather_layer = text_layer_create(GRect(0, 93, 144, 30));
    text_layer_set_background_color(s_weather_layer, GColorClear);
    text_layer_set_text_color(s_weather_layer, GColorWhite);
    text_layer_set_text_alignment(s_weather_layer, GTextAlignmentRight);
    //text_layer_set_text(s_weather_layer, "Loading...");
    text_layer_set_font(s_weather_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_layer));


    // Weather Tomorrow
    s_forecast1_layer = text_layer_create(GRect(-10,125,68,47));
    text_layer_set_background_color(s_forecast1_layer, GColorClear);
    text_layer_set_text_color(s_forecast1_layer, GColorWhite);
    text_layer_set_text_alignment(s_forecast1_layer, GTextAlignmentCenter);
    //text_layer_set_text(s_forecast1_layer, "...");
    text_layer_set_font(s_forecast1_layer, s_weather_font);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_forecast1_layer));
    
    // Weather Tomorrow + 1
    s_forecast2_layer = text_layer_create(GRect(38,125,68,47));
    text_layer_set_background_color(s_forecast2_layer, GColorClear);
    text_layer_set_text_color(s_forecast2_layer, GColorWhite);
    text_layer_set_text_alignment(s_forecast2_layer, GTextAlignmentCenter);
    //text_layer_set_text(s_forecast2_layer, "...");
    text_layer_set_font(s_forecast2_layer, s_weather_font);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_forecast2_layer));
    
    // Weather Tomorrow + 2
    s_forecast3_layer = text_layer_create(GRect(86,125,68,47));
    text_layer_set_background_color(s_forecast3_layer, GColorClear);
    text_layer_set_text_color(s_forecast3_layer, GColorWhite);
    text_layer_set_text_alignment(s_forecast3_layer, GTextAlignmentCenter);
    //text_layer_set_text(s_forecast3_layer, "...");
    text_layer_set_font(s_forecast3_layer, s_weather_font);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_forecast3_layer));
    
    
    // Sunrise/Sunset
    s_sun_layer = text_layer_create(GRect(0,152,144,17));
    text_layer_set_background_color(s_sun_layer, GColorClear);
    text_layer_set_text_color(s_sun_layer, GColorWhite);
    text_layer_set_text_alignment(s_sun_layer, GTextAlignmentLeft);
    //text_layer_set_text(s_sun_layer, "00:00 - 00:00");
    text_layer_set_font(s_sun_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_sun_layer));
    
    // Moon
    s_moon_layer = text_layer_create(GRect(0,152,144,17));
    text_layer_set_background_color(s_moon_layer, GColorClear);
    text_layer_set_text_color(s_moon_layer, GColorWhite);
    text_layer_set_text_alignment(s_moon_layer, GTextAlignmentRight);
    //text_layer_set_text(s_moon_layer, "00:00 - 00:00");
    text_layer_set_font(s_moon_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_moon_layer));
    
    
    
    
    
    // Register with Services
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
    battery_state_service_subscribe(handle_battery);
    bluetooth_connection_service_subscribe(handle_bluetooth);
    
    // Register callbacks
    app_message_register_inbox_received(inbox_received_callback);
    app_message_register_inbox_dropped(inbox_dropped_callback);
    app_message_register_outbox_failed(outbox_failed_callback);
    app_message_register_outbox_sent(outbox_sent_callback);

    // Open AppMessage
    app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
    
    
    // Make sure the time is displayed from the start
    update_time();
}

static void main_window_unload(Window *window) {

    tick_timer_service_unsubscribe();
    battery_state_service_unsubscribe();
    bluetooth_connection_service_unsubscribe();
    
    
    // Destroy layer and path
    layer_destroy(s_path_layer);
    gpath_destroy(s_path);

    
    //Destroy GBitmap
    //gbitmap_destroy(s_background_bitmap);

    //Destroy BitmapLayer
    //bitmap_layer_destroy(s_background_layer);

    // Destroy TextLayer
    text_layer_destroy(s_time_layer);
    text_layer_destroy(s_tz1_layer);
    text_layer_destroy(s_tz2_layer);
    //text_layer_destroy(s_date_layer);
    //text_layer_destroy(s_battery_layer);
    //text_layer_destroy(s_connection_layer);    
    //text_layer_destroy(s_nextbus_layer);
    //text_layer_destroy(s_bus0123_layer);
    //text_layer_destroy(s_bus456_layer);

    // Destroy weather elements
    //text_layer_destroy(s_temp_layer);
    text_layer_destroy(s_weather_layer);
    text_layer_destroy(s_weathershadow_layer);
    text_layer_destroy(s_forecast1_layer);
    text_layer_destroy(s_forecast2_layer);
    text_layer_destroy(s_forecast3_layer);
    text_layer_destroy(s_sun_layer);
    text_layer_destroy(s_moon_layer);


//    layer_destroy(s_graph_layer);

    // Destroy custom fonts
    fonts_unload_custom_font(s_time_font);
    fonts_unload_custom_font(s_weather_font);
}





static void init() {
    // Create main Window element and assign to pointer
    s_main_window = window_create();

    // Set handlers to manage the elements inside the Window
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = main_window_load,
        .unload = main_window_unload
    });

    // Show the Window on the watch, with animated=true
    window_stack_push(s_main_window, true);

    
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
