#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

byte up_arrow[] = { B00100, B01110, B11111, B00100, B00100, B00100, B00000, B00000 };
byte down_arrow[] = { B00000, B00000, B00100, B00100, B00100, B11111, B01110, B00100 };

#define new_state_1(s){state1 = s;}
#define new_state_2(s){state2 = s;}
#define purple 5
#define red 1
#define green 2
#define yellow 3
#define white 7

#define DEBUG
#ifdef DEBUG
#define debug_print(x) Serial.println(x)
#else
#define debug_print(x)
#endif

#ifdef __arm__
extern "C" char* sbrk(int incr);
#else // __ARM__
extern char *__brkval;
#endif // __arm__

int freeMemory() {
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif // __arm__
}

void sortArray(String channelsArr[], int number_of_channels) {  //function to sort the arrays into alphabetical order which store the channels
  for (int i = 0; i < (number_of_channels - 1); i++) {
    for (int j = 0; j < (number_of_channels - (i + 1)); j++) {
      char channelLetter1 = channelsArr[j].charAt(1);
      char channelLetter2 = channelsArr[j + 1].charAt(1);
      if (channelLetter1 > channelLetter2) {
        String temp = channelsArr[j];
        channelsArr[j] = channelsArr[j + 1];
        channelsArr[j + 1] = temp;
      }
    }
  }
}

boolean add_value_array(String values[], int values_array_length, String value) { //function that returns a boolean which determines if a value for a channel can be added to the array for RECENT extension
  int count = 0;//creates a count of the number of values currently defined for a channel
  boolean add_channel = false;
  for (int i = 0; i < values_array_length; i++) {
    if (values[i].substring(1, 2) == value.substring(1, 2)) { //check to see if both have the same channel letter
      count += 1;
    }
  }
  if (count < 6) {//check to see if the maximum number of values allowed to be stored for channel has been reached
    add_channel = true;
  } else {
    debug_print("Maximum values for that channel reached for RECENT extension");
  }

  return add_channel;
}

String value_format(String str) {//function which puts the correct amount of spacing between channel letter and value for lcd
  String updated_string; //stores updated string with correct spacing for printing on lcd
  switch (str.length()) {
    case 5:
      updated_string = str.substring(1, 5); //No need to add space but remove "C"
      break;
    case 4:
      updated_string = str.substring(1, 2) + " " + str.substring(2, 4);
      break;
    case 3:
      updated_string = str.substring(1, 2) + "  " + str.substring(2, 3);
      break;
  }

  if (str.length() > 5) {
    updated_string = str.substring(1, str.length()); //just return full Channel name without "C"
  }
  return updated_string;
}

boolean channel_has_value(String channel, String values_arr[], int arr_length) { //function to check whether a channel has received a value yet
  boolean value_exists = false;
  String channelLetter = channel.substring(1, 2);
  for (int i = 0; i < arr_length; i++) {
    if (channelLetter == values_arr[i].substring(1, 2)) {//compare the channel letter of the channel and the channel letters of the values in array
      value_exists = true;
    }
  }
  return value_exists;
}

void add_most_recent_value(String list[], int number_of_channels,  String value) {//function to edit the channel string in the array so that to include the most recent value next to the channel letter
  for (int i = 0; i < number_of_channels; i++) {
    if (list[i].substring(1, 2) == value.substring(1, 2)) {
      list[i] = list[i].substring(0, 2) + value.substring(2, value.length());
      switch (value.length()) {
        case 5:
          list[i] = list[i].substring(0, 2) + value.substring(2, 5);
          break;
        case 4:
          list[i] = list[i].substring(0, 2) + value.substring(2, 4);
          break;
        case 3:
          list[i] = list[i].substring(0, 2) + value.substring(2, 3);
          break;
      }
    }
  }
}

int getNumber(String str) {//returns an variable of type integer by grabbing the value portion of a string and casting it
  int x;
  switch (str.length()) {
    case 5:
      x = (str.substring(2, 5)).toInt();
      break;
    case 4:
      x = (str.substring(2, 4)).toInt();
      break;
    case 3:
      x = (str.substring(2, 3)).toInt();
      break;
  }
  return x;
}

