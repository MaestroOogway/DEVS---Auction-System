#ifndef _RATIONAL_HPP__
#define _RATIONAL_HPP__

#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/message_bag.hpp>
#include "../data_structures/message.hpp"

#include "functions.hpp"
#include <limits>
#include <assert.h>
#include <string>
#include <random>

using namespace cadmium; using namespace std;

std::set<int> subastadosR;

void updateUtilityRational(float &utility, int subastadoID, std::vector<Alphas>& alphas, const std::vector<int>& productIDs, int win) {
    std::vector<int> remainingProductIDs;
    std::vector<float> remainingAlphas;
    // Filtrar productos que aún no han sido subastados
    for (int id : productIDs) {
        if (subastadosR.find(id) == subastadosR.end()) {
            remainingProductIDs.push_back(id);
        }
    }
    // Obtener los alphas de los productos restantes (no subastados)
    for (int id : remainingProductIDs) {
        for (const auto& alpha : alphas) {
            if (alpha.id == id) {
                remainingAlphas.push_back(alpha.alpha);
                break;
            }
        }
    }
    // Buscar el alpha del producto que se está subastando
    auto it = std::find_if(alphas.begin(), alphas.end(), [subastadoID](const Alphas& a) {
        return a.id == subastadoID;
    });
    if (it != alphas.end()) {
        float alphaActual = it->alpha;
        utility *= pow(0.1 + win, alphaActual);  // (1+1)^alpha
    }
}

void updateReservePricesRational(std::vector<ReservePrice>& reservePrices, std::vector<Alphas>& alphas,
    const std::vector<int>& productIDs, float totalBudget, int subastadoID){
    // Filtrar productos restantes (excluyendo TODOS los productos ya subastadosR)
    std::vector<int> remainingProductIDs;
    for (int id : productIDs) {
        if (subastadosR.find(id) == subastadosR.end()) { // Si el producto NO ha sido subastado
            remainingProductIDs.push_back(id);
        }
    }
    // Calcular la nueva suma total de los alphas de productos restantes
    float totalAlpha = 0.0f;
    for (int id : remainingProductIDs) {
        auto it = std::find_if(alphas.begin(), alphas.end(), [id](const Alphas& a) {
        return a.id == id;
        });
        if (it != alphas.end()) {
            totalAlpha += it->alpha;
        }
    }
    // Actualizar precios de reserva solo para los productos restantes
    for (auto& rp : reservePrices) {
        if (subastadosR.find(rp.id) != subastadosR.end()) {
            rp.price = 0.0f; // El producto subastado ya no tiene precio de reserva
        } else {
            auto it = std::find_if(alphas.begin(), alphas.end(), [rp](const Alphas& a) {
                return a.id == rp.id;
            });
            if (it != alphas.end()) {
                rp.price = (it->alpha / totalAlpha) * totalBudget;
            }
        }
    }
}

struct Rational_defs { // Port definition
    struct in_initialIP : public in_port<Message_initialIP_t> {};
    struct in_roundResult : public in_port<Message_roundResult_t> {};
    struct in_finalResult : public in_port<Message_finalResults_t> {};
    struct out_bidOffer : public out_port<Message_bidOffer_t> {};
};

