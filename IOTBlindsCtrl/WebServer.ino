/*
 * IOTBlindCtrl - WebServer
 * Webserver for accessing IOTBlindsCtrl and change settings
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 14-5-2021
 * Copyright: Ivo Helwegen
 */

#include "IOTWifi.h"
#include "WebServer.h"
#include "Settings.h"
#include "mqtt.h"
#include "Index.h"
#include "Json.h"
#include "Commands.h"
#include "Blind.h"
#include "Temperature.h"
#include "LightSensor.h"
#include "StateMachine.h"

WebServer server(WEB_PORT);
String cWebServer::menuIndex = "0";

cWebServer::cWebServer() { // constructor
}

void cWebServer::init() {
  /* Setup web pages: root, wifi config pages, so captive portal detectors and not found. */
  server.on("/", handleRoot);
  server.on("/wifi", handleWifi);
  server.on("/blind", handleBlind);
  server.on("/mqtt", handleMqtt);
  server.on("/log", handleLog);
  server.on("/reboot", handleReboot);
  server.on("/generate_204", handleRoot);  //Android captive portal. Maybe not needed. Might be handled by notFound handler.
  server.on("/fwlink", handleRoot);  //Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
  server.on("/menuload", handleMenuLoad);
  server.on("/homeupdate", handleHomeUpdate);
  server.on("/blindcommand", handleBlindCommand);
  server.on("/wifiload", handleWifiLoad);
  server.on("/wifilist", handleWifiList);
  server.on("/wifiupdate", handleWifiUpdate);
  server.on("/wifisave", handleWifiSave);
  server.on("/wifimiscsave", handleWifiMiscSave);
  server.on("/wifiupdateota", HTTP_POST, handleWifiUpdateOTAResult, handleWifiUpdateOTA);
  server.on("/blindload", handleBlindLoad);
  server.on("/blindsave", handleBlindSave);
  server.on("/mqttload", handleMqttLoad);
  server.on("/mqttupdate", handleMqttUpdate);
  server.on("/mqttsave", handleMqttSave);
  server.on("/logload", handleLogLoad);
  server.on("/logupdate", handleLogUpdate);
  server.on("/doreboot", handleDoReboot);
  server.onNotFound(handleNotFound);
  server.begin(); // Web server start
#ifdef DEBUG_WEBSERVER
  Serial.print("Webserver: HTTP server started\n");
#endif
}

void cWebServer::handle() {
  server.handleClient();
}

///////////// PRIVATES ///////////////////////////

boolean cWebServer::isIp(String str) {
  for (size_t i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}

/** IP to String? */
String cWebServer::toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}

 /** Handle root or redirect to captive portal */
void cWebServer::handleRoot() {
  if (captivePortal()) { // If captive portal redirect instead of displaying the page.
    return;
  }
  String Page;
  sendHeader();
  Page = webStart;
  Page += webStyle;
  Page += webBody;
  Page += webHead;
  Page += webHome;
  Page += webEnd;
  menuIndex = "1";
  server.send(200, "text/html", Page);
}

/** Redirect to captive portal if we got a request for another domain. Return true in that case so the page handler do not try to handle the request again. */
boolean cWebServer::captivePortal() {
  if (!isIp(server.hostHeader()) && server.hostHeader() != (String(iotWifi.hostname) + ".local")) {
#ifdef DEBUG_WEBSERVER
    Serial.print("Webserver: Request redirected to captive portal\n");
#endif
    server.sendHeader("Location", String("http://") + toStringIp(server.client().localIP()), true);
    server.send(302, "text/plain", "");   // Empty content inhibits Content-length header so we have to close the socket ourselves.
    server.client().stop(); // Stop is needed because we sent no content length
    return true;
  }
  return false;
}

/** Wifi config page handler */
void cWebServer::handleWifi() {
  String Page;
  sendHeader();
  Page = webStart;
  Page += webStyle;
  Page += webBody;
  Page += webHead;
  Page += webWifi;
  Page += webEnd;
  menuIndex = "2";
  server.send(200, "text/html", Page);
  server.client().stop(); // Stop is needed because we sent no content length
}

