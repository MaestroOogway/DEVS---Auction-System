import os, re
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
from glob import glob
from pathlib import Path
import matplotlib.patches as mpatches

# ------------------ CONFIGURACIÓN ------------------
output_dir = Path(
    "C:/Cadmium-Simulation-Environment/DEVS-Models/"
    "Auction System/graphics/CS2"
)
output_dir.mkdir(parents=True, exist_ok=True)

# ------------------ CARGAR DATOS ------------------
ruta_archivos = Path(
    "C:/Cadmium-Simulation-Environment/DEVS-Models/"
    "Auction System/casos_de_estudio/"
    "caso_de_estudio_2/states"
)
archivos = sorted(ruta_archivos.glob("ABP_output_state_*.csv"))

# Patrones regex
pat_reserva  = re.compile(
    r"ReservePrice for Current Product: \[ ID Product N°\d+ : ([\d.]+) \]"
)
pat_utilidad = re.compile(r"Utility: ([\d.]+)")
pat_SP       = re.compile(r"SP: (\d+)")
pat_N        = re.compile(r"\bN: (\d+)\b")
pat_anxiety  = re.compile(r"Anxiety: ([\d.]+)")
pat_frustration  = re.compile(r"Frustration: ([\d.]+)")

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
            m_a = pat_anxiety.search(line)
            m_f = pat_frustration.search(line)
            if m_u and m_r and SP is not None and m_n and m_a and m_f:
                datos.append({
                    "SP": SP,
                    "N": int(m_n.group(1)),
                    "Tipo": "Afectivo",
                    "ID_Agente": id_agent,
                    "Reserva": float(m_r.group(1)),
                    "Utilidad": float(m_u.group(1)),
                    "Ansiedad": float(m_a.group(1)),  # Solo afectivos
                    "Frustracion": float(m_f.group(1)),  # Solo afectivos
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
                    "Ansiedad": np.nan,            # Asignamos NaN
                })

# Crear DataFrame
df = pd.DataFrame(datos)
df.to_csv(output_dir / "dataset_final.csv", index=False)

sns.set(style="whitegrid")

# 1) Precio de Reserva vs Producto (Boxplot)
order_sp = sorted(df["SP"].unique())
plt.figure(figsize=(12,6))
sns.boxplot(data=df, x="SP", y="Reserva", hue="Tipo", order=order_sp)
plt.title("Boxplot del Precio de Reserva por Producto Subastado")
plt.xlabel("Producto")
plt.ylabel("Precio de Reserva")
plt.legend(title="Agente")
plt.grid(True, axis='both', which='major', linestyle='--', alpha=0.7)
plt.tight_layout()
plt.savefig(output_dir / "boxplot_precio_reserva.png")
plt.clf()

# 2) Utilidad vs Producto (Boxplot)
plt.figure(figsize=(12,6))
sns.boxplot(data=df, x="N", y="Utilidad", hue="Tipo")
plt.title("Boxplot de la Utilidad por Producto Subastado")
plt.xlabel("Producto")
plt.ylabel("Utilidad")
plt.xticks(sorted(df["N"].unique()))
plt.legend(title="Agente")
plt.grid(True, axis='both', which='major', linestyle='--', alpha=0.7)
plt.tight_layout()
plt.savefig(output_dir / "boxplot_utilidad.png")
plt.clf()

# 5) Ansiedad vs Producto (Boxplot) — sólo Afectivo
df_afectivo = df[df["Tipo"]=="Afectivo"]
order_sp_afectivo = sorted(df_afectivo["SP"].unique())

plt.figure(figsize=(12,6))
sns.boxplot(data=df_afectivo, x="SP", y="Ansiedad", order=order_sp_afectivo)
plt.title("Boxplot de la Ansiedad por Producto Subastado")
plt.xlabel("Producto")
plt.ylabel("Ansiedad")
patch = mpatches.Patch(color=sns.color_palette()[0], label='Afectivo')
plt.legend(handles=[patch], title="Agente", loc='upper left')
plt.grid(True, axis='both', which='major', linestyle='--', alpha=0.7)
plt.tight_layout()
plt.savefig(output_dir / "boxplot_ansiedad.png")
plt.clf()

# 7) Frustración vs Producto (Boxplot) — sólo Afectivo y sin outliers en SP=1
df_afectivo = df[df["Tipo"] == "Afectivo"].copy()
order_sp_afectivo = sorted(df_afectivo["SP"].unique())

# Quitar outliers solo en producto 1
prod_1 = df_afectivo[df_afectivo["SP"] == 1]
Q1 = prod_1["Frustracion"].quantile(0.25)
Q3 = prod_1["Frustracion"].quantile(0.75)
IQR = Q3 - Q1
lower_bound = Q1 - 1.5 * IQR
upper_bound = Q3 + 1.5 * IQR
prod_1_sin_outliers = prod_1[
    (prod_1["Frustracion"] >= lower_bound) & (prod_1["Frustracion"] <= upper_bound)
]
otros_productos = df_afectivo[df_afectivo["SP"] != 1]
df_afectivo_filtrado = pd.concat([prod_1_sin_outliers, otros_productos])

