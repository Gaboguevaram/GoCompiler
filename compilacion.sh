#!/bin/bash

# Nombre del ejecutable
EXECUTABLE="compiladorGO"

# Compilar todos los archivos .c en el directorio
gcc -o $EXECUTABLE *.c -g -Wall

# Verificar si la compilación fue exitosa
if [ $? -eq 0 ]; then
    echo "Compilación exitosa."
else
    echo "Error en la compilación."
fi

