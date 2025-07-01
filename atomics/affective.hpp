#ifndef _AFFECTIVE_HPP__
#define _AFFECTIVE_HPP__

#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/message_bag.hpp>
#include "functions.hpp" // Incluir funciones compartidas
#include <limits>
#include <assert.h>
#include <string>
#include <random>
#include <unordered_map>
#include <iomanip> // necesario para std::fixed y std::setprecision
#include "../data_structures/message.hpp"

using namespace cadmium; using namespace std;

std::set<int> subastadosA; //Memoria del cliente afectivo

void updateUtilityA(Utility& utility, int subastadoID, vector<Alphas>& alphas, const vector<int>& productIDs, int win, Emotion& ansiedad, Emotion& frustracion) {
    vector<int> remainingProductIDs;
    vector<float> remainingAlphas;
    // Filtrar productos no subastados aún
    for (int id : productIDs) {
        if (subastadosA.find(id) == subastadosA.end()) {
            remainingProductIDs.push_back(id);
        }
    }
    // Obtener los alphas originales de productos restantes
    for (int id : remainingProductIDs) {
        for (const auto& alpha : alphas) {
            if (alpha.id == id) {
                remainingAlphas.push_back(alpha.alpha);
                break;
            }
        }
    }
    // Buscar el alpha del producto que se está subastando
    auto it = find_if(alphas.begin(), alphas.end(), [subastadoID](const Alphas& a) {
        return a.id == subastadoID;
    });

    if (it != alphas.end()) {
        float alphaOriginal = it->alpha;
        float alphaAdjusted = alphaOriginal * (1.0f + ansiedad.scaled + frustracion.scaled);
        float resultado;
        float pesoA = 0.3f;
        float pesoF = 0.3f;
        float penalizacion = pesoA * ansiedad.scaled + pesoF * frustracion.scaled;
        if (win == 1) {
        resultado = log(pow(1.0f + 1.0f, alphaAdjusted));  // log((1 + 1)^alpha)
        } else {
        resultado = log(pow(1.0f + 0.0f, alphaAdjusted)) - penalizacion; //Equivalente a: resultado = 0 - penalizacion;
        }
        utility.real = max(0.0f, utility.real + resultado);
    }
}

void updateReservePA(vector<ReservePrice>& reservePrices, const vector<Alphas>& alphas, const vector<int>& productIDs, float totalBudget, int subastadoID, Emotion& ansiedad, Emotion& frustracion) {
    vector<int> remainingProductIDs;  // Calcular la nueva suma total de alphas (ajustando solo el del producto subastado)
    unordered_map<int, float> alphaAdjustedMap;
    float totalAlpha = 0.0f;
    for (int id : productIDs) {
        if (subastadosA.find(id) == subastadosA.end()) {   // Filtrar productos restantes (excluyendo TODOS los productos ya subastados)
            remainingProductIDs.push_back(id);
        }
    }

    for (int id : remainingProductIDs) {
        auto it = find_if(alphas.begin(), alphas.end(), [id](const Alphas& a) {
            return a.id == id;
        });

        if (it != alphas.end()) {
            float adjustedAlpha = (id == subastadoID) ?
                it->alpha * (1 + ansiedad.scaled + frustracion.scaled) :
                it->alpha;

            alphaAdjustedMap[id] = adjustedAlpha;
            totalAlpha += adjustedAlpha;
        }
    }
    // Actualizar precios de reserva
    for (auto& rp : reservePrices) {
        if (subastadosA.find(rp.id) != subastadosA.end()) {
            rp.price = 0.0f;
        } else {
            rp.price = roundToSignificantFigures((alphaAdjustedMap[rp.id] / totalAlpha) * totalBudget,3);
        }
    }
}
// Port definition
struct Affective_defs {
    struct in_initialIP : public in_port<Message_initialIP_t> {};
    struct in_roundResult : public in_port<Message_roundResult_t> {};
    struct in_finalResult : public in_port<Message_finalResults_t> {};
    struct out_bidOffer : public out_port<Message_bidOffer_t> {};
};

