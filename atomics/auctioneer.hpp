#ifndef _AUCTIONEER_HPP__
#define _AUCTIONEER_HPP__

#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/message_bag.hpp>
#include "functions.hpp" // Incluir funciones compartidas
#include <limits>
#include <string>
#include <vector>
#include <cassert>
#include <iostream>
#include "../data_structures/message.hpp"

using namespace cadmium;
using namespace std;

// Port definition
struct Auctioneer_defs {
    struct out_initialIP : public out_port<Message_initialIP_t> {}; // Informacion inicial del producto
    struct out_roundResult : public out_port<Message_roundResult_t> {}; // Resultado de la ronda
    struct out_finalResult : public out_port<Message_finalResults_t> {}; // Resultado de la subasta
    struct in_bidOffer : public in_port<Message_bidOffer_t> {}; // Ofertas de los clientes
    struct in_initialIP : public in_port<Message_initialIP_t> {}; // Informacion inicial del producto
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
        bool roundState, auctionState, stageState;   // Estado de la ronda
        bool modelActive;                           // Estado del modelo
        int numberRound;                            // numero de ronda en curso
        int SP;
        vector<Message_initialIP_t> recivedProducts, soldProducts, products; // Productos totales recibidos
        vector<Message_bidOffer_t> offerList; // Lista de ofertas recibidas
    };
    state_type state;
    // constructor
    Auctioneer() { state = {false, false, false, false, 0, 1, {}, {}, {}, {}}; } // Constructor
    // Internal transition
    void internal_transition()
    {
        if (state.auctionState == true && state.offerList.size() <= 1)
        {
            state.numberRound = 0;
            state.roundState = false;
            state.auctionState = false;
            state.products.erase(state.products.begin());
            if (!state.products.empty()){ // Si quedan productos, reactivar el modelo
                state.stageState = true;
            }
            else{
                state.stageState = false; // No quedan más productos, se termina el escenario.
            }
        }
        else if (state.auctionState == false){
            state.auctionState = true;
        }
        state.roundState = false;
        state.modelActive = false; // Apaga el modelo después de procesar la ronda
    }

    // External transition
    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs)
    {
        if (!get_messages<typename Auctioneer_defs::in_initialIP>(mbs).empty())
        {
            state.modelActive = true;
            state.stageState = true;
            auto messages = get_messages<typename Auctioneer_defs::in_initialIP>(mbs);
            state.products.insert(state.products.end(), messages.begin(), messages.end());  // Insertar productos recibidos
            std::vector<int> selectedProductIDs = getRandomProducts();            // Filtrar los 10 productos seleccionados aleatoriamente
            std::vector<Message_initialIP_t> selectedProducts;
            for (const auto& product : state.products)            // Filtrar productos según los IDs seleccionados
            {
                if (std::find(selectedProductIDs.begin(), selectedProductIDs.end(), product.productID) != selectedProductIDs.end())
                {
                    selectedProducts.push_back(product);  // Agregar el producto a la lista seleccionada
                }
            }
            state.products = selectedProducts;                                           
        }
        else if (!get_messages<typename Auctioneer_defs::in_bidOffer>(mbs).empty())
        {
            state.modelActive = true;
            state.offerList.clear();
            auto messages = get_messages<typename Auctioneer_defs::in_bidOffer>(mbs);
            state.offerList.insert(state.offerList.end(), messages.begin(), messages.end());
            // Filtrar elementos con decision = 0
            state.offerList.erase(std::remove_if(state.offerList.begin(), state.offerList.end(),[](const auto &offer){return offer.decision == 0;}), state.offerList.end());
            if (state.offerList.size() > 1){
                if (state.offerList[0].productID != 0) {
                    state.roundState = true;
                    updateBestPrice(state.products[0].initialPrice, state.products[0].bestPrice);
                    state.numberRound++;
                }
                else {
                    state.roundState = false;
                    state.numberRound = 0;
                    if (state.SP < getRandomProducts().size()) {
                        state.SP++;
                    }
                }
            }
            else if (state.offerList.size() <= 1){
                state.roundState = true;
                state.numberRound++;
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

        if (!state.products.empty() && !state.auctionState) {
            vector<Message_initialIP_t> bag_port_initial_out;
            bag_port_initial_out.push_back(state.products[0]);
            get_messages<typename Auctioneer_defs::out_initialIP>(bags) = bag_port_initial_out;
        }
        else if (state.auctionState && (state.offerList.size() <= 1)) {// Equivalente a (state.offerList.size() == 1 || state.offerList.empty())
            vector<Message_finalResults_t> bag_port_out;
            Message_finalResults_t finalResultMessage;
            finalResultMessage.productID = state.products[0].productID;
            finalResultMessage.winnerID = state.offerList.empty() ? 0 : state.offerList[0].clientID;
            finalResultMessage.bestPrice = state.offerList.empty() ? state.products[0].bestPrice : state.offerList[0].priceProposal;
            finalResultMessage.initialPrice = state.products[0].initialPrice;
            finalResultMessage.numberRound = state.numberRound;
            bag_port_out.push_back(finalResultMessage);
            get_messages<typename Auctioneer_defs::out_finalResult>(bags) = bag_port_out;
        }
        else {
            vector<Message_roundResult_t> bag_port_out;
            Message_roundResult_t roundResultMessage;
            roundResultMessage.productID = state.products[0].productID;
            roundResultMessage.bestPrice = state.products[0].bestPrice;
            roundResultMessage.round = state.numberRound;
            bag_port_out.push_back(roundResultMessage);
            get_messages<typename Auctioneer_defs::out_roundResult>(bags) = bag_port_out;
        }

        return bags;
    }
    // Time advance
    TIME time_advance() const
    {
        if (!state.stageState && state.products.empty()){
            return std::numeric_limits<TIME>::infinity();
        }
        return state.modelActive ? TIME("00:00:05:000") : std::numeric_limits<TIME>::infinity();
    }
    // Debug information
    friend ostringstream &operator<<(ostringstream &os, const typename Auctioneer<TIME>::state_type &i) {
        os << (i.products.empty() ? " | No products available." : " | Current ProductID: " + to_string(i.products[0].productID) + " | Current BestPrice: " + to_string(i.products[0].bestPrice))
            << " | numberRound: " << i.numberRound
            << " | roundState: " << i.roundState
            << " | auctionState: " << i.auctionState
            << " | stageState: " << i.stageState
            << " | SP: " << i.SP;
        return os;
    }
};
#endif