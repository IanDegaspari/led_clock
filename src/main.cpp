#include <Arduino.h>
#include <WiFi.h>
#include <FastLED.h>
#include <string.h>
#include "time.h"
#include "structs.h"

#define N 7
#define NLEDS 60

const int ledPin =  25;      // the number of the LED pin
const char* ssid       = "GG";
const char* password   = "maisfacil";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -3*3600;

CRGB leds[NLEDS];
WiFiClient espClient;

//Variables
struct tm timeinfo;
struct colour current_colour;
struct colour aux_colour;
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
  
  struct hour key_hours[N];
  key_hours[0].h = 3;
  key_hours[1].h = 8;
  key_hours[2].h = 12;
  key_hours[3].h = 16;
  key_hours[3].m = 20;
  key_hours[4].h = 18;
  key_hours[5].h = 20;
  key_hours[6].h = 22;

  struct colour key_colours[N];
  key_colours[0].r = 0;
  key_colours[0].g = 0;
  key_colours[0].b = 255;
  key_colours[1].r = 255;
  key_colours[1].g = 0;
  key_colours[1].b = 0;
  key_colours[2].r = 3;   //3, 175, 255
  key_colours[2].g = 175;
  key_colours[2].b = 255;
  key_colours[3].r = 0;
  key_colours[3].g = 255;
  key_colours[3].b = 0;
  key_colours[4].r = 255;
  key_colours[4].g = 89;
  key_colours[4].b = 0;
  key_colours[5].r = 255; //255, 3, 131
  key_colours[5].g = 3;
  key_colours[5].b = 131;
  key_colours[6].r = 0;
  key_colours[6].g = 99;
  key_colours[6].b = 99;

  struct colour current_colour;

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

void set_colour(int r, int g, int b){
  bool check_r, check_g, check_b;
  check_r = check_colour(r);
  check_g = check_colour(g);
  check_b = check_colour(b);

  if (check_r && check_g && check_b){
    for(int i = 0; i < NLEDS; i++)
      leds[i] = CRGB(r, g, b);
    
    FastLED.show();
  }
  return;
}

void setup() {
  int count = 0;
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
  
  FastLED.addLeds<NEOPIXEL, ledPin>(leds, NLEDS);
  current_colour = time_based_colour();
}

void loop() {
  // put your main code here, to run repeatedly:
}