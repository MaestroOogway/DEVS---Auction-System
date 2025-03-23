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

// Generar alphas aleatorios
vector<float> generateRandomAlphasA()
{
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<float> dis(0.0, 1.0);
    vector<float> alpha(10);

    for (float &a : alpha)
        a = dis(gen);

    float sum = accumulate(alpha.begin(), alpha.end(), 0.0f);       // Normalizar en una sola línea
    for (float &a : alpha)
        a /= sum;

    return alpha;
}
// Calcular precios de reserva
vector<float> updateReservePrice(const vector<float> &alphas, float totalBudget, float anxiety, float frustration, int currentProduct) {
    vector<float> reservePrices(alphas.size(), 0.0f);
    vector<float> adjustedAlphas = alphas;
    adjustedAlphas[currentProduct] *= (1 + anxiety + frustration);

    float adjustedSum = accumulate(adjustedAlphas.begin() + currentProduct, adjustedAlphas.end(), 0.0f);
    if (adjustedSum > 0)
        for (int i = currentProduct; i < alphas.size(); ++i)
            reservePrices[i] = (adjustedAlphas[i] / adjustedSum) * totalBudget;

    return reservePrices;
}
// Actualizar frustración del agente
float updateFrustration(float winner, int clientID, float frustration) {
    return (winner == clientID) ? max(0.0f, frustration - 1) : frustration + 1;
}

float updateAnxiety(float anxiety, float sum){
    if(sum>0){
        anxiety = anxiety + 1;
        return anxiety;
    }
    else{
        anxiety = 0;
        return anxiety;
    }
}
// Port definition
struct Affective_defs
{
    struct in_initialIP : public in_port<Message_initialIP_t>
    {
    };
    struct in_roundResult : public in_port<Message_roundResult_t>
    {
    };
    struct in_finalResult : public in_port<Message_finalResults_t>
    {
    };
    struct out_bidOffer : public out_port<Message_bidOffer_t>
    {
    };
};

template <typename TIME>
class Affective
{
public:
    // ports definition
    using input_ports = tuple<typename Affective_defs::in_initialIP, typename Affective_defs::in_roundResult, typename Affective_defs::in_finalResult>;
    using output_ports = tuple<typename Affective_defs::out_bidOffer>;

    // state definition
    struct state_type
    {
        int idAgent, currentProductID;
        float totalBudget, moneySpent, currentBestPrice;
        float utility, anxiety, frustration;
        bool modelActive, decision, waitingNextProduct;
        vector<float> alphas, reservePrices;
        vector<Message_initialIP_t> purschasedProducts;
    };
    state_type state;
    // constructor del modelo
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
        state.alphas = generateRandomAlphasA(); // Inicialización del vector alpha en el constructor
        state.reservePrices = updateReservePrice(state.alphas, state.totalBudget, state.anxiety, state.frustration, 0);
    }
    // funcion de transición interna
    void internal_transition()
    {
        if(state.waitingNextProduct){
            state.waitingNextProduct = false;
        }
        state.modelActive = false;
    }

    // función de transición externa
    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs)
    {

        if (!get_messages<typename Affective_defs::in_finalResult>(mbs).empty()) // Verificar mensajes en el puerto 'finalResult'
        {
            state.modelActive = true;
            auto finalResultMessages = get_messages<typename Affective_defs::in_finalResult>(mbs);
            auto finalResult = finalResultMessages[0];
            state.frustration = updateFrustration(finalResult.winnerID, state.idAgent, state.frustration);
            if (finalResult.winnerID == state.idAgent)
            {
                state.totalBudget = updateTotalBudget(finalResult.bestPrice, state.totalBudget);
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
            state.decision = getDecision(state.currentBestPrice, state.reservePrices[state.currentProductID]);
            if (state.decision){
                state.anxiety = updateAnxiety(state.anxiety, 1);
            }
        }

        else if (!get_messages<typename Affective_defs::in_initialIP>(mbs).empty()) // Verificar si hay mensajes en el puerto 'in_initialIP'
        {
            state.modelActive = true;
            auto initialProductInfoMessages = get_messages<typename Affective_defs::in_initialIP>(mbs);
            auto productInfo = initialProductInfoMessages[0];
            state.currentProductID = productInfo.productID - 1;
            state.currentBestPrice = productInfo.bestPrice;
            state.reservePrices = updateReservePrice(state.alphas, state.totalBudget, state.anxiety, state.frustration, state.currentProductID);
            state.decision = getDecision(state.currentBestPrice, state.reservePrices[state.currentProductID]);
            state.anxiety = updateAnxiety(state.anxiety, 0);
            if (state.decision){
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
            outgoingMessage.productID = state.currentProductID + 1;
            outgoingMessage.priceProposal = state.currentBestPrice;
            outgoingMessage.decision = state.decision;
            bag_port_out.push_back(outgoingMessage);
            get_messages<typename Affective_defs::out_bidOffer>(bags) = bag_port_out;
        }
        else if (state.waitingNextProduct){
            outgoingMessage.clientID = state.idAgent;
            outgoingMessage.productID = 0;
            outgoingMessage.priceProposal = 0;
            outgoingMessage.decision = 1;
            bag_port_out.push_back(outgoingMessage);
            get_messages<typename Affective_defs::out_bidOffer>(bags) = bag_port_out;
        }
        else if(state.decision == false){
            outgoingMessage.clientID = state.idAgent;
            outgoingMessage.productID = state.currentProductID + 1;
            outgoingMessage.priceProposal = state.reservePrices[state.currentProductID];
            outgoingMessage.decision = state.decision;
            bag_port_out.push_back(outgoingMessage);
            get_messages<typename Affective_defs::out_bidOffer>(bags) = bag_port_out;
        }
        return bags;
    }

    // Tiempo de avance
    TIME time_advance() const {
        if(state.modelActive == false){
            return std::numeric_limits<TIME>::infinity();
        }
        return state.modelActive ? TIME("00:00:05:000") : numeric_limits<TIME>::infinity();
    }

    friend ostringstream &operator<<(ostringstream &os, const typename Affective<TIME>::state_type &i)
    {
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
        {
            os << "ID Product N°" << idx+1 << " : " << i.reservePrices[idx] << " - "; // Muestra el índice y el precio de reserva
        }
        return os;
    }
};
#endif