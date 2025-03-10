#!/bin/bash

# Ir al directorio de la subasta
cd /cygdrive/c/Cadmium-Simulation-Environment/DEVS-Models/Auction\ System

# Limpiar y compilar el proyecto
make clean
make all

# Ejecutar el binario
cd bin
./AUCTIONEER_TEST.exe

# Mostrar los resultados
cd ../simulation_results
cat auctioneer_test_output_messages.txt
cat auctioneer_test_output_state.txt
