#ifndef _AUCTIONEER_HPP__
#define _AUCTIONEER_HPP__

#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/message_bag.hpp>

#include <limits>
#include <string>
#include <vector>
#include <cassert>
#include <iostream>
#include "../data_structures/message.hpp"

using namespace cadmium;
using namespace std;

/***** (1) *****/
// Port definition
struct Auctioneer_defs
{
    struct out_initialIP : public out_port<Message_initialIP_t>
    {
    }; // Informacion inicial del producto
    struct out_roundResult : public out_port<Message_roundResult_t>
    {
    }; // Resultado de la ronda
    struct in_bidOffer : public in_port<Message_bidOffer_t>
    {
    }; // Ofertas de los clientes
    struct in_initialIP : public in_port<Message_initialIP_t>
    {
    };
    // Informacion inicial del producto
};

template <typename TIME>
class Auctioneer
{
public:
    // ports definition
    using input_ports = tuple<typename Auctioneer_defs::in_bidOffer, typename Auctioneer_defs::in_initialIP>;
    using output_ports = tuple<typename Auctioneer_defs::out_roundResult, typename Auctioneer_defs::out_initialIP>;

    // state definition
    struct state_type
    {
        int winnerID;                         // ID del ganador
        int productID;                        // ID del producto actual en subasta
        int numberRound;                      // Número de ronda
        float bestPrice;                      // Mejor oferta
        bool modelActive;                     // Indica si el modelo está activo
        bool initialInfoEmitted;
        string roundState;                    // Estado de la ronda
        Message_initialIP_t informationProduct; // Informacion inicial del producto
        vector<Message_bidOffer_t> offerList; // Lista de ofertas recibidas
    };
    state_type state;

    // constructor
    Auctioneer()
    {
        state.offerList.clear();
        state.winnerID = 0;
        state.productID = 0;
        state.numberRound = 0;
        state.bestPrice = 0;
        state.initialInfoEmitted = false;
        state.modelActive = false;
        state.roundState = "open";
    }

    // Internal transition
    void internal_transition()
    {
        if (state.roundState == "closed")
        {
            state.roundState = "open"; // Reabrir para la siguiente ronda.
            state.offerList.clear();   // Limpiar ofertas.
            state.numberRound++;       // Incrementar el número de ronda.
        }
        state.initialInfoEmitted = false; // Resetear la emisión de información inicial.
        state.modelActive = false; // Desactivar el modelo.
    }

    // External transition
    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs)

    {
        // Verificar si hay mensajes en el puerto 'in_initialIP'
        if (!get_messages<typename Auctioneer_defs::in_initialIP>(mbs).empty())
        {
            state.bestPrice = 0;
            state.numberRound = 0;
            auto messages = get_messages<typename Auctioneer_defs::in_initialIP>(mbs);
            auto productInfo = messages[0]; // Procesar el primer mensaje recibido
            state.informationProduct = productInfo;
            state.initialInfoEmitted = true;
            state.modelActive = true;
        }
        // Verificar si hay mensajes en el puerto in_bidOffer
        if (!get_messages<typename Auctioneer_defs::in_bidOffer>(mbs).empty())
        {
            state.modelActive = true;
            auto messages = get_messages<typename Auctioneer_defs::in_bidOffer>(mbs);
            for (const auto &message : messages)
            {
                state.offerList.push_back(message);
                if (message.priceProposal > state.bestPrice)
                {
                    state.productID = message.productID;
                    state.bestPrice = message.priceProposal;
                    state.winnerID = message.clientID;
                }
            }
            // Cerrar la subasta después de procesar las ofertas
            state.roundState = "closed";
        }
    }

    // Confluence transition
    void confluence_transition(TIME e, typename make_message_bags<input_ports>::type mbs)
    {
        internal_transition();
        external_transition(e, move(mbs));
    }

    // Output function
    typename make_message_bags<output_ports>::type output() const
    {
        typename make_message_bags<output_ports>::type bags;

    // Emitir información inicial del producto solo la primera vez
        if (state.initialInfoEmitted) {
            vector<Message_initialIP_t> bag_port_initial_out;
            bag_port_initial_out.push_back(state.informationProduct);
            get_messages<typename Auctioneer_defs::out_initialIP>(bags) = bag_port_initial_out;
            // Una vez emitida la información inicial, desactivar la bandera
        }

        if (state.roundState == "closed")
        {
            vector<Message_roundResult_t> bag_port_out;
            Message_roundResult_t outgoingMessage;
            outgoingMessage.productID= state.productID;
            outgoingMessage.winnerID = state.winnerID;
            outgoingMessage.bestPrice = state.bestPrice;
            outgoingMessage.round = state.numberRound;

            bag_port_out.push_back(outgoingMessage);
            get_messages<typename Auctioneer_defs::out_roundResult>(bags) = bag_port_out;
        }
        return bags;
    }

    // Time advance
    TIME time_advance() const
    {
        return state.modelActive ? TIME("00:00:01:000") : numeric_limits<TIME>::infinity();
    }
    // Debug information
    friend ostringstream &operator<<(ostringstream &os, const typename Auctioneer<TIME>::state_type &i)
    {
        os << "WinnerID: " << i.winnerID
           << " | ProductID:" << i.productID
           << " | BestPrice: " << i.bestPrice;
        return os;
    }
};

#endif
