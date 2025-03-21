#ifndef COMMON_HPP
#define COMMON_HPP

#include <iostream>
#include <random>
#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/message_bag.hpp>


bool waitingNextProduct(){
    return true;
}

bool getDecision(float bestPrice, float reservePrice) {
    return reservePrice >= bestPrice;
}

float updateTotalBudget(float salePrice, float totalBudget)
{
    return totalBudget - salePrice;
}

#endif // COMMON_HPP