void cWebServer::handleBlind() {
  sendHeader();
  String Page;
  Page = webStart;
  Page += webStyle;
  Page += webBody;
  Page += webHead;
  Page += webBlind;
  Page += webEnd;
  menuIndex = "3";
  server.send(200, "text/html", Page);
  server.client().stop(); // Stop is needed because we sent no content length
}

void cWebServer::handleMqtt() {
  sendHeader();
  String Page;
  Page = webStart;
  Page += webStyle;
  Page += webBody;
  Page += webHead;
  Page += webMqtt;
  Page += webEnd;
  menuIndex = "4";
  server.send(200, "text/html", Page);
  server.client().stop(); // Stop is needed because we sent no content length
}

void cWebServer::handleLog() {
  sendHeader();
  String Page;
  Page = webStart;
  Page += webStyle;
  Page += webBody;
  Page += webHead;
  Page += webLog;
  Page += webEnd;
  menuIndex = "5";
  server.send(200, "text/html", Page);
}

void cWebServer::handleReboot() {
  sendHeader();
  String Page;
  Page = webStart;
  Page += webStyle;
  Page += webBody;
  Page += webHead;
  Page += webReboot;
  Page += webEnd;
  menuIndex = "6";
  server.send(200, "text/html", Page);
}

void cWebServer::handleNotFound() {
  if (captivePortal()) { // If captive portal redirect instead of displaying the error page.
    return;
  }
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += String(" ") + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  sendHeader();
  server.send(404, "text/plain", message);
}

void cWebServer::sendHeader() {
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
}

void cWebServer::handleMenuLoad() {
  JSON jString;
  jString.AddItem("index", menuIndex);
  jString.AddItem("ap", iotWifi.isAccessPoint());
  server.send(200, "text/plane", jString.GetJson());
}

String cWebServer::getBlindStatus() {
  String status = "Unknown";
  switch (stateMachine.getState()) {
    case stateMachine.ctrl:
      if (stateMachine.ManualOnly) {
        status = "Idle (manual)";
      } else {
        status = "Idle";
      }
      break;
    case stateMachine.sunny:
      status = "Sunny position";
      break;
    case stateMachine.twilight:
      status = "Twilight position";
      break;
    case stateMachine.darkened:
      status = "Hotday position";
      break;
    case stateMachine.moving:
      status = "Moving";
      break;
  }
  return status;
}

String cWebServer::getTimeStatus() {
  String status = "Unknown";
  if (Clock.isTimeSet()) {
    status = "";
  } else {
    status = "NTP Time not set";
  }
  return status;
}

void cWebServer::handleHomeUpdate() {
  JSON jString;
  jString.AddItem("time", Clock.getFormattedDate() + " " + Clock.getFormattedTime());
  jString.AddItem("timestatus", getTimeStatus());
  jString.AddItem("mqttstatus", getMqttStatus((boolean)settings.getByte(settings.UseMqtt)));
  jString.AddItem("temperature", temp.getRealTimeTemp());
  jString.AddItem("lightsensor", lightSensor.getRaw());
  jString.AddItem("blindstatus", getBlindStatus());
  jString.AddItem("position", blind.getPosition());
  server.send(200, "text/plane", jString.GetJson());
}

void cWebServer::handleBlindCommand() {
  byte Cmd = (byte)server.arg("cmd").toInt();
  cmdQueue.addCommand(Cmd);
  server.send(200, "text/plane", "Ok");
}

