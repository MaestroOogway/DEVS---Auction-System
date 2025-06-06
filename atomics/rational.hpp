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
#include <iomanip> // necesario para std::fixed y std::setprecision

using namespace cadmium; using namespace std;

std::set<int> subastadosR;

void updateUtilityR(float &utility, int subastadoID, std::vector<Alphas>& alphas, const std::vector<int>& productIDs, int win) {
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
        float resultado = pow(1 + win, alphaActual);
        utility *= resultado;
    }
}

void updateReservePR(std::vector<ReservePrice>& reservePrices, std::vector<Alphas>& alphas,
    const std::vector<int>& productIDs, float totalBudget, int subastadoID){
    std::vector<int> remainingProductIDs;    // Filtrar productos restantes (excluyendo TODOS los productos ya subastadosR)
    float totalAlpha = 0.0f;
    for (int id : productIDs) {
        if (subastadosR.find(id) == subastadosR.end()) { // Si el producto NO ha sido subastado
            remainingProductIDs.push_back(id);
        }
    }
    // Calcular la nueva suma total de los alphas de productos restantes
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
                rp.price = roundToSignificantFigures((it->alpha / totalAlpha) * totalBudget,3);
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
        float totalBudget, moneySpent, currentBestPrice, utility, scaledUtility;
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
        state.utility =  generateUtility(state.alphas, getRandomProducts());
        state.scaledUtility = normalize(state.utility, 1.0f, maxUtility(state.alphas, getRandomProducts()));
        state.reservePrices = generateReservePrices(state.alphas, getRandomProducts() ,state.totalBudget);
    }
    // funcion de transición interna
    void internal_transition() {
        if (state.waitingNextProduct){
            state.scaledUtility = std::min(1.0f, std::max(0.00f, roundToSignificantFigures(normalize(state.utility, 1.0f, maxUtility(state.alphas, getRandomProducts())), 2)));
            subastadosR.insert(state.currentProductID);
            state.waitingNextProduct = false;
        }
        state.modelActive = false;
    }
    // función de transición externa
    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
        state.modelActive = true;
        if (!get_messages<typename Rational_defs::in_initialIP>(mbs).empty()){ // Verificar si hay mensajes en el puerto 'in_initialIP'
            auto productInfo = get_messages<typename Rational_defs::in_initialIP>(mbs)[0];
            state.currentProductID = productInfo.productID;
            state.currentBestPrice = productInfo.initialPrice;
            updateReservePR(state.reservePrices, state.alphas, getRandomProducts(), state.totalBudget, state.currentProductID);
            state.decision = getDecision(state.currentBestPrice, state.reservePrices, productInfo.productID);
        }
        else if (!get_messages<typename Rational_defs::in_roundResult>(mbs).empty()){ // Verificar si hay mensajes en el puerto 'in_roundResult'
            auto roundResult = get_messages<typename Rational_defs::in_roundResult>(mbs)[0];
            state.currentBestPrice = roundResult.bestPrice;
            state.decision = getDecision(state.currentBestPrice, state.reservePrices, state.currentProductID);
        }
        else if (!get_messages<typename Rational_defs::in_finalResult>(mbs).empty()){ // Verificar mensajes en el puerto 'finalResult'
            auto finalResult = get_messages<typename Rational_defs::in_finalResult>(mbs)[0];
            if (finalResult.winnerID == state.idAgent){
                state.totalBudget = updateTotalBudget(finalResult.bestPrice, state.totalBudget);
                state.moneySpent = updateMoneySpent(finalResult.bestPrice, state.moneySpent);
                state.purschasedProducts.push_back(finalResult);
            }
            updateUtilityR(state.utility, state.currentProductID, state.alphas, getRandomProducts(), (finalResult.winnerID == state.idAgent));
            state.waitingNextProduct = waitingNextProduct();
            state.decision = false;
        }
    }
    // funcion de transiscion de confluencia, ejecuta la interna y luego la externa.
    void confluence_transition(TIME e, typename make_message_bags<input_ports>::type mbs){
        internal_transition();
        external_transition(TIME(), move(mbs));
    }
    // funcion de salida
    typename make_message_bags<output_ports>::type output() const{
        typename make_message_bags<output_ports>::type bags;
        vector<Message_bidOffer_t> bag_port_out;
        Message_bidOffer_t msg;
        msg.clientID = state.idAgent;
        msg.productID = state.waitingNextProduct ? 0 : state.currentProductID;
        msg.priceProposal = state.decision ? state.currentBestPrice : 0;
        msg.decision = state.waitingNextProduct ? 1 : state.decision;
        bag_port_out.push_back(msg);
        get_messages<typename Rational_defs::out_bidOffer>(bags) = bag_port_out;
        return bags;
    }

    TIME time_advance() const {
        return state.modelActive ? TIME("00:00:05:000") : numeric_limits<TIME>::infinity();
    }

    friend ostringstream &operator<<(ostringstream &os, const typename Rational<TIME>::state_type &i) {
        os << " IdAgent: " << i.idAgent
        << " | Wait Next Product: " << (i.waitingNextProduct ? "true" : "false")
        << " | TotalBudget: " << i.totalBudget
        << " | Utility: " << i.scaledUtility
        << " | Purschased: " << size(i.purschasedProducts)
        << " | ReservePrice for Current Product: ";
        for (const auto &reserve : i.reservePrices) {        // Buscar el precio de reserva del producto actual
            if (reserve.id == i.currentProductID) {  // Si es el producto actual
                os << "[ ID Product N°" << reserve.id << " : " << reserve.price << " ] ";
                break; // Rompemos el bucle para solo mostrar uno
            }
        }
        for (const auto &alpha : i.alphas){
            if (alpha.id == i.currentProductID){
                os << "[ Alpha N°" << alpha.id << " : " << alpha.alpha << " ] ";
                break;
            }
        }
        return os;
    }
};
#endif