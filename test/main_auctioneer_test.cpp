#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/dynamic_model.hpp>
#include <cadmium/modeling/dynamic_coupled.hpp>
#include <cadmium/modeling/dynamic_model_translator.hpp>
#include <cadmium/engine/pdevs_dynamic_runner.hpp>
#include <cadmium/logger/common_loggers.hpp>
#include <NDTime.hpp>
#include "../data_structures/message.hpp"
#include "../atomics/auctioneer.hpp" // Modelo del subastador
#include <cadmium/basic_model/pdevs/iestream.hpp>
#include <iostream>
#include <string>
#include <fstream>

using namespace std;
using namespace cadmium;
using namespace cadmium::basic_models::pdevs;
using TIME = NDTime;

// Definición de puertos para el modelo acoplado
struct top_out : public out_port<Message_roundResult_t>
{
};

// InputReader para mensajes iniciales de interés personal
template <typename T>
class InputReader_initialPI_t : public iestream_input<Message_initialIP_t, T>
{
public:
    InputReader_initialPI_t() = default;
    InputReader_initialPI_t(const char *file_path) : iestream_input<Message_initialIP_t, T>(file_path) {}
};

// InputReader para ofertas
template <typename T>
class InputReader_bidOffer_t : public iestream_input<Message_bidOffer_t, T>
{
public:
    InputReader_bidOffer_t() = default;
    InputReader_bidOffer_t(const char *file_path) : iestream_input<Message_bidOffer_t, T>(file_path) {}
};

int main()
{
    /****** Input Readers *******************/
    const char *pi_input_data = "../input_data/initial_product_information_test.txt"; // Archivo para initialPI
    shared_ptr<dynamic::modeling::model> initialPI_reader;
    initialPI_reader = dynamic::translate::make_dynamic_atomic_model<InputReader_initialPI_t, TIME, const char *>(
        "initialPI_reader", move(pi_input_data));

    const char *bid_input_data = "../input_data/bid_offer_test.txt"; // Archivo para ofertas
    shared_ptr<dynamic::modeling::model> bidOffer_reader;
    bidOffer_reader = dynamic::translate::make_dynamic_atomic_model<InputReader_bidOffer_t, TIME, const char *>(
        "bidOffer_reader", move(bid_input_data));

    /****** Modelo atómico Auctioneer *******************/
    shared_ptr<dynamic::modeling::model> auctioneer_model;
    auctioneer_model = dynamic::translate::make_dynamic_atomic_model<Auctioneer, TIME>("auctioneer_model");

    /******* Modelo Acoplado TOP ********/
    dynamic::modeling::Ports iports_TOP = {};
    dynamic::modeling::Ports oports_TOP = {typeid(top_out)};
    dynamic::modeling::Models submodels_TOP = {initialPI_reader, bidOffer_reader, auctioneer_model};

    dynamic::modeling::EICs eics_TOP = {};
    dynamic::modeling::EOCs eocs_TOP = {
        dynamic::translate::make_EOC<Auctioneer_defs::out_roundResult, top_out>("auctioneer_model")};
    dynamic::modeling::ICs ics_TOP = {
        dynamic::translate::make_IC<iestream_input_defs<Message_bidOffer_t>::out, Auctioneer_defs::in_bidOffer>(
            "bidOffer_reader", "auctioneer_model"),
        dynamic::translate::make_IC<iestream_input_defs<Message_initialIP_t>::out, Auctioneer_defs::in_initialIP>(
            "initialPI_reader", "auctioneer_model")};

    shared_ptr<dynamic::modeling::coupled<TIME>> TOP;
    TOP = make_shared<dynamic::modeling::coupled<TIME>>(
        "TOP", submodels_TOP, iports_TOP, oports_TOP, eics_TOP, eocs_TOP, ics_TOP);

    /*************** Loggers *******************/
    static ofstream out_messages("../simulation_results/auctioneer_test_output_messages.txt");
    struct oss_sink_messages
    {
        static ostream &sink() { return out_messages; }
    };
    static ofstream out_state("../simulation_results/auctioneer_test_output_state.txt");
    struct oss_sink_state
    {
        static ostream &sink() { return out_state; }
    };

    using state = logger::logger<logger::logger_state, dynamic::logger::formatter<TIME>, oss_sink_state>;
    using log_messages = logger::logger<logger::logger_messages, dynamic::logger::formatter<TIME>, oss_sink_messages>;
    using global_time_mes = logger::logger<logger::logger_global_time, dynamic::logger::formatter<TIME>, oss_sink_messages>;
    using global_time_sta = logger::logger<logger::logger_global_time, dynamic::logger::formatter<TIME>, oss_sink_state>;
    using logger_top = logger::multilogger<state, log_messages, global_time_mes, global_time_sta>;

    /************** Runner ********************/
    dynamic::engine::runner<NDTime, logger_top> r(TOP, {0});
    r.run_until(NDTime("04:00:00:000")); // Ajusta el tiempo según sea necesario
    return 0;
}
