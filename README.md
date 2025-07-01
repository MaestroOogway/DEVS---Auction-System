# Auction System DEVS Simulation

Este repositorio contiene el modelo de sistema inteligente autónomo de compra bajo el formalismo DEVS para simular un escenario de subasta con múltiples agentes afectivos y racionales.

## Requisitos

* Windows, Linux o macOS
* Cadmium Simulation Environment
* Python 3.x
* Dependencias de Python: `pandas`, `matplotlib`

## Estructura de carpetas

```text
C:/Cadmium-Simulation-Environment/DEVS-Models/Auction System/
│
├─ run_auction.sh              # Ejecutable principal de la simulación
├─ casos_de_estudio/
│   ├─ caso_de_estudio_{n}/    # Carpetas de salida numeradas donde n = {1, 2, 3}
│       ├─ messages/           # Registros de mensajes intercambiados
│       └─ states/             # Registros de estados de los agentes
```

## Ejecución de la simulación

### 1. Posicionarse en la carpeta del proyecto

```bash
cd "C:/Cadmium-Simulation-Environment/DEVS-Models/Auction System"
```

### 2. Ejecutar el simulador

```bash
./run_auction -a <número de agentes afectivos> -r <número de agentes racionales> -l <presupuesto mínimo> -h <presupuesto máximo> -p <cantidad de productos> 
```

* **-a**: número de agentes afectivos en la subasta
* **-r**: número de agentes racionales en la subasta
* **-l**: presupuesto mínimo (cota inferior) de cada agente
* **-h**: presupuesto máximo (cota superior) de cada agente
* **-p**: número de productos a subastar

### 3. Ejemplo práctico

```bash
./run_auction -a 5 -r 10 -n 10 -l 100 -h 500 -p 10
```

Esto simula una subasta con 5 agentes afectivos, 10 agentes racionales y 10 productos, con presupuestos entre 100 y 500, y guarda los registros en `casos_de_estudio/caso_de_estudio_2/`.

## Visualización de resultados de simulación

Tras ejecutar la simulación, encontrarás dos carpetas principales dentro de cada caso de estudio:

* `messages/`: contiene los registros de todos los mensajes intercambiados entre los agentes.
* `states/`: contiene los registros de los estados internos de cada agente.

## Visualización de gráficos

Puedes visualizar los resultados mediante dos scripts de Python:

* `CS.py`: genera gráficos de líneas y boxplots.
* `CS_BARRAS.py`: genera gráficos de barras.

Para ello:

1. Dirígete al directorio `graphics`, ubicado en la rama principal del repositorio.
2. Ejecuta los scripts con Python:

```bash
cd graphics
python CS.py
python CS_BARRAS.py

## Notas

* Si clonas el repositorio en otro sistema, basta con usar rutas relativas:

  ```bash
  cd "Auction System"
  ```
* Asegúrate de tener instaladas las dependencias de Python:

  ```bash
  pip install pandas matplotlib
  ```
* Para cualquier duda o reporte de bugs, abre un issue en el repositorio.


  ```bash
  cd "Auction System"
  ```
* Asegúrate de tener instaladas las dependencias de Python: `pip install pandas matplotlib`
* Para cualquier duda o reporte de bugs, abre un issue en el repositorio.
