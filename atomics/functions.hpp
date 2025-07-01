#ifndef COMMON_HPP
#define COMMON_HPP

#include <iostream>
#include <random>
#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/message_bag.hpp>
#include <vector>
#include <algorithm>
#include <numeric>
#include <math.h>
#include <cmath>

using namespace std;

float roundToSignificantFigures(float num, int n) { // Función para redondear a n cifras significativas
    if (num == 0.0f) return 0.0f;
    float d = ceil(log10(fabs(num)));
    int power = n - static_cast<int>(d);
    float magnitude = pow(10.0f, power);
    float shifted = round(num * magnitude);
    float result = shifted/magnitude;

    if (fabs(result) < 1e-4f) return 0.0f;

    return result;
}

unsigned int getSharedSeed() {  //Semilla para alphas iguales en racional y afectivo
    static unsigned int seed = random_device{}(); // se evalúa solo una vez
    return seed;
}
// Función para seleccionar 10 números aleatorios de un total de 100
vector<int> getRandomProducts()
{
    static vector<int> sharedNumbers; // Solo se inicializa una vez
    if (sharedNumbers.empty())
    {
        vector<int> numbers(100);                // Generar 100 números
        iota(numbers.begin(), numbers.end(), 1); // Llenar con {, 1, 2, ..., 99,100}
        random_device rd;
        mt19937 gen(rd());
        shuffle(numbers.begin(), numbers.end(), gen);
        sharedNumbers.assign(numbers.begin(), numbers.begin() + 10); // Seleccionar los primeros 10 números
    }
    return sharedNumbers;
}

struct Alphas
{
    int id;
    float alpha;
};

struct ReservePrice
{
    int id;
    float price;
};

struct Utility {
    float real;    // [1..n]
    float scaled;  // [0..1]
};

struct Emotion {
    float real;    // [1..n]
    float scaled;  // [0..1]
};

vector<Alphas> generateRandomAlphas() {
    random_device rd;
    mt19937 gen(getSharedSeed()); 
    //mt19937 gen(rd()); 
    uniform_real_distribution<float> dis(0.10, 0.99);
    vector<Alphas> alphas(100); // Crear un vector de 100 elementos
    for (int i = 0; i < 100; ++i) {
        alphas[i].id = i + 1; // ID del 1 al 100
        alphas[i].alpha = roundToSignificantFigures(dis(gen),2); // dos cifras decimales
    }
    return alphas; // Retornar el vector
}

vector<ReservePrice> generateReservePrices(const vector<Alphas>& alphas, const vector<int>& ids, float totalBudget) {
    vector<ReservePrice> reservePrices;//Estructura con ids y precios de reserva de productos que se subastan
    vector<float> selectedAlphas;      //alphas de productos que se subastan
    for (int id : ids) {
        for (const auto& alpha : alphas) {  //Encuentra los alphas de los productos que se subastan.
            if (alpha.id == id) {
                selectedAlphas.push_back(alpha.alpha);
            break;
            }
        }
    }

    float sumAlphas = accumulate(selectedAlphas.begin(), selectedAlphas.end(), 0.0f); // Suma total denominador
    for (size_t i = 0; i < ids.size(); ++i) {     // Calcular los precios de reserva
        ReservePrice rp;
        rp.id = ids[i];
        rp.price = (selectedAlphas[i] / sumAlphas) * totalBudget; // Fórmula de precios de reserva
        reservePrices.push_back(rp);
    }
    return reservePrices;
}

float maxUtility(const vector<Alphas>& alphas, const vector<int>& ids) {
    vector<float> selectedAlphas;       
    float utility = 1.0f; // inicializamos en 1 porque es un producto
    for (int id : ids) {
        for (const auto& alpha : alphas) {
            if (alpha.id == id) {
                selectedAlphas.push_back(alpha.alpha);
                break;
            }
        }
    }
    for (float alpha : selectedAlphas) {
        utility *= pow(2.0f, alpha); // potencia con base 1
        //utility += alpha;
    }
    return roundToSignificantFigures(utility,2);
}

bool waitingNextProduct()
{
    return true;
}

float updateTotalBudget(float salePrice, float totalBudget)
{
    return totalBudget - salePrice;
}

float updateMoneySpent(float buyPrice, float moneySpent)
{
    return moneySpent + buyPrice;
}

float normalize(float value, float minVal, float maxVal)
{
    return (value - minVal) / (maxVal - minVal);
}

void updateFrustration(float winner, int clientID, Emotion& frustration){
    if(winner==clientID){
        frustration.real = max(0.0f, frustration.real - 1.0f);
    }
    else{
        frustration.real = 10.0f;
    }
    frustration.scaled = normalize(frustration.real, 0, 10);
}

void resetAnxiety(Emotion& anxiety){
    anxiety.real = 0;
    anxiety.scaled = 0;
}

void updateAnxiety(Emotion& anxiety)
{
    anxiety.real = min(20.0f, anxiety.real + 1.0f);
    anxiety.scaled = normalize(anxiety.real, 0, 20);
}

bool getDecision(float bestPrice, const vector<ReservePrice> &reservePrices, int productID)
{
    for (const auto &entry : reservePrices)
    {
        if (entry.id == productID)
        {                                    // Encuentra en id del producto que se esta subastando.
            return entry.price >= bestPrice; // Devuelve si el precio de reserva es mayor o menor que el best price.
        }
    }
    return false; // Si el ID no está en la lista, retornar falso
}

float scaleAlphaForUtility(float alpha, float alphaMax = 10.0f) {
    return min(alpha / alphaMax, 1.0f); // Escala entre 0 y 1
}

void updateBestPrice(float& initialPrice, float& bestPrice) {
    if (bestPrice < 10.0f) {
        bestPrice = bestPrice * 1.3f;
    } else if (bestPrice <= 100.0f) {
        bestPrice = bestPrice * 1.2f;
    } else {
        bestPrice = bestPrice * 1.1f;
    }
    bestPrice = roundToSignificantFigures(bestPrice, 4);
}

float generateUtility(const vector<Alphas>& alphas, const vector<int>& ids) {
    vector<float> selectedAlphas;       
    float utility = 1.0f; // inicializamos en 1 porque es una productoria
    for (int id : ids) {
        for (const auto& alpha : alphas) {
            if (alpha.id == id) {
                selectedAlphas.push_back(alpha.alpha);
                break;
            }
        }
    }
    for (float alpha : selectedAlphas) {
        utility *= pow(1.0f, alpha); // potencia con base 1
    }
    return utility;
}

#endif // COMMON_HPP