void cWebServer::handleWifiLoad() {
  JSON jString;
  if (server.client().localIP() == *(iotWifi.apIP)) {
    jString.AddItem("network", "Soft Access Point");
    jString.AddItem("ssid", iotWifi.APssid);
    jString.AddItem("hostname", "N/A");
    jString.AddItem("ip", toStringIp(WiFi.softAPIP()));
  } else {
    jString.AddItem("network", "Wifi network");
    jString.AddItem("ssid", WiFi.SSID());
    jString.AddItem("whostname", String(iotWifi.hostname) + ".local");
    jString.AddItem("ip", toStringIp(WiFi.localIP()));
  }
  jString.AddItem("mac", WiFi.macAddress());
  jString.AddItem("apssid", iotWifi.APssid);
  jString.AddItem("wlanssid", iotWifi.ssid);
  jString.AddItem("wlanrssi", (int)WiFi.RSSI());
  jString.AddItem("hostname", settings.getString(settings.hostname));
  jString.AddItem("ntpserver", settings.getString(settings.NtpServer));
  jString.AddItem("timezone", (signed char)settings.getByte(settings.NtpZone));
  jString.AddItem("usedst", (boolean)settings.getByte(settings.UseDST));
  jString.AddItem("appversion", String(APPVERSION));
  jString.AddItem("reboottime", Clock.getFormattedBootDate() + " " + Clock.getFormattedBootTime());
  jString.AddItem("rebootreason0", chiller.getResetReason(0));
  jString.AddItem("rebootreason1", chiller.getResetReason(1));
  jString.AddItem("heapmem", chiller.getHeapMem());
  jString.AddItem("progmem", chiller.getProgramMem());
  jString.AddItem("sdkversion", chiller.getVersion());
  jString.AddItem("cpufreq", chiller.getCPUFreq());
  server.send(200, "text/plane", jString.GetJson());
}

void cWebServer::handleWifiList() {
  JSON jString;
  int n = WiFi.scanNetworks();
  if (n > 0) {
    String WLANlist[n];
    for (int i = 0; i < n; i++) {
      JSON jItem;
      jItem.AddItem("ssid", WiFi.SSID(i));
      jItem.AddItem("content", WiFi.SSID(i) + ((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "" : "*") + " (" + WiFi.RSSI(i) + " dBm)");
      jItem.AddItem("select", WiFi.SSID(i) == iotWifi.ssid);
      WLANlist[i] = jItem.GetJson();
    }
    jString.AddArray("", WLANlist, n);
  }
  server.send(200, "text/plane", jString.GetJson());
}

void cWebServer::handleWifiUpdate() {
  JSON jString;
  jString.AddItem("wlanrssi", (int)WiFi.RSSI());
  server.send(200, "text/plane", jString.GetJson());
}

/** Handle the WLAN save form and redirect to WLAN config page again */
void cWebServer::handleWifiSave() {
  iotWifi.ssid = server.arg("inetwork");
  String password = server.arg("ipassword");
  settings.set(settings.ssid, iotWifi.ssid);
  settings.set(settings.password, password);
  settings.update();
  server.sendHeader("Location", "wifi", true);
  server.send(302, "text/plain", "");    // Empty content inhibits Content-length header so we have to close the socket ourselves.
  server.client().stop(); // Stop is needed because we sent no content length
  iotWifi.connect();
}

void cWebServer::handleWifiMiscSave() {
  byte bval;
  String sval;
  sval = server.arg("hostname");
  settings.set(settings.hostname, sval);
  sval = server.arg("ntpserver");
  settings.set(settings.NtpServer, sval);
  bval = (byte)server.arg("timezone").toInt();
  settings.set(settings.NtpZone, bval);
  bval = (byte)(server.arg("usedst")=="on");
  settings.set(settings.UseDST, bval);
  settings.update();
  Clock.updateSettings();
  server.sendHeader("Location", "wifi", true);
  server.send(302, "text/plain", "");    // Empty content inhibits Content-length header so we have to close the socket ourselves.
  server.client().stop(); // Stop is needed because we sent no content length
}

void cWebServer::handleWifiUpdateOTA() {
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
#ifdef DEBUG_WEBSERVER
    Serial.printf("Update: %s\n", upload.filename.c_str());
#endif
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
#ifdef DEBUG_WEBSERVER
      Update.printError(Serial);
#endif
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    /* flashing firmware to ESP*/
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
#ifdef DEBUG_WEBSERVER
      Update.printError(Serial);
#endif
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (Update.end(true)) { //true to set the size to the current progress
#ifdef DEBUG_WEBSERVER
      Serial.printf("Update Success: %u\nLet's reboot\n", upload.totalSize);
#endif
    } else {
#ifdef DEBUG_WEBSERVER
      Update.printError(Serial);
#endif
    }
  }
}

