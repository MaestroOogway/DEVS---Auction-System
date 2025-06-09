# Auction System DEVS Simulation

Este repositorio contiene el modelo de sistema inteligente autónomo de compra bajo el formalismo DEVS para simular un escenario de subasta con múltiples agentes.

# Requisitos

* Windows, Linux o macOS
* [Cadmium Simulation Environment](https://cadmiumframework.org)
* Python 3.x
* Dependencias de Python: pandas, matplotlib

# Estructura de carpetas

```
C:/Cadmium-Simulation-Environment/DEVS-Models/Auction System/
│
├─ run_auction            # Ejecutable principal de la simulación
├─ casos_de_estudio/      # Carpetas de salida numeradas (1, 2, 3)
└─ graphics/              # Scripts Python para generar gráficos
   └─ CS1.py              # Generador de gráficas a partir de registros
```

# Ejecución de la simulación

1. Abre una terminal y posiciónate en la carpeta del proyecto:

   ```bash
   cd "C:/Cadmium-Simulation-Environment/DEVS-Models/Auction System"
   ```

2. Ejecuta el simulador con los siguientes parámetros:

   ```bash
   ./run_auction \
     -t <número de agentes> \
     -a <cantidad de productos> \
     -pi <presupuesto mínimo> \
     -ps <presupuesto máximo> \
     -c <carpeta de salida>
   ```

   * **-t**: número de agentes participantes en la subasta
   * **-a**: número de productos a subastar
   * **-pi**: presupuesto mínimo (cota inferior) de cada agente
   * **-ps**: presupuesto máximo (cota superior) de cada agente
   * **-c**: carpeta de salida donde se almacenarán los registros (valores permitidos: 1, 2 o 3)

3. Ejemplo práctico:

   ```bash
   ./run_auction -t 5 -a 10 -pi 100 -ps 500 -c 2
   ```

   Esto simula una subasta con 5 agentes, 10 productos, presupuestos entre 100 y 500, y guarda los registros en `casos_de_estudio/2/`.

# Visualización de resultados

1. Cambia al directorio de gráficos:

   ```bash
   cd graphics
   ```

2. Ejecuta el script Python para generar las gráficas:

   ```bash
   python CS1.py -c <1|2|3>
   ```

   * **-c**: número de carpeta de resultados que usaste en la simulación.

3. Ejemplo:

   ```bash
   python CS1.py -c 2
   ```

   Generará gráficos de barras y boxplots basados en los registros de la carpeta `casos_de_estudio/2/`.

# Notas

* Si clonas el repositorio en otro sistema, basta con usar rutas relativas:

  ```bash
  cd "Auction System"
  ```
* Asegúrate de tener instaladas las dependencias de Python: `pip install pandas matplotlib`
* Para cualquier duda o reporte de bugs, abre un issue en el repositorio.