String get_average(String value_array[], int number_of_values, String channel_array[], int channel_posistion_display) {//returns average for a channel
  String average;
  String channel = channel_array[channel_posistion_display]; //get the channel for which we want to find the average for
  int number_of_values_for_channel = 0;
  int sum = 0;

  for (int i = 0; i < number_of_values; i++) {
    if (value_array[i].substring(1, 2) == channel.substring(1, 2)) {
      sum += getNumber(value_array[i]);
      number_of_values_for_channel += 1;
    }
  }
  int x = sum / number_of_values_for_channel;
  average = String(x);//casting int to String to display on lcd

  return average;
}

void display_channel_names_and_avg(String channels[], String values[], int values_array_length, int array_length, String channels_for_display[], int channel_position_bottom, int channel_position_top) {
  for (int i = 0; i < array_length; i++) {
    if (channels[i].substring(1, 2) == channels_for_display[channel_position_top].substring(1, 2)) {
      if (channel_has_value(channels_for_display[channel_position_top], values, values_array_length)) {//Check whether there has been a value added to this channel, otherwise don't display channel name and average
        String channel_name = channels[i].substring(2, channels[i].length());
        if (channel_name.length() <= 5) {
          lcd.setCursor(11, 0);
          lcd.print(channel_name);
        }
        lcd.setCursor(6, 0);
        lcd.print("," + get_average(values, values_array_length, channels, channel_position_top));
      }
    }
  }
  for (int i = 0; i < array_length; i++) {
    if (channels[i].substring(1, 2) == channels_for_display[channel_position_bottom].substring(1, 2)) {
      if (channel_has_value(channels_for_display[channel_position_bottom], values, values_array_length)) {//Check whether there has been a value added to this channel, otherwise don't display channel name and average
        String channel_name = channels[i].substring(2, channels[i].length());
        if (channel_name.length() <= 5) {
          lcd.setCursor(11, 1);
          lcd.print(channel_name);
        }
        lcd.setCursor(6, 1);
        lcd.print("," + get_average(values, values_array_length, channels, channel_position_bottom));
      }
    }
  }
}

String get_channel_name(String channels[], int array_length, String channels_for_display[], int channel_position_lcd) { //function used to get the full channel name for a channel (SCROLL extension)
  String channel_name;
  for (int i = 0; i < array_length; i++) {
    if (channels[i].substring(1, 2) == channels_for_display[channel_position_lcd].substring(1, 2)) {
      channel_name = channels[i].substring(2, channels[i].length());
    }
  }
  return channel_name;
}

boolean scroll_top_channel_bool(String channels[], int array_length, String channels_for_display[], int channel_position_top) {//check if channel at the top of lcd needs to be scrolled
  boolean scroll_channel;
  String channel = get_channel_name(channels, array_length, channels_for_display, channel_position_top);

  if (channel.length() <= 5) {
    scroll_channel = false;
  } else {
    scroll_channel = true;
  }
  return scroll_channel;
}

boolean scroll_bottom_channel_bool(String channels[], int array_length, String channels_for_display[], int channel_position_bottom) {
  boolean scroll_channel;
  String channel = get_channel_name(channels, array_length, channels_for_display, channel_position_bottom);

  if (channel.length() <= 5) {
    scroll_channel = false;
  } else {
    scroll_channel = true;
  }
  return scroll_channel;
}

