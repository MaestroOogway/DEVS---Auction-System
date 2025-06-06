#!/bin/bash
# Limpiar y compilar el proyecto
echo "Limpiando archivos previos..."
make clean

ejecuciones=100
echo "Compilando el proyecto..."
if ! make all; then
    echo "Error en la compilación. Abortando."
    exit 1
fi

# Ejecutar el programa 100 veces
echo "Ejecutando el programa $ejecuciones veces..."
cd bin || { echo "Error: No se encontró el directorio 'bin'"; exit 1; }

for ((i=1; i<=ejecuciones; i++)); do
    echo "Ejecución $i..."
    if ! ./ABP.exe ../input_data/initial_product_information_test.txt $i; then
        echo "Error en la ejecución de ABP.exe en la iteración $i"
        exit 1
    fi
done

echo "Script finalizado con éxito."
