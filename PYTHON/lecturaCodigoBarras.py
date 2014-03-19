#!/usr/bin/python
# -*- coding: utf-8 -*-

import struct
import time
import sys
sys.path.insert(0, '/usr/lib/python2.7/bridge/')

from bridgeclient import BridgeClient as bridgeclient

# Funciones

def esTeclaNumerica(value):
    return value >= 458782 and value <= 458791

def getNumero(value):
    return (value - 458781)%10

def esTeclaLetra(value):
    return (value >= 458756 and value <= 458781)

def getLetra(value):
    posicionValue = value - 458756
    asciiValue = posicionValue + 65
    return str(unichr(asciiValue))

# 458792 -> Intro
# 458976 + 458765 (Control + j => Line Feed)
def esTeclaControl(value):
    return value == 458792 or \
            value == 458976 or \
            value == 458765

# Inicio programa

infile_path = "/dev/input/event1"

#long int, long int, unsigned short, unsigned short, unsigned int
FORMAT = 'llHHI'
EVENT_SIZE = struct.calcsize(FORMAT)

#open file in binary mode
in_file = open(infile_path, "rb")

event = in_file.read(EVENT_SIZE)

teclasPulsadas = dict()
escribiendoLetras = False
codigoBarras = ''
bridgeCliente = bridgeclient()

while event:
    (tv_sec, tv_usec, type, code, value) = struct.unpack(FORMAT, event)

    # Importa este orden de la condición, si se altera puede imprimir
    # carácteres escritos muy deprisa en otro orden
    if value not in teclasPulsadas:
        teclasPulsadas[value] = 'S'
    else:
        if esTeclaControl(value) and escribiendoLetras:
            escribiendoLetras = False
            # print codigoBarras

            bridgeCliente.put('codebar', codigoBarras)

            codigoBarras = ''
        elif not esTeclaControl(value) and \
                (esTeclaNumerica(value) or \
                esTeclaLetra(value)):
            escribiendoLetras = True
            if esTeclaNumerica(value):
                codigoBarras += str(getNumero(value))
            elif esTeclaLetra(value):
                codigoBarras += getTeclaLetra(value)
        del teclasPulsadas[value]

    event = in_file.read(EVENT_SIZE)
in_file.close()