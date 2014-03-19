Fribone - Arduino Yún
===========================

License GPLv2
http://www.gnu.org/licenses/gpl-2.0.html

Desarrollo
=======================

El software para Arduino Yún se divide en dos partes:

* Arduino

    Desarrollo de la comunicación con el Hardware (leds, botones, pantalla tft lcd...). Además, recibe a través del Bridge los códigos de barras del lector y a través de peticiones Http envía los datos a un servidor.

* Python (Linino S.O.)

    Desarrollo de un script en Python para procesar los dígitos enviados por el lector de código de barras al puerto USB (/dev/input/event1) y transmitidos al código Arduino a través del Bridge.