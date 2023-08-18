/* 
 * IOTBlindCtrl - LEDs
 * Simple JSON encoder
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 23-5-2021
 * Copyright: Ivo Helwegen
 */

#ifndef JSON_h
#define JSON_h

#define NUMITEMS(arg) ((unsigned int) (sizeof (arg) / sizeof (arg [0])))

class JSON {
public:
  JSON();
  String GetJson();
  void AddItem(String tag, JSON item);
  void AddItem(String tag, String item);
  void AddItem(String tag, char* item);
  void AddItem(String tag, int item);
  void AddItem(String tag, float item);
  void AddItem(String tag, boolean item);
  void AddArray(String tag, String item[], int n);
  void Clear();
private:
  void AddToJsonString(String result);
  String JsonString;
  boolean arrayOnly;
};

#endif
