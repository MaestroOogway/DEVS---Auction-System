#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/dynamic_model.hpp>
#include <cadmium/modeling/dynamic_coupled.hpp>
#include <cadmium/modeling/dynamic_model_translator.hpp>
#include <cadmium/engine/pdevs_dynamic_runner.hpp>
#include <cadmium/logger/common_loggers.hpp>
#include <NDTime.hpp>
#include "../data_structures/message.hpp"
#include "../atomics/affective.hpp"
#include <cadmium/basic_model/pdevs/iestream.hpp>
#include <iostream>
#include <string>
#include <fstream>

using namespace std;
using namespace cadmium;
using namespace cadmium::basic_models::pdevs;
using TIME = NDTime;

// Definición de puertos para el modelo acoplado
struct top_out : public out_port<Message_bidOffer_t>
{
}; // Cambia según la salida esperada de Affective

// InputReader para enviar mensajes de entrada
template <typename T>
class InputReader_initialPI_t : public iestream_input<Message_initialIP_t, T>
{
public:
    InputReader_initialPI_t() = default;
    InputReader_initialPI_t(const char *file_path) : iestream_input<Message_initialIP_t, T>(file_path) {}
};

template <typename T>
class InputReader_roundResult_t : public iestream_input<Message_roundResult_t, T>
{
public:
    InputReader_roundResult_t() = default;
    InputReader_roundResult_t(const char *file_path) : iestream_input<Message_roundResult_t, T>(file_path) {}
};

template <typename T>
class InputReader_finalResult_t : public iestream_input<Message_finalResults_t, T>
{
public:
    InputReader_finalResult_t() = default;
    InputReader_finalResult_t(const char *file_path) : iestream_input<Message_finalResults_t, T>(file_path) {}
};

int main()
{
    /****** Input Reader instanciación *******************/
    const char *i_input_data = "../input_data/initial_product_information_test.txt"; // Ruta al archivo de prueba
    shared_ptr<dynamic::modeling::model> input_reader;
    input_reader = dynamic::translate::make_dynamic_atomic_model<InputReader_initialPI_t, TIME, const char *>(
        "input_reader", move(i_input_data));

    const char *rr_input_data = "../input_data/round_result_test.txt"; // Archivo de prueba para resultados de ronda
    shared_ptr<dynamic::modeling::model> round_result_reader;
    round_result_reader = dynamic::translate::make_dynamic_atomic_model<InputReader_roundResult_t, TIME, const char *>(
        "round_result_reader", move(rr_input_data));

    const char *fr_input_data = "../input_data/final_result_test.txt"; // Archivo de prueba para resultados de ronda
    shared_ptr<dynamic::modeling::model> final_result_reader;
    final_result_reader = dynamic::translate::make_dynamic_atomic_model<InputReader_finalResult_t, TIME, const char *>(
        "final_result_reader", move(fr_input_data));

    /****** Modelo atómico Affective *******************/
    shared_ptr<dynamic::modeling::model> affective_model;
    affective_model = dynamic::translate::make_dynamic_atomic_model<Affective, TIME>("affective_model");

    /******* Modelo Acoplado TOP ********/
    dynamic::modeling::Ports iports_TOP = {};
    dynamic::modeling::Ports oports_TOP = {typeid(top_out)};
    dynamic::modeling::Models submodels_TOP = {input_reader, round_result_reader, final_result_reader, affective_model};

    dynamic::modeling::EICs eics_TOP = {}; // No hay puertos de entrada en este ejemplo
    dynamic::modeling::EOCs eocs_TOP = {
        dynamic::translate::make_EOC<Affective_defs::out_bidOffer, top_out>("affective_model")};
    dynamic::modeling::ICs ics_TOP = {
        dynamic::translate::make_IC<iestream_input_defs<Message_initialIP_t>::out, Affective_defs::in_initialIP>(
            "input_reader", "affective_model"),
        dynamic::translate::make_IC<iestream_input_defs<Message_roundResult_t>::out, Affective_defs::in_roundResult>(
            "round_result_reader", "affective_model"),
        dynamic::translate::make_IC<iestream_input_defs<Message_finalResults_t>::out, Affective_defs::in_finalResult>(
            "final_result_reader", "affective_model")};

    shared_ptr<dynamic::modeling::coupled<TIME>> TOP;
    TOP = make_shared<dynamic::modeling::coupled<TIME>>(
        "TOP", submodels_TOP, iports_TOP, oports_TOP, eics_TOP, eocs_TOP, ics_TOP);

    /*************** Loggers *******************/
    static ofstream out_messages("../simulation_results/affective_test_output_messages.txt");
    struct oss_sink_messages
    {
        static ostream &sink()
        {
            return out_messages;
        }
    };
    static ofstream out_state("../simulation_results/affective_test_output_state.txt");
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

    /************** Runner ********************/
    dynamic::engine::runner<NDTime, logger_top> r(TOP, {0});
    r.run_until(NDTime("04:00:00:000")); // Ajusta el tiempo según lo necesario
    return 0;
}