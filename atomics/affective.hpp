#ifndef _AFFECTIVE_HPP__
#define _AFFECTIVE_HPP__

#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/message_bag.hpp>

#include "functions.hpp" // Incluir funciones compartidas
#include <limits>
#include <assert.h>
#include <string>
#include <random>
#include "../data_structures/message.hpp"

using namespace cadmium;
using namespace std;

std::set<int> subastados;

std::vector<Alphas> generateRandomAlphasA() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0, 1.0);
    std::vector<Alphas> alphas(100); // Crear un vector de 100 elementos
    for (int i = 0; i < 100; ++i) {
        alphas[i].id = i + 1; // ID del 1 al 100
        alphas[i].alpha = dis(gen); // Número aleatorio entre 0 y 1
    }
    return alphas; // Retornar el vector
}

std::vector<ReservePrice> generateReservePricesA(const std::vector<Alphas>& alphas, const std::vector<int>& ids, float totalBudget) {
    std::vector<ReservePrice> reservePrices;
    std::vector<float> selectedAlphas;
    // Extraer los alphas correspondientes a los productos seleccionados
    for (int id : ids) {
        for (const auto& alpha : alphas) {
            if (alpha.id == id) {
                selectedAlphas.push_back(alpha.alpha);
            break;
            }
        }
    }
    // Calcular la suma total de los alphas seleccionados
    float sumAlphas = std::accumulate(selectedAlphas.begin(), selectedAlphas.end(), 0.0f);
    // Calcular los precios de reserva
    for (size_t i = 0; i < ids.size(); ++i) {
        ReservePrice rp;
        rp.id = ids[i];
        rp.price = (selectedAlphas[i] / sumAlphas) * totalBudget; // Fórmula de precios de reserva
        reservePrices.push_back(rp);
    }
    return reservePrices;
}

void updateReservePrices(std::vector<ReservePrice>& reservePrices, std::vector<Alphas>& alphas,
                         const std::vector<int>& productIDs, float totalBudget,
                         int subastadoID, float ansiedad, float frustracion) {
    // Encuentra el alpha del producto subastado y ajústalo
    for (auto& alpha : alphas) {
        if (alpha.id == subastadoID) {
            alpha.alpha += ansiedad + frustracion;
            break;
        }
    }
    // Filtrar productos restantes (excluyendo TODOS los productos ya subastados)
    std::vector<int> remainingProductIDs;
    for (int id : productIDs) {
        if (subastados.find(id) == subastados.end()) { // Si el producto NO ha sido subastado
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
        if (subastados.find(rp.id) != subastados.end()) {
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
        float utility, anxiety, frustration;
        bool modelActive, decision, waitingNextProduct;
        vector<Alphas> alphas;
        vector<ReservePrice> reservePrices;
        vector<Message_finalResults_t> purschasedProducts;
    };
    state_type state;

    Affective() = default;
    Affective(int id, float budget) noexcept{
        state.idAgent = id;
        state.totalBudget = budget;
        state.moneySpent = 0;
        state.utility = 0;
        state.anxiety = 0;
        state.frustration = 0;
        state.modelActive = false;
        state.decision = false;
        state.waitingNextProduct = false;
        state.purschasedProducts.clear();
        state.alphas = generateRandomAlphasA(); // Inicialización del vector alpha en el constructor
        state.reservePrices = generateReservePricesA(state.alphas, getRandomProducts(), state.totalBudget);
    }
    // funcion de transición interna
    void internal_transition(){
        if (state.waitingNextProduct){
            subastados.insert(state.currentProductID);
            state.waitingNextProduct = false;
        }
        state.modelActive = false;
    }

    // función de transición externa
    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs){
        if (!get_messages<typename Affective_defs::in_finalResult>(mbs).empty()){ // Verificar mensajes en el puerto 'finalResult'
            state.modelActive = true;
            auto finalResultMessages = get_messages<typename Affective_defs::in_finalResult>(mbs);
            auto finalResult = finalResultMessages[0];
            state.frustration = updateFrustration(finalResult.winnerID, state.idAgent, state.frustration);
            if (finalResult.winnerID == state.idAgent)
            {
                state.totalBudget = updateTotalBudget(finalResult.bestPrice, state.totalBudget);
                state.moneySpent = updateMoneySpent(finalResult.bestPrice, state.moneySpent);
                state.purschasedProducts.push_back(finalResult);
            }
            state.waitingNextProduct = waitingNextProduct();
            state.decision = false;
        }

        else if (!get_messages<typename Affective_defs::in_roundResult>(mbs).empty()) // Verificar si hay mensajes en el puerto 'in_roundResult'
        {
            state.modelActive = true;
            auto roundResultMessages = get_messages<typename Affective_defs::in_roundResult>(mbs);
            auto roundResult = roundResultMessages[0];
            state.currentBestPrice = roundResult.bestPrice;
            state.decision = getDecision(state.currentBestPrice, state.reservePrices, state.currentProductID);
            if (state.decision)
            {
                state.anxiety = updateAnxiety(state.anxiety, 1);
            }
        }

        else if (!get_messages<typename Affective_defs::in_initialIP>(mbs).empty()) // Verificar si hay mensajes en el puerto 'in_initialIP'
        {
            state.modelActive = true;
            auto initialProductInfoMessages = get_messages<typename Affective_defs::in_initialIP>(mbs);
            auto productInfo = initialProductInfoMessages[0];
            state.currentProductID = productInfo.productID;
            state.currentBestPrice = productInfo.bestPrice;
            updateReservePrices(state.reservePrices, state.alphas, getRandomProducts(), state.totalBudget, state.currentProductID, state.anxiety, state.frustration);
            state.decision = getDecision(state.currentBestPrice, state.reservePrices, state.currentProductID);
            state.anxiety = updateAnxiety(state.anxiety, 0);
            if (state.decision)
            {
                state.anxiety = updateAnxiety(state.anxiety, 1);
            }
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
            get_messages<typename Affective_defs::out_bidOffer>(bags) = bag_port_out;
        }
        else if (state.decision == false)
        {
            outgoingMessage.clientID = state.idAgent;
            outgoingMessage.productID = state.waitingNextProduct ? 0 : state.currentProductID;
            outgoingMessage.priceProposal = 0;
            outgoingMessage.decision = state.waitingNextProduct ? 1 : state.decision;
            bag_port_out.push_back(outgoingMessage);
            get_messages<typename Affective_defs::out_bidOffer>(bags) = bag_port_out;
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

    friend ostringstream &operator<<(ostringstream &os, const typename Affective<TIME>::state_type &i) {
        os << " IdAgent: " << i.idAgent
           << " | TotalBudget: " << i.totalBudget
           << " | MoneySpent: " << i.moneySpent
           << " | Utility: " << i.utility
           << " | Decision: " << i.decision
           << " | Anxiety: " << i.anxiety
           << " | Frustration: " << i.frustration
           << " | ModelActive: " << (i.modelActive ? "true" : "false")
           << " | Wait Next Product: " << (i.waitingNextProduct ? "true" : "false")
           << " | ReservePrices: ";
           for (size_t idx = 0; idx < i.reservePrices.size(); ++idx)
           os << "ID Product N°" << i.reservePrices[idx].id << " : " << i.reservePrices[idx].price << " - ";
        os << " | Purchased Products: " << (i.purschasedProducts.empty() ? "None" : "");
        for (const auto &product : i.purschasedProducts) os << "[Product ID: " << product.productID << "] ";
        return os;
    }
};
#endif