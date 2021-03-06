#include <Arduino.h>
#include <WiFi.h>
#include <FastLED.h>
#include <string.h>
#include "time.h"
#include "structs.h"

#define N 8
#define NLEDS 86
#define SEGMENT_SIZE 4
#define DIGIT_SIZE 28
#define MINUTES_SIZE 71
#define SECONDS_SIZE 86

const int ledPin =  4;      // the number of the LED pin
const char* ssid       = "Dalva 2.4";
const char* password   = "carlosbaum";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -3*3600;

CRGB leds[NLEDS];
WiFiClient espClient;

//Variables
struct tm timeinfo;
struct hour key_hours[N];
struct colour key_colours[N];
struct colour current_colour;
struct colour green, yellow, blue;
int count = 0;
int previous;
long next_time_seconds, previous_time_seconds, total_seconds;
int diff_r, diff_g, diff_b;
float percentage;
int r, g, b;

void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

struct colour time_based_colour(){

  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    current_colour.r = 0;
    current_colour.g = 99;
    current_colour.b = 99;
    return current_colour;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

  while(key_hours[count].h*60*60 + key_hours[count].m*60 + key_hours[count].s <= timeinfo.tm_hour*60*60 + timeinfo.tm_min*60 + timeinfo.tm_sec){
    count++;
    if (count == N){
      count = 0;
      break;
    }
  }
  
  if (count == 0)
    previous = N - 1;
  else
    previous = count - 1;

  if (timeinfo.tm_hour*60*60 + timeinfo.tm_min*60 + timeinfo.tm_sec >= key_hours[N-1].h*60*60 + key_hours[N-1].m*60 + key_hours[N-1].s){
    next_time_seconds = key_hours[count].h*60*60 + 24*60*60;
    previous_time_seconds = key_hours[previous].h*60*60;
  }
  else if (timeinfo.tm_hour*60*60 + timeinfo.tm_min*60 + timeinfo.tm_sec < key_hours[0].h*60*60 + key_hours[0].m*60 + key_hours[0].s){
    next_time_seconds = key_hours[count].h*60*60;
    previous_time_seconds = (key_hours[N-1].h - 24)*60*60;
  }
  else{
    next_time_seconds = key_hours[count].h*60*60 + key_hours[count].m*60 + key_hours[count].s;
    previous_time_seconds = key_hours[previous].h*60*60 + key_hours[previous].m*60 + key_hours[previous].s;
  }

  total_seconds = next_time_seconds - previous_time_seconds;
  diff_r = key_colours[count].r - key_colours[previous].r;
  diff_g = key_colours[count].g - key_colours[previous].g;
  diff_b = key_colours[count].b - key_colours[previous].b;
  
  percentage = 1 - (next_time_seconds - (timeinfo.tm_hour*60*60.0 + timeinfo.tm_min*60.0 + timeinfo.tm_sec))/total_seconds;
  Serial.println(percentage);
  current_colour.r = key_colours[previous].r + diff_r*percentage; 
  current_colour.g = key_colours[previous].g + diff_g*percentage; 
  current_colour.b = key_colours[previous].b + diff_b*percentage;  
  return current_colour;
}

bool check_colour(int x){
  return (x >= 0 && x <= 200);
}

void set_colour(int r, int g, int b, int start, int end){
  bool check_r, check_g, check_b;
  check_r = check_colour(r);
  check_g = check_colour(g);
  check_b = check_colour(b);

  if (check_r && check_g && check_b){
    if (end > 0){
      for(int i = start; i < end; i++)
        leds[i] = CRGB(r, g, b);
    }
    else{
      leds[start] = CRGB(r, g, b);
    }
  }
  return;
}

void prepare_hour_leds(int hour){
  int segments[10][7] =  {{0, 1, 2, 3, 4, 5, -1},
                          {2, 3, -1},
                          {1, 2, 4, 5, 6, -1},
                          {1, 2, 3, 4, 6, -1},
                          {0, 2, 3, 6, -1},
                          {0, 1, 3, 4, 6, -1},
                          {0, 1, 3, 4, 5, 6, -1},
                          {1, 2, 3, -1},
                          {0, 1, 2, 3, 4, 5, 6},
                          {0, 1, 2, 3, 6, -1}};

  String digit_1, digit_2;
  int first_digit, second_digit;
  if (hour < 10){
    first_digit = 0;
    second_digit = hour;
  }
  else{
    digit_1 = String(hour)[0];
    digit_2 = String(hour)[1];
    first_digit = digit_1.toInt();
    second_digit = digit_2.toInt();
  }

  for (int i = 0; i < 7 && segments[first_digit][i] != -1; i++){
    set_colour(current_colour.r, current_colour.g, current_colour.b, segments[first_digit][i]*SEGMENT_SIZE, segments[first_digit][i]*SEGMENT_SIZE+SEGMENT_SIZE);
  }

  for (int i = 0; i < 7 && segments[second_digit][i] != -1; i++){
    set_colour(current_colour.r, current_colour.g, current_colour.b, segments[second_digit][i]*SEGMENT_SIZE+DIGIT_SIZE, segments[second_digit][i]*SEGMENT_SIZE+DIGIT_SIZE+SEGMENT_SIZE);
  }
}

