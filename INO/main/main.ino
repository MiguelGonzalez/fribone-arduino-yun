// Librerías
#include <Bridge.h>
#include <FileIO.h>
#include "HttpClient.h"

// Pantalla TFT
#include <SPI.h>
#include <TFT.h>
#define sd_cs  4
#define lcd_cs 10
#define dc     9
#define rst    8

TFT TFTscreen = TFT(lcd_cs, dc, rst);

//Lector RFID
#include <SoftwareSerial.h>
#define rxPin 11
#define txPin 12
SoftwareSerial RFID = SoftwareSerial(rxPin, txPin); // RX and TX

// Botones
const int buttonUp = 13;
const int buttonRight = 7;
const int buttonDown = 6;
const int buttonLeft = 5;

int buttonUpState = 0;
int buttonRightState = 0;
int buttonDownState = 0;
int buttonLeftState = 0;

// Servidor
HttpClient client;
const String urlRest = "http://www.fribone.miguelgonzalezgomez.es/rest/";
const String vincularLectorRest = "activar/";
const String entrarProductoRest = "entrar/";
const String sacarProductoRest = "sacar/";

// Otras variables
char arrayTag[10];
char arrayCodigoBarras[13];
String token = "";
String idFrigorifico = "";
char stateApplication;
unsigned long timerCheckBarCode;

//Variables para vincular el lector
boolean leidoPrimerCodigoBarras;
char codigoBarrasUno[13];
char codigoBarrasDos[13];

void setup() {
  TFTscreen.begin();

  TFTscreen.background(255, 255, 255);
  TFTscreen.stroke(0, 0, 255);
  TFTscreen.setTextSize(1);
  TFTscreen.text("Cargando sistema...", 10, 15);

  TFTscreen.stroke(0, 0, 0);
  TFTscreen.setTextSize(1);

  pinMode(buttonUp, INPUT);
  pinMode(buttonRight, INPUT);
  pinMode(buttonDown, INPUT);
  pinMode(buttonLeft, INPUT);

  // Se carga al iniciar Linino
  Bridge.begin();
  FileSystem.begin();
  // Let SO run scripts
  delay(1000);

  RFID.begin(9600);
  timerCheckBarCode = millis();

  if(FileSystem.exists("/mnt/sda1/data/token.txt")) {
    File dataFile = FileSystem.open("/mnt/sda1/data/token.txt", FILE_READ);
    char charLeido;
    while((charLeido = dataFile.read()) != -1) {
      token += charLeido;
    }
    dataFile.close();
    stateApplication = '1';
  } else {
    leidoPrimerCodigoBarras = false;
    stateApplication = '0';
  }
}

void loop() {
  if(stateApplication == '1') {
    logger("Inicializando sketch");
    logger("Token leido:");
    logger(token);

    printHome();
  } else if(stateApplication == '0') {
    logger("Inicializando sketch");
    logger("Token inexistente");

    printVincular();
  }

  procesarEventosBotones();

  if(leerYProcesarEtiquetaRFID()) {
    if(stateApplication == 'E') {
      entrarProducto(false);
    }
    if(stateApplication == 'S') {
      sacarProducto(false);
    }
  }
  if(leerYProcesarCodigoBarras()) {
    if(stateApplication == 'V') {
      if(!leidoPrimerCodigoBarras) {
        leidoPrimerCodigoBarras = true;
        int i;
        for(i = 0; i<12; i++) {
          codigoBarrasUno[i] = arrayCodigoBarras[i];
        }
        codigoBarrasUno[12] = '\0';

        logger("Leído código de barras uno:");
        loggerArray(codigoBarrasUno);
      } else {
        int i;
        for(i = 0; i<12; i++) {
          codigoBarrasDos[i] = arrayCodigoBarras[i];
        }
        codigoBarrasDos[12] = '\0';

        logger("Leído código de barras dos:");
        loggerArray(codigoBarrasDos);

        iniciarVinculacion();
      }
    }
    TFTscreen.text(arrayCodigoBarras, 28, 40);
    if(stateApplication == 'E') {
      entrarProducto(true);
    }
    if(stateApplication == 'S') {
      sacarProducto(true);
    }
  }
}

boolean procesarEventosBotones() {
  buttonUpState = digitalRead(buttonUp);
  if(buttonUpState == HIGH) {
    if(stateApplication == 'H') {
      printSeleccionarFrigo();
    }

    return true;
  }

  buttonRightState = digitalRead(buttonRight);
  if(buttonRightState == HIGH) {
    if(stateApplication == 'H') {
      printSacarProducto();
    }

    return true;
  }
  buttonDownState = digitalRead(buttonDown);
  if(buttonDownState == HIGH) {
    if(stateApplication == 'H') {
      printDesvincular();
    } else if(stateApplication == 'D') {
      FileSystem.remove("/mnt/sda1/data/token.txt");

      printVincular();
    }

    return true;
  }

  buttonLeftState = digitalRead(buttonLeft);
  if(buttonLeftState == HIGH) {
    if(stateApplication == 'H') {
      printEntrarProducto();
    } else if(stateApplication != 'H') {
      printHome();
    }

    return true;
  }
  return false;
}

/*
 * Eventos contra el servidor
*/

void iniciarVinculacion() {
  String requestUrl = urlRest + vincularLectorRest + codigoBarrasUno + "/" + codigoBarrasDos;
  logger("Iniciando vinculación");
  logger("URL: " + requestUrl);
  client.get(requestUrl);

  String response = "";
  char charIn;
  while (client.available()) {
    charIn = client.read();
    response += charIn;
  }
  logger("Respuesta recibida: " + response);

  if(response == "" || response == "ERROR") {
    printErrorVincular();
  } else {
    token = response;

    File dataFile = FileSystem.open("/mnt/sda1/data/token.txt", FILE_WRITE);
    int i;
    for(i=0; i<token.length(); i++) {
      dataFile.write(token[i]);
    }

    dataFile.close();
    printHome();
  }
}

