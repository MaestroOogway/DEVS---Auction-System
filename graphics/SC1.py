import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import glob
import re

# Función para extraer datos de un archivo de simulación
def parse_simulation_log(file_path):
    data = []
    with open(file_path, 'r', encoding='utf-8') as file:
        lines = file.readlines()

    timestamp = None
    affective_price = None
    rational_price = None
    best_price = None
    affective_utility = None
    rational_utility = None

    for line in lines:
        timestamp_match = re.match(r'(\d{2}:\d{2}:\d{2}:\d{3})', line)
        if timestamp_match:
            if all(v is not None for v in [affective_price, rational_price, best_price, affective_utility, rational_utility]):
                data.append({
                    "Timestamp": timestamp,
                    "Affective_Price": affective_price,
                    "Rational_Price": rational_price,
                    "Best_Price": best_price,
                    "Affective_Utility": affective_utility,
                    "Rational_Utility": rational_utility
                })
            timestamp = timestamp_match.group()
            affective_price = rational_price = best_price = None
            affective_utility = rational_utility = None

        if "affective_1" in line:
            affective_price_match = re.search(r'ReservePrice for Current Product: \[ ID Product N°\d+ : ([\d\.]+) \]', line)
            affective_utility_match = re.search(r'Utility: ([\d\.]+)', line)
            if affective_price_match:
                affective_price = float(affective_price_match.group(1))
            if affective_utility_match:
                affective_utility = float(affective_utility_match.group(1))

        if "rational_2" in line:
            rational_price_match = re.search(r'ReservePrice for Current Product: \[ ID Product N°\d+ : ([\d\.]+) \]', line)
            rational_utility_match = re.search(r'Utility: ([\d\.]+)', line)
            if rational_price_match:
                rational_price = float(rational_price_match.group(1))
            if rational_utility_match:
                rational_utility = float(rational_utility_match.group(1))

        best_price_match = re.search(r'Current BestPrice: ([\d\.]+)', line)
        if best_price_match:
            best_price = float(best_price_match.group(1))

    # Último registro
    if all(v is not None for v in [affective_price, rational_price, best_price, affective_utility, rational_utility]):
        data.append({
            "Timestamp": timestamp,
            "Affective_Price": affective_price,
            "Rational_Price": rational_price,
            "Best_Price": best_price,
            "Affective_Utility": affective_utility,
            "Rational_Utility": rational_utility
        })

    df = pd.DataFrame(data)
    df["Timestamp"] = pd.to_datetime(df["Timestamp"], format='%H:%M:%S:%f')
    return df

# Leer y procesar todos los archivos
dataframes = []
file_paths = glob.glob("C:/Cadmium-Simulation-Environment/DEVS-Models/Auction System/casos_de_estudio/caso_de_estudio_1/states/ABP_output_state_*.csv")

for file_path in file_paths:
    df = parse_simulation_log(file_path)
    dataframes.append(df)

# Unir todos los datos en un solo DataFrame
all_data = pd.concat(dataframes)
all_data.sort_values(by="Timestamp", inplace=True)

# Calcular media y desviación estándar
grouped_data = all_data.groupby("Timestamp").agg({
    "Affective_Price": ['mean', 'std'],
    "Rational_Price": ['mean', 'std'],
    "Best_Price": ['mean', 'std'],
    "Affective_Utility": ['mean', 'std'],
    "Rational_Utility": ['mean', 'std']
}).reset_index()

# === Gráfico combinado de precios de reserva ===
plt.figure(figsize=(12, 6))

# Afectivo
sns.lineplot(x=grouped_data['Timestamp'], y=grouped_data[('Affective_Price', 'mean')],
             label="Affective Mean", color='blue')
plt.fill_between(
    grouped_data['Timestamp'],
    grouped_data[('Affective_Price', 'mean')] - grouped_data[('Affective_Price', 'std')],
    grouped_data[('Affective_Price', 'mean')] + grouped_data[('Affective_Price', 'std')],
    alpha=0.2, color='blue'
)

# Racional
sns.lineplot(x=grouped_data['Timestamp'], y=grouped_data[('Rational_Price', 'mean')],
             label="Rational Mean", color='orange')
plt.fill_between(
    grouped_data['Timestamp'],
    grouped_data[('Rational_Price', 'mean')] - grouped_data[('Rational_Price', 'std')],
    grouped_data[('Rational_Price', 'mean')] + grouped_data[('Rational_Price', 'std')],
    alpha=0.2, color='orange'
)

plt.xlabel("Time")
plt.ylabel("Reserve Price")
plt.title("Reserve Price (Mean & Std Dev) - Affective vs Rational")
plt.xticks(rotation=45)
plt.legend()
plt.grid()
plt.tight_layout()
plt.show()

# === Gráfico combinado de utilidad: Afectivo vs Racional ===
plt.figure(figsize=(12, 6))

# Utilidad Afectiva
sns.lineplot(x=grouped_data['Timestamp'], y=grouped_data[('Affective_Utility', 'mean')],
             label="Affective Utility Mean", color='green')
plt.fill_between(
    grouped_data['Timestamp'],
    grouped_data[('Affective_Utility', 'mean')] - grouped_data[('Affective_Utility', 'std')],
    grouped_data[('Affective_Utility', 'mean')] + grouped_data[('Affective_Utility', 'std')],
    alpha=0.2, color='green'
)

# Utilidad Racional
sns.lineplot(x=grouped_data['Timestamp'], y=grouped_data[('Rational_Utility', 'mean')],
             label="Rational Utility Mean", color='purple')
plt.fill_between(
    grouped_data['Timestamp'],
    grouped_data[('Rational_Utility', 'mean')] - grouped_data[('Rational_Utility', 'std')],
    grouped_data[('Rational_Utility', 'mean')] + grouped_data[('Rational_Utility', 'std')],
    alpha=0.2, color='purple'
)

plt.xlabel("Time")
plt.ylabel("Utility")
plt.title("Utility (Mean & Std Dev) - Affective vs Rational")
plt.xticks(rotation=45)
plt.legend()
plt.grid()
plt.tight_layout()
plt.show()
