// Librerías
#include <Bridge.h>

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

// Otras variables
char arrayTag[10];
char arrayCodigoBarras[12];
int r = 255;
int g = 255;
int b = 255;

unsigned long timerCheckBarCode;

void setup() {
  TFTscreen.begin();

  TFTscreen.background(r, g, b);
  TFTscreen.stroke(0, 0, 255);
  TFTscreen.setTextSize(1);
  TFTscreen.text("Cargando sistema...", 10, 15);

  pinMode(buttonUp, INPUT);
  pinMode(buttonRight, INPUT);
  pinMode(buttonDown, INPUT);
  pinMode(buttonLeft, INPUT);

  // Se carga al iniciar Linino
  Bridge.begin();
  // Let SO run scripts
  delay(10000);

  RFID.begin(9600);

  TFTscreen.background(r, g, b);
  TFTscreen.stroke(0, 0, 255);
  TFTscreen.setTextSize(1);
  TFTscreen.text("Esperando una etiqueta", 10, 15);

  timerCheckBarCode = millis();
}

void loop() {
  if(procesarEventosBotones()) {
    TFTscreen.background(r, g, b);
    TFTscreen.stroke(0, 0, 255);
    TFTscreen.setTextSize(1);
    TFTscreen.text("Esperando una etiqueta", 10, 15);
  }

  if(leerYProcesarEtiquetaRFID()) {
    TFTscreen.background(r, g, b);
    TFTscreen.stroke(0, 0, 255);
    TFTscreen.setTextSize(1);
    TFTscreen.println();

    TFTscreen.text("Etiqueta leida", 10, 15);
    TFTscreen.setTextSize(2);

    TFTscreen.text(arrayTag, 20, 40);
  }
  if(leerYProcesarCodigoBarras()) {
    TFTscreen.background(r, g, b);
    TFTscreen.stroke(0, 0, 255);
    TFTscreen.setTextSize(1);
    TFTscreen.println();

    TFTscreen.text("Código de barras leido", 10, 15);
    TFTscreen.setTextSize(2);

    TFTscreen.text(arrayCodigoBarras, 20, 40);
  }
}

boolean procesarEventosBotones() {
  buttonUpState = digitalRead(buttonUp);
  if(buttonUpState == HIGH) {
    // Morado
    r = 50;
    g = 0;
    b = 150;

    return true;
  }

  buttonRightState = digitalRead(buttonRight);
  if(buttonRightState == HIGH) {
    //Magenta
    r = 238;
    g = 22;
    b = 180;

    return true;
  }
  buttonDownState = digitalRead(buttonDown);
  if(buttonDownState == HIGH) {
    // Amarillo
    r = 238;
    g = 223;
    b = 22;

    return true;
  }

  buttonLeftState = digitalRead(buttonLeft);
  if(buttonLeftState == HIGH) {
    // Azul claro
    r = 0;
    g = 255;
    b = 255;

    return true;
  }
  return false;
}

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

    int caracteresLeidos = Bridge.get("codebar", arrayCodigoBarras, 12);

    if(caracteresLeidos != 0) {
      Bridge.put("codebar","");
      return true;
    }
  }
  return false;
}