void entrarProducto(boolean esCodigoBarras) {
  String requestUrl = urlRest + entrarProductoRest + token + "/" + idFrigorifico + "/";
  if(esCodigoBarras) {
    requestUrl += arrayCodigoBarras;
  } else {
    requestUrl += arrayTag;
  }

  String response = "";
  char charIn;
  while (client.available()) {
    charIn = client.read();
    response += charIn;
  }

  if(response == "" || response == "ERROR") {

  } else {

  }
}

void sacarProducto(boolean esCodigoBarras) {
  String requestUrl = urlRest + sacarProductoRest + token + "/" + idFrigorifico + "/";
  if(esCodigoBarras) {
    requestUrl += arrayCodigoBarras;
  } else {
    requestUrl += arrayTag;
  }

  String response = "";
  char charIn;
  while (client.available()) {
    charIn = client.read();
    response += charIn;
  }

  if(response == "" || response == "ERROR") {

  } else {

  }
}

/*
 * Lectura de RFID y Códigos de Barras
*/

boolean leerYProcesarEtiquetaRFID() {
  int leidoTag = 0;

  String tag;
  while(RFID.available() > 0) {
      leidoTag = 1;
      int rfid = RFID.read();

      if(rfid != 13 && rfid != 10) {
         tag += (char)rfid;
      }
  }
  if(leidoTag) {
    tag.toCharArray(arrayTag, 10);
  }

  return leidoTag != 0;
}

boolean leerYProcesarCodigoBarras() {
  // Each 500 msg
  if(millis() - timerCheckBarCode > 500) {
    timerCheckBarCode = millis();

    int caracteresLeidos = Bridge.get("codebar", arrayCodigoBarras, 13);

    if(caracteresLeidos != 0) {
      Bridge.put("codebar","");
      return true;
    }
  }
  return false;
}

/*
 * Interfaz
*/

void printHome() {
  stateApplication = 'H';
  TFTscreen.background(255, 255, 255);

  TFTscreen.fill(204,204,204);

  TFTscreen.rect(0, 0, 160, 25);
  TFTscreen.rect(0, 48, 75, 35);
  TFTscreen.rect(85, 48, 75, 35);
  TFTscreen.rect(0, 103, 160, 25);

  TFTscreen.text("Seleccion frigo.", 28, 10);
  TFTscreen.text("Entrar", 5, 57);
  TFTscreen.text("Productos", 5, 67);
  TFTscreen.text("Sacar", 95, 57);
  TFTscreen.text("Productos", 95, 67);
  TFTscreen.text("Desvincular", 47, 112);
}

void printVincular() {
  stateApplication = 'V';
  TFTscreen.background(160, 160, 160);

  TFTscreen.rect(0, 0, 160, 25);

  TFTscreen.text("Vincular lector", 28, 10);
}

void printErrorVincular() {
  stateApplication = 'V';
  TFTscreen.background(160, 160, 160);

  TFTscreen.rect(0, 0, 160, 25);

  TFTscreen.text("Vincular lector", 28, 10);
  TFTscreen.text("ERROR", 28, 40);
  TFTscreen.text("Vuelva a intentarlo", 28, 60);
  leidoPrimerCodigoBarras = false;
}


void printEntrarProducto() {
  stateApplication = 'E';
  TFTscreen.background(160, 250, 160);

  TFTscreen.rect(0, 0, 160, 25);
  TFTscreen.text("Entrar productos", 28, 10);

  TFTscreen.fill(204,204,204);
  TFTscreen.rect(0, 28, 60, 25);
  TFTscreen.text("Volver", 5, 37);
}

void printSacarProducto() {
  stateApplication = 'S';
  TFTscreen.background(250, 160, 160);

  TFTscreen.rect(0, 0, 160, 25);
  TFTscreen.text("Sacar productos", 28, 10);

  TFTscreen.fill(204,204,204);
  TFTscreen.rect(0, 28, 60, 25);
  TFTscreen.text("Volver", 5, 37);
}

void printSeleccionarFrigo() {
  stateApplication = 'S';
  TFTscreen.background(255,210,165);

  TFTscreen.rect(0, 0, 160, 25);
  TFTscreen.text("Seleccionar frigo", 28, 10);

  TFTscreen.fill(204,204,204);
  TFTscreen.rect(0, 28, 60, 25);
  TFTscreen.text("Volver", 5, 37);
}

void printDesvincular() {
  stateApplication = 'D';
  TFTscreen.background(160, 160, 160);

  TFTscreen.rect(0, 0, 160, 25);

  TFTscreen.text("Desvincular lector", 28, 10);

  TFTscreen.fill(204,204,204);
  TFTscreen.rect(0, 28, 60, 25);
  TFTscreen.text("Volver", 5, 37);

  TFTscreen.rect(0, 103, 160, 25);
  TFTscreen.text("ACEPTAR", 47, 112);
}

/*
 * Logger
*/

void logger(String data) {
  File dataFile = FileSystem.open("/mnt/sda1/data/log.txt", FILE_APPEND);
  dataFile.println(data);
  /*int i;
  for(i = 0; i<data.length(); i++) {
    dataFile.write(data[i]);
  }
  dataFile.write('\n');*/
  dataFile.close();
}

void loggerArray(char texto[]) {
  File dataFile = FileSystem.open("/mnt/sda1/data/log.txt", FILE_APPEND);
  int i = 0;
  while(texto[i] != '\0') {
    dataFile.write(texto[i]);
    i++;
  }
  dataFile.write('\n');
  dataFile.close();
}