template <typename TIME>
class Rational {
public:
    using input_ports = tuple<typename Rational_defs::in_initialIP, typename Rational_defs::in_roundResult, typename Rational_defs::in_finalResult>;
    using output_ports = tuple<typename Rational_defs::out_bidOffer>;
    struct state_type { // Estado del agente
        int idAgent, currentProductID;
        float totalBudget, moneySpent, currentBestPrice, utility;
        bool modelActive, decision, waitingNextProduct;
        vector<Alphas> alphas;
        vector<ReservePrice> reservePrices;
        vector<Message_finalResults_t> purschasedProducts;
    } state;
    Rational() = default;
    Rational(int id, float budget) noexcept{
        state.idAgent = id;
        state.totalBudget = budget;
        state.moneySpent = 0;
        state.modelActive = false;
        state.decision = false;
        state.waitingNextProduct = false;
        state.purschasedProducts.clear();
        state.alphas = generateRandomAlphas(); // Inicialización del vector alpha en el constructor
        state.utility = generateUtility(state.alphas, getRandomProducts());
        state.reservePrices = generateReservePrices(state.alphas, getRandomProducts() ,state.totalBudget);
    }
    // funcion de transición interna
    void internal_transition()
    {
        if (state.waitingNextProduct){
            subastadosR.insert(state.currentProductID);
            state.waitingNextProduct = false;
        }
        state.modelActive = false;
    }
    // función de transición externa
    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs)
    {

        if (!get_messages<typename Rational_defs::in_finalResult>(mbs).empty()) // Verificar mensajes en el puerto 'finalResult'
        {
            state.modelActive = true;
            auto finalResultMessages = get_messages<typename Rational_defs::in_finalResult>(mbs);
            auto finalResult = finalResultMessages[0];
            state.currentProductID = finalResult.productID;
            if (finalResult.winnerID == state.idAgent){
                state.totalBudget = updateTotalBudget(finalResult.bestPrice, state.totalBudget);
                state.moneySpent = updateMoneySpent(finalResult.bestPrice, state.moneySpent);
                state.purschasedProducts.push_back(finalResult);
            }
            updateUtilityRational(state.utility, state.currentProductID, state.alphas, getRandomProducts(), (finalResult.winnerID == state.idAgent ? 1 : 0));
            state.waitingNextProduct = waitingNextProduct();
            state.decision = false;
        }
        else if (!get_messages<typename Rational_defs::in_roundResult>(mbs).empty()) // Verificar si hay mensajes en el puerto 'in_roundResult'
        {
            state.modelActive = true;
            auto roundResultMessages = get_messages<typename Rational_defs::in_roundResult>(mbs);
            auto roundResult = roundResultMessages[0];
            state.currentBestPrice = roundResult.bestPrice;
            state.decision = getDecision(state.currentBestPrice, state.reservePrices, state.currentProductID);
        }
        else if (!get_messages<typename Rational_defs::in_initialIP>(mbs).empty()) // Verificar si hay mensajes en el puerto 'in_initialIP'
        {
            state.modelActive = true;
            auto initialProductInfoMessages = get_messages<typename Rational_defs::in_initialIP>(mbs);
            auto productInfo = initialProductInfoMessages[0];
            state.currentProductID = productInfo.productID;
            state.currentBestPrice = productInfo.bestPrice;
            updateReservePricesRational(state.reservePrices, state.alphas, getRandomProducts(), state.totalBudget, state.currentProductID);
            state.decision = getDecision(state.currentBestPrice, state.reservePrices, state.currentProductID);
        }
    }
    // funcion de transiscion de confluencia, ejecuta la interna y luego la externa.
    void confluence_transition(TIME e, typename make_message_bags<input_ports>::type mbs)
    {
        internal_transition();
        external_transition(TIME(), move(mbs));
    }
    // funcion de salida
    typename make_message_bags<output_ports>::type output() const
    {
        typename make_message_bags<output_ports>::type bags; // Crear un mensaje para enviar, basado en el estado del modelo
        vector<Message_bidOffer_t> bag_port_out;
        Message_bidOffer_t outgoingMessage; // Asociar el mensaje al puerto de salida
        if (state.decision == true)
        {
            outgoingMessage.clientID = state.idAgent;
            outgoingMessage.productID = state.currentProductID;
            outgoingMessage.priceProposal = state.currentBestPrice;
            outgoingMessage.decision = state.decision;
            bag_port_out.push_back(outgoingMessage);
            get_messages<typename Rational_defs::out_bidOffer>(bags) = bag_port_out;
        }
        else if (state.decision == false)
        {
            outgoingMessage.clientID = state.idAgent;
            outgoingMessage.productID = state.waitingNextProduct ? 0 : state.currentProductID;
            outgoingMessage.priceProposal = 0;
            outgoingMessage.decision = state.waitingNextProduct ? 1 : state.decision;
            bag_port_out.push_back(outgoingMessage);
            get_messages<typename Rational_defs::out_bidOffer>(bags) = bag_port_out;
        }
        return bags;
    }
    // Tiempo de avance
    TIME time_advance() const
    {
        if (state.modelActive == false)
        {
            return std::numeric_limits<TIME>::infinity();
        }
        return state.modelActive ? TIME("00:00:05:000") : numeric_limits<TIME>::infinity();
    }

    friend ostringstream &operator<<(ostringstream &os, const typename Rational<TIME>::state_type &i) {
        os << " IdAgent: " << i.idAgent
        << " | Wait Next Product: " << (i.waitingNextProduct ? "true" : "false")
        << " | TotalBudget: " << i.totalBudget
        << " | Utility: " << i.utility
        << " | ReservePrice for Current Product: ";
        // Buscar el precio de reserva del producto actual
        for (const auto &reserve : i.reservePrices) {
            if (reserve.id == i.currentProductID) {  // Si es el producto actual
                os << "[ ID Product N°" << reserve.id << " : " << reserve.price << " ] ";
                break; // Rompemos el bucle para solo mostrar uno
            }
        }
        return os;
    }
};
#endif