void prepare_minutes_leds(int min){
  int complete_leds, active_led;
  complete_leds = min/4;
  active_led = min%4;
  set_colour(current_colour.r, current_colour.g, current_colour.b, MINUTES_SIZE-complete_leds, MINUTES_SIZE); 
  switch(active_led){                                              
    case 1:
      set_colour(yellow.r, yellow.g, yellow.b, MINUTES_SIZE-complete_leds-1, -1);
      break;
    case 2:
      set_colour(green.r, green.g, green.b, MINUTES_SIZE-complete_leds-1, -1);
      break;
    case 3:
      set_colour(blue.r, blue.g, blue.b, MINUTES_SIZE-complete_leds-1, -1);
      break;
  }
}

void prepare_seconds_leds(int sec){
  int complete_leds, active_led;
  complete_leds = sec/4;
  active_led = sec%4;
  set_colour(current_colour.r, current_colour.g, current_colour.b, MINUTES_SIZE, MINUTES_SIZE+complete_leds);
  switch (active_led){                                            
    case 1:
      set_colour(yellow.r, yellow.g, yellow.b, MINUTES_SIZE+complete_leds, -1);
      break;
    case 2:
      set_colour(green.r, green.g, green.b, MINUTES_SIZE+complete_leds, -1);
      break;
    case 3:
      set_colour(blue.r, blue.g, blue.b, MINUTES_SIZE+complete_leds, -1);
      break;
  }
}

void setup() {
  Serial.begin(9600);

  
  //connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println(" CONNECTED");

  //init and get the time
  configTime(gmtOffset_sec, 0, ntpServer);
  printLocalTime();

  //initialise hours
  key_hours[0].h = 0;
  key_hours[1].h = 3;
  key_hours[2].h = 6;
  key_hours[3].h = 9;
  key_hours[4].h = 12;
  key_hours[5].h = 10;
  key_hours[6].h = 18;
  key_hours[7].h = 21;

  //initialise colours
  key_colours[0].r = 25; //25,0,25 midnight purple
  key_colours[0].g = 0;
  key_colours[0].b = 25;
  key_colours[1].r = 3; //3, 0, 43  blue
  key_colours[1].g = 7;
  key_colours[1].b = 30;
  key_colours[2].r = 20;   //70, 70, 0 yellow
  key_colours[2].g = 20;
  key_colours[2].b = 0;
  key_colours[3].r = 25;   //200, 82, 0 orange
  key_colours[3].g = 5;
  key_colours[3].b = 0;
  key_colours[4].r = 25;   //200, 0, 0 red
  key_colours[4].g = 0;
  key_colours[4].b = 0;
  key_colours[5].r = 30; //200, 82, 0 orange
  key_colours[5].g = 10;
  key_colours[5].b = 0;
  key_colours[6].r = 0;  //7, 199, 23 light green
  key_colours[6].g = 30;
  key_colours[6].b = 10;
  key_colours[7].r = 4;   //4, 118, 199 light blue
  key_colours[7].g = 7;
  key_colours[7].b = 30;
  green.r = 20;
  green.g = 20;
  green.b = 0;
  yellow.r = 0;
  yellow.g = 25;
  yellow.b = 0;
  blue.r = 25;
  blue.g = 0;
  blue.b = 0;

  FastLED.addLeds<NEOPIXEL, ledPin>(leds, NLEDS);
}

void clear_leds(){
  set_colour(0, 0, 0, 0, NLEDS);
}

void loop() {
  //Get time
  current_colour = time_based_colour();

  //Clear LEDs
  clear_leds();

  //Set hour LEDs
  prepare_hour_leds(timeinfo.tm_hour);

  //Set minutes LEDs
  prepare_minutes_leds(timeinfo.tm_min);

  //Set seconds LEDs
  prepare_seconds_leds(timeinfo.tm_sec);

  //Light the LEDs
  FastLED.show();
  
  //Just a sec...
  delay(1000);
}