#!/bin/bash

# Limpiar y compilar el proyecto
echo "Limpiando archivos previos..."
make clean

echo "Compilando el proyecto..."
if ! make all; then
    echo "Error en la compilación. Abortando."
    exit 1
fi

# Ejecutar el programa
echo "Ejecutando el programa..."
cd bin || { echo "Error: No se encontró el directorio 'bin'"; exit 1; }

if ! ./ABP.exe ../input_data/initial_product_information_test.txt; then
    echo "Error en la ejecución de ABP.exe"
    exit 1
fi
echo "Script finalizado con éxito."