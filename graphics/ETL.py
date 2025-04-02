import pandas as pd
import re
import matplotlib.pyplot as plt
import seaborn as sns
import glob

# Función para extraer datos de un archivo de simulación
def parse_simulation_log(file_path):
    data = []
    with open(file_path, 'r', encoding='utf-8') as file:
        lines = file.readlines()
    
    timestamp = None
    affective_price = None
    rational_price = None
    best_price = None
    
    for line in lines:
        timestamp_match = re.match(r'(\d{2}:\d{2}:\d{2}:\d{3})', line)
        if timestamp_match:
            if affective_price is not None and rational_price is not None and best_price is not None:
                data.append({
                    "Timestamp": timestamp,
                    "Affective_Price": affective_price,
                    "Rational_Price": rational_price,
                    "Best_Price": best_price
                })
            timestamp = timestamp_match.group()
            affective_price = None
            rational_price = None
            best_price = None
        
        affective_match = re.search(r'ReservePrice for Current Product: \[ ID Product N°\d+ : ([\d\.]+) \]', line)
        rational_match = re.search(r'ReservePrice for Current Product: \[ ID Product N°\d+ : ([\d\.]+) \]', line)
        best_price_match = re.search(r'Current BestPrice: ([\d\.]+)', line)
        
        if "affective_1" in line and affective_match:
            affective_price = float(affective_match.group(1))
        
        if "rational_2" in line and rational_match:
            rational_price = float(rational_match.group(1))
        
        if best_price_match:
            best_price = float(best_price_match.group(1))
    
    if affective_price is not None and rational_price is not None and best_price is not None:
        data.append({
            "Timestamp": timestamp,
            "Affective_Price": affective_price,
            "Rational_Price": rational_price,
            "Best_Price": best_price
        })
    
    df = pd.DataFrame(data)
    df["Timestamp"] = pd.to_datetime(df["Timestamp"], format='%H:%M:%S:%f')
    return df

# Leer y procesar los 100 archivos
dataframes = []
file_paths = glob.glob("C:/Cadmium-Simulation-Environment/DEVS-Models/Auction System/caso_de_estudio_1/ABP_output_state_*.csv")

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
    "Best_Price": ['mean', 'std']
}).reset_index()

# Graficar la evolución de la media y desviación estándar de Affective Price
plt.figure(figsize=(12, 6))

# Graficar la media
sns.lineplot(x=grouped_data['Timestamp'], y=grouped_data[('Affective_Price', 'mean')], label="Affective Mean", marker="o")

# Agregar la banda para la desviación estándar
plt.fill_between(
    grouped_data['Timestamp'], 
    grouped_data[('Affective_Price', 'mean')] - grouped_data[('Affective_Price', 'std')], 
    grouped_data[('Affective_Price', 'mean')] + grouped_data[('Affective_Price', 'std')], 
    alpha=0.2, label="Affective Std Dev"
)

plt.xlabel("Time")
plt.ylabel("Reserve Price")
plt.title("Evolution of Affective Reserve Price with Std Dev")
plt.xticks(rotation=45)
plt.legend()
plt.grid()
plt.show()

# Graficar la evolución de la media y desviación estándar de Rational Price
plt.figure(figsize=(12, 6))

# Graficar la media
sns.lineplot(x=grouped_data['Timestamp'], y=grouped_data[('Rational_Price', 'mean')], label="Rational Mean", marker="o")

# Agregar la banda para la desviación estándar
plt.fill_between(
    grouped_data['Timestamp'], 
    grouped_data[('Rational_Price', 'mean')] - grouped_data[('Rational_Price', 'std')], 
    grouped_data[('Rational_Price', 'mean')] + grouped_data[('Rational_Price', 'std')], 
    alpha=0.2, label="Rational Std Dev"
)

plt.xlabel("Time")
plt.ylabel("Reserve Price")
plt.title("Evolution of Rational Reserve Price with Std Dev")
plt.xticks(rotation=45)
plt.legend()
plt.grid()
plt.show()

# Graficar la evolución de la media y desviación estándar de Best Price
plt.figure(figsize=(12, 6))

# Graficar la media
sns.lineplot(x=grouped_data['Timestamp'], y=grouped_data[('Best_Price', 'mean')], label="Best Price Mean", marker="o")

# Agregar la banda para la desviación estándar
plt.fill_between(
    grouped_data['Timestamp'], 
    grouped_data[('Best_Price', 'mean')] - grouped_data[('Best_Price', 'std')], 
    grouped_data[('Best_Price', 'mean')] + grouped_data[('Best_Price', 'std')], 
    alpha=0.2, label="Best Price Std Dev"
)

plt.xlabel("Time")
plt.ylabel("Best Price")
plt.title("Evolution of Best Price with Std Dev")
plt.xticks(rotation=45)
plt.legend()
plt.grid()
plt.show()

# Graficar boxplot de Affective Price (Vertical)
plt.figure(figsize=(12, 6))
sns.boxplot(y=all_data['Affective_Price'], color="skyblue", fliersize=5)
plt.title("Boxplot of Affective Price")
plt.ylabel("Affective Price")
plt.grid(True)
plt.show()

# Graficar boxplot de Rational Price (Vertical)
plt.figure(figsize=(12, 6))
sns.boxplot(y=all_data['Rational_Price'], color="salmon", fliersize=5)
plt.title("Boxplot of Rational Price")
plt.ylabel("Rational Price")
plt.grid(True)
plt.show()

# Graficar boxplot de Best Price (Vertical)
plt.figure(figsize=(12, 6))
sns.boxplot(y=all_data['Best_Price'], color="lightgreen", fliersize=5)
plt.title("Boxplot of Best Price")
plt.ylabel("Best Price")
plt.grid(True)
plt.show()