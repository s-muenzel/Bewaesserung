#include "WebS.h"

#include "Ventil.h"
#include "Speicher.h"

#include <FS.h>
#include <ESP8266WebServer.h>

ESP8266WebServer server(80);

bool __Admin_Mode_An;

///////// Hilfsfunktionen
//format bytes
String formatBytes(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + " B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + " KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + " MB";
  } else {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
  }
}
//Stunde aus Text
uint8_t parseZeit_Stunde(String s) {
  return min(23, max(0, int(s.toInt())));
}
//Minute aus Text
uint8_t parseZeit_Minute(String s) {
  return min(59, max(0, int(s.substring(3).toInt())));
}

void handleRoot() {
  D_PRINTLN("handleRoot");
  if (SPIFFS.exists("/top.htm")) {
    File file = SPIFFS.open("/top.htm", "r");
    server.streamFile(file, "text/html");
    file.close();
  } else {
    server.send(404, "text/plain", "file error");
  }
}

void handleCSS() {
  D_PRINTLN("handleCSS");
  if (SPIFFS.exists("/style.css")) {
    File file = SPIFFS.open("/style.css", "r");
    server.streamFile(file, "text/css");
    file.close();
  } else {
    server.send(404, "text/plain", "file error");
  }
}

void handleWeckzeit() {
  char temp[2000];
  time_t t = now(); // Store the current time in time
  time_t u = __WZ.lese_Uhrzeit();
  D_PRINTF("Webaufruf / um %2d:%02d:%02d\n", hour(t), minute(t), second(t));
  snprintf(temp, 2000,
           "<html><head><meta charset='UTF-8'><link rel='stylesheet' type='text/css' href='style.css'></head>\
<body><form action='/Setze_WZ' method='POST'><span>\
<div class='Tag'><div>Uhrzeit</div><input type='time' name='U' value='%02d:%02d'></div>\
<div class='Tag'><div>Dauer</div><input type='int' name='D' value='%03d'></div>\
<div class='Tag'><div>Max. Feuchte</div><input type='float' name='F' value='%2.1f'></div>\
</span><span><input type='submit' name='ok' value='ok'></span></form></body></html>",
           hour(u), minute(u),
           __WZ.lese_Dauer() / 60,
           __WZ.lese_Feuchte()
          );
  server.send(200, "text/html", temp);
}

void handleEinmalAus() {
  time_t t = now(); // Store the current time in time
  D_PRINTF("Webaufruf /handleEinmalAus um %2d:%2d:%2d\n", hour(t), minute(t), second(t));
  if (__WV.Laeuft()) {
    D_PRINTF("EinmalAus\n");
    __WV.Stop();
    server.sendHeader("Location", "/");
    server.send(303, "text/html", "Location: /");
  } else {
    D_PRINTF("laeuft ja gar nicht, kann kein EinmalAus machen\n");
    server.sendHeader("Location", "/");
    server.send(303, "text/html", "Location: /");
  }
}

void handleStart() {
  time_t t = now(); // Store the current time in time
  D_PRINTF("Webaufruf /Start um %2d:%2d:%2d\n", hour(t), minute(t), second(t));
  if (__Admin_Mode_An) {
    D_PRINTF("Starte Bewaesserung\n");
    __WV.Start(__WZ.lese_Dauer());
    server.sendHeader("Location", "/Konfig");
    server.send(303, "text/html", "Location: /Konfig");
  } else {
    D_PRINTLN("KEIN ADMIN MODE - tue nix\n");
    server.send(403, "text/plain", "Kein Admin-Mode!");
  }
}

void handleReset() {
  time_t t = now(); // Store the current time in time
  D_PRINTF("Webaufruf /Reset um %2d:%2d:%2d\n", hour(t), minute(t), second(t));
  if (__Admin_Mode_An) {
    D_PRINTF("Resette JETZT\n");
    server.send(200, "text/plain", "Resetting - reload now");
    delay(1000);
    ESP.restart();
  } else {
    D_PRINTLN("KEIN ADMIN MODE - tue nix\n");
    server.send(403, "text/plain", "Kein Admin-Mode!");
  }
}

void handleSetzeWeckzeit() {
  time_t t = now(); // Store the current time in time
  D_PRINTF("Webaufruf /Setze_WZ um %2d:%2d:%2d\n", hour(t), minute(t), second(t));

  for (int i = 0; i < server.args(); i++) {
    if (server.argName(i) == "U") {
      time_t u;
      tmElements_t tmSet;
      tmSet.Year = 1;
      tmSet.Month = 1;
      tmSet.Day = 1;
      tmSet.Second = 0;
      tmSet.Hour = parseZeit_Stunde(server.arg(i));
      tmSet.Minute = parseZeit_Minute(server.arg(i));
      u = makeTime(tmSet);
      __WZ.setze_Uhrzeit(u);
      D_PRINTF("Neue Uhrzeit:%02d:%02d\n", hour(u), minute(u));
    } else if (server.argName(i) == "D") {
      __WZ.setze_Dauer(server.arg(i).toInt() * 60);
      D_PRINTF("Neue Dauer:%d\n", __WZ.lese_Dauer());
    } else if (server.argName(i) == "F") {
      __WZ.setze_Feuchte(server.arg(i).toFloat());
      D_PRINTF("Neue Feuchte:%f\n", __WZ.lese_Feuchte());
    }
  }
  __WZ.speichern();
  server.sendHeader("Location", "/WeckZeit");
  server.send(303, "text/html", "Location:/WeckZeit");
}

