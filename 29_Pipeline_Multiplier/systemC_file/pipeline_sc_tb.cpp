#include "DUT/pipeline_sc.h"
#include "stimulus/pipeline_sc_stimulus.h"

#include <systemc>

#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

// Independent two-state reference for the RTL register schedule. It uses
// native 32x32 integer products rather than the DUT's recursive sc_logic
// gate model, and calculates expected output after each individual clock.
struct PipelineCycleReference {
    struct AddResult {
        std::uint64_t sum;
        bool carry;
    };

    std::uint64_t xtemp = 0;
    std::uint64_t ytemp = 0;
    std::uint64_t temp1 = 0;
    std::uint64_t temp2 = 0;
    std::uint64_t temp3 = 0;
    std::uint64_t temp4 = 0;
    bool tempc1 = false;
    bool tempc2 = false;
    bool has = false;
    bool haca = false;
    std::uint64_t temps2 = 0;

    static AddResult add64(std::uint64_t a, std::uint64_t b) {
        const std::uint64_t sum = a + b;
        return {sum, sum < a};
    }

    static std::uint64_t multiply32(
        std::uint32_t a, std::uint32_t b) {
        return static_cast<std::uint64_t>(a) *
               static_cast<std::uint64_t>(b);
    }

    static sc_dt::sc_lv<128> assemble(
        std::uint64_t upper,
        std::uint64_t middle,
        std::uint64_t lower) {
        sc_dt::sc_lv<128> result(sc_dt::SC_LOGIC_0);
        for (unsigned int bit = 0; bit < 32; ++bit) {
            if (((lower >> bit) & UINT64_C(1)) != 0) {
                result[static_cast<int>(bit)] = sc_dt::SC_LOGIC_1;
            }
            if (((middle >> bit) & UINT64_C(1)) != 0) {
                result[static_cast<int>(32 + bit)] = sc_dt::SC_LOGIC_1;
            }
        }
        for (unsigned int bit = 0; bit < 64; ++bit) {
            if (((upper >> bit) & UINT64_C(1)) != 0) {
                result[static_cast<int>(64 + bit)] = sc_dt::SC_LOGIC_1;
            }
        }
        return result;
    }

    sc_dt::sc_lv<128> step(
        std::uint64_t input_x,
        std::uint64_t input_y,
        bool reset_high) {
        const std::uint32_t x_low = static_cast<std::uint32_t>(xtemp);
        const std::uint32_t x_high = static_cast<std::uint32_t>(xtemp >> 32);
        const std::uint32_t y_low = static_cast<std::uint32_t>(ytemp);
        const std::uint32_t y_high = static_cast<std::uint32_t>(ytemp >> 32);

        const std::uint64_t pp1 = multiply32(x_low, y_low);
        const std::uint64_t pp2 = multiply32(x_low, y_high);
        const std::uint64_t pp3 = multiply32(x_high, y_low);
        const std::uint64_t pp4 = multiply32(x_high, y_high);

        const AddResult sum1 = add64(temp2, temp3);
        const AddResult sum2 = add64(sum1.sum, temp1 >> 32);
        const std::uint64_t sum3_operand =
            (temps2 >> 32) |
            (static_cast<std::uint64_t>(has) << 32) |
            (static_cast<std::uint64_t>(haca) << 33);
        const AddResult sum3 = add64(temp4, sum3_operand);

        const bool next_has = tempc1 != tempc2;
        const bool next_haca = tempc1 && tempc2;

        xtemp = reset_high ? 0 : input_x;
        ytemp = reset_high ? 0 : input_y;
        temp1 = pp1;
        temp2 = pp2;
        temp3 = pp3;
        temp4 = pp4;
        tempc1 = sum1.carry;
        tempc2 = sum2.carry;
        has = next_has;
        haca = next_haca;
        temps2 = sum2.sum;

        return assemble(sum3.sum, sum2.sum, pp1);
    }
};