template <typename TIME>
class Affective {
public:
    using input_ports = tuple<typename Affective_defs::in_initialIP, typename Affective_defs::in_roundResult, typename Affective_defs::in_finalResult>;
    using output_ports = tuple<typename Affective_defs::out_bidOffer>;
    struct state_type {
        int idAgent, currentProductID;
        float totalBudget, moneySpent, currentBestPrice;
        bool modelActive, decision, waitingNextProduct;
        Emotion anxiety, frustration;
        Utility utility;
        vector<Alphas> alphas;
        vector<ReservePrice> reservePrices;
        vector<int> purschasedProducts;
    };
    state_type state;
    Affective() = default;
    Affective(int id, float budget) noexcept{
        state.idAgent = id;
        state.totalBudget = budget;
        state.moneySpent = 0;
        state.anxiety.real = state.anxiety.scaled = 0;
        state.frustration.real = state.frustration.scaled = 0;
        state.modelActive = false;
        state.decision = false;
        state.waitingNextProduct = false;
        state.purschasedProducts.clear();
        state.alphas = generateRandomAlphas(); // Inicialización del vector alpha en el constructor
        state.utility.real = generateUtility(state.alphas, getRandomProducts());
        state.utility.scaled = normalize(state.utility.real, 1.0f, maxUtility(state.alphas, getRandomProducts()));
        state.reservePrices = generateReservePrices(state.alphas, getRandomProducts(), state.totalBudget);
    }
    // funcion de transición interna
    void internal_transition(){
        if (state.waitingNextProduct){
            subastadosA.insert(state.currentProductID);
            state.waitingNextProduct = false;
        }
        state.modelActive = false;
    }
    // función de transición externa
    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs){
        state.modelActive = true;
        if (!get_messages<typename Affective_defs::in_initialIP>(mbs).empty()){
            auto productInfo = get_messages<typename Affective_defs::in_initialIP>(mbs)[0];
            state.currentProductID = productInfo.productID;
            state.currentBestPrice = productInfo.initialPrice;
            updateReservePA(state.reservePrices, state.alphas, getRandomProducts(), state.totalBudget, state.currentProductID, state.anxiety, state.frustration);
            state.decision = getDecision(state.currentBestPrice, state.reservePrices, state.currentProductID);
            resetAnxiety(state.anxiety);
        }
        else if (!get_messages<typename Affective_defs::in_roundResult>(mbs).empty()){
            auto roundResult = get_messages<typename Affective_defs::in_roundResult>(mbs)[0];
            state.currentBestPrice = roundResult.bestPrice;
            state.decision = getDecision(state.currentBestPrice, state.reservePrices, state.currentProductID);
        }
        else if (!get_messages<typename Affective_defs::in_finalResult>(mbs).empty()){
            auto finalResult = get_messages<typename Affective_defs::in_finalResult>(mbs)[0];
            if (finalResult.winnerID == state.idAgent){
                state.totalBudget = updateTotalBudget(finalResult.bestPrice, state.totalBudget);
                state.moneySpent = updateMoneySpent(finalResult.bestPrice, state.moneySpent);
                state.purschasedProducts.push_back(1);
            }
            else{
                state.purschasedProducts.push_back(0);
            }
            updateUtilityA(state.utility, state.currentProductID, state.alphas, getRandomProducts(), (finalResult.winnerID == state.idAgent), state.anxiety, state.frustration);
            state.utility.scaled = roundToSignificantFigures(min(1.0f, max(0.00f, normalize(state.utility.real, 1.0f, maxUtility(state.alphas, getRandomProducts() )))), 2);
            updateFrustration(finalResult.winnerID, state.idAgent, state.frustration);
            state.waitingNextProduct = waitingNextProduct();
            state.decision = false;
        }
        if (state.decision){
            updateAnxiety(state.anxiety);
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
        get_messages<typename Affective_defs::out_bidOffer>(bags) = bag_port_out;
        return bags;
    }

    TIME time_advance() const {
        return state.modelActive ? TIME("00:00:05:000") : numeric_limits<TIME>::infinity();
    }

    friend ostringstream &operator<<(ostringstream &os, const typename Affective<TIME>::state_type &i) {
        os << " IdAgent: " << i.idAgent
        << " | Wait Next Product: " << (i.waitingNextProduct ? "true" : "false")
        << " | TotalBudget: " << i.totalBudget
        << " | Utility: " << i.utility.scaled
        << " | N: " << i.purschasedProducts.size()
        << " | Anxiety: " << i.anxiety.scaled
        << " | Frustration: " << i.frustration.scaled
        << " | Purchased: [";
        for (size_t idx = 0; idx < i.purschasedProducts.size(); ++idx) {
            os << i.purschasedProducts[idx];
            if (idx != i.purschasedProducts.size() - 1)
                os << ", ";
            }
        os << "]";
        os << " | ReservePrice for Current Product: ";
        for (const auto &reserve : i.reservePrices) { // Buscar el precio de reserva del producto actual
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