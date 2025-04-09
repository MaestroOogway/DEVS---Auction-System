import re
import glob
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

# Función para extraer datos de múltiples agentes en un archivo
def parse_simulation_log(file_path):
    data = []
    with open(file_path, 'r', encoding='utf-8') as file:
        lines = file.readlines()

    timestamp = None
    affective_prices = []
    rational_prices = []
    affective_utils = []
    rational_utils = []
    best_price = None

    for line in lines:
        timestamp_match = re.match(r'(\d{2}:\d{2}:\d{2}:\d{3})', line)
        if timestamp_match:
            if timestamp and best_price is not None and affective_prices and rational_prices:
                data.append({
                    "Timestamp": timestamp,
                    "Affective_Price_Mean": sum(affective_prices) / len(affective_prices),
                    "Affective_Price_Std": pd.Series(affective_prices).std(),
                    "Rational_Price_Mean": sum(rational_prices) / len(rational_prices),
                    "Rational_Price_Std": pd.Series(rational_prices).std(),
                    "Affective_Utility_Mean": sum(affective_utils) / len(affective_utils),
                    "Affective_Utility_Std": pd.Series(affective_utils).std(),
                    "Rational_Utility_Mean": sum(rational_utils) / len(rational_utils),
                    "Rational_Utility_Std": pd.Series(rational_utils).std(),
                    "Best_Price": best_price
                })

            timestamp = timestamp_match.group()
            affective_prices = []
            rational_prices = []
            affective_utils = []
            rational_utils = []
            best_price = None

        # Agentes afectivos
        if "affective_" in line:
            price_match = re.search(r'ReservePrice for Current Product: \[ ID Product N°\d+ : ([\d\.]+) \]', line)
            utility_match = re.search(r'Utility: ([\d\.]+)', line)
            if price_match:
                affective_prices.append(float(price_match.group(1)))
            if utility_match:
                affective_utils.append(float(utility_match.group(1)))

        # Agentes racionales
        if "rational_" in line:
            price_match = re.search(r'ReservePrice for Current Product: \[ ID Product N°\d+ : ([\d\.]+) \]', line)
            utility_match = re.search(r'Utility: ([\d\.]+)', line)
            if price_match:
                rational_prices.append(float(price_match.group(1)))
            if utility_match:
                rational_utils.append(float(utility_match.group(1)))

        # Precio del subastador
        best_price_match = re.search(r'Current BestPrice: ([\d\.]+)', line)
        if best_price_match:
            best_price = float(best_price_match.group(1))

    # Guardar último timestamp
    if timestamp and best_price is not None and affective_prices and rational_prices:
        data.append({
            "Timestamp": timestamp,
            "Affective_Price_Mean": sum(affective_prices) / len(affective_prices),
            "Affective_Price_Std": pd.Series(affective_prices).std(),
            "Rational_Price_Mean": sum(rational_prices) / len(rational_prices),
            "Rational_Price_Std": pd.Series(rational_prices).std(),
            "Affective_Utility_Mean": sum(affective_utils) / len(affective_utils),
            "Affective_Utility_Std": pd.Series(affective_utils).std(),
            "Rational_Utility_Mean": sum(rational_utils) / len(rational_utils),
            "Rational_Utility_Std": pd.Series(rational_utils).std(),
            "Best_Price": best_price
        })

    df = pd.DataFrame(data)
    df["Timestamp"] = pd.to_datetime(df["Timestamp"], format='%H:%M:%S:%f')
    return df

# === Leer y combinar todos los archivos ===
file_paths = glob.glob("C:/Cadmium-Simulation-Environment/DEVS-Models/Auction System/casos_de_estudio/caso_de_estudio_2/states/ABP_output_state_*.csv")
all_data = pd.concat([parse_simulation_log(fp) for fp in file_paths])
all_data.sort_values("Timestamp", inplace=True)

# === Gráfico combinado de precios de reserva ===
plt.figure(figsize=(12, 6))
sns.lineplot(x=all_data["Timestamp"], y=all_data["Affective_Price_Mean"], label="Affective Mean", color="blue")
plt.fill_between(all_data["Timestamp"],
                 all_data["Affective_Price_Mean"] - all_data["Affective_Price_Std"],
                 all_data["Affective_Price_Mean"] + all_data["Affective_Price_Std"],
                 alpha=0.2, color="blue")

sns.lineplot(x=all_data["Timestamp"], y=all_data["Rational_Price_Mean"], label="Rational Mean", color="orange")
plt.fill_between(all_data["Timestamp"],
                 all_data["Rational_Price_Mean"] - all_data["Rational_Price_Std"],
                 all_data["Rational_Price_Mean"] + all_data["Rational_Price_Std"],
                 alpha=0.2, color="orange")

plt.xlabel("Time")
plt.ylabel("Reserve Price")
plt.title("Reserve Price (Mean & Std Dev) - Affective vs Rational")
plt.xticks(rotation=45)
plt.legend()
plt.grid()
plt.tight_layout()
plt.show()

# === Gráfico combinado de utilidad (afectivo y racional) ===
plt.figure(figsize=(12, 6))

# Línea para agentes afectivos
sns.lineplot(x=all_data["Timestamp"], y=all_data["Affective_Utility_Mean"], label="Affective Utility Mean", color="green")
plt.fill_between(all_data["Timestamp"],
                 all_data["Affective_Utility_Mean"] - all_data["Affective_Utility_Std"],
                 all_data["Affective_Utility_Mean"] + all_data["Affective_Utility_Std"],
                 alpha=0.2, color="green")

# Línea para agentes racionales
sns.lineplot(x=all_data["Timestamp"], y=all_data["Rational_Utility_Mean"], label="Rational Utility Mean", color="purple")
plt.fill_between(all_data["Timestamp"],
                 all_data["Rational_Utility_Mean"] - all_data["Rational_Utility_Std"],
                 all_data["Rational_Utility_Mean"] + all_data["Rational_Utility_Std"],
                 alpha=0.2, color="purple")

# Detalles del gráfico
plt.xlabel("Time")
plt.ylabel("Utility")
plt.title("Affective vs Rational Utility (Mean & Std Dev)")
plt.xticks(rotation=45)
plt.legend()
plt.grid()
plt.tight_layout()
plt.show()