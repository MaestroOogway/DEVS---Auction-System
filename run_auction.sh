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

cd ..

# Verificar y mostrar los resultados de la simulación
cd simulation_results || { echo "Error: No se encontró el directorio 'simulation_results'"; exit 1; }

echo "Mostrando resultados de la simulación:"
if [[ -f ABP_output_messages.txt ]]; then
    echo "Contenido de ABP_output_messages.txt:"
    cat ABP_output_messages.txt
else
    echo "Advertencia: No se encontró ABP_output_messages.txt"
fi

if [[ -f ABP_output_state.txt ]]; then
    echo "Contenido de ABP_output_state.txt:"
    cat ABP_output_state.txt
else
    echo "Advertencia: No se encontró ABP_output_state.txt"
fi

echo "Script finalizado con éxito."