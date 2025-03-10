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

// Funciones de decisión
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

float reservePrice(std::vector<float>& level, float totalbudget)
{
    float sumLevel = std::accumulate(vec.begin(), vec.end(), 0);
    float reservePrice = (state.alpha / sumLevel) * state.totalBudget;
    return reservePrice;
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
        float reservePrice;
        bool modelActive;
        vector<float> alpha;
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
        state.reservePrice = 0;
        state.modelActive = false;
        state.alpha = generateRandomAlphas();            // Inicialización del vector alpha en el constructor
    }
    // funcion de transición interna
    void internal_transition()
    {
        state.modelActive = false;
    }

    // función de transición externa
    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs)
    {
        // Verificar si hay mensajes en el puerto 'in_initialIP'
        if (!get_messages<typename Affective_defs::in_initialIP>(mbs).empty())
        {
            state.modelActive = true;
            auto messages = get_messages<typename Affective_defs::in_initialIP>(mbs);
            auto productInfo = messages[0];
        }

        if (!get_messages<typename Affective_defs::in_finalResult>(mbs).empty()) // Verificar mensajes en el puerto 'finalResult'
        {
            state.modelActive = true;
            auto finalResultMessages = get_messages<typename Affective_defs::in_finalResult>(mbs);
            auto finalResult = finalResultMessages[0];
        }

        else if (!get_messages<typename Affective_defs::in_roundResult>(mbs).empty()) // Verificar si hay mensajes en el puerto 'in_roundResult'
        {
            state.modelActive = true;
            auto roundResultMessages = get_messages<typename Affective_defs::in_roundResult>(mbs);
            auto roundResult = roundResultMessages[0];
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
        outgoingMessage.clientID = state.idAgent;
        outgoingMessage.productID = state.bidOffer.productID;
        outgoingMessage.priceProposal = state.bidOffer.priceProposal;
        bag_port_out.push_back(outgoingMessage);
        get_messages<typename Affective_defs::out_bidOffer>(bags) = bag_port_out;

        return bags;
    }

    TIME time_advance() const
    {
        if (state.modelActive)
        {
            // Tiempo definido para la próxima transición interna (ajustar según la lógica)
            return TIME("00:00:01:000");
        }
        else
        {
            // No hay transición pendiente
            return numeric_limits<TIME>::infinity();
        }
    }

    friend ostringstream &operator<<(ostringstream &os, const typename Affective<TIME>::state_type &i)
    {
        os << " IdAgent: " << i.idAgent
           << " | PurchasedProducts: " << i.purchasedProducts
           << " | TotalBudget: " << i.totalBudget
           << " | MoneySpent: " << i.moneySpent
           << " | Utility: " << i.utility
           << " | Anxiety: " << i.anxiety
           << " | Frustration: " << i.frustration
           << " | ReservePrice: " << i.reservePrice
           << " | ModelActive: " << (i.modelActive ? "true" : "false");
        return os;
    }
};
#endif