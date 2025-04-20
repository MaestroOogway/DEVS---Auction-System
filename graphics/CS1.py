import os
import re
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
from glob import glob

# Ruta donde están tus archivos de entrada
ruta_archivos = "C:/Cadmium-Simulation-Environment/DEVS-Models/Auction System/casos_de_estudio/caso_de_estudio_1/states"
archivos = sorted(glob(os.path.join(ruta_archivos, "ABP_output_state_*.csv")))

# Ruta donde guardar resultados
output_dir = os.path.join(ruta_archivos, "C:/Cadmium-Simulation-Environment/DEVS-Models/Auction System/graphics/CS1")
os.makedirs(output_dir, exist_ok=True)  # ✅ Crear carpeta si no existe

datos = []

# Patrones
pat_reserva = re.compile(r"ReservePrice for Current Product: \[ ID Product N°\d+ : ([\d.]+) \]")
pat_utilidad = re.compile(r"Utility: ([\d.]+)")
pat_SP = re.compile(r"SP: (\d+)")

for archivo in archivos:
    with open(archivo, "r", encoding="utf-8") as f:
        lines = [line.strip() for line in f if line.strip()]
    
    timestamp = ""
    reserva_afectivo = None
    reserva_racional = None
    utilidad_afectivo = None
    utilidad_racional = None
    SP = None
    
    for i, line in enumerate(lines):
        if re.match(r"^\d{2}:\d{2}:\d{2}:\d{3}$", line):
            timestamp = line
        
        elif "State for model auctioneer_model" in line:
            sp_match = pat_SP.search(line)
            if sp_match:
                SP = int(sp_match.group(1))
        
        elif "State for model affective_1" in line:
            util_match = pat_utilidad.search(line)
            res_match = pat_reserva.search(line)
            if util_match:
                utilidad_afectivo = float(util_match.group(1))
            if res_match:
                reserva_afectivo = float(res_match.group(1))
        
        elif "State for model rational_2" in line:
            util_match = pat_utilidad.search(line)
            res_match = pat_reserva.search(line)
            if util_match:
                utilidad_racional = float(util_match.group(1))
            if res_match:
                reserva_racional = float(res_match.group(1))
        
        if all(x is not None for x in [timestamp, reserva_afectivo, reserva_racional, utilidad_afectivo, utilidad_racional, SP]):
            if 1 <= SP <= 10:
                datos.append({
                    "SP": SP,
                    "reserva_afectivo": reserva_afectivo,
                    "reserva_racional": reserva_racional,
                    "utilidad_afectivo": utilidad_afectivo,
                    "utilidad_racional": utilidad_racional,
                })
            reserva_afectivo = reserva_racional = utilidad_afectivo = utilidad_racional = SP = None

# Crear DataFrame
df = pd.DataFrame(datos)

# ✅ Guardar dataset como CSV
df.to_csv(os.path.join(output_dir, "dataset_final.csv"), index=False)

# ---------------------- GRAFICOS ----------------------
sns.set(style="whitegrid")

# ---- Precio de Reserva por SP ----
df_reserva = pd.melt(df, id_vars=["SP"], value_vars=["reserva_afectivo", "reserva_racional"],
                     var_name="Tipo", value_name="Precio de Reserva")
df_reserva["Tipo"] = df_reserva["Tipo"].map({
    "reserva_afectivo": "Afectivo",
    "reserva_racional": "Racional"
})

plt.figure(figsize=(10, 6))
sns.boxplot(data=df_reserva, x="SP", y="Precio de Reserva", hue="Tipo")
plt.title("Precio de Reserva por Producto (SP)")
plt.xlabel("Subasta del Producto")
plt.ylabel("Precio de Reserva")
plt.legend(title="Agente")
plt.tight_layout()
plt.savefig(os.path.join(output_dir, "precio_reserva_por_sp.png"))  # ✅ Guardar en carpeta
# plt.show()  # ❌ Desactivado

# ---- Utilidad por SP ----
df_utilidad = pd.melt(df, id_vars=["SP"], value_vars=["utilidad_afectivo", "utilidad_racional"],
                      var_name="Tipo", value_name="Utilidad")
df_utilidad["Tipo"] = df_utilidad["Tipo"].map({
    "utilidad_afectivo": "Afectivo",
    "utilidad_racional": "Racional"
})

plt.figure(figsize=(10, 6))
sns.boxplot(data=df_utilidad, x="SP", y="Utilidad", hue="Tipo")
plt.title("Utilidad por Producto (SP)")
plt.xlabel("Subasta del Producto")
plt.ylabel("Utilidad")
plt.legend(title="Agente")
plt.tight_layout()
plt.savefig(os.path.join(output_dir, "utilidad_por_sp.png"))  # ✅
# plt.show()

# ---- Precio de Reserva con Media y Desviación ----
mean_afectivo = df.groupby("SP")["reserva_afectivo"].mean()
std_afectivo = df.groupby("SP")["reserva_afectivo"].std()

mean_racional = df.groupby("SP")["reserva_racional"].mean()
std_racional = df.groupby("SP")["reserva_racional"].std()

sp = mean_afectivo.index

plt.figure(figsize=(12, 6))
plt.plot(sp, mean_afectivo, label="Afectivo", color='blue')
plt.fill_between(sp, mean_afectivo - std_afectivo, mean_afectivo + std_afectivo, color='blue', alpha=0.3)

plt.plot(sp, mean_racional, label="Racional", color='orange')
plt.fill_between(sp, mean_racional - std_racional, mean_racional + std_racional, color='orange', alpha=0.3)

plt.xticks(sp)
plt.title("Precio de Reserva por Producto (Media ± Desviación Estándar)")
plt.xlabel("Subasta del Producto")
plt.ylabel("Precio de Reserva")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig(os.path.join(output_dir, "media_con_sombra.png"))  # ✅
# plt.show()

# ---- Utilidad con Media y Desviación ----
utilidad_afectiva_mean = df.groupby('SP')['utilidad_afectivo'].mean()
utilidad_afectiva_std = df.groupby('SP')['utilidad_afectivo'].std()

utilidad_racional_mean = df.groupby('SP')['utilidad_racional'].mean()
utilidad_racional_std = df.groupby('SP')['utilidad_racional'].std()

sp = utilidad_afectiva_mean.index

plt.figure(figsize=(12, 6))
plt.plot(sp, utilidad_afectiva_mean, label='Afectivo', color='blue')
plt.fill_between(sp, utilidad_afectiva_mean - utilidad_afectiva_std, utilidad_afectiva_mean + utilidad_afectiva_std, color='blue', alpha=0.2)

plt.plot(sp, utilidad_racional_mean, label='Racional', color='orange')
plt.fill_between(sp, utilidad_racional_mean - utilidad_racional_std, utilidad_racional_mean + utilidad_racional_std, color='orange', alpha=0.2)

plt.title("Utilidad por Producto (Media ± Desviación Estándar)")
plt.xlabel("Subasta del Producto")
plt.ylabel("Utilidad")
plt.legend()
plt.grid(True)
plt.xticks(sp)
plt.tight_layout()
plt.savefig(os.path.join(output_dir, "utilidad_con_sombra.png"))  # ✅
# plt.show()
