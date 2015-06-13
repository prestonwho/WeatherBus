#include <pebble.h>


/*#define KEY_TEMPERATURE 1
#define KEY_SUN 2
#define KEY_FORECAST_1 3
#define KEY_FORECAST_2 4
#define KEY_FORECAST_3 5
#define KEY_CONDITIONS_0 6
#define KEY_MOON 7
#define KEY_WEATHER_TS 8
#define KEY_FORECAST_4 9
#define KEY_FORECAST_5 10
#define KEY_FORECAST_6 11
#define KEY_FORECAST_7 12
#define KEY_FORECAST_8 13
#define KEY_FORECAST_9 14
*/
    
#define KEY_WX_TMSTAMP 0
#define KEY_SUN        1
#define KEY_MOON       2
#define KEY_CURR_TEMP  3
#define KEY_CURR_LOC   4
#define KEY_CURR_WX   10
#define KEY_FCAST_1   11
#define KEY_FCAST_2   12
#define KEY_FCAST_3   13
#define KEY_FCAST_4   14
#define KEY_FCAST_5   15
#define KEY_FCAST_6   16
#define KEY_FCAST_7   17
#define KEY_FCAST_8   18
#define KEY_FCAST_9   19


#define NUMBER_OF_SECONDS_TO_SHOW_SECONDS_AFTER_TAP 120
    

static GRect date_bounds[2];
static GRect bus_bounds[2];
static GRect time_bounds[2];
static GRect currentweather_bounds[2];
static GRect forecast123_bounds[2];
static GRect forecast456_bounds[2];
static GRect forecast789_bounds[2];
static GRect loc_bounds[2];
static GRect astro_bounds[2];
    
static Window *s_main_window;
static Layer *s_date_layer;
static Layer *s_bus_layer;
static Layer *s_time_layer;
static Layer *s_currentweather_layer;
static Layer *s_forecast123_layer;
static Layer *s_forecast456_layer;
static Layer *s_forecast789_layer;
static Layer *s_loc_layer;
static Layer *s_astro_layer;

static PropertyAnimation *s_date_animation;
static PropertyAnimation *s_time_animation;
static PropertyAnimation *s_bus_animation;
static PropertyAnimation *s_currentweather_animation;
static PropertyAnimation *s_forecast123_animation;
static PropertyAnimation *s_forecast456_animation;
static PropertyAnimation *s_forecast789_animation;
static PropertyAnimation *s_loc_animation;
static PropertyAnimation *s_astro_animation;

//static TextLayer *s_tz1_layer;
//static TextLayer *s_tz2_layer;



static Layer *s_path_layer;

// GPath describes the shape
static GPath *s_path;
//GColor backgroundColor = GColorLightGray;
static GPathInfo PATH_INFO = {
    .num_points = 4,
    .points = (GPoint[]) { {-1, 10}, {145, 10}, {145, 40}, {-1, 40 } }
};


//static TextLayer *s_bus456_layer;

static int last_tap_ts;
static bool allow_redraw = true;
static uint8_t mode = 0;

bool isShowingSeconds = false;
//time_t timeOfLastTap = 0;

#define MINSCOUNT 9
    
static int mins[] = { //304,
                      //340,
                      //359,
                      //371,
                      //382,
                      //391,
                      //401,
                      //411,
                      //421,
                      //436,
                      //456,
                      //475,
                      //525,
                      (5*60)+46,
                      (6*60)+16,
                      (6*60)+35,
                      (6*60)+56,
                      (7*60)+43,
    
                      (16*60)+05,
                      (16*60)+33,
                      (17*60)+05,
                      (17*60)+35 };
                      //904,
                      //940,
                      //959,
                      //973,
                      //983,
                      //1008,
                      //1023,
                      //1042,
                      //1061 };





//var homeAM = [ '5:05', '5:41', '6:01', '6:13', '6:24', '6:34', '6:44', '6:54', '7:04', '7:19', '7:39', '7:58', '8:48' ];
//var workPM = [ '15:04', '15:40', '15:59', '16:13', '16:23', '16:48', '17:03', '17:22', '17:41' ];"


//static Layer *s_graph_layer;

static GFont s_time_font;
static GFont s_tiny_font;
static GFont s_14b_font;
static GFont s_18b_font;

//static BitmapLayer *s_background_layer;
//static GBitmap *s_background_bitmap;

// Create a long-lived buffer
static char time_buffer[] = "00 00";
static char seconds_buffer[] = ":00";
static char tz1buffer[] = "00";
static char tz2buffer[] = "00";

static char datebuffer[] = "Sat 2015-05-23";
static char nextbus[] = "99999";
static char bus0123[] = "88:88 (99999)\n88:88 (99999)\n88:88 (99999)\n88:88 (99999)";
static char battery_text[] = "100";
static char bluetooth_buffer[] = " ";


// WEATHER TEXTS
static char temperature_buffer[8];
static char weather_buffer[128];
static char loc_buffer[128];
static char forecast1_buffer[128];
static char forecast2_buffer[128];
static char forecast3_buffer[128];
static char forecast4_buffer[128];
static char forecast5_buffer[128];
static char forecast6_buffer[128];
static char forecast7_buffer[128];
static char forecast8_buffer[128];
static char forecast9_buffer[128];
static char sun_buffer[32];
static char moon_buffer[32];
static int  weather_ts;
static char weather_age[] = "Forecast age 99999 min";






    
//static char tempTime[] = "00:00";

//static char bus456[] = "99999\n99999\n99999";

static int etas[MINSCOUNT] = { 0 };



static void trigger_weather_update() {
    
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);

    // Send the message!
    app_message_outbox_send();
    
}


