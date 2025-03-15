#ifndef _RATIONAL_HPP__
#define _RATIONAL_HPP__

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
vector<float> generateRandomAlphasRational()
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
vector<float> generateReservePrices(const vector<float>& level, float totalbudget)
{
    float sumLevel = accumulate(level.begin(), level.end(), 0.0f);
    vector<float> prices(level.size());

    for (size_t i = 0; i < level.size(); i++)
    {
        prices[i] = (level[i] / sumLevel) * totalbudget;
    }

    return prices;
}
// Determinar si la oferta es aceptable
bool getDecisionRational(float bestPrice, float reservePrice) {
    return reservePrice >= bestPrice;
}

float updateTotalBudgetRational(float salePrice, float totalBudget)
{
    return totalBudget - salePrice;
}

// Port definition

struct Rational_defs
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
class Rational
{
public:
    // ports definition
    using input_ports = tuple<typename Rational_defs::in_initialIP, typename Rational_defs::in_roundResult, typename Rational_defs::in_finalResult>;
    using output_ports = tuple<typename Rational_defs::out_bidOffer>;

    // state definition
    struct state_type
    {
        int idAgent, currentProductID;
        float totalBudget, moneySpent, currentBestPrice;
        float utility;
        bool modelActive, decision;
        vector<float> alphas, reservePrices;
    };
    state_type state;
    // constructor del modelo
    Rational()
    {
        state.idAgent = 2;
        state.totalBudget = 5000;
        state.moneySpent = 0;
        state.utility = 0;
        state.modelActive = false;
        state.decision = false;
        state.alphas = generateRandomAlphasRational(); // Inicialización del vector alpha en el constructor
        state.reservePrices = generateReservePrices(state.alphas, state.totalBudget);
    }
    // funcion de transición interna
    void internal_transition()
    {
        state.modelActive = false;
    }

    // función de transición externa
    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs)
    {

        if (!get_messages<typename Rational_defs::in_finalResult>(mbs).empty()) // Verificar mensajes en el puerto 'finalResult'
        {
            auto finalResultMessages = get_messages<typename Rational_defs::in_finalResult>(mbs);
            auto finalResult = finalResultMessages[0];
            if (finalResult.winnerID == state.idAgent)
            {
                state.totalBudget = updateTotalBudgetRational(finalResult.bestPrice, state.totalBudget);
            }
            else
            {
                state.totalBudget = state.totalBudget;
            }
            state.modelActive = false;
        }

        else if (!get_messages<typename Rational_defs::in_roundResult>(mbs).empty()) // Verificar si hay mensajes en el puerto 'in_roundResult'
        {
            state.modelActive = true;
            auto roundResultMessages = get_messages<typename Rational_defs::in_roundResult>(mbs);
            auto roundResult = roundResultMessages[0];
            state.currentBestPrice = roundResult.bestPrice;
            state.decision = getDecisionRational(state.currentBestPrice, state.reservePrices[state.currentProductID]);
        }

        if (!get_messages<typename Rational_defs::in_initialIP>(mbs).empty()) // Verificar si hay mensajes en el puerto 'in_initialIP'
        {
            state.modelActive = true;
            auto initialProductInfoMessages = get_messages<typename Rational_defs::in_initialIP>(mbs);
            auto productInfo = initialProductInfoMessages[0];
            state.currentProductID = productInfo.productID - 1;
            state.currentBestPrice = productInfo.bestPrice;
            state.decision = getDecisionRational(state.currentBestPrice, state.reservePrices[state.currentProductID]);
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
            get_messages<typename Rational_defs::out_bidOffer>(bags) = bag_port_out;
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

    friend ostringstream &operator<<(ostringstream &os, const typename Rational<TIME>::state_type &i)
    {
        os << " IdAgent: " << i.idAgent
           << " | TotalBudget: " << i.totalBudget
           << " | MoneySpent: " << i.moneySpent
           << " | Utility: " << i.utility
           << " | Decision: " << i.decision
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