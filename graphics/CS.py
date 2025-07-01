import os, re
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
from glob import glob
from pathlib import Path

# ------------------ CONFIGURACIÓN ------------------
output_dir = Path("C:/Cadmium-Simulation-Environment/DEVS-Models/Auction System/graphics/CS1")
output_dir.mkdir(parents=True, exist_ok=True)

# ------------------ CARGAR DATOS ------------------
ruta_archivos = Path("C:/Cadmium-Simulation-Environment/DEVS-Models/Auction System/casos_de_estudio/caso_de_estudio_1/states")
archivos = sorted(ruta_archivos.glob("ABP_output_state_*.csv"))

# Patrones regex
pat_reserva = re.compile(r"ReservePrice for Current Product: \[ ID Product N°\d+ : ([\d.]+) \]")
pat_utilidad = re.compile(r"Utility: ([\d.]+)")
pat_SP = re.compile(r"SP: (\d+)")
pat_N = re.compile(r"\bN: (\d+)\b")

datos = []

for archivo in archivos:
    with open(archivo, "r", encoding="utf-8") as f:
        lines = [line.strip() for line in f if line.strip()]

    SP = None
    for line in lines:
        # Extraer SP del auctioneer
        if "State for model auctioneer_model" in line:
            m_sp = pat_SP.search(line)
            if m_sp:
                SP = int(m_sp.group(1))

        # Afectivos
        if "State for model affective_" in line:
            id_agent = int(re.search(r"affective_(\d+)", line).group(1))
            m_u = pat_utilidad.search(line)
            m_r = pat_reserva.search(line)
            m_n = pat_N.search(line)
            if m_u and m_r and SP is not None and m_n:
                datos.append({
                    "SP": SP,
                    "N": int(m_n.group(1)),
                    "Tipo": "Afectivo",
                    "ID_Agente": id_agent,
                    "Reserva": float(m_r.group(1)),
                    "Utilidad": float(m_u.group(1)),
                })

        # Racionales
        if "State for model rational_" in line:
            id_agent = int(re.search(r"rational_(\d+)", line).group(1))
            m_u = pat_utilidad.search(line)
            m_r = pat_reserva.search(line)
            m_n = pat_N.search(line)
            if m_u and m_r and SP is not None and m_n:
                datos.append({
                    "SP": SP,
                    "N": int(m_n.group(1)),
                    "Tipo": "Racional",
                    "ID_Agente": id_agent,
                    "Reserva": float(m_r.group(1)),
                    "Utilidad": float(m_u.group(1)),
                })

# Crear DataFrame
df = pd.DataFrame(datos)
df.to_csv(output_dir / "dataset_final.csv", index=False)

# ------------------ GRAFICOS ------------------
sns.set(style="whitegrid")

# 1) Precio de Reserva vs SP (Boxplot)
plt.figure(figsize=(12,6))
sns.boxplot(data=df, x="SP", y="Reserva", hue="Tipo")
plt.title("Precio de Reserva por SP y Tipo de Agente")
plt.xlabel("Producto")
plt.ylabel("Precio de Reserva")
plt.xticks(sorted(df["SP"].unique()), rotation=90)
plt.legend(title="Tipo")
plt.tight_layout()
plt.savefig(output_dir / "boxplot_precio_reserva.png")
plt.clf()

# 2) Utilidad vs N (Boxplot)
plt.figure(figsize=(12,6))
sns.boxplot(data=df, x="N", y="Utilidad", hue="Tipo")
plt.title("Utilidad por N y Tipo de Agente")
plt.xlabel("Producto")
plt.ylabel("Utilidad")
plt.xticks(sorted(df["N"].unique()), rotation=90)
plt.legend(title="Tipo")
plt.tight_layout()
plt.savefig(output_dir / "boxplot_utilidad.png")
plt.clf()

# 3) Precio de Reserva vs SP (Media ± Desviación)
df_res = df.groupby(["SP","Tipo"])['Reserva'].agg(['mean','std']).reset_index()
plt.figure(figsize=(12,6))
for t in df_res['Tipo'].unique():
    sub = df_res[df_res['Tipo']==t]
    plt.plot(sub['SP'], sub['mean'], label=f"{t} Media")
    plt.fill_between(sub['SP'], sub['mean']-sub['std'], sub['mean']+sub['std'], alpha=0.3)
plt.title("Media ± Desviación del Precio de Reserva por Producto")
plt.xlabel("Producto")
plt.ylabel("Precio de Reserva")
plt.xticks(sorted(df['SP'].unique()), rotation=90)
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig(output_dir / "lineas_precio_reserva.png")
plt.clf()

# 4) Utilidad vs N (Media ± Desviación)
df_util = df.groupby(["N","Tipo"])['Utilidad'].agg(['mean','std']).reset_index()
plt.figure(figsize=(12,6))
for t in df_util['Tipo'].unique():
    sub = df_util[df_util['Tipo']==t]
    plt.plot(sub['N'], sub['mean'], label=f"{t} Media")
    plt.fill_between(sub['N'], sub['mean']-sub['std'], sub['mean']+sub['std'], alpha=0.3)
plt.title("Media ± Desviación de la Utilidad por N")
plt.xlabel("N")
plt.ylabel("Utilidad")
plt.xticks(sorted(df['N'].unique()), rotation=90)
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig(output_dir / "lineas_utilidad.png")
plt.clf()
