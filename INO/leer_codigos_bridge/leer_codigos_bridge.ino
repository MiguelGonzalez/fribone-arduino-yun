#include <Bridge.h>
#include <Process.h>
#include "HttpClient.h"

String urlRest = "http://www.miguelgonzalezgomez.es/fribone/public/index.php/user/miguelgonzalezgomez@gmail.com/product/";
char codeBarValue[24];
HttpClient client;

void setup() {
  pinMode(5, OUTPUT); //Sin conexion
  pinMode(6, OUTPUT); //Con conexion
  pinMode(7, OUTPUT); //Aviso Codigo Barras Leido
  //Serial.begin(9600);
   
  digitalWrite(5, HIGH);
  digitalWrite(6, LOW);
  digitalWrite(7, LOW);
   
  Bridge.begin();
  delay(2000);
  //Serial.println("Proceso lanzado");
}

void loop() {
  if(hayWifi()) {
    digitalWrite(5, LOW);
    digitalWrite(6, HIGH);
    
    leerYProcesarCodigoBarras();
    
    delay(250);
  } else {
    delay(5000);
  }
}

boolean hayWifi() {
  Process wifiCheck;
  wifiCheck.runShellCommand("/usr/bin/pretty-wifi-info.lua");
  
  String responseWifi = "";
  while (wifiCheck.available() > 0) {
    responseWifi += wifiCheck.read();
  }
  if(responseWifi.indexOf("00:00:00:00:00:00") == -1) {
    //Serial.println("Hay conexin WiFi");
    return true;
  } else {
    //Serial.println("No hay conexin WiFi"); 
    return false;
  }  
}

void leerYProcesarCodigoBarras() {
  int caracteresLeidos = Bridge.get("codebar",codeBarValue,24);
 
  if(caracteresLeidos != 0) {
    digitalWrite(7, HIGH);
    Bridge.put("codebar","");
    //Serial.println("Codebar: " + (String)codeBarValue);
    
    String requestUrl = urlRest + codeBarValue;
    client.get(requestUrl);
    
    delay(250);
    digitalWrite(7, LOW);
  }
}
