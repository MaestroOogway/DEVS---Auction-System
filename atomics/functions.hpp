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

unsigned int getSharedSeed() {  //Semilla para alphas iguales en racional y afectivo
    static unsigned int seed = std::random_device{}(); // se evalúa solo una vez
    return seed;
}
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

std::vector<Alphas> generateRandomAlphas() {
    std::random_device rd;
    std::mt19937 gen(getSharedSeed());  //cambiar a gen(rd()) para aleatorizar entre instancias 
    std::uniform_real_distribution<float> dis(0.0, 1.0);
    std::vector<Alphas> alphas(100); // Crear un vector de 100 elementos
    for (int i = 0; i < 100; ++i) {
        alphas[i].id = i + 1; // ID del 1 al 100
        alphas[i].alpha = dis(gen); // Número aleatorio entre 0 y 1
    }
    return alphas; // Retornar el vector
}

vector<ReservePrice> generateReservePrices(const std::vector<Alphas>& alphas, const std::vector<int>& ids, float totalBudget) {
    std::vector<ReservePrice> reservePrices;//Estructura con ids y precios de reserva de productos que se subastan
    std::vector<float> selectedAlphas;      //alphas de productos que se subastan
    for (int id : ids) {
        for (const auto& alpha : alphas) {  //Encuentra los alphas de los productos que se subastan.
            if (alpha.id == id) {
                selectedAlphas.push_back(alpha.alpha);
            break;
            }
        }
    }

    float sumAlphas = std::accumulate(selectedAlphas.begin(), selectedAlphas.end(), 0.0f); // Suma total denominador
    for (size_t i = 0; i < ids.size(); ++i) {     // Calcular los precios de reserva
        ReservePrice rp;
        rp.id = ids[i];
        rp.price = (selectedAlphas[i] / sumAlphas) * totalBudget; // Fórmula de precios de reserva
        reservePrices.push_back(rp);
    }
    return reservePrices;
}

float generateUtility(const std::vector<Alphas>& alphas, const std::vector<int>& ids) {
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

float scaleAlphaForUtility(float alpha, float alphaMax = 10.0f) {
    return std::min(alpha / alphaMax, 1.0f); // Escala entre 0 y 1
}

/*" | Alphas :";
for (const auto &alpha : i.alphas){
    os << "[ ID Product N°" << alpha.id << " : " << alpha.alpha << " ] ";
}*/

#endif // COMMON_HPP