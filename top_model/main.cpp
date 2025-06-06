// Cadmium Simulator headers
#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/dynamic_model.hpp>
#include <cadmium/modeling/dynamic_model_translator.hpp>
#include <cadmium/engine/pdevs_dynamic_runner.hpp>
#include <cadmium/logger/common_loggers.hpp>

// Time class header
#include <NDTime.hpp>

// Messages structures
#include "../data_structures/message.hpp"

// Atomic model headers
#include <cadmium/basic_model/pdevs/iestream.hpp> //Atomic model for inputs
#include "../atomics/affective.hpp"
#include "../atomics/auctioneer.hpp"
#include "../atomics/rational.hpp"

// C++ headers
#include <iostream>
#include <chrono>
#include <algorithm>
#include <string>
#include <random>

using namespace std;
using namespace cadmium;
using namespace cadmium::basic_models::pdevs;

using TIME = NDTime;

float generateBudget()
{
    static std::mt19937 rng(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<float> dist(600.0f, 700.0f);   //Precios caso 1: 700-800; caso 2: 1600-1700; caso 3: 1600-1700
    return dist(rng);
}

/***** Define input port for coupled models *****/
struct in_initialIP_ABP : public in_port<Message_initialIP_t> // Informacion inicial del producto
{
};
/***** Define output ports for coupled model *****/
struct out_bidOffer_ABP : public out_port<Message_bidOffer_t> // Ofertas de los clientes
{
};
struct out_roundResult_ABP : public out_port<Message_roundResult_t> // Resultado de la ronda
{
};
struct out_finalResult_ABP : public out_port<Message_finalResults_t> // Resultado de la subasta
{
};

/****** Input Reader atomic model declaration *******************/
template <typename T>
class InputReader_initialPI_t : public iestream_input<Message_initialIP_t, T>
{
public:
    InputReader_initialPI_t() = default;
    InputReader_initialPI_t(const char *file_path) : iestream_input<Message_initialIP_t, T>(file_path) {}
};
int main(int argc, char **argv)
{
    if (argc < 3)
    {
        cout << "Program used with wrong parameters. The program must be invoked as follow:";
        cout << argv[0] << " path to the input file " << endl;
        return 1;
    }

    std::string run_id = argv[2]; // Número de ejecución

    string messages_filename = "../casos_de_estudio/caso_de_estudio_1/messages/ABP_output_messages_" + run_id + ".csv";
    string state_filename = "../casos_de_estudio/caso_de_estudio_1/states/ABP_output_state_" + run_id + ".csv";

    // Parámetros configurables
    int num_affective_clients = 1;
    int num_rational_clients = 1;

    /****** Input Reader atomic model instantiation *******************/
    string input = argv[1];
    const char *i_input = input.c_str();

    shared_ptr<dynamic::modeling::model> input_reader = dynamic::translate::make_dynamic_atomic_model<InputReader_initialPI_t, TIME, const char *>("input_reader", move(i_input));
    /****** Instanciacion del Subastador *******************/
    shared_ptr<dynamic::modeling::model> auctioneer_model = dynamic::translate::make_dynamic_atomic_model<Auctioneer, TIME>("auctioneer_model");
    /****** Instanciación dinámica de Clientes Afectivos y Racionales *******************/
    vector<shared_ptr<dynamic::modeling::model>> affective_clients;
    vector<shared_ptr<dynamic::modeling::model>> rational_clients;

    for (int i = 1; i <= num_affective_clients; i++)
    {
        float randomBudget = generateBudget();
        affective_clients.push_back(dynamic::translate::make_dynamic_atomic_model<Affective, TIME, int, float>("affective_" + to_string(i), move(i), move(randomBudget)));
    }

    for (int i = num_affective_clients + 1; i <= num_rational_clients + num_affective_clients; i++)
    {
        float randomBudget = generateBudget();
        rational_clients.push_back(dynamic::translate::make_dynamic_atomic_model<Rational, TIME, int, float>("rational_" + to_string(i), move(i), move(randomBudget)));
    }

    /******* ABP SIMULATOR COUPLED MODEL ********/
    dynamic::modeling::Ports iports_ABP = {typeid(in_initialIP_ABP)};
    dynamic::modeling::Ports oports_ABP = {typeid(out_roundResult_ABP), typeid(out_finalResult_ABP)};

    dynamic::modeling::Models submodels_ABP = {auctioneer_model};
    submodels_ABP.insert(submodels_ABP.end(), affective_clients.begin(), affective_clients.end());
    submodels_ABP.insert(submodels_ABP.end(), rational_clients.begin(), rational_clients.end());

    // EICs (External Input Couplings)
    dynamic::modeling::EICs eics_ABP = {
        cadmium::dynamic::translate::make_EIC<in_initialIP_ABP, Auctioneer_defs::in_initialIP>("auctioneer_model")};

    // EOCs (External Output Couplings)
    dynamic::modeling::EOCs eocs_ABP = {
        dynamic::translate::make_EOC<Auctioneer_defs::out_roundResult, out_roundResult_ABP>("auctioneer_model"),
        dynamic::translate::make_EOC<Auctioneer_defs::out_finalResult, out_finalResult_ABP>("auctioneer_model")};

    // ICs (Internal Couplings)
    dynamic::modeling::ICs ics_ABP;

    // Subastador envía productos a los clientes
    for (const auto &affective : affective_clients)
    {
        ics_ABP.push_back(dynamic::translate::make_IC<Auctioneer_defs::out_initialIP, Affective_defs::in_initialIP>("auctioneer_model", affective->get_id()));
    }
    for (const auto &rational : rational_clients)
    {
        ics_ABP.push_back(dynamic::translate::make_IC<Auctioneer_defs::out_initialIP, Rational_defs::in_initialIP>("auctioneer_model", rational->get_id()));
    }

    // Clientes envían ofertas al subastador
    for (const auto &affective : affective_clients)
    {
        ics_ABP.push_back(dynamic::translate::make_IC<Affective_defs::out_bidOffer, Auctioneer_defs::in_bidOffer>(affective->get_id(), "auctioneer_model"));
    }
    for (const auto &rational : rational_clients)
    {
        ics_ABP.push_back(dynamic::translate::make_IC<Rational_defs::out_bidOffer, Auctioneer_defs::in_bidOffer>(rational->get_id(), "auctioneer_model"));
    }

    // Subastador manda resultado de cada ronda a los clientes
    for (const auto &affective : affective_clients)
    {
        ics_ABP.push_back(dynamic::translate::make_IC<Auctioneer_defs::out_roundResult, Affective_defs::in_roundResult>("auctioneer_model", affective->get_id()));
    }
    for (const auto &rational : rational_clients)
    {
        ics_ABP.push_back(dynamic::translate::make_IC<Auctioneer_defs::out_roundResult, Rational_defs::in_roundResult>("auctioneer_model", rational->get_id()));
    }

    // Subastador manda resultado final a los clientes
    for (const auto &affective : affective_clients)
    {
        ics_ABP.push_back(dynamic::translate::make_IC<Auctioneer_defs::out_finalResult, Affective_defs::in_finalResult>("auctioneer_model", affective->get_id()));
    }
    for (const auto &rational : rational_clients)
    {
        ics_ABP.push_back(dynamic::translate::make_IC<Auctioneer_defs::out_finalResult, Rational_defs::in_finalResult>("auctioneer_model", rational->get_id()));
    }

    shared_ptr<dynamic::modeling::coupled<TIME>> ABP_SIMULATOR;
    ABP_SIMULATOR = make_shared<dynamic::modeling::coupled<TIME>>(
        "ABP_SIMULATOR", submodels_ABP, iports_ABP, oports_ABP, eics_ABP, eocs_ABP, ics_ABP);

    /******* TOP COUPLED MODEL ********/
    dynamic::modeling::Ports iports_TOP = {};
    dynamic::modeling::Ports oports_TOP = {typeid(out_roundResult_ABP), typeid(out_finalResult_ABP)};

    dynamic::modeling::Models submodels_TOP = {ABP_SIMULATOR, input_reader};

    dynamic::modeling::EICs eics_TOP = {};
    dynamic::modeling::EOCs eocs_TOP = {
        dynamic::translate::make_EOC<out_roundResult_ABP, out_roundResult_ABP>("ABP_SIMULATOR"),
        dynamic::translate::make_EOC<out_finalResult_ABP, out_finalResult_ABP>("ABP_SIMULATOR")};

    dynamic::modeling::ICs ics_TOP = {
        dynamic::translate::make_IC<iestream_input_defs<Message_initialIP_t>::out, in_initialIP_ABP>("input_reader", "ABP_SIMULATOR")};

    shared_ptr<dynamic::modeling::coupled<TIME>> TOP;
    TOP = make_shared<dynamic::modeling::coupled<TIME>>(
        "TOP", submodels_TOP, iports_TOP, oports_TOP, eics_TOP, eocs_TOP, ics_TOP);

    /*************** Loggers *******************/
    static ofstream out_messages(messages_filename);
    struct oss_sink_messages
    {
        static ostream &sink()
        {
            return out_messages;
        }
    };
    static ofstream out_state(state_filename);
    struct oss_sink_state
    {
        static ostream &sink()
        {
            return out_state;
        }
    };

    using state = logger::logger<logger::logger_state, dynamic::logger::formatter<TIME>, oss_sink_state>;
    using log_messages = logger::logger<logger::logger_messages, dynamic::logger::formatter<TIME>, oss_sink_messages>;
    using global_time_mes = logger::logger<logger::logger_global_time, dynamic::logger::formatter<TIME>, oss_sink_messages>;
    using global_time_sta = logger::logger<logger::logger_global_time, dynamic::logger::formatter<TIME>, oss_sink_state>;
    using logger_top = logger::multilogger<state, log_messages, global_time_mes, global_time_sta>;

    /************** Runner call ************************/
    dynamic::engine::runner<NDTime, logger_top> r(TOP, {0});
    r.run_until_passivate();

    return 0;
}