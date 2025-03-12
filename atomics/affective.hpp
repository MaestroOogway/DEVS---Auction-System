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
vector<float> generateRandomAlphas()
{
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<float> dis(0.1, 1.0);

    vector<float> alpha(4);
    for (float &a : alpha)
        a = dis(gen);

    // Normalizar en una sola línea
    float sum = accumulate(alpha.begin(), alpha.end(), 0.0f);
    for (float &a : alpha)
        a /= sum;

    return alpha;
}

// Calcular precios de reserva

vector<float> updateReservePrice(const vector<float> &alphas, float totalBudget, float anxiety, float frustration, int currentProduct)
{
    vector<float> reservePrices(alphas.size(), 0.0f); // Vector para almacenar precios de reserva
    vector<float> adjustedAlphas = alphas;            // Copia de los α originales
    float adjustedSum = 0.0f;

    // Aplicar el ajuste al producto actual
    adjustedAlphas[currentProduct] *= (1 + anxiety + frustration);

    // Calcular la suma de α solo desde el producto actual hasta el último
    for (int i = currentProduct; i < alphas.size(); i++)
    {
        adjustedSum += adjustedAlphas[i];
    }

    // Evitar división por cero y calcular precios de reserva SOLO para productos desde el actual en adelante
    if (adjustedSum > 0)
    {
        for (int i = currentProduct; i < alphas.size(); i++)
        {
            reservePrices[i] = (adjustedAlphas[i] / adjustedSum) * totalBudget;
        }
    }
    return reservePrices;
}

bool getDecision(float bestPrice, float reservePrice)
{
    if (reservePrice >= bestPrice)
    {
        return true;
    }
    else
    {
        return false;
    }
}

float updateFrustration(float winner, int clientID, float frustration)
{
    if (winner == clientID)
    {
        if (frustration > 0)
        {
            return frustration - 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return frustration + 1;
    }
}

float updateTotalBudget(float salePrice, float totalBudget)
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
        int idAgent;
        int purchasedProducts;
        float totalBudget;
        float moneySpent;
        float utility;
        float anxiety;
        float frustration;
        bool modelActive;
        bool decision;
        int currentProductID;
        float currentBestPrice;
        vector<float> alphas;
        vector<float> reservePrices;
    };
    state_type state;
    // constructor del modelo
    Affective()
    {
        state.idAgent = 1;
        state.purchasedProducts = 0;
        state.totalBudget = 100000;
        state.moneySpent = 0;
        state.utility = 0;
        state.anxiety = 0;
        state.frustration = 0;
        state.modelActive = false;
        state.decision = false;
        state.alphas = generateRandomAlphas(); // Inicialización del vector alpha en el constructor
        state.reservePrices.clear();
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
            state.modelActive = true;
            auto finalResultMessages = get_messages<typename Affective_defs::in_finalResult>(mbs);
            auto finalResult = finalResultMessages[0];
            state.frustration = updateFrustration(finalResult.winnerID, state.idAgent, state.frustration);
            if (finalResult.winnerID == state.idAgent)
            {
                state.totalBudget = updateTotalBudget(finalResult.bestPrice, state.totalBudget);
            }
            else
            {
                state.totalBudget = state.totalBudget;
            }
        }

        else if (!get_messages<typename Affective_defs::in_roundResult>(mbs).empty()) // Verificar si hay mensajes en el puerto 'in_roundResult'
        {
            state.modelActive = true;
            state.anxiety = state.anxiety + 1;
            auto roundResultMessages = get_messages<typename Affective_defs::in_roundResult>(mbs);
            auto roundResult = roundResultMessages[0];
            state.currentBestPrice = roundResult.bestPrice;
            state.decision = getDecision(state.currentBestPrice, state.reservePrices[state.currentProductID - 1]);
        }

        if (!get_messages<typename Affective_defs::in_initialIP>(mbs).empty()) // Verificar si hay mensajes en el puerto 'in_initialIP'
        {
            state.modelActive = true;
            auto initialProductInfoMessages = get_messages<typename Affective_defs::in_initialIP>(mbs);
            auto productInfo = initialProductInfoMessages[0];
            state.currentProductID = productInfo.productID;
            state.currentBestPrice = productInfo.bestPrice;
            state.reservePrices = updateReservePrice(state.alphas, state.totalBudget, state.anxiety, state.frustration, state.currentProductID - 1);
            state.decision = getDecision(state.currentBestPrice, state.reservePrices[state.currentProductID - 1]);
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
            outgoingMessage.productID = state.currentProductID;
            outgoingMessage.priceProposal = state.currentBestPrice;
            bag_port_out.push_back(outgoingMessage);
            get_messages<typename Affective_defs::out_bidOffer>(bags) = bag_port_out;
        }
        return bags;
    }

    TIME time_advance() const
    {
        if (state.modelActive)
        {
            return TIME("00:00:01:000"); // Tiempo definido para la próxima transición interna (ajustar según la lógica)
        }
        else
        {
            return numeric_limits<TIME>::infinity(); // No hay transición pendiente
        }
    }

    friend ostringstream &operator<<(ostringstream &os, const typename Affective<TIME>::state_type &i)
    {
        os << " IdAgent: " << i.idAgent
           << " | PurchasedProducts: " << i.purchasedProducts
           << " | TotalBudget: " << i.totalBudget
           << " | MoneySpent: " << i.moneySpent
           << " | Utility: " << i.utility
           << " | Decision" << i.decision
           << " | Anxiety: " << i.anxiety
           << " | Frustration: " << i.frustration
           << " | ModelActive: " << (i.modelActive ? "true" : "false")
           << " | ReservePrices: ";
        for (size_t idx = 0; idx < i.reservePrices.size(); ++idx)
        {
            os << "ID Product:°[" << idx << "] " << i.reservePrices[idx] << " "; // Muestra el índice y el precio de reserva
        }
        return os;
    }
};
#endif