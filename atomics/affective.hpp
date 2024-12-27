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

/***** (0) *****/
// Funciones de decision

bool makeDecision(Message_roundResult_t roundResult, int agentID)
{
    if (roundResult.winnerID == agentID)
    {
        return true;
    }
    else
    {
        return false;
    }
}

Message_bidOffer_t getPriceProposal(int productID, float bestPrice, float ranking, float totalBudget)
{
    Message_bidOffer_t bid;
    bid.productID = productID;
    if (totalBudget >= bestPrice)
    {
        // Calcular la oferta considerando ranking como peso
        bid.priceProposal = bestPrice + ranking * 20;
    }
    else
    {
        bid.priceProposal = 0; // No puja
    }
    return bid;
}

/***** (1) *****/
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
        Message_initialIP_t informationProduct;
        Message_finalResults_t finalResult;
        Message_bidOffer_t bidOffer;
        Message_bidOffer_t lastBid;
        Message_roundResult_t roundResult;
        int idAgent;
        int purchasedProducts;
        float totalBudget;
        float moneySpent;
        float utility;
        float anxiety;
        float frustration;
        float reservePrice;
        bool modelActive;
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
            auto messages = get_messages<typename Affective_defs::in_initialIP>(mbs);
            auto productInfo = messages[0]; // Procesar el primer mensaje recibido
            state.informationProduct = productInfo;
            state.modelActive = true;
            // Decidir si realizar una oferta
            state.bidOffer = getPriceProposal(productInfo.productID, productInfo.bestPrice, productInfo.ranking, state.totalBudget);
            state.lastBid = state.bidOffer;
        }

        // Verificar si hay mensajes en el puerto 'in_roundResult'
        if (!get_messages<typename Affective_defs::in_roundResult>(mbs).empty())
        {
            auto roundResultMessages = get_messages<typename Affective_defs::in_roundResult>(mbs);
            auto roundResult = roundResultMessages[0]; // Procesar el mensaje de roundResult
            bool decision;
            state.modelActive = true;
            // Decidir si realizar una oferta
            decision = makeDecision(roundResult, state.idAgent);
            if (decision == true)
            {
                state.bidOffer = state.lastBid;
            }
            else
            {
                state.bidOffer = getPriceProposal(roundResult.productID, roundResult.bestPrice, state.informationProduct.ranking, state.totalBudget);
                state.lastBid = state.bidOffer;
            }
        }
        // Verificar mensajes en el puerto 'finalResult'
        if (!get_messages<typename Affective_defs::in_finalResult>(mbs).empty())
        {
            auto finalResultMessages = get_messages<typename Affective_defs::in_finalResult>(mbs);
            auto finalResult = finalResultMessages[0];
            state.finalResult = finalResult;
            if(state.finalResult.clientID == state.idAgent){
                state.totalBudget = state.totalBudget - state.finalResult.bestPrice;
                state.moneySpent = state.moneySpent + state.finalResult.bestPrice;
                state.purchasedProducts = state.purchasedProducts + 1;
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
        typename make_message_bags<output_ports>::type bags;
        vector<Message_bidOffer_t> bag_port_out;

        // Crear un mensaje para enviar, basado en el estado del modelo
        Message_bidOffer_t outgoingMessage;
        outgoingMessage.clientID = state.idAgent;
        outgoingMessage.productID = state.bidOffer.productID;
        outgoingMessage.priceProposal = state.bidOffer.priceProposal;

        bag_port_out.push_back(outgoingMessage);

        // Asociar el mensaje al puerto de salida
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