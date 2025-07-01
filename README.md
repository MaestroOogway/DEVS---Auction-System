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
├─ run_auction            # Ejecutable principal de la simulación
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
./run_auction \
  -a <número de agentes afectivos> \
  -r <número de agentes racionales> \
  -n <cantidad de productos> \
  -pi <presupuesto mínimo> \
  -ps <presupuesto máximo> \
  -c <número de caso de estudio>
```

* **-a**: número de agentes afectivos en la subasta
* **-r**: número de agentes racionales en la subasta
* **-n**: número de productos a subastar
* **-pi**: presupuesto mínimo (cota inferior) de cada agente
* **-ps**: presupuesto máximo (cota superior) de cada agente
* **-c**: número del caso de estudio (se creará la carpeta `casos_de_estudio/caso_de_estudio_{n}/`)

### 3. Ejemplo práctico

```bash
./run_auction -a 5 -r 10 -n 10 -pi 100 -ps 500 -c 2
```

Esto simula una subasta con 5 agentes afectivos, 10 agentes racionales y 10 productos, con presupuestos entre 100 y 500, y guarda los registros en `casos_de_estudio/caso_de_estudio_2/`.

## Visualización de resultados

Tras ejecutar la simulación, encontrarás dos carpetas principales dentro de cada caso de estudio:

* `messages/`: contiene los registros de todos los mensajes intercambiados entre los agentes.
* `states/`: contiene los registros de los estados internos de cada agente.

Puedes usar un script de Python para procesar y graficar estos datos. Por ejemplo, si creas un archivo `visualize_results.py`, podrías llamarlo así:

```bash
python visualize_results.py \
  "C:/Cadmium-Simulation-Environment/DEVS-Models/Auction System/casos_de_estudio/caso_de_estudio_1/messages" \
  "C:/Cadmium-Simulation-Environment/DEVS-Models/Auction System/casos_de_estudio/caso_de_estudio_1/states"
```

Dentro de `visualize_results.py` podrías leer los CSV o logs generados y usar `pandas` y `matplotlib` para crear gráficos de:

* Número de pujas realizadas por tipo de agente.
* Evolución del estado emocional de los agentes a lo largo de la subasta.

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