//function below to check if all recent values for channels on display are in range
byte check_recent_values_range(String channels_for_display[], int array_length, String max_min_channels[], int max_min_channels_length, String values_arr[], int arr_length) {
  byte backlight_colour = 7;
  boolean ValueOutOfRangeMax = false;
  boolean ValueOutOfRangeMin = false;
  for (int i = 0; i < array_length; i++) {
    if (channel_has_value(channels_for_display[i], values_arr, arr_length)) { //only do range check if the channel on display has received a value
      int recent_value = getNumber(channels_for_display[i]);
      String channel_letter = channels_for_display[i].substring(1, 2);
      int channelMax = 255;//stores the maximum for the recent value's channel
      int channelMin = 0;//stores the minimum for the recent values's channel

      for (int i = 0; i < max_min_channels_length; i++) { //check max/min
        if (max_min_channels[i].substring(0, 1) == "X" && max_min_channels[i].substring(1, 2) == channel_letter) {
          //if (recent_value > getNumber(max_min_channels[i])) {
          channelMax = getNumber(max_min_channels[i]);
          //}
        } else if (max_min_channels[i].substring(0, 1) == "N" && max_min_channels[i].substring(1, 2) == channel_letter) {
          //if (recent_value < getNumber(max_min_channels[i])) {
          channelMin = getNumber(max_min_channels[i]);
          //}
        }
      }
      if ((channelMax < channelMin) && recent_value > channelMax) {//takes into account if the channels maximum is set to be lower than the channels minimum (the maximum is prioritised)
        ValueOutOfRangeMax = true;
        backlight_colour = red;
        debug_print(F("RED"));
      } else if ((channelMax < channelMin) && recent_value < channelMax) {
        debug_print(F("No colour change needed"));
      } else if ((channelMax > channelMin) && recent_value > channelMax) {
        ValueOutOfRangeMax = true;
        backlight_colour = red;
      } else if ((channelMax > channelMin) && recent_value < channelMin) {
        debug_print(channelMax);
        debug_print(channelMin);
        ValueOutOfRangeMin = true;
        backlight_colour = green;
      }
    }
  }
  if (ValueOutOfRangeMax == true && ValueOutOfRangeMin == true) { //statement to check if both cases of being out of range is true so that to set backlight to yellow
    backlight_colour = yellow;
  } else if (ValueOutOfRangeMax == false && ValueOutOfRangeMin == false) {
    backlight_colour = white;
  }
  return backlight_colour;
}

enum state_e {SYNCHRONISATION = 8, CHECK_INPUT};
enum state_f {WAITING_PRESS = 8, UPDATE_DISPLAY, SCROLL_DOWN, SCROLL_UP, WAITING_RELEASE};

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(100);
  lcd.begin(16, 2);
  lcd.setBacklight(5);
  lcd.clear();
}