void handleKonfig() {
  if (__Admin_Mode_An) {
    char temp[2000];
    time_t t = now(); // Store the current time in time
    D_PRINTF("Webaufruf /Konfig um %2d:%02d:%02d\n", hour(t), minute(t), second(t));
    snprintf(temp, 2000,
             "<html><head><meta charset='UTF-8'><link rel='stylesheet' type='text/css' href='style.css'></head>\
<body><form action='/Setze_Konfig' method='POST'><span>\
<div class='Tag'>Hostname<input type='text' name='H' value='%s'></div>\
</span><span><input type='submit' name='ok' value='ok'></span></form>\
<form action='/Start'><span><div class='Tag'>Starte jetzt</div></span><span><input type='submit' name='ok' value='ok'></span></form>\
<form action='/Reset'><span><div class='Tag'>Neustart</div></span><span><input type='submit' name='ok' value='ok'></span></form>\
</body></html>",
             __WZ.lese_Hostname()
            );
    server.send(200, "text/html", temp);
  } else {
    char temp[2000];
    time_t t = now(); // Store the current time in time
    D_PRINTF("Webaufruf /Konfig um %2d:%02d:%02d\n", hour(t), minute(t), second(t));
    snprintf(temp, 2000,
             "<html><head><meta charset='UTF-8'><link rel='stylesheet' type='text/css' href='style.css'></head>\
<body><span>\
<div class='Tag'>Hostname:<span class='Rechts'>%s</span></div>\
</span><span></span></body></html>",
             __WZ.lese_Hostname()
            );
    server.send(200, "text/html", temp);
  }
}

void handleSetzeKonfig() {
  time_t t = now(); // Store the current time in time
  D_PRINTF("Webaufruf /Setze_Konfig um %2d:%2d:%2d\n", hour(t), minute(t), second(t));
  if (__Admin_Mode_An) {
    for (int i = 0; i < server.args(); i++) {
      if (server.argName(i) == "H") {
        D_PRINTF("Konfig: Hostname: %s\n", server.arg(i).c_str());
        __WZ.setze_Hostname(server.arg(i).c_str());
      }
    }
    __WZ.speichern();
    server.sendHeader("Location", "/Konfig");
    server.send(303, "text/html", "Location:/Konfig");
  } else {
    D_PRINTLN("KEIN ADMIN MODE - tue nix\n");
    server.send(403, "text/plain", "Kein Admin-Mode!");
  }
}

void handleOTA() {
  if (!server.authenticate(admin_user, admin_pw)) {
    server.requestAuthentication(DIGEST_AUTH, "Admin-Mode", "Admin Mode failed");
  } else {
    __WebS.Admin_Mode();
    __OTA.Bereit();
    server.send(200, "text/plain", "OTA eingeschaltet");
  }
}

void handleStatus() {
  char temp[1000];
  time_t t = now(); // Store the current time in time
  snprintf(temp, 1000,
           "{ \"Zeit\" : \"%2d:%02d:%02d\", \"ZeitStatus\" : %d, \"Aktiv\" : %d, \"Admin\" : %d}", hour(t), minute(t), second(t), timeStatus(), __WV.Laeuft(), __Admin_Mode_An);
  server.send(200, "application/json", temp);
}

void handleFavIcon() {
  D_PRINTLN("handleFavIcon");
  if (SPIFFS.exists("/favicon.ico")) {
    File file = SPIFFS.open("/favicon.ico", "r");
    server.streamFile(file, "image/x-icon");
    file.close();
  } else {
    server.send(404, "text/plain", "file error");
  }
}

