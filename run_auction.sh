#!/usr/bin/env bash
# Valores por defecto
EJECUCIONES=100
PRODUCTOS="../input_data/initial_product_information_test.txt"

usage() {
  echo "Uso: $0 -a num_afectivos -r num_racionales -l presupuesto_min -u presupuesto_max -p num_productos"
  exit 1
}

# Parseo de opciones
while getopts ":a:r:l:u:p:" opt; do
  case $opt in
    a) N_AFECTIVOS="$OPTARG" ;;
    r) N_RACIONALES="$OPTARG" ;;
    l) P_MIN="$OPTARG" ;;
    u) P_MAX="$OPTARG" ;;
    p) N_PRODUCTOS="$OPTARG" ;;
    *) usage ;;
  esac
done

# Verifico que todos los flags obligatorios estén presentes
if [ -z "$N_AFECTIVOS" ] || [ -z "$N_RACIONALES" ] || [ -z "$P_MIN" ] || [ -z "$P_MAX" ] || [ -z "$N_PRODUCTOS" ]; then
  echo "Faltan parámetros obligatorios."
  usage
fi

if [ "$N_AFECTIVOS" -gt 1 ] || [ "$N_RACIONALES" -gt 1 ]; then
    numero_salida=2
else
    numero_salida=1
fi

echo "Parámetros:"
echo "  Afectivos:          $N_AFECTIVOS"
echo "  Racionales:         $N_RACIONALES"
echo "  Presupuesto mínimo: $P_MIN"
echo "  Presupuesto máximo: $P_MAX"
echo "  Cantidad de productos : $N_PRODUCTOS"
echo "Se realizarán $EJECUCIONES iteraciones..."
echo "Directorio de salida: ../casos_de_estudio/caso_de_estudio_$numero_salida"

# Limpiar y compilar el proyecto
echo "Limpiando archivos previos..."
make clean

echo "Compilando el proyecto..."
if ! make all; then
  echo "Error en la compilación. Abortando."
  exit 1
fi

# Cambio de directorio y ejecución en loop
echo "Ejecutando $EJECUCIONES veces..."
cd bin || { echo "Error: No se encontró el directorio 'bin'"; exit 1; }

for ((i=1; i<=EJECUCIONES; i++)); do
    echo "Ejecución $i"
  ./ABP.exe \
    -a "$N_AFECTIVOS" \
    -r "$N_RACIONALES" \
    -l "$P_MIN" \
    -u "$P_MAX" \
    -p "$N_PRODUCTOS" \
    "$PRODUCTOS" \
    "$i"
done

echo "Script finalizado con éxito."
exit 0