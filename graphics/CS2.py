import os
import re
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
from glob import glob

# ------------------ CONFIGURACIÓN ------------------

# Ruta de salida
output_dir = "C:/Cadmium-Simulation-Environment/DEVS-Models/Auction System/graphics/CS2"
os.makedirs(output_dir, exist_ok=True)

# ------------------ CARGAR DATOS ------------------

# Ruta de archivos de entrada
ruta_archivos = "C:/Cadmium-Simulation-Environment/DEVS-Models/Auction System/casos_de_estudio/caso_de_estudio_2/states"
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
                    "Tipo": "Afectivo",
                    "ID_Agente": agent_id,
                    "SP": SP,
                    "Utilidad": float(util_match.group(1)),
                    "Reserva": float(res_match.group(1))
                })

        elif "State for model rational_" in line:
            agent_id = int(re.search(r"rational_(\d+)", line).group(1))
            util_match = pat_utilidad.search(line)
            res_match = pat_reserva.search(line)
            if util_match and res_match and SP is not None:
                datos.append({
                    "Tipo": "Racional",
                    "ID_Agente": agent_id,
                    "SP": SP,
                    "Utilidad": float(util_match.group(1)),
                    "Reserva": float(res_match.group(1))
                })

# Crear DataFrame
df = pd.DataFrame(datos)

# Guardar el dataset
df.to_csv(os.path.join(output_dir, "dataset_final.csv"), index=False)

# ------------------ GRAFICOS ------------------

sns.set(style="whitegrid")

# Gráfico 1 - Precio de Reserva
plt.figure(figsize=(12, 6))
sns.boxplot(data=df, x="SP", y="Reserva", hue="Tipo")
plt.title("Precio de Reserva por Producto (SP) y Tipo de Agente")
plt.xlabel("Subasta del Producto")
plt.ylabel("Precio de Reserva")
plt.legend(title="Tipo de Agente")
plt.grid(True)
plt.tight_layout()
plt.savefig(os.path.join(output_dir, "boxplot_precio_reserva.png"))
plt.close()

# Gráfico 2 - Utilidad
plt.figure(figsize=(12, 6))
sns.boxplot(data=df, x="SP", y="Utilidad", hue="Tipo")
plt.title("Utilidad por Producto (SP) y Tipo de Agente")
plt.xlabel("Subasta del Producto")
plt.ylabel("Utilidad")
plt.legend(title="Tipo de Agente")
plt.grid(True)
plt.tight_layout()
plt.savefig(os.path.join(output_dir, "boxplot_utilidad.png"))
plt.close()

# Gráfico 3 - Media ± std Precio de Reserva
df_reserva = df.groupby(["SP", "Tipo"])["Reserva"].agg(["mean", "std"]).reset_index()
plt.figure(figsize=(12, 6))
for tipo in ["Afectivo", "Racional"]:
    data = df_reserva[df_reserva["Tipo"] == tipo]
    plt.plot(data["SP"], data["mean"], label=f"{tipo} Media")
    plt.fill_between(data["SP"], data["mean"] - data["std"], data["mean"] + data["std"], alpha=0.3)
plt.title("Media ± Desviación Estándar del Precio de Reserva por SP")
plt.xlabel("Subasta del Producto")
plt.ylabel("Precio de Reserva")
plt.legend()
plt.grid(True)
plt.xticks(ticks=sorted(df["SP"].unique()))
plt.tight_layout()
plt.savefig(os.path.join(output_dir, "media_sombra_precio_reserva.png"))
plt.close()

# Gráfico 4 - Media ± std Utilidad
df_utilidad = df.groupby(["SP", "Tipo"])["Utilidad"].agg(["mean", "std"]).reset_index()
plt.figure(figsize=(12, 6))
for tipo in ["Afectivo", "Racional"]:
    data = df_utilidad[df_utilidad["Tipo"] == tipo]
    plt.plot(data["SP"], data["mean"], label=f"{tipo} Media")
    plt.fill_between(data["SP"], data["mean"] - data["std"], data["mean"] + data["std"], alpha=0.3)
plt.title("Media ± Desviación Estándar de la Utilidad por SP")
plt.xlabel("Subasta del Producto")
plt.ylabel("Utilidad")
plt.legend()
plt.grid(True)
plt.xticks(ticks=sorted(df["SP"].unique()))
plt.tight_layout()
plt.savefig(os.path.join(output_dir, "media_sombra_utilidad.png"))
plt.close()