void cWebServer::handleWifiUpdateOTAResult() {
  if (Update.hasError()) {
    server.send(400, "text/plain", Update.errorString());
    server.client().stop();
  } else {
    handleDoReboot();
  }
}

void cWebServer::handleBlindLoad() {
  JSON jString;
  float ratio;
  float threshold;
  float sampletime;
  short itime;
  threshold = (float)settings.getShort(settings.OutputThreshold);
  sampletime = (float)settings.getShort(settings.SampleTime);
  itime = round((threshold/(float)settings.getShort(settings.TwilightIGain))*(sampletime/MILLI));
  jString.AddItem("twilightitime", itime);
  itime = round((threshold/(float)settings.getShort(settings.SunnyIGain))*(sampletime/MILLI));
  jString.AddItem("sunnyitime", itime);
  jString.AddItem("sampletime", (unsigned short)sampletime);
  ratio = round((threshold/MAXSHORT) * PERCENT);
  jString.AddItem("outputratio", ratio);
  jString.AddItem("opentime", settings.getShort(settings.FixedOpen));
  jString.AddItem("closetime", settings.getShort(settings.FixedClose));
  jString.AddItem("twilightthreshold", settings.getShort(settings.TwilightThreshold));
  jString.AddItem("twilighthysteresis", settings.getShort(settings.TwilightHysterises));
  jString.AddItem("sunnythreshold", settings.getShort(settings.SunnyThreshold));
  jString.AddItem("sunnyhysteresis", settings.getShort(settings.SunnyHysterises));
  jString.AddItem("sunnyenabletemp", settings.getFloat(settings.SunnyEnableTemp));
  jString.AddItem("hotdaytemp", settings.getFloat(settings.HotDayTemp));
  jString.AddItem("hotdayenabletimeout", settings.getShort(settings.HotDayEnableTimeout));
  jString.AddItem("sensorenabled", (boolean)settings.getByte(settings.SensorEnabled));
  jString.AddItem("motormovetime", settings.getShort(settings.MotorMoveTime));
  jString.AddItem("motorsyncperc", settings.getByte(settings.MotorSyncPerc));
  jString.AddItem("motorsunnypos", settings.getByte(settings.MotorSunnyPos));
  jString.AddItem("motorenabled", (boolean)settings.getByte(settings.MotorEnabled));
  server.send(200, "text/plane", jString.GetJson());
}