static void update_time() {

    
    // temp variables
    int i, j, k, a;

    
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
    
    //-if(etas[0] <= 99) {
        //text_layer_set_font(s_nextbus_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
        snprintf(nextbus, sizeof(nextbus), "%d", etas[0]);
    //-} else {
        //text_layer_set_font(s_nextbus_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
        //nbHours = etas[0] / 60;               // integer math; should be rounded down.
        //nbMinutes = etas[0] - (nbHours * 60); // remove the whole hours leaving just the minutes
    //-    snprintf(nextbus, sizeof(nextbus), "%dh", (etas[0]+30) / 60);
    //-}
    
    snprintf(bus0123, sizeof(bus0123), "%02d:%02d (%d)\n%02d:%02d (%d)\n%02d:%02d (%d)", //\n%02d:%02d (%d)", 
             ((etas[0] + minOfDay - ((etas[0] + minOfDay) % 60)) / 60) % 24, ((etas[0] + minOfDay) % 60), etas[0], 
             ((etas[1] + minOfDay - ((etas[1] + minOfDay) % 60)) / 60) % 24, ((etas[1] + minOfDay) % 60), etas[1],
             ((etas[2] + minOfDay - ((etas[2] + minOfDay) % 60)) / 60) % 24, ((etas[2] + minOfDay) % 60), etas[2]); //,
             //((etas[3] + minOfDay - ((etas[3] + minOfDay) % 60)) / 60) % 24, ((etas[3] + minOfDay) % 60), etas[3]);
    

    // Write the current hours and minutes into the buffer
    if(clock_is_24h_style() == true) {
        //Use 2h hour format
        strftime(time_buffer, sizeof(time_buffer), "%H %M", tick_time);
    } else {
        //Use 12 hour format
        strftime(time_buffer, sizeof(time_buffer), "%I %M", tick_time);
    }
    
    
    strftime(datebuffer,  sizeof("Sat 2015-05-23"), "%a %F", tick_time);
    snprintf(tz1buffer,   sizeof(tz1buffer),        "%02d",  (gmt_time->tm_hour));
    snprintf(tz1buffer, sizeof(tz1buffer), "%02d", (gmt_time->tm_hour));
    snprintf(tz2buffer, sizeof(tz2buffer), "%02d", (tick_time->tm_hour + 3) % 24);
    
    if (isShowingSeconds) {
        // isShowingSeconds is toggled by a watch bump to save battery.
        //snprintf(tz1buffer, sizeof(tz1buffer), ":");
        strftime(seconds_buffer, sizeof(seconds_buffer), "%S", tick_time);
    }
    
    
    //snprintf(weather_age, sizeof(weather_age),      "%ld",   (time(NULL) - weather_ts) / 60);
    
    
    // Display this time on the TextLayer
    //text_layer_set_text(s_time_layer, buffer);
    //text_layer_set_text(s_tz1_layer, tz1buffer);
    //text_layer_set_text(s_tz2_layer, tz2buffer);

    
    layer_mark_dirty(s_time_layer);

    
    // If weather is more than 30 minutes old, update... but don't try more often than once every 10 minutes.
    if(tick_time->tm_min % 10 == 0 && ((time(NULL) - weather_ts) > (30 * 60))) {
        
        trigger_weather_update();
    }
    
    
}


static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    
    if (isShowingSeconds) {
        if ((time(NULL) - last_tap_ts) > NUMBER_OF_SECONDS_TO_SHOW_SECONDS_AFTER_TAP) {
            // We are showing seconds, but it has been more than 2
            // minutes since our wrist was tapped. To save processing,
            // stop showing seconds (revert back to one minute updates).
            isShowingSeconds = false;
            tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
        }
    }
    
    update_time();
}