SC_MODULE(pipeline_sc_testbench) {
    sc_core::sc_signal<sc_dt::sc_lv<64>> x_sig;
    sc_core::sc_signal<sc_dt::sc_lv<64>> y_sig;
    sc_core::sc_signal<sc_dt::sc_logic> clk_sig;
    sc_core::sc_signal<sc_dt::sc_logic> reset_sig;
    sc_core::sc_signal<sc_dt::sc_lv<128>> prod_sig;

    pipeline dut;

    std::ofstream simulation_log;
    std::ofstream checks_csv;
    unsigned int check_count = 0;
    unsigned int failure_count = 0;

    SC_CTOR(pipeline_sc_testbench)
        : x_sig("x"),
          y_sig("y"),
          clk_sig("clk"),
          reset_sig("reset"),
          prod_sig("prod"),
          dut("dut") {
        dut.x(x_sig);
        dut.y(y_sig);
        dut.clk(clk_sig);
        dut.reset(reset_sig);
        dut.prod(prod_sig);
        SC_THREAD(run);
    }

    static sc_dt::sc_lv<64> from_uint64(std::uint64_t value) {
        sc_dt::sc_lv<64> result(sc_dt::SC_LOGIC_0);
        for (int bit = 0; bit < 64; ++bit) {
            if (((value >> bit) & UINT64_C(1)) != 0) {
                result[bit] = sc_dt::SC_LOGIC_1;
            }
        }
        return result;
    }

    static sc_dt::sc_lv<128> expected_product(
        std::uint64_t x_value, std::uint64_t y_value) {
        sc_dt::sc_biguint<128> x_wide = x_value;
        sc_dt::sc_biguint<128> y_wide = y_value;
        const sc_dt::sc_biguint<128> product = x_wide * y_wide;

        sc_dt::sc_lv<128> result(sc_dt::SC_LOGIC_0);
        for (int bit = 0; bit < 128; ++bit) {
            if (product[bit]) {
                result[bit] = sc_dt::SC_LOGIC_1;
            }
        }
        return result;
    }

    static std::string csv_quote(const std::string& value) {
        std::string escaped;
        escaped.reserve(value.size() + 2);
        escaped.push_back('"');
        for (const char character : value) {
            if (character == '"') {
                escaped.push_back('"');
            }
            escaped.push_back(character);
        }
        escaped.push_back('"');
        return escaped;
    }

    static std::string time_ns() {
        std::ostringstream value;
        value << std::fixed << std::setprecision(3)
              << sc_core::sc_time_stamp().to_seconds() * 1.0e9;
        return value.str();
    }

    static std::string logic_char(sc_dt::sc_logic value) {
        return std::string(1, value.to_char());
    }

    template <int WIDTH>
    static std::string logic_hex(const sc_dt::sc_lv<WIDTH>& value) {
        static constexpr char HEX[] = "0123456789abcdef";
        std::string result("0x");
        for (int top = WIDTH - 1; top >= 0; top -= 4) {
            unsigned int nibble = 0;
            bool all_zero_or_one = true;
            bool all_z = true;
            for (int offset = 0; offset < 4; ++offset) {
                const sc_dt::sc_logic bit = value[top - offset];
                if (bit == sc_dt::SC_LOGIC_1) {
                    nibble |= (1U << (3U - static_cast<unsigned int>(offset)));
                    all_z = false;
                } else if (bit == sc_dt::SC_LOGIC_0) {
                    all_z = false;
                } else {
                    all_zero_or_one = false;
                    if (bit != sc_dt::SC_LOGIC_Z) {
                        all_z = false;
                    }
                }
            }
            if (all_zero_or_one) {
                result.push_back(HEX[nibble]);
            } else {
                result.push_back(all_z ? 'z' : 'x');
            }
        }
        return result;
    }

    void record_check(
        const std::string& test_name,
        const sc_dt::sc_lv<128>& expected,
        const sc_dt::sc_lv<128>& actual,
        const std::string& details) {
        ++check_count;
        const bool passed = (expected == actual);
        if (!passed) {
            ++failure_count;
        }

        const std::string time = time_ns();
        const std::string x_value = logic_hex(x_sig.read());
        const std::string y_value = logic_hex(y_sig.read());
        const std::string expected_text = logic_hex(expected);
        const std::string actual_text = logic_hex(actual);
        const std::string status = passed ? "PASS" : "FAIL";

        checks_csv << time << ","
                   << csv_quote(test_name) << ","
                   << logic_char(clk_sig.read()) << ","
                   << logic_char(reset_sig.read()) << ","
                   << csv_quote(x_value) << ","
                   << csv_quote(y_value) << ","
                   << csv_quote(expected_text) << ","
                   << csv_quote(actual_text) << ","
                   << status << ","
                   << csv_quote(details) << "\n";
        if (!checks_csv.good()) {
            ++failure_count;
            std::cerr << "FAIL: unable to write checks.csv at "
                      << time << " ns\n";
        }

        std::ostringstream line;
        line << "[t=" << time << " ns] " << test_name
             << " clk=" << logic_char(clk_sig.read())
             << " reset=" << logic_char(reset_sig.read())
             << " x=" << x_value
             << " y=" << y_value
             << " expected_prod=" << expected_text
             << " actual_prod=" << actual_text
             << " " << status;
        if (!details.empty()) {
            line << " (" << details << ")";
        }

        std::cout << line.str() << "\n";
        simulation_log << line.str() << "\n";
        if (!passed) {
            std::cerr << line.str() << "\n";
        }
        if (!simulation_log.good()) {
            ++failure_count;
            std::cerr << "FAIL: unable to write simulation.log at "
                      << time << " ns\n";
        }
    }

    void tick() {
        wait(5, sc_core::SC_NS);
        clk_sig.write(sc_dt::SC_LOGIC_1);
        wait(1, sc_core::SC_NS);
        clk_sig.write(sc_dt::SC_LOGIC_0);
        wait(4, sc_core::SC_NS);
    }

    void wait_five_edges_with_inputs_held() {
        // Input capture plus four downstream register updates. This is the
        // observed stable-input settle interval of the RTL register chain.
        for (unsigned int edge = 0; edge < 5; ++edge) {
            tick();
        }
    }

    void run() {
        simulation_log.open("simulation.log", std::ios::out | std::ios::trunc);
        checks_csv.open("checks.csv", std::ios::out | std::ios::trunc);
        if (!simulation_log.is_open() || !checks_csv.is_open()) {
            std::cerr << "FAIL: cannot create out/simulation.log or "
                         "out/checks.csv\n";
            failure_count = 1;
            sc_core::sc_stop();
            return;
        }

        checks_csv
            << "time_ns,test,clk,reset,x_logic_hex,y_logic_hex,"
               "expected_prod_logic_hex,actual_prod_logic_hex,status,details\n";
        const char* rtl_digest = std::getenv("PIPELINE_RTL_SHA256");
        const std::string source_id =
            rtl_digest == nullptr ? "unset" : rtl_digest;
        simulation_log
            << "pipeline SystemC self-check; seed=0x"
            << std::hex << PIPELINE_RANDOM_SEED << std::dec << "\n";
        simulation_log << "RUN_ID=pipeline_seed29_rtl_" << source_id << "\n";

        clk_sig.write(sc_dt::SC_LOGIC_0);
        reset_sig.write(sc_dt::SC_LOGIC_1);
        x_sig.write(from_uint64(UINT64_C(0x123456789ABCDEF0)));
        y_sig.write(from_uint64(UINT64_C(0xFEDCBA9876543210)));
        wait(sc_core::SC_ZERO_TIME);

        const sc_dt::sc_lv<128> all_x(sc_dt::SC_LOGIC_X);
        record_check(
            "uninitialized_register_state",
            all_x,
            prod_sig.read(),
            "pipeline registers have no declaration-time initialization");

        wait(1, sc_core::SC_NS);
        record_check(
            "synchronous_reset_no_edge_effect",
            all_x,
            prod_sig.read(),
            "reset asserted while clk=0; no posedge has occurred");

        for (unsigned int edge = 0; edge < 5; ++edge) {
            tick();
        }
        const sc_dt::sc_lv<128> zero(sc_dt::SC_LOGIC_0);
        record_check(
            "active_high_reset_flush",
            zero,
            prod_sig.read(),
            "five rising edges with synchronous reset held high");

        const std::vector<PipelineInputVector> vectors =
            pipeline_test_vectors();
        bool tested_nonzero_reset_hold = false;
        for (const PipelineInputVector& vector : vectors) {
            reset_sig.write(sc_dt::SC_LOGIC_0);
            x_sig.write(from_uint64(vector.x));
            y_sig.write(from_uint64(vector.y));
            wait_five_edges_with_inputs_held();

            record_check(
                vector.name,
                expected_product(vector.x, vector.y),
                prod_sig.read(),
                "stable operands held for five rising edges");

            if (!tested_nonzero_reset_hold && vector.x == 1 && vector.y == 1) {
                const sc_dt::sc_lv<128> held_product = prod_sig.read();
                reset_sig.write(sc_dt::SC_LOGIC_1);
                wait(1, sc_core::SC_NS);
                record_check(
                    "reset_assertion_between_edges",
                    held_product,
                    prod_sig.read(),
                    "synchronous reset does not change output before a rising edge");

                for (unsigned int edge = 0; edge < 5; ++edge) {
                    tick();
                }
                record_check(
                    "reset_reasserted_flush",
                    zero,
                    prod_sig.read(),
                    "five rising edges after synchronous reset assertion");
                tested_nonzero_reset_hold = true;
            }
        }

        // Verilog procedural `if (reset)` takes the else branch when reset is
        // X or Z. This directed X case checks that known operands are sampled.
        reset_sig.write(sc_dt::SC_LOGIC_X);
        x_sig.write(from_uint64(UINT64_C(0xFFFFFFFF00000001)));
        y_sig.write(from_uint64(UINT64_C(0x00000001FFFFFFFF)));
        wait_five_edges_with_inputs_held();
        record_check(
            "unknown_reset_selects_normal_sampling",
            expected_product(
                UINT64_C(0xFFFFFFFF00000001),
                UINT64_C(0x00000001FFFFFFFF)),
            prod_sig.read(),
            "four-state if condition X follows the RTL else branch");

        reset_sig.write(sc_dt::SC_LOGIC_Z);
        x_sig.write(from_uint64(UINT64_C(0xFEDCBA9876543210)));
        y_sig.write(from_uint64(UINT64_C(0x0123456789ABCDEF)));
        wait_five_edges_with_inputs_held();
        record_check(
            "high_impedance_reset_selects_normal_sampling",
            expected_product(
                UINT64_C(0xFEDCBA9876543210),
                UINT64_C(0x0123456789ABCDEF)),
            prod_sig.read(),
            "four-state if condition Z follows the RTL else branch");

        reset_sig.write(sc_dt::SC_LOGIC_0);
        x_sig.write(sc_dt::sc_lv<64>(sc_dt::SC_LOGIC_X));
        y_sig.write(from_uint64(UINT64_C(0)));
        wait_five_edges_with_inputs_held();
        record_check(
            "unknown_data_masked_by_zero",
            zero,
            prod_sig.read(),
            "gate-level partial products resolve X AND 0 to zero");

        x_sig.write(sc_dt::sc_lv<64>(sc_dt::SC_LOGIC_Z));
        y_sig.write(from_uint64(UINT64_C(0)));
        wait_five_edges_with_inputs_held();
        record_check(
            "high_impedance_data_masked_by_zero",
            zero,
            prod_sig.read(),
            "gate-level partial products resolve Z AND 0 to zero");

        reset_sig.write(sc_dt::SC_LOGIC_1);
        x_sig.write(from_uint64(UINT64_C(0)));
        y_sig.write(from_uint64(UINT64_C(0)));
        for (unsigned int edge = 0; edge < 5; ++edge) {
            tick();
        }
        record_check(
            "stream_reference_reset_prime",
            zero,
            prod_sig.read(),
            "five reset edges establish the known reference state");

        PipelineCycleReference reference;
        for (std::size_t index = 8; index < vectors.size(); ++index) {
            const PipelineInputVector& vector = vectors[index];
            reset_sig.write(sc_dt::SC_LOGIC_0);
            x_sig.write(from_uint64(vector.x));
            y_sig.write(from_uint64(vector.y));
            const sc_dt::sc_lv<128> expected =
                reference.step(vector.x, vector.y, false);
            tick();
            std::ostringstream name;
            name << "cycle_level_stream_" << std::setw(2)
                 << std::setfill('0') << (index - 8);
            record_check(
                name.str(),
                expected,
                prod_sig.read(),
                "one changing input per edge; independent register-state oracle");
        }

        checks_csv.flush();
        simulation_log
            << "SUMMARY checks=" << check_count
            << " failures=" << failure_count
            << " seed=0x" << std::hex << PIPELINE_RANDOM_SEED << std::dec
            << "\n"
            << "ARTIFACTS checks.csv simulation.log wave.vcd "
               "pipeline_sc_tb build.log\n";
        simulation_log.flush();

        std::cout << "SUMMARY checks=" << check_count
                  << " failures=" << failure_count
                  << " seed=0x" << std::hex << PIPELINE_RANDOM_SEED
                  << std::dec << "\n"
                  << "SOURCE_ID rtl_closure_sha256=" << source_id << "\n"
                  << "ARTIFACTS out/checks.csv out/simulation.log "
                     "out/wave.vcd out/pipeline_sc_tb out/build.log\n";
        if (failure_count != 0) {
            std::cerr << "Self-check failed; see out/checks.csv and "
                         "out/simulation.log\n";
        }

        checks_csv.close();
        simulation_log.close();
        sc_core::sc_stop();
    }
};