void loop() {
  static state_e state1 = SYNCHRONISATION;
  static state_f state2 = WAITING_PRESS;
  static byte print_q = 1; //variable that is changed when character X received
  static String channels[6] = {};//array that stores channels (letters and descriptions)
  static byte array_length = 0;
  static String max_min_channels[12] = {};//array for storing the maximim and minimum values for all declared channels
  static byte max_min_channels_length = 0;
  static String channels_for_display [6] = {};//array for storing channels that will be used to be displayed on lcd
  static byte number_of_channels_for_display = 0;
  static String values [36] = {};//array that stores the values for channels
  static byte values_array_length = 0;
  static byte channel_position_top = 0;//index for scrolling up and down on lcd
  static byte channel_position_bottom = 1;
  static long press_time;
  static byte updown; //Store which button was pressed
  static byte last_b = 0; //Store for the last button press
  static byte scrollpostop = 0;//index for scrolling name on top of lcd
  static byte scrollposbot = 0;
  static unsigned long now = millis();
  static boolean scroll_top_channel = false;//boolean used to check if the channel description at the top of the LCD needs to be scrolled for SCROLL extension
  static boolean scroll_bottom_channel = false;

  if (print_q == 1) {
    Serial.print("Q");
    delay(1000);
  }

  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');

    switch (state1) {
      case SYNCHRONISATION:
        input.trim();
        if (input == "X") {
          Serial.println(F(""));//prints empty line before printing extensions below
          Serial.println(F("UDCHARS,FREERAM,RECENT,NAMES,SCROLL"));
          lcd.setBacklight(white);
          print_q = 0;
          new_state_1(CHECK_INPUT);
          break;
        } else {
          Serial.println(F("NOT X"));
          break;
        }

      case CHECK_INPUT:
        input.trim();
        if (input.substring(0, 1) == "C") {
          boolean add_channel = true;
          for (int i = 0; i < array_length; i++) {
            if (channels[i].substring(0, 2) == input.substring(0, 2) || channels_for_display[i].substring(0, 2) == input.substring(0, 2)) {//check to see if channel already exists
              add_channel = false;
              channels[i] = input;
              channels_for_display[i] = input;
              debug_print(F("channel name updated"));
            }
          }
          if (add_channel == true && array_length < 6) {//check to see if the channel does not already exist and that the array to store channels is not full
            debug_print(F("Channel added"));
            channels[array_length] = input;//add new defined channel to array
            channels_for_display[number_of_channels_for_display] = input;
            array_length += 1;//increment count for number of channels in array
            number_of_channels_for_display += 1;

            sortArray(channels_for_display, number_of_channels_for_display);//sort channels alphabetically in channels_for_display array
            sortArray(channels, array_length);//sort channels alphabetically in channels array

            if (array_length == 1) {//first channel has now been added so must be printed to the lcd
              lcd.setCursor(0, 0);
              String str = value_format(channels_for_display[0]);
              lcd.print(str);
            } else if (array_length == 2) {
              lcd.clear();
              lcd.setCursor(0, 0);
              String str = value_format(channels_for_display[0]);
              lcd.print(str);

              lcd.setCursor(0, 1);
              String str2 = value_format(channels_for_display[1]);
              lcd.print(str2);
              display_channel_names_and_avg(channels, values, values_array_length, array_length, channels_for_display, channel_position_bottom, channel_position_top);
              scroll_top_channel = scroll_top_channel_bool(channels, array_length, channels_for_display, channel_position_top);
              scroll_bottom_channel = scroll_bottom_channel_bool(channels, array_length, channels_for_display, channel_position_bottom);

            } else if (array_length == 3) { //reprints channels on display with down arrow as a third channel has now been added
              lcd.clear();
              lcd.setCursor(0, 0);
              String str1 = value_format(channels_for_display[0]);
              lcd.print(str1);

              lcd.createChar(0, down_arrow);
              lcd.setCursor(0, 1);
              lcd.write(0);
              lcd.setCursor(1, 1);
              String str2 = value_format(channels_for_display[1]);
              lcd.print(str2);
              display_channel_names_and_avg(channels, values, values_array_length, array_length, channels_for_display, channel_position_bottom, channel_position_top);
              scroll_top_channel = scroll_top_channel_bool(channels, array_length, channels_for_display, channel_position_top);
              scroll_bottom_channel = scroll_bottom_channel_bool(channels, array_length, channels_for_display, channel_position_bottom);
            }
          }else{
            debug_print(F("Max channels reached"));
            }
          /*debug_print(F("---------------------------------"));
          debug_print(F("-----------channels--------------"));
          debug_print(F("---------------------------------"));
          for (int i = 0; i <= array_length; i++) {
            Serial.println(channels[i]);
          }
          debug_print(F("---------------------------------"));
          debug_print(F("------channels for display-------"));
          debug_print(F("---------------------------------"));
          for (int i = 0; i < number_of_channels_for_display; i++) {
            Serial.println(channels_for_display[i]);
          }
          debug_print(F("---------------------------------"));
          debug_print(F("-------------max/min-------------"));
          debug_print(F("---------------------------------"));
          for (int i = 0; i < max_min_channels_length; i++) {
            Serial.println(max_min_channels[i]);
          }
          debug_print(F("---------------------------------"));
          debug_print(F("-------------values--------------"));
          debug_print(F("---------------------------------"));
          for (int i = 0; i < values_array_length; i++) {
            Serial.println(values[i]);
          }*/
          break;
        }
        else if (input.substring(0, 1) == "V" || input.substring(0, 1) == "X" || input.substring(0, 1) == "N") {
          debug_print(F("Value/MAX/MIN"));
          boolean add_value = false;
          String channel_check = "C";
          channel_check += input.substring(1, 2);
          for (int i = 0; i < array_length; i++) {
            if (channels[i].substring(0, 2) == channel_check) {
              add_value = true;
            }
          }
          //conditional checks to see if the input has a channel, has the right format(length < 5), has a value(checking length > 2) and is within range
          if ((add_value == true && (input.length() > 2 && input.length() <= 5) && (getNumber(input) <= 255 && getNumber(input) >= 0))) {
            if (input.substring(0, 1) == "V") {//only Value inputs to be added to values array and not maximum and minimum inputs
              if (add_value_array(values, values_array_length, input)) { //check if value can be added for determining average
                values[values_array_length] = input;
                values_array_length += 1;
              }
            } else {
              boolean add_max_min = true;
              for (int i = 0; i < max_min_channels_length; i++) {
                if (max_min_channels[i].substring(0, 2) == input.substring(0, 2)) {
                  add_max_min = false;//there already exists a max/min for a certain channel so replace with new declared max/min and do not add to array
                  max_min_channels[i] = input;
                  lcd.setBacklight(check_recent_values_range(channels_for_display, array_length, max_min_channels, max_min_channels_length, values, values_array_length));//sets backlight from the byte returned by function
                }
              }
              if (add_max_min == true) {
                max_min_channels[max_min_channels_length] = input;//Adds minimum or maximum to correct array
                max_min_channels_length += 1;
                lcd.setBacklight(check_recent_values_range(channels_for_display, array_length, max_min_channels, max_min_channels_length, values, values_array_length));
              }
            }
            if (input.substring(0, 1) == "V") { //only update channel with Value input and not Maximum or Minimum
              add_most_recent_value(channels_for_display, number_of_channels_for_display, input);
              new_state_2(UPDATE_DISPLAY);
              lcd.setBacklight(check_recent_values_range(channels_for_display, array_length, max_min_channels, max_min_channels_length, values, values_array_length));
            }
          }
        } else {
          String errorMsg = F("ERROR: ");
          Serial.println(errorMsg += input);
        }
        /*debug_print(F("---------------------------------"));
        debug_print(F("-----------channels--------------"));
        debug_print(F("---------------------------------"));
        for (int i = 0; i <= array_length; i++) {
          Serial.println(channels[i]);
        }
        debug_print(F("---------------------------------"));
        debug_print(F("------channels for display-------"));
        debug_print(F("---------------------------------"));
        for (int i = 0; i < number_of_channels_for_display; i++) {
          Serial.println(channels_for_display[i]);
        }
        debug_print(F("---------------------------------"));
        debug_print(F("-------------max/min-------------"));
        debug_print(F("---------------------------------"));
        for (int i = 0; i < max_min_channels_length; i++) {
          Serial.println(max_min_channels[i]);
        }
        debug_print(F("---------------------------------"));
        debug_print(F("-------------values--------------"));
        debug_print(F("---------------------------------"));
        for (int i = 0; i < values_array_length; i++) {
          Serial.println(values[i]);
        }*/
        break;
    }
  }

  uint8_t buttons = lcd.readButtons();

  switch (state2) {
    case WAITING_PRESS:
      if (buttons & BUTTON_UP) {
        debug_print(F("pressed up"));
        new_state_2(SCROLL_UP);
        delay(100);
      }
      if (buttons & BUTTON_DOWN) {
        debug_print(F("pressed down"));
        new_state_2(SCROLL_DOWN);
        delay(100);
      } else if (buttons & BUTTON_SELECT & (print_q == 0)) {//check if syncroniation has been completed
        press_time = millis();
        updown = buttons;
        new_state_2(WAITING_RELEASE);
      }

      if (scroll_top_channel && channel_has_value(channels_for_display[channel_position_top], values, values_array_length)) { //check if the channel is long enough and has received a value
        String scroller_channel = get_channel_name(channels, array_length, channels_for_display, channel_position_top) + " ";

        if (millis() - now > 500) {
          now = millis();
          scrollpostop ++;
          if (scrollpostop >= scroller_channel.length()) {
            scrollpostop = 0;
          }
        }
        lcd.setCursor(11, 0);
        lcd.print(scroller_channel.substring(scrollpostop, scrollpostop + 16));
        lcd.setCursor(11, 0);
      }

      if (scroll_bottom_channel && channel_has_value(channels_for_display[channel_position_bottom], values, values_array_length)) {//check if the channel is long enough and has received a value
        String scroller_channel = get_channel_name(channels, array_length, channels_for_display, channel_position_bottom) + " ";

        if (millis() - now > 500) {
          now = millis();
          scrollposbot ++;
          if (scrollposbot >= scroller_channel.length()) {
            scrollposbot = 0;
          }
        }

        lcd.setCursor(11, 1);
        lcd.print(scroller_channel.substring(scrollposbot, scrollposbot + 16));
        lcd.setCursor(11, 1);
      }
      break;

    case SCROLL_UP:
      if (channel_position_top > 0) {//check if channel at top of lcd is not the first in the array by comparing its index to 0
        channel_position_top --;
        channel_position_bottom --;
        if (channel_position_top == 0) {
          lcd.clear();
          lcd.setCursor(0, 0);
          String str1 = value_format(channels_for_display[channel_position_top]);
          lcd.print(str1);

          lcd.createChar(0, down_arrow);
          lcd.setCursor(0, 1);
          lcd.write(0);
          lcd.setCursor(1, 1);
          String str2 = value_format(channels_for_display[channel_position_bottom]);
          lcd.print(str2);

          display_channel_names_and_avg(channels, values, values_array_length, array_length, channels_for_display, channel_position_bottom, channel_position_top);
          scroll_top_channel = scroll_top_channel_bool(channels, array_length, channels_for_display, channel_position_top);
          scroll_bottom_channel = scroll_bottom_channel_bool(channels, array_length, channels_for_display, channel_position_bottom);
        }
        else {
          lcd.clear();
          lcd.createChar(0, up_arrow);
          lcd.write(0);
          lcd.setCursor(1, 0);
          String str1 = value_format(channels_for_display[channel_position_top]);
          lcd.print(str1);

          lcd.createChar(1, down_arrow);
          lcd.setCursor(0, 1);
          lcd.write(1);
          lcd.setCursor(1, 1);
          String str2 = value_format(channels_for_display[channel_position_bottom]);
          lcd.print(str2);

          display_channel_names_and_avg(channels, values, values_array_length, array_length, channels_for_display, channel_position_bottom, channel_position_top);
          scroll_top_channel = scroll_top_channel_bool(channels, array_length, channels_for_display, channel_position_top);
          scroll_bottom_channel = scroll_bottom_channel_bool(channels, array_length, channels_for_display, channel_position_bottom);
        }
      }
      new_state_2(WAITING_PRESS);
      break;

    case SCROLL_DOWN:
      if (channel_position_bottom < (number_of_channels_for_display - 1)) {
        channel_position_top ++;
        channel_position_bottom ++;
        if (channel_position_bottom == number_of_channels_for_display - 1) {
          lcd.clear();
          lcd.createChar(0, up_arrow);
          lcd.write(0);
          lcd.setCursor(1, 0);
          String str1 = value_format(channels_for_display[channel_position_top]);
          lcd.print(str1);

          lcd.setCursor(0, 1);
          String str2 = value_format(channels_for_display[channel_position_bottom]);
          lcd.print(str2);

          display_channel_names_and_avg(channels, values, values_array_length, array_length, channels_for_display, channel_position_bottom, channel_position_top);
          scroll_top_channel = scroll_top_channel_bool(channels, array_length, channels_for_display, channel_position_top);
          scroll_bottom_channel = scroll_bottom_channel_bool(channels, array_length, channels_for_display, channel_position_bottom);
        }
        else {
          lcd.clear();
          lcd.createChar(0, up_arrow);
          lcd.write(0);
          lcd.setCursor(1, 0);
          String str1 = value_format(channels_for_display[channel_position_top]);
          lcd.print(str1);

          lcd.createChar(1, down_arrow);
          lcd.setCursor(0, 1);
          lcd.write(1);
          lcd.setCursor(1, 1);
          String str2 = value_format(channels_for_display[channel_position_bottom]);
          lcd.print(str2);

          display_channel_names_and_avg(channels, values, values_array_length, array_length, channels_for_display, channel_position_bottom, channel_position_top);
          scroll_top_channel = scroll_top_channel_bool(channels, array_length, channels_for_display, channel_position_top);
          scroll_bottom_channel = scroll_bottom_channel_bool(channels, array_length, channels_for_display, channel_position_bottom);
        }
      }
      new_state_2(WAITING_PRESS);
      break;

    case WAITING_RELEASE:
      if (millis() - press_time >= 1000) {
        press_time = millis();
        lcd.clear();
        lcd.setBacklight(purple);
        lcd.setCursor(0, 0);
        lcd.print(F("F116673"));//display Student ID
        lcd.setCursor(0, 1);
        lcd.print(F("SRAM: "));
        lcd.setCursor(6, 1);
        lcd.print(freeMemory());//display free SRAM
        delay(200);

        int b = lcd.readButtons();
        int released = ~b & last_b;//looking for buttons that where pressed last time and not pressed now (not now and last_time)
        last_b = b; // Save
        if (released & updown) {//button held down for longer than one second now check if button has been released
          debug_print(F("button released"));
          lcd.setBacklight(check_recent_values_range(channels_for_display, array_length, max_min_channels, max_min_channels_length, values, values_array_length));//set backlight colour from purple back to what it was before the select button was pressed and held
          new_state_2(UPDATE_DISPLAY);
        }
      } else {//Select button has not been held down for longer than 1 second so check if it has been released
        int b = lcd.readButtons();
        int released = ~b & last_b;
        last_b = b; // Save
        if (released & updown) {
          debug_print(F("button released"));
          lcd.setBacklight(check_recent_values_range(channels_for_display, array_length, max_min_channels, max_min_channels_length, values, values_array_length));
          new_state_2(UPDATE_DISPLAY);
        }
      }
      break;

    case UPDATE_DISPLAY: //case for updating the Channels currently on display after a recent value has been received
      lcd.clear();
      if (number_of_channels_for_display == 1) {
        lcd.setCursor(0, 0);
        String str1 = value_format(channels_for_display[channel_position_top]);
        lcd.print(str1);
        display_channel_names_and_avg(channels, values, values_array_length, array_length, channels_for_display, channel_position_bottom, channel_position_top);
        scroll_top_channel = scroll_top_channel_bool(channels, array_length, channels_for_display, channel_position_top);
      } else if (number_of_channels_for_display >= 3 && channel_position_top == 0) {//channels on display are the top two and there are more than 2 channels so print down arrow
        lcd.clear();
        lcd.setCursor(0, 0);
        String str1 = value_format(channels_for_display[channel_position_top]);
        lcd.print(str1);

        lcd.createChar(0, down_arrow);
        lcd.setCursor(0, 1);
        lcd.write(0);
        lcd.setCursor(1, 1);
        String str2 = value_format(channels_for_display[channel_position_bottom]);
        lcd.print(str2);

        display_channel_names_and_avg(channels, values, values_array_length, array_length, channels_for_display, channel_position_bottom, channel_position_top);
        scroll_top_channel = scroll_top_channel_bool(channels, array_length, channels_for_display, channel_position_top);
        scroll_bottom_channel = scroll_bottom_channel_bool(channels, array_length, channels_for_display, channel_position_bottom);
      } else if (channel_position_bottom == (number_of_channels_for_display - 1)) { //channels on display are the bottom two so print up arrow
        lcd.clear();
        lcd.createChar(0, up_arrow);
        lcd.write(0);
        lcd.setCursor(1, 0);
        String str1 = value_format(channels_for_display[channel_position_top]);
        lcd.print(str1);

        lcd.setCursor(0, 1);
        String str2 = value_format(channels_for_display[channel_position_bottom]);
        lcd.print(str2);
        display_channel_names_and_avg(channels, values, values_array_length, array_length, channels_for_display, channel_position_bottom, channel_position_top);
        scroll_top_channel = scroll_top_channel_bool(channels, array_length, channels_for_display, channel_position_top);
        scroll_bottom_channel = scroll_bottom_channel_bool(channels, array_length, channels_for_display, channel_position_bottom);
      } else {//print channels with both down and up arrow
        if (number_of_channels_for_display > 0) { //prevent printing of arrows if select button is pressed and this state is enetered before any channels have been defined
          lcd.clear();
          lcd.createChar(0, up_arrow);
          lcd.write(0);
          lcd.setCursor(1, 0);
          String str1 = value_format(channels_for_display[channel_position_top]);
          lcd.print(str1);

          lcd.createChar(1, down_arrow);
          lcd.setCursor(0, 1);
          lcd.write(1);
          lcd.setCursor(1, 1);
          String str2 = value_format(channels_for_display[channel_position_bottom]);
          lcd.print(str2);

          display_channel_names_and_avg(channels, values, values_array_length, array_length, channels_for_display, channel_position_bottom, channel_position_top);
          scroll_top_channel = scroll_top_channel_bool(channels, array_length, channels_for_display, channel_position_top);
          scroll_bottom_channel = scroll_bottom_channel_bool(channels, array_length, channels_for_display, channel_position_bottom);
        }
      }
      new_state_2(WAITING_PRESS);
      break;
  }
}