static void inbox_received_callback(DictionaryIterator *iterator, void *context) {

    // Read first item
    Tuple *t = dict_read_first(iterator);

    // For all items
    while(t != NULL) {
        // Which key was received?
        switch(t->key) {
            case KEY_CURR_TEMP:
                snprintf(temperature_buffer, sizeof(temperature_buffer), "%d", (int)t->value->int32);
                break;
            
            case KEY_CURR_WX:
                snprintf(weather_buffer, sizeof(weather_buffer), "%s", t->value->cstring);
                break;
            
            case KEY_CURR_LOC:
                snprintf(loc_buffer, sizeof(loc_buffer), "%s", t->value->cstring);
                break;
            
            case KEY_FCAST_1:
                snprintf(forecast1_buffer, sizeof(forecast1_buffer), "%s", t->value->cstring);
                break;
            
            case KEY_FCAST_2:
                snprintf(forecast2_buffer, sizeof(forecast2_buffer), "%s", t->value->cstring);
                break;
            
            case KEY_FCAST_3:
                snprintf(forecast3_buffer, sizeof(forecast3_buffer), "%s", t->value->cstring);
                break;

            case KEY_FCAST_4:
                snprintf(forecast4_buffer, sizeof(forecast4_buffer), "%s", t->value->cstring);
                break;
            
            case KEY_FCAST_5:
                snprintf(forecast5_buffer, sizeof(forecast5_buffer), "%s", t->value->cstring);
                break;
            
            case KEY_FCAST_6:
                snprintf(forecast6_buffer, sizeof(forecast6_buffer), "%s", t->value->cstring);
                break;

            case KEY_FCAST_7:
                snprintf(forecast7_buffer, sizeof(forecast7_buffer), "%s", t->value->cstring);
                break;
            
            case KEY_FCAST_8:
                snprintf(forecast8_buffer, sizeof(forecast8_buffer), "%s", t->value->cstring);
                break;
            
            case KEY_FCAST_9:
                snprintf(forecast9_buffer, sizeof(forecast9_buffer), "%s", t->value->cstring);
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

    
    weather_ts = time(NULL);

    // save weather info for future use... so I can have weather when face loads, before http response.
    persist_write_string(KEY_CURR_TEMP, temperature_buffer);
    persist_write_string(KEY_CURR_WX, weather_buffer);
    persist_write_string(KEY_FCAST_1, forecast1_buffer);
    persist_write_string(KEY_FCAST_2, forecast2_buffer);
    persist_write_string(KEY_FCAST_3, forecast3_buffer);
    persist_write_string(KEY_FCAST_4, forecast4_buffer);
    persist_write_string(KEY_FCAST_5, forecast5_buffer);
    persist_write_string(KEY_FCAST_6, forecast6_buffer);
    persist_write_string(KEY_FCAST_7, forecast7_buffer);
    persist_write_string(KEY_FCAST_8, forecast8_buffer);
    persist_write_string(KEY_FCAST_9, forecast9_buffer);
    persist_write_string(KEY_SUN, sun_buffer);
    persist_write_string(KEY_MOON, moon_buffer);
    persist_write_string(KEY_CURR_LOC, loc_buffer);
    persist_write_int(KEY_WX_TMSTAMP,weather_ts);
    
    
    // hey... we gots some new data. Wanna show it?
    layer_mark_dirty(s_path_layer);

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
    
    snprintf(bluetooth_buffer, sizeof(bluetooth_buffer), (connected ? "*" : ""));
    
    if(connected) {
        
        //BUG: triggering the weather update here is causing crashes.
        //trigger_weather_update();
    }
    
    layer_mark_dirty(s_path_layer);

}

void animation_started(Animation *animation, void *data) {
    // Animation started!
    allow_redraw = false;
}


void animation_stopped(Animation *animation, bool finished, void *data) {
    // Animation stopped!
    allow_redraw = true;
}

static void trigger_time_animation(uint8_t delta) {

    uint8_t prevmode = mode;
    mode = ((mode + delta) % 2);

    
    // Create the date animation
    s_date_animation = property_animation_create_layer_frame((Layer*) s_date_layer, &date_bounds[prevmode], &date_bounds[mode]);
    animation_set_duration((Animation*) s_date_animation, 1000);
    animation_schedule((Animation*) s_date_animation);

    s_bus_animation = property_animation_create_layer_frame((Layer*) s_bus_layer, &bus_bounds[prevmode], &bus_bounds[mode]);
    animation_set_duration((Animation*) s_bus_animation, 1000);
    animation_schedule((Animation*) s_bus_animation);
    
    s_forecast123_animation = property_animation_create_layer_frame((Layer*) s_forecast123_layer, &forecast123_bounds[prevmode], &forecast123_bounds[mode]);
    animation_set_duration((Animation*) s_forecast123_animation, 1000);
    animation_schedule((Animation*) s_forecast123_animation);
    
    s_forecast456_animation = property_animation_create_layer_frame((Layer*) s_forecast456_layer, &forecast456_bounds[prevmode], &forecast456_bounds[mode]);
    animation_set_duration((Animation*) s_forecast456_animation, 1000);
    animation_schedule((Animation*) s_forecast456_animation);
    
    s_forecast789_animation = property_animation_create_layer_frame((Layer*) s_forecast789_layer, &forecast789_bounds[prevmode], &forecast789_bounds[mode]);
    animation_set_duration((Animation*) s_forecast789_animation, 1000);
    animation_schedule((Animation*) s_forecast789_animation);
    
    s_loc_animation = property_animation_create_layer_frame((Layer*) s_loc_layer, &loc_bounds[prevmode], &loc_bounds[mode]);
    animation_set_duration((Animation*) s_loc_animation, 1000);
    animation_schedule((Animation*) s_loc_animation);
    
    s_currentweather_animation = property_animation_create_layer_frame((Layer*) s_currentweather_layer, &currentweather_bounds[prevmode], &currentweather_bounds[mode]);
    animation_set_duration((Animation*) s_currentweather_animation, 1000);
    animation_schedule((Animation*) s_currentweather_animation);
    
    

    s_time_animation = property_animation_create_layer_frame((Layer*) s_time_layer, &time_bounds[prevmode], &time_bounds[mode]);
    animation_set_curve((Animation*) s_time_animation, AnimationCurveEaseInOut);
    animation_set_duration((Animation*) s_time_animation, 1000);
    
    // You may set handlers to listen for the start and stop events
    animation_set_handlers((Animation*) s_time_animation, (AnimationHandlers) {
        .started = (AnimationStartedHandler) animation_started,
        .stopped = (AnimationStoppedHandler) animation_stopped,
    }, NULL);

    // Schedule to occur ASAP with default settings
    animation_schedule((Animation*) s_time_animation);

}


static void handle_tap(AccelAxisType axis, int32_t direction) {
    
    int temp_now_ts = time(NULL);
    
    if (!isShowingSeconds)
    {
        // We aren't showing seconds, let's show them and switch
        // to the second_unit timer subscription.
        isShowingSeconds = true;

        // Immediatley update the time so our tap looks very responsive.
        //struct tm *tick_time = localtime(&last_tap_ts);
        //update_time(tick_time);

        // Resubscribe to the tick timer at every second.
        tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
    }
    
    
    // two taps within 5 seconds: switch modes.
    if(last_tap_ts && ((last_tap_ts + 5) > temp_now_ts)) {
        trigger_time_animation(1);
    }
    last_tap_ts = temp_now_ts;
    

    switch (axis) {
        case ACCEL_AXIS_X:
            if (direction > 0) {
                APP_LOG(APP_LOG_LEVEL_INFO, "X axis positive.");
            } else {
                APP_LOG(APP_LOG_LEVEL_INFO, "X axis negative.");
            }
            break;
        
        case ACCEL_AXIS_Y:
            if (direction > 0) {
                APP_LOG(APP_LOG_LEVEL_INFO, "Y axis positive.");
            } else {
                APP_LOG(APP_LOG_LEVEL_INFO, "Y axis negative.");
            }
            break;
        
        case ACCEL_AXIS_Z:
            if (direction > 0) {
                APP_LOG(APP_LOG_LEVEL_INFO, "Z axis positive.");
            } else {
                APP_LOG(APP_LOG_LEVEL_INFO, "Z axis negative.");
            }
            break;
    }

    
    
}





//works??
//static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
    //uint8_t clicks = click_number_of_clicks_counted(recognizer);
    //if(clicks > 1) {
//    trigger_time_animation(1);
    //}
//}

//works??
//static void click_config_provider(void *context) {
    // Register the ClickHandlers
    //window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);
//    window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
//}

static void draw_text_shadowed(GContext * ctx, const char * text, GFont const font,
        const int x, const int y, const int w, const int h, const GTextOverflowMode overflow_mode,
        const GTextAlignment alignment, const GTextLayoutCacheRef layout,
        //const int fuzzN, const int fuzzE, const int fuzzS, const int fuzzW,
        const int fuzzNE, const int fuzzSE, const int fuzzSW, const int fuzzNW,
        GColor fuzzColor, GColor textColor) {
    
    if(allow_redraw) {
        graphics_context_set_text_color(ctx, fuzzColor);
        if(fuzzNW) graphics_draw_text(ctx, text, font, GRect(x-fuzzNW,y-fuzzNW,w,h), overflow_mode, alignment, layout);
        if(fuzzNE) graphics_draw_text(ctx, text, font, GRect(x+fuzzNE,y-fuzzNE,w,h), overflow_mode, alignment, layout);
        if(fuzzSE) graphics_draw_text(ctx, text, font, GRect(x+fuzzSE,y+fuzzSE,w,h), overflow_mode, alignment, layout);
        if(fuzzSW) graphics_draw_text(ctx, text, font, GRect(x-fuzzSW,y+fuzzSW,w,h), overflow_mode, alignment, layout);
    }
    graphics_context_set_text_color(ctx, textColor);
    graphics_draw_text(ctx, text, font, GRect(x,y,w,h), overflow_mode, alignment, layout);
}

static void date_layer_update_proc(Layer *layer, GContext *ctx) {
    
    //if(!allow_redraw) return;
    
    //APP_LOG(APP_LOG_LEVEL_INFO, "date_layer_update_proc!");
    
    // Draw the date
    #ifdef PBL_COLOR
        
        draw_text_shadowed(ctx, datebuffer, s_14b_font,
                           0,-5,144,17,
                           GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL,
                           0,1,0,0,
                           GColorBlack, GColorWhite);
        
        // Watch Battery
        draw_text_shadowed(ctx, battery_text, s_14b_font,
                           3,-5,139,15,
                           GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL,
                           0,1,0,0,
                           GColorBlack, GColorWhite);
        
        // Bluetooth Connection
        draw_text_shadowed(ctx, bluetooth_buffer, s_14b_font,
                           0,-5,141,17,
                           GTextOverflowModeWordWrap, GTextAlignmentRight, NULL,
                           0,1,0,0,
                           GColorBlack, GColorWhite);
    
        if(allow_redraw) {
            graphics_context_set_stroke_color(ctx, GColorBlack); // GColorFromRGB(255, 0, 0));
            graphics_draw_line(ctx, GPoint(0,11), GPoint(144,11));
        }
    
    #else
    
        graphics_context_set_text_color(ctx, GColorWhite);
        graphics_draw_text(ctx, datebuffer, s_14b_font, GRect(0,-5,144,17), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
        graphics_draw_text(ctx, battery_text, s_14b_font, GRect(3,-5,139,15), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
        graphics_draw_text(ctx, bluetooth_buffer, s_14b_font, GRect(0,-5,141,17), GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);
        
    #endif
        
        
    graphics_context_set_stroke_color(ctx, GColorWhite); // GColorFromRGB(255, 0, 0));
    graphics_draw_line(ctx, GPoint(0,10), GPoint(144,10));
    
}

static void time_layer_update_proc(Layer *layer, GContext *ctx) {
    
    //if(!allow_redraw) return;
    
    APP_LOG(APP_LOG_LEVEL_INFO, "time_layer_update_proc!");

    #ifdef PBL_COLOR
        graphics_context_set_fill_color(ctx, GColorOxfordBlue);
    #else
        graphics_context_set_fill_color(ctx, GColorBlack);
    #endif
    
    graphics_fill_rect(ctx, GRect(0, 0, 144, 168), 0, GCornerNone);
    
   
    #ifdef PBL_COLOR
        
        if(allow_redraw) {
            graphics_context_set_stroke_color(ctx, GColorBlack); // GColorFromRGB(255, 0, 0));
            graphics_draw_line(ctx, GPoint(0,1), GPoint(144,1));
        }
    
    #endif
    
        
    graphics_context_set_stroke_color(ctx, GColorWhite); // GColorFromRGB(255, 0, 0));
    graphics_draw_line(ctx, GPoint(0,0), GPoint(144,0));    
    
    // Draw the time's background rectangle
    
    
    #ifdef PBL_COLOR
        
        if(allow_redraw) {
            graphics_context_set_fill_color(ctx, GColorBlack);
            graphics_fill_rect(ctx, GRect(3, 6, 140, 45), 0, GCornerNone);
        }
    
    #endif
        
        
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, GRect(2, 5, 140, 45), 0, GCornerNone);
    
    
    #ifdef PBL_COLOR
        
        draw_text_shadowed(ctx, time_buffer, s_time_font,
                           0,-5,144,50,
                           GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL,
                           0,1,0,0,
                           GColorWhite, GColorBlack);
        
        draw_text_shadowed(ctx, tz1buffer, s_18b_font,
                           58,6,29,21,
                           GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL,
                           0,1,0,0,
                           GColorWhite, GColorBlack);
        
        draw_text_shadowed(ctx, tz2buffer, s_18b_font,
                           58,24,29,21,
                           GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL,
                           0,1,0,0,
                           GColorWhite, GColorBlack);
    
    #else
        
        graphics_context_set_text_color(ctx, GColorBlack);
        graphics_draw_text(ctx, time_buffer, s_time_font, GRect(0,-5,144,50), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
        graphics_draw_text(ctx, tz1buffer, s_18b_font, GRect(58,6,29,21), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
        graphics_draw_text(ctx, tz2buffer, s_18b_font, GRect(58,24,29,21), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);

    #endif
        
        
    
    if(isShowingSeconds) {
        
        //APP_LOG(APP_LOG_LEVEL_INFO, "(seconds) |%s|", seconds_buffer);
        draw_text_shadowed(ctx, seconds_buffer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK),
                           0, 18, 144, 25,
                           GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL,
                           2,2,2,2,
                           GColorBlack, GColorWhite);
    }
    
    
    #ifdef PBL_COLOR
        
        if(allow_redraw) {
            graphics_context_set_stroke_color(ctx, GColorBlack); // GColorFromRGB(255, 0, 0));
            graphics_draw_line(ctx, GPoint(0,55), GPoint(144,55));
        }
    
    #endif
        
        
    graphics_context_set_stroke_color(ctx, GColorWhite); // GColorFromRGB(255, 0, 0));
    graphics_draw_line(ctx, GPoint(0,54), GPoint(144,54));

}

static void bus_layer_update_proc(Layer *layer, GContext *ctx) {

    #ifdef PBL_COLOR
        
        if(allow_redraw) {
            graphics_context_set_stroke_color(ctx, GColorBlack); // GColorFromRGB(255, 0, 0));
            graphics_draw_line(ctx, GPoint(0,1), GPoint(144,1));
        }
    
    #endif
        
        
    graphics_context_set_stroke_color(ctx, GColorWhite); // GColorFromRGB(255, 0, 0));
    graphics_draw_line(ctx, GPoint(0,0), GPoint(144,0));
    
    
    #ifdef PBL_COLOR
        
        // Three buses following the next one
        draw_text_shadowed(ctx, bus0123, s_tiny_font,
                           1,1,68,47,
                           GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL,
                           0,1,0,0,
                           GColorBlack, GColorWhite);
        
        
        // Next Bus Time
        draw_text_shadowed(ctx, nextbus, s_time_font,
                           0,-11,144,39,
                           GTextOverflowModeWordWrap, GTextAlignmentRight, NULL,
                           0,2,0,0,
                           GColorBlack, GColorWhite);
        
        
        if(allow_redraw) {
            graphics_context_set_stroke_color(ctx, GColorBlack); // GColorFromRGB(255, 0, 0));
            graphics_draw_line(ctx, GPoint(0,31), GPoint(144,31));
        }
    
    #else
        
        graphics_context_set_text_color(ctx, GColorWhite);
        graphics_draw_text(ctx, bus0123, s_tiny_font, GRect(1,1,68,47), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
        graphics_draw_text(ctx, nextbus, s_time_font, GRect(0,-11,144,39), GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);
        
    #endif
    
    
    graphics_context_set_stroke_color(ctx, GColorWhite); // GColorFromRGB(255, 0, 0));
    graphics_draw_line(ctx, GPoint(0,30), GPoint(144,30));
}


static void currentweather_layer_update_proc(Layer *layer, GContext *ctx) {

    // CURRENT WEATHER ROW
    
    
    // small text for age of weather
    snprintf(weather_age, sizeof(weather_age),      "(age %ld min)",   (time(NULL) - weather_ts) / 60);
    
    
    #ifdef PBL_COLOR
        
        if(allow_redraw) {
            graphics_context_set_stroke_color(ctx, GColorBlack); // GColorFromRGB(255, 0, 0));
            graphics_draw_line(ctx, GPoint(0,1), GPoint(144,1));
        }
        
    #endif
        
        
    graphics_context_set_stroke_color(ctx, GColorWhite); // GColorFromRGB(255, 0, 0));
    graphics_draw_line(ctx, GPoint(0,0), GPoint(144,0));
    
    
    #ifdef PBL_COLOR
        
        draw_text_shadowed(ctx, temperature_buffer, s_time_font,
                           0,-11,144,39,
                           GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL,
                           1,1,1,1,
                           GColorBlack, GColorWhite);
        
        // current conditions
        draw_text_shadowed(ctx, weather_buffer, s_tiny_font,
                           0,1,144,30,
                           GTextOverflowModeWordWrap, GTextAlignmentRight, NULL,
                           1,1,1,1,
                           GColorBlack, GColorWhite);
        
        
        draw_text_shadowed(ctx, weather_age, s_tiny_font,
                           0,19,144,15,
                           GTextOverflowModeWordWrap, GTextAlignmentRight, NULL,
                           1,1,1,1,
                           GColorBlack, GColorWhite);
        
        
        if(allow_redraw) {
            graphics_context_set_stroke_color(ctx, GColorBlack); // GColorFromRGB(255, 0, 0));
            graphics_draw_line(ctx, GPoint(0,31), GPoint(144,31));
        }
    
    #else
        
        graphics_context_set_text_color(ctx, GColorWhite);
        graphics_draw_text(ctx, temperature_buffer, s_time_font, GRect(0,-11,144,39), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
        graphics_draw_text(ctx, weather_buffer, s_tiny_font, GRect(0,1,144,30), GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);
        graphics_draw_text(ctx, weather_age, s_tiny_font, GRect(0,19,144,15), GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);
    
    #endif
    
    
    graphics_context_set_stroke_color(ctx, GColorWhite); // GColorFromRGB(255, 0, 0));
    graphics_draw_line(ctx, GPoint(0,30), GPoint(144,30));
    

}
    
static void forecast123_layer_update_proc(Layer *layer, GContext *ctx) {
    
    #ifdef PBL_COLOR
    
        if(allow_redraw) {
            graphics_context_set_stroke_color(ctx, GColorBlack); // GColorFromRGB(255, 0, 0));
            graphics_draw_line(ctx, GPoint(0,1), GPoint(144,1));
        }
    
    #endif
        
        
    graphics_context_set_stroke_color(ctx, GColorWhite); // GColorFromRGB(255, 0, 0));
    graphics_draw_line(ctx, GPoint(0,0), GPoint(144,0));
    
    
    #ifdef PBL_COLOR
        
        draw_text_shadowed(ctx, forecast1_buffer, s_tiny_font,
                           -10,1,68,47,
                           GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL,
                           1,1,1,1,
                           GColorBlack, GColorWhite);
        draw_text_shadowed(ctx, forecast2_buffer, s_tiny_font,
                           38,1,68,47,
                           GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL,
                           1,1,1,1,
                           GColorBlack, GColorWhite);
        draw_text_shadowed(ctx, forecast3_buffer, s_tiny_font,
                           86,1,68,47,
                           GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL,
                           1,1,1,1,
                           GColorBlack, GColorWhite);
        
        
        if(allow_redraw) {
            graphics_context_set_stroke_color(ctx, GColorBlack); // GColorFromRGB(255, 0, 0));
            graphics_draw_line(ctx, GPoint(0,31), GPoint(144,31));
        }
    
    #else
        
        graphics_context_set_text_color(ctx, GColorWhite);
        graphics_draw_text(ctx, forecast1_buffer, s_tiny_font, GRect(-10,1,68,47), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
        graphics_draw_text(ctx, forecast2_buffer, s_tiny_font, GRect(38,1,68,47), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
        graphics_draw_text(ctx, forecast3_buffer, s_tiny_font, GRect(86,1,68,47), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
        
    #endif
        
        
    graphics_context_set_stroke_color(ctx, GColorWhite); // GColorFromRGB(255, 0, 0));
    graphics_draw_line(ctx, GPoint(0,30), GPoint(144,30));

}



static void forecast456_layer_update_proc(Layer *layer, GContext *ctx) {
    
    #ifdef PBL_COLOR
    
        if(allow_redraw) {
            graphics_context_set_stroke_color(ctx, GColorBlack); // GColorFromRGB(255, 0, 0));
            graphics_draw_line(ctx, GPoint(0,1), GPoint(144,1));
        }
    
    #endif
        
        
    graphics_context_set_stroke_color(ctx, GColorWhite); // GColorFromRGB(255, 0, 0));
    graphics_draw_line(ctx, GPoint(0,0), GPoint(144,0));
    
    
    #ifdef PBL_COLOR
        
        draw_text_shadowed(ctx, forecast4_buffer, s_tiny_font,
                           -10,1,68,47,
                           GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL,
                           1,1,1,1,
                           GColorBlack, GColorWhite);
        draw_text_shadowed(ctx, forecast5_buffer, s_tiny_font,
                           38,1,68,47,
                           GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL,
                           1,1,1,1,
                           GColorBlack, GColorWhite);
        draw_text_shadowed(ctx, forecast6_buffer, s_tiny_font,
                           86,1,68,47,
                           GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL,
                           1,1,1,1,
                           GColorBlack, GColorWhite);
        
        
        if(allow_redraw) {
            graphics_context_set_stroke_color(ctx, GColorBlack); // GColorFromRGB(255, 0, 0));
            graphics_draw_line(ctx, GPoint(0,31), GPoint(144,31));
        }
    
    #else
        
        graphics_context_set_text_color(ctx, GColorWhite);
        graphics_draw_text(ctx, forecast4_buffer, s_tiny_font, GRect(-10,1,68,47), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
        graphics_draw_text(ctx, forecast5_buffer, s_tiny_font, GRect(38,1,68,47), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
        graphics_draw_text(ctx, forecast6_buffer, s_tiny_font, GRect(86,1,68,47), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
        
    #endif
        
        
    graphics_context_set_stroke_color(ctx, GColorWhite); // GColorFromRGB(255, 0, 0));
    graphics_draw_line(ctx, GPoint(0,30), GPoint(144,30));

}

static void forecast789_layer_update_proc(Layer *layer, GContext *ctx) {
    
    #ifdef PBL_COLOR
    
        if(allow_redraw) {
            graphics_context_set_stroke_color(ctx, GColorBlack); // GColorFromRGB(255, 0, 0));
            graphics_draw_line(ctx, GPoint(0,1), GPoint(144,1));
        }
    
    #endif
        
        
    graphics_context_set_stroke_color(ctx, GColorWhite); // GColorFromRGB(255, 0, 0));
    graphics_draw_line(ctx, GPoint(0,0), GPoint(144,0));
    
    
    #ifdef PBL_COLOR
        
        draw_text_shadowed(ctx, forecast7_buffer, s_tiny_font,
                           -10,1,68,47,
                           GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL,
                           1,1,1,1,
                           GColorBlack, GColorWhite);
        draw_text_shadowed(ctx, forecast8_buffer, s_tiny_font,
                           38,1,68,47,
                           GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL,
                           1,1,1,1,
                           GColorBlack, GColorWhite);
        draw_text_shadowed(ctx, forecast9_buffer, s_tiny_font,
                           86,1,68,47,
                           GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL,
                           1,1,1,1,
                           GColorBlack, GColorWhite);
        
        
        if(allow_redraw) {
            graphics_context_set_stroke_color(ctx, GColorBlack); // GColorFromRGB(255, 0, 0));
            graphics_draw_line(ctx, GPoint(0,31), GPoint(144,31));
        }
    
    #else
        
        graphics_context_set_text_color(ctx, GColorWhite);
        graphics_draw_text(ctx, forecast7_buffer, s_tiny_font, GRect(-10,1,68,47), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
        graphics_draw_text(ctx, forecast8_buffer, s_tiny_font, GRect(38,1,68,47), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
        graphics_draw_text(ctx, forecast9_buffer, s_tiny_font, GRect(86,1,68,47), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
        
    #endif
        
        
    graphics_context_set_stroke_color(ctx, GColorWhite); // GColorFromRGB(255, 0, 0));
    graphics_draw_line(ctx, GPoint(0,30), GPoint(144,30));

}

   
static void loc_layer_update_proc(Layer *layer, GContext *ctx) {
    
    #ifdef PBL_COLOR
    
        draw_text_shadowed(ctx, loc_buffer, s_18b_font,
                           0, 0, 144, 30,
                           GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL,
                           0,1,0,0,
                           GColorBlack, GColorWhite);
    
    #else
        
        graphics_context_set_text_color(ctx, GColorWhite);
        graphics_draw_text(ctx, loc_buffer, s_18b_font, GRect(0,0,144,30), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
        
    #endif
        
}


static void astro_layer_update_proc(Layer *layer, GContext *ctx) {
}

static void layer_update_proc(Layer *layer, GContext *ctx) {
    
    
    

    
    
    
    // SUN/MOON ROW
    
    
    draw_text_shadowed(ctx, sun_buffer, fonts_get_system_font(FONT_KEY_GOTHIC_14),
                       3,152,141,17,
                       GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL,
                       0,1,0,0,
                       GColorBlack, GColorWhite);
    
    draw_text_shadowed(ctx, moon_buffer, fonts_get_system_font(FONT_KEY_GOTHIC_14),
                       0,152,141,17,
                       GTextOverflowModeWordWrap, GTextAlignmentRight, NULL,
                       0,1,0,0,
                       GColorBlack, GColorWhite);
}


static void main_window_load(Window *window) {

    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
    
    
    date_bounds[0] = GRect(0, 0, 144, 12);
    //date_bounds[1] = GRect(-145, 0, 144, 12);
    date_bounds[1] = GRect(0, -40, 144, 12);
    
    
    bus_bounds[0] = GRect(0,10,144,31);
    //bus_bounds[1] = GRect(145,10,144,31);
    bus_bounds[1] = GRect(0,-30,144,31);
    
    
    time_bounds[0] = GRect(0, 40, 144, 56);
    //time_bounds[1] = GRect(0, 112, 144, 56);
    time_bounds[1] = GRect(0, 0, 144, 56);
    
    
    
    
    
    
    //static GRect currentweather_bounds[2];
    //static GRect forecast_bounds[2];
    
    //static GRect astro_bounds[2];
    
    currentweather_bounds[0] = GRect(0,94,144,32);
    currentweather_bounds[1] = GRect(-145,94,144,32);
    
    forecast123_bounds[0] = GRect(0,124,144,32);
    forecast123_bounds[1] = GRect(0,0,144,32);
    
    forecast456_bounds[0] = GRect(-145,30,144,32);
    forecast456_bounds[1] = GRect(0,30,144,32);
    
    forecast789_bounds[0] = GRect(145,60,144,32);
    forecast789_bounds[1] = GRect(0,60,144,32);
    
    loc_bounds[0] = GRect(-145,89,144,20);
    loc_bounds[1] = GRect(0,89,144,20);
    
    
    // Create second custom font, apply it and add to Window
    s_tiny_font = //fonts_get_system_font(FONT_KEY_GOTHIC_14);
        fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DEJAVU_SANS_9));
    s_time_font =
        fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_LECO_50));
        //fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DEJAVU_SANS_COND_47));
    s_14b_font =
        fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
    s_18b_font =
        fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
    
    
    // Create GPath object
    s_path = gpath_create(&PATH_INFO);

    // Create Layer that the path will be drawn on
    s_path_layer = layer_create(bounds);
    layer_set_update_proc(s_path_layer, layer_update_proc);
    layer_add_child(window_layer, s_path_layer);
    
    
    
    
    if(persist_exists(KEY_CURR_TEMP)) persist_read_string(KEY_CURR_TEMP, temperature_buffer, sizeof(temperature_buffer));
    if(persist_exists(KEY_SUN))       persist_read_string(KEY_SUN,       sun_buffer,         sizeof(sun_buffer));
    if(persist_exists(KEY_FCAST_1))   persist_read_string(KEY_FCAST_1,   forecast1_buffer,   sizeof(forecast1_buffer));
    if(persist_exists(KEY_FCAST_2))   persist_read_string(KEY_FCAST_2,   forecast2_buffer,   sizeof(forecast2_buffer));
    if(persist_exists(KEY_FCAST_3))   persist_read_string(KEY_FCAST_3,   forecast3_buffer,   sizeof(forecast3_buffer));
    if(persist_exists(KEY_FCAST_4))   persist_read_string(KEY_FCAST_4,   forecast4_buffer,   sizeof(forecast4_buffer));
    if(persist_exists(KEY_FCAST_5))   persist_read_string(KEY_FCAST_5,   forecast5_buffer,   sizeof(forecast5_buffer));
    if(persist_exists(KEY_FCAST_6))   persist_read_string(KEY_FCAST_6,   forecast6_buffer,   sizeof(forecast6_buffer));
    if(persist_exists(KEY_FCAST_7))   persist_read_string(KEY_FCAST_7,   forecast7_buffer,   sizeof(forecast7_buffer));
    if(persist_exists(KEY_FCAST_8))   persist_read_string(KEY_FCAST_8,   forecast8_buffer,   sizeof(forecast8_buffer));
    if(persist_exists(KEY_FCAST_9))   persist_read_string(KEY_FCAST_9,   forecast9_buffer,   sizeof(forecast9_buffer));
    if(persist_exists(KEY_MOON))      persist_read_string(KEY_MOON,      moon_buffer,        sizeof(moon_buffer));
    if(persist_exists(KEY_CURR_WX))   persist_read_string(KEY_CURR_WX,   weather_buffer,     sizeof(weather_buffer));
    if(persist_exists(KEY_CURR_LOC))  persist_read_string(KEY_CURR_LOC,  loc_buffer,         sizeof(loc_buffer));

    if(persist_exists(KEY_WX_TMSTAMP)) weather_ts = persist_read_int(KEY_WX_TMSTAMP);
    
    

    
    #ifdef PBL_COLOR
        window_set_background_color(s_main_window, GColorBlue); // GColorOxfordBlue); //GColorOxfordBlue);
    #else
        window_set_background_color(s_main_window, GColorBlack); //GColorOxfordBlue);
    #endif
    
        
    // Create date Layer
    s_date_layer = layer_create(date_bounds[mode]);
    layer_set_update_proc(s_date_layer, date_layer_update_proc);
    layer_add_child(window_layer, s_date_layer);
    
    // Create bus Layer
    s_bus_layer       = layer_create(bus_bounds[mode]);
    layer_set_update_proc(s_bus_layer, bus_layer_update_proc);
    layer_add_child(window_layer, s_bus_layer);
    
    s_currentweather_layer = layer_create(currentweather_bounds[mode]);
    layer_set_update_proc(s_currentweather_layer, currentweather_layer_update_proc);
    layer_add_child(window_layer, s_currentweather_layer);

    s_forecast123_layer       = layer_create(forecast123_bounds[mode]);
    layer_set_update_proc(s_forecast123_layer, forecast123_layer_update_proc);
    layer_add_child(window_layer, s_forecast123_layer);
    
    s_forecast456_layer       = layer_create(forecast456_bounds[mode]);
    layer_set_update_proc(s_forecast456_layer, forecast456_layer_update_proc);
    layer_add_child(window_layer, s_forecast456_layer);
    
    s_forecast789_layer       = layer_create(forecast789_bounds[mode]);
    layer_set_update_proc(s_forecast789_layer, forecast789_layer_update_proc);
    layer_add_child(window_layer, s_forecast789_layer);

    s_loc_layer          = layer_create(loc_bounds[mode]);
    layer_set_update_proc(s_loc_layer, loc_layer_update_proc);
    layer_add_child(window_layer, s_loc_layer);
    
    s_astro_layer          = layer_create(astro_bounds[mode]);
    layer_set_update_proc(s_astro_layer, astro_layer_update_proc);
    layer_add_child(window_layer, s_astro_layer);
    
    // Create time Layer
    s_time_layer = layer_create(time_bounds[mode]);
    layer_set_update_proc(s_time_layer, time_layer_update_proc);
    layer_add_child(window_layer, s_time_layer);
    
    
    handle_battery(battery_state_service_peek());
    handle_bluetooth(bluetooth_connection_service_peek());

        
    // Register with Services
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
    battery_state_service_subscribe(handle_battery);
    bluetooth_connection_service_subscribe(handle_bluetooth);
    accel_tap_service_subscribe(handle_tap);
    
    //works??
    //window_set_click_config_provider(s_main_window, click_config_provider);
    
    
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
    accel_tap_service_unsubscribe();

    
    
    // Destroy layer and path
    layer_destroy(s_path_layer);
    gpath_destroy(s_path);

    
    //Destroy GBitmap
    //gbitmap_destroy(s_background_bitmap);

    //Destroy BitmapLayer
    //bitmap_layer_destroy(s_background_layer);

    // Destroy Layers
    layer_destroy(s_date_layer);
    layer_destroy(s_bus_layer);
    layer_destroy(s_time_layer);
    layer_destroy(s_currentweather_layer);
    layer_destroy(s_forecast123_layer);
    layer_destroy(s_forecast456_layer);
    layer_destroy(s_forecast789_layer);
    layer_destroy(s_loc_layer);
    layer_destroy(s_astro_layer);


    //text_layer_destroy(s_tz1_layer);
    //text_layer_destroy(s_tz2_layer);



//    layer_destroy(s_graph_layer);

    // Destroy custom fonts
    fonts_unload_custom_font(s_time_font);
    fonts_unload_custom_font(s_tiny_font);
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
