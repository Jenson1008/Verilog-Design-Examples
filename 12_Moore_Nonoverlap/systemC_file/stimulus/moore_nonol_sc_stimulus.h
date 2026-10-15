#ifndef MOORE_NONOL_SC_STIMULUS_H
#define MOORE_NONOL_SC_STIMULUS_H

#include <systemc>
#include <fstream>
#include <iostream>
#include <string>

SC_MODULE(moore_nonol_sc_stimulus) {
    sc_core::sc_out<sc_dt::sc_logic> in{"in"}, clk{"clk"}, reset{"reset"};
    sc_core::sc_in<sc_dt::sc_logic> out{"out"};

    unsigned checks = 0, failures = 0;
    unsigned history = 0, sample_count = 0;
    bool expected_pulse = false;
    bool csv_error = false;
    std::ofstream csv;

    sc_dt::sc_logic reference_output() const {
        return expected_pulse
            ? sc_dt::SC_LOGIC_1 : sc_dt::SC_LOGIC_0;
    }

    void check(const std::string& name, sc_dt::sc_logic expected) {
        const sc_dt::sc_logic actual = out.read();
        const bool pass = actual == expected;
        ++checks;
        if (!pass) ++failures;
        const char* status = pass ? "PASS" : "FAIL";
        const double time_ns = sc_core::sc_time_stamp().to_seconds() * 1e9;
        csv << time_ns << ',' << name << ',' << in.read().to_char() << ','
            << clk.read().to_char() << ',' << reset.read().to_char() << ','
            << expected.to_char() << ',' << actual.to_char() << ',' << status << '\n';
        csv.flush();
        if (!csv && !csv_error) {
            csv_error = true;
            ++failures;
            std::cerr << "ERROR: writing moore_nonol_sc.csv failed at test=" << name << '\n';
        }
        std::cout << "t=" << sc_core::sc_time_stamp() << " test=" << name
                  << " in=" << in.read().to_char() << " clk=" << clk.read().to_char()
                  << " reset=" << reset.read().to_char() << " expected_out="
                  << expected.to_char() << " actual_out=" << actual.to_char() << ' ' << status << '\n';
        if (!pass)
            std::cerr << "FAIL t=" << sc_core::sc_time_stamp() << " test=" << name
                      << " in=" << in.read().to_char() << " clk=" << clk.read().to_char()
                      << " reset=" << reset.read().to_char() << " expected_out="
                      << expected.to_char() << " actual_out=" << actual.to_char() << '\n';
    }

    void asynchronous_reset(const std::string& name) {
        reset.write(sc_dt::SC_LOGIC_1);
        history = 0;
        sample_count = 0;
        expected_pulse = false;
        wait(1, sc_core::SC_NS);
        check(name + "_asserted_without_clock", sc_dt::SC_LOGIC_0);
        reset.write(sc_dt::SC_LOGIC_0);
        wait(1, sc_core::SC_NS);
        check(name + "_released_without_clock", sc_dt::SC_LOGIC_0);
    }

    void step(unsigned bit, const std::string& name) {
        in.write(bit ? sc_dt::SC_LOGIC_1 : sc_dt::SC_LOGIC_0);
        wait(1, sc_core::SC_NS);
        check(name + "_between_edges", reference_output());

        clk.write(sc_dt::SC_LOGIC_1);
        wait(1, sc_core::SC_NS);
        history = ((history << 1U) | bit) & 0xFU;
        if (sample_count < 4) ++sample_count;
        expected_pulse = sample_count == 4 && history == 0b1101;
        if (expected_pulse) {
            // This RTL's S4 transitions start a fresh, nonoverlapping match on the next bit.
            history = 0;
            sample_count = 0;
        }
        check(name + "_after_rising_edge", reference_output());

        clk.write(sc_dt::SC_LOGIC_0);
        wait(1, sc_core::SC_NS);
        check(name + "_after_falling_edge", reference_output());
    }

    void directed_step(unsigned bit, const std::string& name,
                       sc_dt::sc_logic expected_before,
                       sc_dt::sc_logic expected_after) {
        in.write(bit ? sc_dt::SC_LOGIC_1 : sc_dt::SC_LOGIC_0);
        wait(1, sc_core::SC_NS);
        check(name + "_between_edges", expected_before);
        clk.write(sc_dt::SC_LOGIC_1);
        wait(1, sc_core::SC_NS);
        check(name + "_after_rising_edge", expected_after);
        clk.write(sc_dt::SC_LOGIC_0);
        wait(1, sc_core::SC_NS);
        check(name + "_after_falling_edge", expected_after);
    }

    void run_word(const std::string& name, const std::string& bits) {
        for (std::size_t index = 0; index < bits.size(); ++index)
            step(bits[index] == '1', name + "_bit_" + std::to_string(index));
    }

    void run() {
        csv.open("moore_nonol_sc.csv", std::ios::trunc);
        if (!csv) {
            std::cerr << "ERROR: cannot create moore_nonol_sc.csv\n";
            failures = 1;
            std::cout << "SUMMARY checks=0 failures=1 csv=moore_nonol_sc.csv vcd=moore_nonol_sc_wave.vcd\n";
            sc_core::sc_stop();
            return;
        }
        csv << "time_ns,test_name,in,clk,reset,expected_out,actual_out,status\n";
        in.write(sc_dt::SC_LOGIC_0);
        clk.write(sc_dt::SC_LOGIC_0);
        reset.write(sc_dt::SC_LOGIC_0);
        wait(1, sc_core::SC_NS);
        check("uninitialized_state_before_reset", sc_dt::SC_LOGIC_X);
        asynchronous_reset("initial_async_reset");

        // Independent oracle: consume 1101 as a nonoverlapping four-bit word, then restart.
        run_word("state_s0_loops_and_s0_s1_s0", "0010");
        asynchronous_reset("reset_before_s1_s2_s3_s4_path");
        run_word("detect_1101", "1101");
        asynchronous_reset("reset_before_s2_self_loop");
        run_word("s2_self_loop", "1110");
        asynchronous_reset("reset_before_s3_to_s0");
        run_word("s3_to_s0", "1100");
        asynchronous_reset("reset_before_s4_to_s0");
        run_word("s4_to_s0", "11010");
        asynchronous_reset("reset_before_s4_to_s1");
        run_word("s4_to_s1", "11011");
        asynchronous_reset("reset_before_back_to_back_nonoverlap");
        run_word("back_to_back_nonoverlap_11011101", "11011101");

        asynchronous_reset("reset_before_unknown_input");
        in.write(sc_dt::SC_LOGIC_X);
        wait(1, sc_core::SC_NS);
        check("X_input_before_edge", sc_dt::SC_LOGIC_0);
        clk.write(sc_dt::SC_LOGIC_1);
        wait(1, sc_core::SC_NS);
        check("X_input_creates_invalid_latched_state", sc_dt::SC_LOGIC_0);
        clk.write(sc_dt::SC_LOGIC_0);
        wait(1, sc_core::SC_NS);
        in.write(sc_dt::SC_LOGIC_1);
        wait(1, sc_core::SC_NS);
        check("invalid_state_holds_output_between_edges", sc_dt::SC_LOGIC_0);
        clk.write(sc_dt::SC_LOGIC_1);
        wait(1, sc_core::SC_NS);
        check("invalid_state_holds_output_at_clock", sc_dt::SC_LOGIC_0);
        clk.write(sc_dt::SC_LOGIC_0);
        wait(1, sc_core::SC_NS);
        asynchronous_reset("recovery_after_unknown_state");

        // A Verilog posedge includes 0->X. With reset=X, procedural if takes its else branch.
        in.write(sc_dt::SC_LOGIC_1);
        wait(1, sc_core::SC_NS);
        check("prepare_next_state_before_X_reset", sc_dt::SC_LOGIC_0);
        reset.write(sc_dt::SC_LOGIC_X);
        wait(1, sc_core::SC_NS);
        check("X_reset_event_takes_else_branch", sc_dt::SC_LOGIC_0);
        reset.write(sc_dt::SC_LOGIC_0);
        wait(1, sc_core::SC_NS);
        check("X_reset_deassertion_without_clock", sc_dt::SC_LOGIC_0);
        directed_step(1, "X_reset_loaded_S1_then_input_1", sc_dt::SC_LOGIC_0, sc_dt::SC_LOGIC_0);
        directed_step(0, "X_reset_loaded_S1_then_input_10", sc_dt::SC_LOGIC_0, sc_dt::SC_LOGIC_0);
        directed_step(1, "X_reset_loaded_S1_then_input_101", sc_dt::SC_LOGIC_0, sc_dt::SC_LOGIC_1);
        asynchronous_reset("cleanup_after_X_reset_test");

        csv.close();
        if (!csv && !csv_error) {
            csv_error = true;
            ++failures;
            std::cerr << "ERROR: closing moore_nonol_sc.csv failed\n";
        }
        std::cout << "SUMMARY checks=" << checks << " failures=" << failures
                  << " csv=moore_nonol_sc.csv vcd=moore_nonol_sc_wave.vcd\n";
        if (failures) std::cerr << "SUMMARY checks=" << checks << " failures=" << failures << '\n';
        sc_core::sc_stop();
    }

    SC_CTOR(moore_nonol_sc_stimulus) { SC_THREAD(run); }
};

#endif
