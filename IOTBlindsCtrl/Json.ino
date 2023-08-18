/* 
 * IOTBlindCtrl - LEDs
 * Simple JSON encoder
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 23-5-2021
 * Copyright: Ivo Helwegen
 */

#include "Json.h"

JSON::JSON() {
  Clear();
}

String JSON::GetJson() {
  if (arrayOnly) {
    return JsonString;
  } else {
    return "{" + JsonString + "}";
  }
}

void JSON::AddItem(String tag, JSON item) {
  String result = "";
  if ((tag.length() == 0) || (arrayOnly)) {
    return;
  }    
  result = "\"" + tag + "\":" + item.GetJson();
  AddToJsonString(result);
}

void JSON::AddItem(String tag, String item) {
  String result = "";
  if ((tag.length() == 0) || (arrayOnly)) {
    return;
  }    
  result = "\"" + tag + "\":\"" + item + "\"";
  AddToJsonString(result);
}

void JSON::AddItem(String tag, char* item) {
  String result = "";
  if ((tag.length() == 0) || (arrayOnly)) {
    return;
  }    
  result = "\"" + tag + "\":\"" + String(item) + "\"";
  AddToJsonString(result);
}

void JSON::AddItem(String tag, int item) {
  String result = "";
  if ((tag.length() == 0) || (arrayOnly)) {
    return;
  } 
  result = "\"" + tag + "\":" + String(item);
  AddToJsonString(result);   
}

void JSON::AddItem(String tag, float item) {
  String result = "";
  if ((tag.length() == 0) || (arrayOnly)) {
    return;
  }
  result = "\"" + tag + "\":" + String(item);
  AddToJsonString(result);
}

void JSON::AddItem(String tag, boolean item) {
  String result = "";
  String sItem = "false";
  if ((tag.length() == 0) || (arrayOnly)) {
    return;
  }
  if (item) {
    sItem = "true";
  }
  result = "\"" + tag + "\":" + sItem;
  AddToJsonString(result);
}

void JSON::AddArray(String tag, String item[], int n) {
  String result = "";
  if (tag.length() == 0) {
    arrayOnly = true;
  } else if (arrayOnly) {
    return;
  } else {
    result = "\"" + tag + "\":";
  }
  result += "[";
  for(int i = 0; i < n; i++) {
    if (i > 0) {
      result += ",";
    }
    if ((item[i].charAt(0) == '{') || (item[i].charAt(0) == '[')) {
      result += item[i];
    } else {
      result += "\"" + item[i] + "\"";
    }
  }
  result += "]";
  AddToJsonString(result);
}

void JSON::Clear() {
  JsonString = "";
  arrayOnly = false;
}

///////////// PRIVATES ///////////////////////////

void JSON::AddToJsonString(String result) {
  if ((JsonString.length() == 0) || (arrayOnly)) {
    JsonString = result;
  } else {
    JsonString += "," + result;
  }
}
