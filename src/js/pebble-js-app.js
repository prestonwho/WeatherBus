var xhrRequest = function (url, type, callback) {
    var xhr = new XMLHttpRequest();
    
    xhr.onload = function () {
        callback(this.responseText);
    };
    
    xhr.open(type, url);
    xhr.send();
};

function locationSuccess(pos) {
    // Construct URL
    //var url = "http://api.openweathermap.org/data/2.5/weather?lat=" +
    //    pos.coords.latitude + "&lon=" + pos.coords.longitude;

    var url = "http://api.wunderground.com/api/1a654e3055c497ee/geolookup/conditions/forecast/astronomy/q/" +
        pos.coords.latitude + "," + pos.coords.longitude + ".json";
    
    // Send request to Weather Underground
    xhrRequest(url, 'GET', 
               function(responseText) {
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   // responseText contains a JSON object with weather info
                   /*responseText = responseText.replace(/Partly/g, 'P');
                   responseText = responseText.replace(/Mostly/g, 'M');
                   responseText = responseText.replace(/Chance of (a )*([^"]*)/g, '$2?');
                   responseText = responseText.replace(/Thunderstorm/g, 'T-Stm');
                   responseText = responseText.replace(/Over/g, 'O-');*/
                   
                   responseText = responseText.replace(/Partly/g, 'P')
                                              .replace(/Mostly/g, 'M')
                                              .replace(/Chance of (a )*([^"]*)/g, '$2?')
                                              .replace(/Thunderstorm/g, 'T-Stm')
                                              .replace(/Over/g, 'O\'');
        
        
                   var json = JSON.parse(responseText);

                   console.log(responseText);

                   // Temperature in Kelvin requires adjustment
                   //var temperature = Math.round(((json.main.temp - 273.15) * 1.8) + 32);

                   var temperature = json.current_observation.temp_f;
                   var conditions = json.current_observation.weather;

                   console.log("Temperature is " + temperature);

                   // Conditions
                   //var conditions = json.weather[0].main;      
                   console.log("Conditions are " + conditions);

                   // Assemble dictionary using our keys
                   var dictionary = {
                       "KEY_TEMPERATURE": json.current_observation.temp_f,
                       //"KEY_FEELSLIKE":   Math.round(parseFloat(json.current_observation.feelslike_f)),
                       //"KEY_CONDITIONS":  json.current_observation.weather,
                       //"KEY_CURREPOCH":   json.current_observation.local_epoch,
                       
                       //"KEY_F0DAY":       json.forecast.simpleforecast.forecastday[0].date.weekday.substring(0,2),
                       
                       "KEY_CONDITIONS_0":    json.forecast.simpleforecast.forecastday[0].high.fahrenheit + "°/" +
                                              json.forecast.simpleforecast.forecastday[0].low.fahrenheit + "° " +
                                              "(" + json.forecast.simpleforecast.forecastday[0].pop + "% pop)\n" +
                                              "(feel " + Math.round(parseFloat(json.current_observation.feelslike_f)) + "°) " + json.current_observation.weather,
                                            
                       
                       "KEY_FORECAST_1":  json.forecast.simpleforecast.forecastday[1].date.weekday.substring(0,2) + " " +
                                              json.forecast.simpleforecast.forecastday[1].pop + "%\n" +
                                              json.forecast.simpleforecast.forecastday[1].high.fahrenheit + "°/" +
                                              json.forecast.simpleforecast.forecastday[1].low.fahrenheit + "°\n" +
                                              json.forecast.simpleforecast.forecastday[1].conditions,
                       
                       "KEY_FORECAST_2":  json.forecast.simpleforecast.forecastday[2].date.weekday.substring(0,2) + " " +
                                              json.forecast.simpleforecast.forecastday[2].pop + "%\n" +
                                              json.forecast.simpleforecast.forecastday[2].high.fahrenheit + "°/" +
                                              json.forecast.simpleforecast.forecastday[2].low.fahrenheit + "°\n" +
                                              json.forecast.simpleforecast.forecastday[2].conditions,
                       
                       "KEY_FORECAST_3":  json.forecast.simpleforecast.forecastday[3].date.weekday.substring(0,2) + " " +
                                              json.forecast.simpleforecast.forecastday[3].pop + "%\n" +
                                              json.forecast.simpleforecast.forecastday[3].high.fahrenheit + "°/" +
                                              json.forecast.simpleforecast.forecastday[3].low.fahrenheit + "°\n" +
                                              json.forecast.simpleforecast.forecastday[3].conditions,
                       
                       "KEY_SUN":         "S: " + (json.sun_phase.sunrise.hour < 10 ? "0" : "") + json.sun_phase.sunrise.hour + 
                                              ":" + json.sun_phase.sunrise.minute + "-" +
                                              (json.sun_phase.sunset.hour < 10 ? "0" : "") + json.sun_phase.sunset.hour +
                                              ":" + json.sun_phase.sunset.minute,
                       
                       "KEY_MOON":        "M: " + json.moon_phase.percentIlluminated + "%/" +
                                              json.moon_phase.ageOfMoon,
                   };
                   

                   console.log("KEY_SUN = " + dictionary.KEY_SUN);
                   console.log("KEY_MOON = " + dictionary.KEY_MOON);
                   
                   console.log("KEY_FORECAST_1 = " + dictionary.KEY_FORECAST_1);
                   console.log("KEY_FORECAST_2 = " + dictionary.KEY_FORECAST_2);
                   console.log("KEY_FORECAST_3 = " + dictionary.KEY_FORECAST_3);
                   
                   
                   // Send to Pebble
                   Pebble.sendAppMessage(dictionary,
                                         function(e) {
                                             console.log("Weather info sent to Pebble successfully!");
                                         },
                                         function(e) {
                                             console.log("Error sending weather info to Pebble!");
                                         }
                                        );
               }      
              );
}                                

function locationError(err) {
    console.log("Error requesting location!");
}

function getWeather() {
    navigator.geolocation.getCurrentPosition(
        locationSuccess,
        locationError,
        {timeout: 15000, maximumAge: 60000}
    );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
                        function(e) {
                            console.log("PebbleKit JS ready!");

                            // Get the initial weather
                            getWeather();
                        }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
                        function(e) {
                            console.log("AppMessage received!");
                            getWeather();
                        }                     
);
