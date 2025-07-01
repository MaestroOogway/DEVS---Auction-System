import os, re
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from glob import glob

# ===========  A) CONFIGURACIÓN DE RUTAS  ===========

ruta_estados = "C:/Cadmium-Simulation-Environment/DEVS-Models/Auction System/casos_de_estudio/caso_de_estudio_1/states"
archivos = sorted(glob(os.path.join(ruta_estados, "ABP_output_state_*.csv")))

output_dir = "C:/Cadmium-Simulation-Environment/DEVS-Models/Auction System/graphics/CS1"
os.makedirs(output_dir, exist_ok=True)

# ===========  B) PATRÓN GENERICo para Extraer Purchased (ajusta 25 a tu nº de productos)  ===========

N_PRODUCTOS = 10  # cambia a tu número real de productos
pat_purchased = re.compile(
    r"Purchased:\s*\[\s*([01]" +
    r"(?:\s*,\s*[01]){" + str(N_PRODUCTOS-1) + r"})\s*\]"
)

# ===========  C) INICIALIZACIÓN DE ACUMULADORES  ===========

# Un acumulador para cada categoría, de longitud N_PRODUCTOS
compras_afectivo = [0]*N_PRODUCTOS
compras_racional = [0]*N_PRODUCTOS

# ===========  D) PROCESAMIENTO DE CADA ARCHIVO  ===========

for ruta in archivos:
    # Diccionarios para guardar sólo el ÚLTIMO vector de cada agente
    latest_afectives = {}   # clave: IdAgent (int), valor: lista de 0/1
    latest_racionals  = {}

    with open(ruta, "r", encoding="utf-8") as f:
        for linea in f:
            linea = linea.strip()
            if not linea:
                continue

            # Capturamos el modelo y el IdAgent
            # Ejemplo de línea: "State for model affective_3 is  IdAgent: 3 | ... Purchased: [...]"
            m_mod = re.match(r"State for model (affective|rational)_(\d+)", linea)
            if not m_mod:
                continue
            tipo, agent_id = m_mod.group(1), int(m_mod.group(2))

            # Ahora buscamos el patrón Purchased
            m_pur = pat_purchased.search(linea)
            if not m_pur:
                continue

            vector = [int(x) for x in m_pur.group(1).split(",")]

            # Sobrescribimos el último vector para este agente
            if tipo == "affective":
                latest_afectives[agent_id] = vector
            else:  # rational
                latest_racionals[agent_id] = vector

    # Comprobamos que al menos un agente de cada tipo tenga un vector final
    if not latest_afectives or not latest_racionals:
        print(f"¡Atención! faltan agentes en {os.path.basename(ruta)}")
        continue

    # Sumamos elemento a elemento todos los vectores finales de los agentes de cada tipo
    combined_aff = [ sum(col) for col in zip(*latest_afectives.values()) ]
    combined_rat = [ sum(col) for col in zip(*latest_racionals.values()) ]

    # Acumulamos al total global
    for i in range(N_PRODUCTOS):
        compras_afectivo[i] += combined_aff[i]
        compras_racional[i]  += combined_rat[i]

# ===========  E) GUARDAR CSV RESUMEN  ===========

productos = [i+1 for i in range(N_PRODUCTOS)]
df = pd.DataFrame({
    "Producto": productos,
    "Compras_Afectivo": compras_afectivo,
    "Compras_Racional": compras_racional
})
csv_path = os.path.join(output_dir, "dataset_compras.csv")
df.to_csv(csv_path, index=False)
print(f"CSV guardado en: {csv_path}")

# ===========  F) GRÁFICO DE BARRAS AGRUPADAS  ===========

indices = np.arange(N_PRODUCTOS)
ancho = 0.35

plt.figure(figsize=(12,6))
plt.bar(indices - ancho/2, compras_afectivo, width=ancho, label="Afectivo", edgecolor="black")
plt.bar(indices + ancho/2, compras_racional,  width=ancho, label="Racional",  edgecolor="black")

plt.xticks(indices, productos, rotation=45)
plt.xlabel("Producto")
plt.ylabel("Veces comprado (suma de todos los agentes)")
plt.title("Compras por Producto: Agentes Afectivos vs Racionales (n agentes)")
plt.legend()
plt.tight_layout()

img_path = os.path.join(output_dir, "barras_cantidad_compras.png")
plt.savefig(img_path)
plt.close()
print(f"Gráfico guardado en: {img_path}")