void cWebServer::handleBlindSave() {
  unsigned short val;
  float fval;
  byte bval;
  float threshold;
  float sampletime;
  short itime;
  fval = server.arg("outputratio").toFloat();
  threshold = (float)(MAXSHORT/(PERCENT/fval));
  sampletime = server.arg("sampletime").toFloat();
  itime = server.arg("twilightitime").toFloat();
  val = (unsigned short)round((threshold/itime)*(sampletime/MILLI));
  settings.set(settings.TwilightIGain, val);
  itime = server.arg("sunnyitime").toFloat();
  val = (unsigned short)round((threshold/itime)*(sampletime/MILLI));
  settings.set(settings.SunnyIGain, val);
  val = (unsigned short)sampletime;
  settings.set(settings.SampleTime, val);
  val = (unsigned short)threshold;
  settings.set(settings.OutputThreshold, val);
  val = (unsigned short)server.arg("opentimeh").toInt() * HMIN + (unsigned short)server.arg("opentimem").toInt();
  settings.set(settings.FixedOpen, val);
  val = (unsigned short)server.arg("closetimeh").toInt() * HMIN + (unsigned short)server.arg("closetimem").toInt();
  settings.set(settings.FixedClose, val);
  val = (unsigned short)server.arg("twilightthreshold").toInt();
  settings.set(settings.TwilightThreshold, val);
  val = (unsigned short)server.arg("twilighthysteresis").toInt();
  settings.set(settings.TwilightHysterises, val);
  val = (unsigned short)server.arg("sunnythreshold").toInt();
  settings.set(settings.SunnyThreshold, val);
  val = (unsigned short)server.arg("sunnyhysteresis").toInt();
  settings.set(settings.SunnyHysterises, val);
  fval = server.arg("sunnyenabletemp").toFloat();
  settings.set(settings.SunnyEnableTemp, fval);
  fval = server.arg("hotdaytemp").toFloat();
  settings.set(settings.HotDayTemp, fval);
  val = (unsigned short)server.arg("hotdayenabletimeouth").toInt() * HMIN + (unsigned short)server.arg("hotdayenabletimeoutm").toInt();
  settings.set(settings.HotDayEnableTimeout, val);
  bval = (byte)(server.arg("sensorenabled")=="on");
  settings.set(settings.SensorEnabled, bval);
  val = (unsigned short)server.arg("motormovetime").toInt();
  settings.set(settings.MotorMoveTime, val);
  bval = (byte)server.arg("motorsyncperc").toInt();
  settings.set(settings.MotorSyncPerc, bval);
  bval = (byte)server.arg("motorsunnypos").toInt();
  settings.set(settings.MotorSunnyPos, bval);
  bval = (byte)(server.arg("motorenabled")=="on");
  settings.set(settings.MotorEnabled, bval);
  settings.update();
  server.sendHeader("Location", "blind", true);
  server.send(302, "text/plain", "");    // Empty content inhibits Content-length header so we have to close the socket ourselves.
  server.client().stop(); // Stop is needed because we sent no content length
}

String cWebServer::getMqttStatus(boolean UseMqtt) {
  String status = "Disabled";
  if (UseMqtt) {
    if (mqtt.connected) {
      status = "Connected";
    } else {
      status = "Disconnected";
    }
  }
  return status;
}

void cWebServer::handleMqttLoad() {
  JSON jString;
  boolean UseMqtt = (boolean)settings.getByte(settings.UseMqtt);
  jString.AddItem("clientid", mqtt.clientId);
  jString.AddItem("status", getMqttStatus(UseMqtt));
  jString.AddItem("brokeraddress", settings.getString(settings.brokerAddress));
  jString.AddItem("mqttport", settings.getShort(settings.mqttPort));
  jString.AddItem("mqttusername", settings.getString(settings.mqttUsername));
  jString.AddItem("mqttpassword", settings.getString(settings.mqttPassword));
  jString.AddItem("maintopic", settings.getString(settings.mainTopic));
  jString.AddItem("mqttqos", settings.getByte(settings.mqttQos));
  jString.AddItem("mqttretain", (boolean)settings.getByte(settings.mqttRetain));
  jString.AddItem("usemqtt", UseMqtt);
  int publishLen = (sizeof(PublishTopics) / sizeof(topics));
  int subscribeLen = (sizeof(SubscribeTopics) / sizeof(topics));
  String arraystr[publishLen + subscribeLen];
  for (int i = 0; i < publishLen; i++) {
    JSON jArrItem;
    jArrItem.AddItem("tag", PublishTopics[i].tag);
    jArrItem.AddItem("topic", mqtt.buildTopic(PublishTopics[i].tag));
    jArrItem.AddItem("value", mqtt.getValue(PublishTopics[i].tag));
    jArrItem.AddItem("description", PublishTopics[i].description);
    arraystr[i] = jArrItem.GetJson();
  }
  for (int i = 0; i < subscribeLen; i++) {
    JSON jArrItem;
    jArrItem.AddItem("tag", SubscribeTopics[i].tag);
    jArrItem.AddItem("topic", mqtt.buildTopic(SubscribeTopics[i].tag));
    jArrItem.AddItem("value", String(""));
    jArrItem.AddItem("description", SubscribeTopics[i].description);
    arraystr[publishLen + i] = jArrItem.GetJson();
  }
  jString.AddArray("topics", arraystr, publishLen + subscribeLen);
  server.send(200, "text/plane", jString.GetJson());
}

