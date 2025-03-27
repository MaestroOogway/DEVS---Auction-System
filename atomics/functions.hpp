#ifndef COMMON_HPP
#define COMMON_HPP

#include <iostream>
#include <random>
#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/message_bag.hpp>
#include <vector>
#include <algorithm>
#include <numeric>

// Función para seleccionar 10 números aleatorios de un total de 100
std::vector<int> getRandomNumbers() {
    static std::vector<int> sharedNumbers;  // Solo se inicializa una vez
    if (sharedNumbers.empty()) {
        // Generar 100 números
        std::vector<int> numbers(100);
        std::iota(numbers.begin(), numbers.end(), 1);  // Llenar con {0, 1, 2, ..., 99}
        // Mezclar los números de manera aleatoria
        std::random_device rd;
        std::mt19937 gen(rd());
        std::shuffle(numbers.begin(), numbers.end(), gen);
        // Seleccionar los primeros 10 números
        sharedNumbers.assign(numbers.begin(), numbers.begin() + 10);
    }
    return sharedNumbers;
}

bool waitingNextProduct() {
    return true;
}
bool getDecision(float bestPrice, float reservePrice) {
    return reservePrice >= bestPrice;
}

float updateTotalBudget(float salePrice, float totalBudget) {
    return totalBudget - salePrice;
}

float updateMoneySpent(float buyPrice, float moneySpent) {
    return moneySpent + buyPrice;
}

// Actualizar frustración del agente afectivo
float updateFrustration(float winner, int clientID, float frustration) {
    return (winner == clientID) ? std::max(0.0f, frustration - 1) : frustration + 1;
}

// Actualizar ansiedad del agente afectivo
float updateAnxiety(float anxiety, float sum) {
    if (sum > 0) {
        anxiety = anxiety + 1;
        return anxiety;
    } else {
        anxiety = 0;
        return anxiety;
    }
}

#endif // COMMON_HPP
