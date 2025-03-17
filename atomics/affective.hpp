#ifndef _AFFECTIVE_HPP__
#define _AFFECTIVE_HPP__

#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/message_bag.hpp>

#include <limits>
#include <assert.h>
#include <string>
#include <random>
#include "../data_structures/message.hpp"

using namespace cadmium;
using namespace std;

// Generar alphas aleatorios
vector<float> generateRandomAlphasAffective()
{
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<float> dis(0.1, 1.0);
    vector<float> alpha(4);

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

// Determinar si la oferta es aceptable
bool getDecisionAffective(float bestPrice, float reservePrice) {
    return reservePrice >= bestPrice;
}

// Actualizar frustración del agente
float updateFrustration(float winner, int clientID, float frustration) {
    return (winner == clientID) ? max(0.0f, frustration - 1) : frustration + 1;
}

float updateTotalBudgetAffective(float salePrice, float totalBudget)
{
    return totalBudget - salePrice;
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
        bool modelActive, decision;
        vector<float> alphas, reservePrices;
        vector<Message_initialIP_t> purschasedProducts;
    };
    state_type state;
    // constructor del modelo
    Affective()
    {
        state.idAgent = 1;
        state.totalBudget = 5000;
        state.moneySpent = 0;
        state.utility = 0;
        state.anxiety = 0;
        state.frustration = 0;
        state.modelActive = false;
        state.decision = false;
        state.alphas = generateRandomAlphasAffective(); // Inicialización del vector alpha en el constructor
        state.reservePrices = updateReservePrice(state.alphas, state.totalBudget, state.anxiety, state.frustration, 0);
    }
    // funcion de transición interna
    void internal_transition()
    {
        state.modelActive = false;
    }

    // función de transición externa
    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs)
    {

        if (!get_messages<typename Affective_defs::in_finalResult>(mbs).empty()) // Verificar mensajes en el puerto 'finalResult'
        {
            auto finalResultMessages = get_messages<typename Affective_defs::in_finalResult>(mbs);
            auto finalResult = finalResultMessages[0];
            state.frustration = updateFrustration(finalResult.winnerID, state.idAgent, state.frustration);
            if (finalResult.winnerID == state.idAgent)
            {
                state.totalBudget = updateTotalBudgetAffective(finalResult.bestPrice, state.totalBudget);
            }
            state.modelActive = false;
        }

        else if (!get_messages<typename Affective_defs::in_roundResult>(mbs).empty()) // Verificar si hay mensajes en el puerto 'in_roundResult'
        {
            state.modelActive = true;
            auto roundResultMessages = get_messages<typename Affective_defs::in_roundResult>(mbs);
            auto roundResult = roundResultMessages[0];
            state.currentBestPrice = roundResult.bestPrice;
            state.decision = getDecisionAffective(state.currentBestPrice, state.reservePrices[state.currentProductID]);
            state.anxiety = state.anxiety + 1;
        }

        if (!get_messages<typename Affective_defs::in_initialIP>(mbs).empty()) // Verificar si hay mensajes en el puerto 'in_initialIP'
        {
            state.modelActive = true;
            auto initialProductInfoMessages = get_messages<typename Affective_defs::in_initialIP>(mbs);
            auto productInfo = initialProductInfoMessages[0];
            state.currentProductID = productInfo.productID - 1;
            state.currentBestPrice = productInfo.bestPrice;
            state.reservePrices = updateReservePrice(state.alphas, state.totalBudget, state.anxiety, state.frustration, state.currentProductID);
            state.decision = getDecisionAffective(state.currentBestPrice, state.reservePrices[state.currentProductID]);
            state.anxiety = 0;
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
           << " | ReservePrices: ";
        for (size_t idx = 0; idx < i.reservePrices.size(); ++idx)
        {
            os << "ID Product N°" << idx+1 << " : " << i.reservePrices[idx] << " - "; // Muestra el índice y el precio de reserva
        }
        return os;
    }
};
#endif