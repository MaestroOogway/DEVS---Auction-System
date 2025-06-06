import os
import re
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
from glob import glob

# ------------------ CONFIGURACIÓN ------------------
output_dir = "C:/Cadmium-Simulation-Environment/DEVS-Models/Auction System/graphics/CS3"
os.makedirs(output_dir, exist_ok=True)

# ------------------ CARGAR DATOS ------------------
ruta_archivos = "C:/Cadmium-Simulation-Environment/DEVS-Models/Auction System/casos_de_estudio/caso_de_estudio_3/states"
archivos = sorted(glob(os.path.join(ruta_archivos, "ABP_output_state_*.csv")))

# Patrones regex
pat_reserva = re.compile(r"ReservePrice for Current Product: \[ ID Product N°\d+ : ([\d.]+) \]")
pat_utilidad = re.compile(r"Utility: ([\d.]+)")
pat_SP = re.compile(r"SP: (\d+)")

datos = []

for archivo in archivos:
    with open(archivo, "r", encoding="utf-8") as f:
        lines = [line.strip() for line in f if line.strip()]

    timestamp = ""
    SP = None

    for i, line in enumerate(lines):
        if re.match(r"^\d{2}:\d{2}:\d{2}:\d{3}$", line):
            timestamp = line

        elif "State for model auctioneer_model" in line:
            sp_match = pat_SP.search(line)
            if sp_match:
                SP = int(sp_match.group(1))

        elif "State for model affective_" in line:
            agent_id = int(re.search(r"affective_(\d+)", line).group(1))
            util_match = pat_utilidad.search(line)
            res_match = pat_reserva.search(line)
            if util_match and res_match and SP is not None:
                datos.append({
                    "SP": SP,
                    "Tipo": "Afectivo",
                    "ID_Agente": agent_id,
                    "Reserva": float(res_match.group(1)),
                    "Utilidad": float(util_match.group(1)),
                })

# Crear DataFrame
df = pd.DataFrame(datos)

# Guardar el dataset
df.to_csv(os.path.join(output_dir, "dataset_final.csv"), index=False)

# ------------------ GRAFICOS ------------------
sns.set(style="whitegrid")

# Gráfico 1 - Precio de Reserva
plt.figure(figsize=(12, 6))
sns.boxplot(data=df, x="SP", y="Reserva", color="lightcoral")
plt.title("Precio de Reserva por Producto (SP) - Agentes Afectivos")
plt.xlabel("Subasta del Producto")
plt.ylabel("Precio de Reserva")
plt.grid(True)
plt.tight_layout()
plt.savefig(os.path.join(output_dir, "precio_reserva_boxplot_afectivos.png"))
plt.close()

# Gráfico 2 - Utilidad
plt.figure(figsize=(12, 6))
sns.boxplot(data=df, x="SP", y="Utilidad", color="skyblue")
plt.title("Utilidad por Producto (SP) - Agentes Afectivos")
plt.xlabel("Subasta del Producto")
plt.ylabel("Utilidad")
plt.grid(True)
plt.tight_layout()
plt.savefig(os.path.join(output_dir, "utilidad_boxplot_afectivos.png"))
plt.close()

# Gráfico 3 - Media ± std Precio de Reserva
df_reserva = df.groupby("SP")["Reserva"].agg(["mean", "std"]).reset_index()
plt.figure(figsize=(12, 6))
plt.plot(df_reserva["SP"], df_reserva["mean"], label="Media Precio Reserva", color="coral")
plt.fill_between(df_reserva["SP"], df_reserva["mean"] - df_reserva["std"],
                 df_reserva["mean"] + df_reserva["std"], alpha=0.3, color="coral")
plt.title("Media ± Desviación Estándar del Precio de Reserva (Agentes Afectivos)")
plt.xlabel("Subasta del Producto")
plt.ylabel("Precio de Reserva")
plt.grid(True)
plt.tight_layout()
plt.savefig(os.path.join(output_dir, "precio_reserva_barra_afectivos.png"))
plt.close()

# Gráfico 4 - Media ± std Utilidad
df_utilidad = df.groupby("SP")["Utilidad"].agg(["mean", "std"]).reset_index()
plt.figure(figsize=(12, 6))
plt.plot(df_utilidad["SP"], df_utilidad["mean"], label="Media Utilidad", color="blue")
plt.fill_between(df_utilidad["SP"], df_utilidad["mean"] - df_utilidad["std"],
                 df_utilidad["mean"] + df_utilidad["std"], alpha=0.3, color="blue")
plt.title("Media ± Desviación Estándar de la Utilidad (Agentes Afectivos)")
plt.xlabel("Subasta del Producto")
plt.ylabel("Utilidad")
plt.grid(True)
plt.tight_layout()
plt.savefig(os.path.join(output_dir, "utilidad_barra_afectivos.png"))
plt.close()