int sc_main(int, char**) {
    sc_core::sc_set_time_resolution(1, sc_core::SC_PS);
    pipeline_sc_testbench testbench("testbench");

    sc_core::sc_trace_file* trace_file =
        sc_core::sc_create_vcd_trace_file("wave");
    if (trace_file == nullptr) {
        std::cerr << "FAIL: cannot create out/wave.vcd\n";
        return 2;
    }
    trace_file->set_time_unit(1, sc_core::SC_NS);
    sc_core::sc_trace(trace_file, testbench.clk_sig, "clk");
    sc_core::sc_trace(trace_file, testbench.reset_sig, "reset");
    sc_core::sc_trace(trace_file, testbench.x_sig, "x");
    sc_core::sc_trace(trace_file, testbench.y_sig, "y");
    sc_core::sc_trace(trace_file, testbench.prod_sig, "prod");
    sc_core::sc_trace(trace_file, testbench.dut.xtemp_reg, "xtemp");
    sc_core::sc_trace(trace_file, testbench.dut.ytemp_reg, "ytemp");
    sc_core::sc_trace(trace_file, testbench.dut.temp1_reg, "temp1");
    sc_core::sc_trace(trace_file, testbench.dut.temp2_reg, "temp2");
    sc_core::sc_trace(trace_file, testbench.dut.temp3_reg, "temp3");
    sc_core::sc_trace(trace_file, testbench.dut.temp4_reg, "temp4");
    sc_core::sc_trace(trace_file, testbench.dut.tempc1_reg, "tempc1");
    sc_core::sc_trace(trace_file, testbench.dut.tempc2_reg, "tempc2");
    sc_core::sc_trace(trace_file, testbench.dut.has_reg, "has");
    sc_core::sc_trace(trace_file, testbench.dut.haca_reg, "haca");
    sc_core::sc_trace(trace_file, testbench.dut.temps2_reg, "temps2");
    sc_core::sc_trace(trace_file, testbench.dut.temps3_reg, "temps3");

    sc_core::sc_start();
    sc_core::sc_close_vcd_trace_file(trace_file);
    return testbench.failure_count == 0 ? 0 : 1;
}