plt.figure(figsize=(12,6))
sns.boxplot(data=df_afectivo_filtrado, x="SP", y="Frustracion", order=order_sp_afectivo)
plt.title("Boxplot de la Frustración por Producto Subastado")
plt.xlabel("Producto")
plt.ylabel("Frustración")
patch = mpatches.Patch(color=sns.color_palette()[0], label='Afectivo')
plt.legend(handles=[patch], title="Agente", loc='upper left')
plt.grid(True, axis='both', which='major', linestyle='--', alpha=0.7)
plt.tight_layout()
plt.savefig(output_dir / "boxplot_frustracion.png")
plt.clf()

# 3) Precio de Reserva vs Producto (Media ± Desviación)
df_res = df.groupby(["SP","Tipo"])["Reserva"].agg(["mean","std"]).reset_index()
plt.figure(figsize=(12,6))
for t in df_res["Tipo"].unique():
    sub = df_res[df_res["Tipo"]==t]
    plt.plot(sub["SP"], sub["mean"], marker='o', label=f"{t} Media")
    plt.fill_between(
        sub["SP"],
        sub["mean"] - sub["std"],
        sub["mean"] + sub["std"],
        alpha=0.3
    )
plt.title("Media y Desviación Estándar del Precio de Reserva por Producto Subastado")
plt.xlabel("Producto")
plt.ylabel("Precio de Reserva")
plt.xticks(sorted(df["SP"].unique()))
plt.legend(title="Agente")  # Cambiado
plt.grid(True)
plt.tight_layout()
plt.savefig(output_dir / "lineas_precio_reserva.png")
plt.clf()

# 4) Utilidad vs Producto (Media ± Desviación)
df_util = df.groupby(["N","Tipo"])["Utilidad"].agg(["mean","std"]).reset_index()
plt.figure(figsize=(12,6))
for t in df_util["Tipo"].unique():
    sub = df_util[df_util["Tipo"]==t]
    plt.plot(sub["N"], sub["mean"], marker='o', label=f"{t} Media")
    plt.fill_between(
        sub["N"],
        sub["mean"] - sub["std"],
        sub["mean"] + sub["std"],
        alpha=0.3
    )
plt.title("Media y Desviación Estándar de la Utilidad por Producto Subastado")
plt.xlabel("Producto")
plt.ylabel("Utilidad")
plt.xticks(sorted(df["N"].unique()))
plt.legend(title="Agente")  # Cambiado
plt.grid(True)
plt.tight_layout()
plt.savefig(output_dir / "lineas_utilidad.png")
plt.clf()

# 6) Ansiedad por Producto (Media ± Desviación) — sólo afectivos
df_anx = (
    df[df["Tipo"]=="Afectivo"]
    .groupby("SP")["Ansiedad"]
    .agg(["mean","std"])
    .reset_index()
)
plt.figure(figsize=(12,6))
plt.plot(df_anx["SP"], df_anx["mean"], marker='o', label="Afectivo Media")
plt.fill_between(
    df_anx["SP"],
    df_anx["mean"] - df_anx["std"],
    df_anx["mean"] + df_anx["std"],
    alpha=0.3
)
plt.title("Media y Desviación Estándar de la Ansiedad por Producto Subastado")
plt.xlabel("Producto")
plt.ylabel("Ansiedad")
plt.xticks(sorted(df_anx["SP"]))
plt.grid(True)

patch = mpatches.Patch(color=sns.color_palette()[0], label='Afectivo Media')
plt.legend(handles=[patch], title="Agente")

plt.tight_layout()
plt.savefig(output_dir / "lineas_ansiedad.png")
plt.clf()

# 8) Frustración vs Producto (Media ± Desviación) — sólo afectivos
df_frus = (
    df[df["Tipo"] == "Afectivo"]
    .groupby("SP")["Frustracion"]
    .agg(["mean", "std"])
    .reset_index()
)

plt.figure(figsize=(12, 6))
plt.plot(df_frus["SP"], df_frus["mean"], marker='o', label="Afectivo Media")
plt.fill_between(
    df_frus["SP"],
    df_frus["mean"] - df_frus["std"],
    df_frus["mean"] + df_frus["std"],
    alpha=0.3
)
plt.title("Media y Desviación Estándar de la Frustración por Producto Subastado")
plt.xlabel("Producto")
plt.ylabel("Frustración")
plt.xticks(sorted(df_frus["SP"]))
plt.grid(True)
plt.legend(title="Agente")  # Cambiado
plt.tight_layout()
plt.savefig(output_dir / "lineas_frustracion.png")
plt.clf()

# --- TABLA DE RESUMEN POR TIPO DE AGENTE ---
cols_to_summarize = ['Reserva', 'Utilidad', 'Ansiedad', 'Frustracion']

df_summary_tipo = (
    df
    .groupby('Tipo')[cols_to_summarize]
    .agg(['min', 'max', 'mean', 'std'])
)

# Aplanar multi-índice de columnas
df_summary_tipo.columns = ['_'.join(col) for col in df_summary_tipo.columns]
df_summary_tipo = df_summary_tipo.reset_index()

# Guardar o mostrar
df_summary_tipo.to_csv(output_dir / "tabla_resumen.csv", index=False)
print(df_summary_tipo)
