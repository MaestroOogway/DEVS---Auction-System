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

// Función para seleccionar 10 números aleatorios de un total de 100
std::vector<int> getRandomProducts()
{
    static std::vector<int> sharedNumbers; // Solo se inicializa una vez
    if (sharedNumbers.empty())
    {
        std::vector<int> numbers(100);                // Generar 100 números
        std::iota(numbers.begin(), numbers.end(), 1); // Llenar con {, 1, 2, ..., 99,100}
        std::random_device rd;
        std::mt19937 gen(rd());
        std::shuffle(numbers.begin(), numbers.end(), gen);
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

float generateUtilityAffective(const std::vector<Alphas>& alphas, const std::vector<int>& ids) {
    std::vector<float> selectedAlphas;       
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
        utility *= pow(1.0f, alpha); // potencia con base 1
    }
    return utility;
}

float generateUtilityRational(const std::vector<Alphas>& alphas, const std::vector<int>& ids) {
    std::vector<float> selectedAlphas;       
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
        utility *= pow(1.0f, alpha); // potencia con base 1
    }
    return utility;
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

// Actualizar frustración del agente afectivo
float updateFrustration(float winner, int clientID, float frustration)
{
    return (winner == clientID) ? std::max(0.0f, frustration - 1) : frustration + 1;
}

// Actualizar ansiedad del agente afectivo
float updateAnxiety(float anxiety, float sum)
{
    if (sum > 0)
    {
        anxiety = anxiety + 1;
        return anxiety;
    }
    else
    {
        anxiety = 0;
        return anxiety;
    }
}

bool getDecision(float bestPrice, const std::vector<ReservePrice> &reservePrices, int productID)
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

#endif // COMMON_HPP