void handleNotFound() {
  time_t t = now(); // Store the current time in time
  D_PRINTF("Webaufruf - unbekannte Seite %s um %2d:%02d:%02d\n", server.uri().c_str(), hour(t), minute(t), second(t));
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ":" + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

File fsUploadFile;
void handleHochladen() {
  if (__Admin_Mode_An) {
    if (server.uri() != "/Hochladen") {
      return;
    }
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      String filename = upload.filename;
      if (!filename.startsWith("/")) {
        filename = "/" + filename;
      }
      D_PRINT("handleHochladen Name: "); D_PRINTLN(filename);
      fsUploadFile = SPIFFS.open(filename, "w");
      filename = String();
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      //D_PRINT("handleHochladen Data: "); D_PRINTLN(upload.currentSize);
      if (fsUploadFile) {
        fsUploadFile.write(upload.buf, upload.currentSize);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (fsUploadFile) {
        fsUploadFile.close();
      }
      D_PRINT("handleHochladen Groesse: "); D_PRINTLN(upload.totalSize);
    }
  }
}

void handleLoeschen() {
  if (__Admin_Mode_An) {
    if ((server.args() == 0) || (server.argName(0) != "datei")) {
      return server.send(500, "text/plain", "BAD ARGS");
    }
    String path = server.arg(0);
    D_PRINTLN("handleLoeschen: " + path);
    if (path == "/") {
      return server.send(500, "text/plain", "BAD PATH");
    }
    if (!SPIFFS.exists(path)) {
      return server.send(404, "text/plain", "FileNotFound");
    }
    SPIFFS.remove(path);
    server.sendHeader("Location", "/Dateien");
    server.send(303, "text/html", "Location:/Dateien");
    path = String();
  } else {
    D_PRINTLN("KEIN ADMIN MODE - tue nix\n");
    server.send(403, "text/plain", "Kein Admin-Mode!");
  }
}


void handleDateien() {
  D_PRINTLN("Seite handleDateien");

  String output;
  output = "<html><head><meta charset='UTF-8'><link rel='stylesheet' type='text/css' href='style.css'></head><body>";
  if (__Admin_Mode_An) {
    output += String("<form action='/Hochladen' method='post' enctype='multipart/form-data'><span><div class='Tag'><input type='file' name='name'></div></span><span><input class='button' type='submit' value='Upload'></span></form>");
  }

  Dir dir = SPIFFS.openDir("/");
  while (dir.next()) {
    File entry = dir.openFile("r");
    if (__Admin_Mode_An) {
      output += String("<form action='/Loeschen' method='post'>");
    }
    output += String("<span><div class='Tag'><span>");
    output += entry.name() + String("</span><span class='Rechts'>") + formatBytes(entry.size());
    if (__Admin_Mode_An) {
      output += String("</span><input type='text' style='display:none' name='datei' value='") + entry.name();
      output += String("'></div></span><span><input class='button' type='submit' value='l&ouml;schen'></span></form>");
    } else {
      output += String("</span></div>");
    }
    D_PRINTF("File '%s', Size %d\n", entry.name(), entry.size());
    entry.close();
  }
  output += "</body></html>";
  server.send(200, "text/html", output);
}
///////////

WebS::WebS() {
  __Admin_Mode_An = false;
}

void WebS::Beginn() {
  if (!SPIFFS.begin()) {
    D_PRINTLN("Failed to mount file system");
    SPIFFS.format();
  }
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      D_PRINTF("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
    D_PRINTF("\n");
  }

  server.on("/",              handleRoot);          // Anzeige Weckzeiten und Möglichkeit Weckzeiten zu setzen. Auch Link zu Konfig und Dateien
  server.on("/EinmalAus",     handleEinmalAus);     // Falls eine Bewässerung läuft --> Stop aufrufen
  server.on("/Status",        handleStatus);        // liefert lokale Zeit, Admin- und Aktiv-Status per JSON
  server.on("/WeckZeit",      handleWeckzeit);      // Anzeige Weckzeiten und Möglichkeit Weckzeiten zu setzen
  server.on("/Setze_WZ",      handleSetzeWeckzeit); // Speichert die neuen Weckzeiten ab
  server.on("/OTA",           handleOTA);           // Anschalten von OTA
  server.on("/Konfig",        handleKonfig);        // Zeigt die Konfig-Daten an
  server.on("/Setze_Konfig",  handleSetzeKonfig);   // Speichert die neuen Weckzeiten ab
  server.on("/Start",         handleStart);         // nur noch zu Testzwecken - startet einen Ventil
  server.on("/Reset",         handleReset);         // Neustart, nötig wenn z.B. Hostname geändert wurde oder Admin-Mode zurückgesetzt werden soll
  server.on("/Dateien",       handleDateien);       // Datei-Operationen (upload, delete)
  server.on("/Loeschen",      handleLoeschen);      // Delete (spezifische Datei)
  server.on("/Hochladen", HTTP_POST, []() { //first callback is called after the request has ended with all parsed arguments
    if (__Admin_Mode_An) {
      server.sendHeader("Location", "/Dateien");
      server.send(303, "text/html", "Location:/Dateien");
    } else {
      server.send(403, "text/plain", "Kein Admin-Mode!");
    }
  },  handleHochladen);        //second callback handles file uploads at that location
  server.on("/favicon.ico",   handleFavIcon);       // liefert das Favicon.ico
  server.on("/style.css",     handleCSS);           // liefert das Stylesheet
  server.onNotFound(          handleNotFound);      // Fallback

  server.begin();
}

void WebS::Admin_Mode() {
  __Admin_Mode_An = true;
}

void WebS::Tick() {
  server.handleClient();
}