void cWebServer::handleMqttUpdate() {
  JSON jString;
  boolean UseMqtt = (boolean)settings.getByte(settings.UseMqtt);
  jString.AddItem("clientid", mqtt.clientId);
  jString.AddItem("status", getMqttStatus(UseMqtt));
  int publishLen = (sizeof(PublishTopics) / sizeof(topics));
  String arraystr[publishLen];
  for (int i = 0; i < publishLen; i++) {
    JSON jArrItem;
    jArrItem.AddItem("tag", PublishTopics[i].tag);
    jArrItem.AddItem("value", mqtt.getValue(PublishTopics[i].tag));
    arraystr[i] = jArrItem.GetJson();
  }
  jString.AddArray("topics", arraystr, publishLen);
  server.send(200, "text/plane", jString.GetJson());
}

void cWebServer::handleMqttSave() {
  String sval;
  String sval2;
  byte bval;
  unsigned short val;
  sval = server.arg("brokeraddress");
  settings.set(settings.brokerAddress, sval);
  val = (unsigned short)server.arg("mqttport").toInt();
  settings.set(settings.mqttPort, val);
  sval = server.arg("mqttusername");
  settings.set(settings.mqttUsername, sval);
  sval = server.arg("mqttpassword");
  settings.set(settings.mqttPassword, sval);
  sval = server.arg("maintopic");
  sval2 = mqtt.fixTopic(sval);
  settings.set(settings.mainTopic, sval2);
  bval = (byte)server.arg("mqttqos").toInt();
  settings.set(settings.mqttQos, bval);
  bval = (byte)(server.arg("mqttretain")=="on");
  settings.set(settings.mqttRetain, bval);
  bval = (byte)(server.arg("usemqtt")=="on");
  settings.set(settings.UseMqtt, bval);
  settings.update();
  server.sendHeader("Location", "mqtt", true);
  server.send(302, "text/plain", "");    // Empty content inhibits Content-length header so we have to close the socket ourselves.
  server.client().stop(); // Stop is needed because we sent no content length
}

void cWebServer::handleLogLoad() {
  JSON jString;
  String datastr[2];

  datastr[0] = "Logging started @ " + Clock.getFormattedDate() + " " + Clock.getFormattedTime();
  datastr[1] = "Time, LightSensor (I-sun, I-twi), Temperature, Position, Blind status";
  jString.AddArray("", datastr, 2);
  server.send(200, "text/plane", jString.GetJson());
}

void cWebServer::handleLogUpdate() {
  JSON jString;
  String datastr[1];
  datastr[0] = Clock.getFormattedTime() + ", " + String(lightSensor.getRaw()) + " (" + String(lightSensor.OutSunny) + ", " + String(lightSensor.OutTwilight) + "), " + String(temp.getRealTimeTemp()) + ", " + String(blind.getPosition()) + ", " + getBlindStatus();
  jString.AddArray("", datastr, 1);
  server.send(200, "text/plane", jString.GetJson());
}

void cWebServer::handleDoReboot() {
#ifdef DEBUG_WEBSERVER
  Serial.print("Rebooting ...\n");
#endif
  sendHeader();
  String Page;
  Page = String(webRebooting);
  server.send(200, "text/html", Page);
  server.client().stop(); // Stop is needed because we sent no content length
  ESP.restart();
}

cWebServer webServer;
