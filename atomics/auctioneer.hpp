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

// Port definition
struct Auctioneer_defs
{
    struct out_initialIP : public out_port<Message_initialIP_t> // Informacion inicial del producto
    {
    };
    struct out_roundResult : public out_port<Message_roundResult_t> // Resultado de la ronda
    {
    };
    struct out_finalResult : public out_port<Message_finalResults_t> // Resultado de la subasta
    {
    };
    struct in_bidOffer : public in_port<Message_bidOffer_t> // Ofertas de los clientes
    {
    };
    struct in_initialIP : public in_port<Message_initialIP_t> // Informacion inicial del producto
    {
    };
};

template <typename TIME>
class Auctioneer
{
public:
    // ports definition
    using input_ports = tuple<typename Auctioneer_defs::in_bidOffer, typename Auctioneer_defs::in_initialIP>;
    using output_ports = tuple<typename Auctioneer_defs::out_roundResult, typename Auctioneer_defs::out_initialIP, typename Auctioneer_defs::out_finalResult>;

    // state definition
    struct state_type
    {
        bool roundState;                      // Estado de la ronda
        bool auctionState;                    // Estado de la subasta
        bool stageState;                      // Estado del escenario experimental
        bool modelActive;                     // Estado del modelo
        int numberRound;                      // numero de ronda en curso
        vector<Message_initialIP_t> products; // Lista de productos recibidos
        vector<Message_bidOffer_t> offerList; // Lista de ofertas recibidas
    };
    state_type state;

    // constructor
    Auctioneer()
    {
        state.roundState = false;
        state.auctionState = false;
        state.stageState = false;
        state.modelActive = false;
        state.numberRound = 0;
        state.products.clear();
        state.offerList.clear();
    }

    // Internal transition
    void internal_transition()
    {
        if (state.auctionState && state.offerList.size() == 1) 
        {
            state.numberRound = 0;
            state.products.erase(state.products.begin());
            state.roundState = false;
            state.auctionState = false;    
            if (state.products.empty() == false) {      // Si quedan productos, reactivar el modelo para emitir el siguiente producto
                state.stageState = true;
            } else {
                state.stageState = false;               // No quedan más productos, se termina el escenario.
            }    
        }
        state.roundState = false;
        state.auctionState = true;
        state.modelActive = false; // Apaga el modelo después de procesar la ronda
    }

    // External transition
    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs)
    {
        if (!get_messages<typename Auctioneer_defs::in_initialIP>(mbs).empty())
        {
            state.modelActive = true;
            state.stageState = true;
            auto messages = get_messages<typename Auctioneer_defs::in_initialIP>(mbs); // Procesar los productos y almacenarlos.
            for (const auto &message : messages)
            {
                state.products.push_back(message);
            }
        }

        else if (!get_messages<typename Auctioneer_defs::in_bidOffer>(mbs).empty())
        {
            state.modelActive = true;
            state.roundState = true;
            state.numberRound = state.numberRound + 1;
            state.offerList.clear();                                                 //Vaciar lista de ofertas actuales para recivir las nuevas.
            auto messages = get_messages<typename Auctioneer_defs::in_bidOffer>(mbs);
            for (const auto &message : messages)
            {
                state.offerList.push_back(message);
            }
            if (state.offerList.size() > 1)
            {
                state.products[0].bestPrice = state.products[0].bestPrice*1.1;
            }
            else if (state.offerList.size()==0){

            }
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
        if (!state.products.empty() && state.numberRound == 0)              // Iniciar la subasta por un producto.
        { 
            vector<Message_initialIP_t> bag_port_initial_out;
            bag_port_initial_out.push_back(state.products[0]);
            get_messages<typename Auctioneer_defs::out_initialIP>(bags) = bag_port_initial_out;
        }

        else if (state.auctionState == true && state.offerList.size() == 1)      // Solo queda un postor, se declara ganador.
        {
            vector<Message_finalResults_t> bag_port_out;
            Message_finalResults_t finalResultMessage;
            finalResultMessage.productID = state.offerList[0].productID;
            finalResultMessage.winnerID = state.offerList[0].clientID;
            finalResultMessage.bestPrice = state.offerList[0].priceProposal;
            finalResultMessage.initialPrice = state.products[0].initialPrice;
            finalResultMessage.numberRound = state.numberRound;
            bag_port_out.push_back(finalResultMessage);
            get_messages<typename Auctioneer_defs::out_finalResult>(bags) = bag_port_out;
            
            if (state.products.size() > 1) {                               // Emitir el siguiente producto si hay más en la lista
                vector<Message_initialIP_t> bag_port_initial_out;
                bag_port_initial_out.push_back(state.products[1]);          // El siguiente producto
                get_messages<typename Auctioneer_defs::out_initialIP>(bags) = bag_port_initial_out;
            }
        }

        else if(state.auctionState == true && state.offerList.size() == 0){
            vector<Message_finalResults_t> bag_port_out;
            Message_finalResults_t finalResultMessage;
            finalResultMessage.productID = state.offerList[0].productID;
            finalResultMessage.winnerID = 0;
            finalResultMessage.bestPrice = state.offerList[0].priceProposal;
            finalResultMessage.initialPrice = state.products[0].initialPrice;
            finalResultMessage.numberRound = state.numberRound;
            bag_port_out.push_back(finalResultMessage);
            get_messages<typename Auctioneer_defs::out_finalResult>(bags) = bag_port_out;
            
            if (state.products.size() > 1) {                               // Emitir el siguiente producto si hay más en la lista
                vector<Message_initialIP_t> bag_port_initial_out;
                bag_port_initial_out.push_back(state.products[1]);          // El siguiente producto
                get_messages<typename Auctioneer_defs::out_initialIP>(bags) = bag_port_initial_out;
            }
        }

        else 
        {
            vector<Message_roundResult_t> bag_port_out;
            Message_roundResult_t roundResultMessage;
            roundResultMessage.clientID = 0;                                // Corregir esto que va a contener esta variable del mensaje.
            roundResultMessage.bestPrice = state.products[0].bestPrice;
            roundResultMessage.productID = state.products[0].productID;
            roundResultMessage.round = state.numberRound;
            bag_port_out.push_back(roundResultMessage);
            get_messages<typename Auctioneer_defs::out_roundResult>(bags) = bag_port_out;
        }
        return bags;
    }

    // Time advance
    TIME time_advance() const
    {
        if(state.stageState==false && state.products.empty()){
            return std::numeric_limits<TIME>::infinity();
        }
        return state.modelActive ? TIME("00:00:05:000") : numeric_limits<TIME>::infinity();
    }

    // Debug information
    friend ostringstream &operator<<(ostringstream &os, const typename Auctioneer<TIME>::state_type &i)
    {
        if (!i.products.empty()) // Verificar que haya al menos un producto en la lista
        {
            os << " | ProductID: " << i.products[0].productID
               << " | BestPrice: " << i.products[0].bestPrice;
        }
        else
        {
            os << " | No products available.";
        }
        os << " | numberRound: " << i.numberRound
           << " | roundState: " << i.roundState
           << " | auctionState: " << i.auctionState
           << " | stageState: " << i.stageState
           << " | modelState: " << i.modelActive;
        return os;
    }
};
#endif