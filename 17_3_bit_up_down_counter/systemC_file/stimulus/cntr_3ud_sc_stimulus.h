#ifndef CNTR_3UD_SC_STIMULUS_H
#define CNTR_3UD_SC_STIMULUS_H

#include <systemc>
#include <fstream>
#include <iostream>
#include <string>

SC_MODULE(cntr_3ud_sc_stimulus) {
    sc_core::sc_out<sc_dt::sc_logic> up_down{"up_down"};
    sc_core::sc_out<bool> clk{"clk"}, reset{"reset"};
    sc_core::sc_in<sc_dt::sc_lv<3>> bin_count{"bin_count"};
    unsigned checks = 0, failures = 0, expected = 0;
    bool expected_known = false, csv_error = false;
    std::ofstream csv;

    void check(const std::string& name, sc_dt::sc_logic direction) {
        sc_dt::sc_lv<3> expected_bits;
        if (expected_known) expected_bits = sc_dt::sc_lv<3>(sc_dt::sc_uint<3>(expected));
        else expected_bits = "XXX";
        const sc_dt::sc_lv<3> actual = bin_count.read();
        const bool pass = actual == expected_bits;
        ++checks;
        if (!pass) ++failures;
        const char* status = pass ? "PASS" : "FAIL";
        const double time_ns = sc_core::sc_time_stamp().to_seconds() * 1e9;
        const std::string exp = expected_bits.to_string(), got = actual.to_string();
        csv << time_ns << ',' << name << ',' << reset.read() << ',' << direction.to_char()
            << ',' << exp << ',' << got << ',' << status << '\n';
        csv.flush();
        if (!csv && !csv_error) {
            csv_error = true;
            ++failures;
            std::cerr << "ERROR: writing cntr_3ud_sc.csv failed at test=" << name << '\n';
        }
        std::cout << "t=" << sc_core::sc_time_stamp() << " test=" << name
                  << " reset=" << reset.read() << " up_down=" << direction.to_char()
                  << " expected_bin_count=" << exp << " actual_bin_count=" << got << ' ' << status << '\n';
        if (!pass) std::cerr << "FAIL t=" << sc_core::sc_time_stamp() << " test=" << name
                             << " reset=" << reset.read() << " up_down=" << direction.to_char()
                             << " expected_bin_count=" << exp << " actual_bin_count=" << got << '\n';
    }

    void cycle(const std::string& name, sc_dt::sc_logic direction, bool reset_value) {
        up_down.write(direction);
        reset.write(reset_value);
        wait(1, sc_core::SC_NS);
        check(name + "_before_rising_edge", direction);

        clk.write(true);
        wait(1, sc_core::SC_NS);
        if (reset_value) {
            expected = 0;
            expected_known = true;
        } else if (expected_known) {
            // Independent numeric reference with modulo-8 wraparound.
            expected = direction == sc_dt::SC_LOGIC_1
                ? (expected + 7U) % 8U : (expected + 1U) % 8U;
        }
        check(name + "_after_rising_edge", direction);
        clk.write(false);
        wait(1, sc_core::SC_NS);
    }

    void run() {
        csv.open("cntr_3ud_sc.csv", std::ios::trunc);
        if (!csv) {
            std::cerr << "ERROR: cannot create cntr_3ud_sc.csv\n";
            failures = 1;
            std::cout << "SUMMARY checks=0 failures=1 csv=cntr_3ud_sc.csv vcd=cntr_3ud_sc_wave.vcd\n";
            sc_core::sc_stop();
            return;
        }
        csv << "time_ns,test_name,reset,up_down,expected_bin_count,actual_bin_count,status\n";
        clk.write(false);
        reset.write(false);
        up_down.write(sc_dt::SC_LOGIC_0);
        wait(1, sc_core::SC_NS);
        check("uninitialized_register_before_reset", up_down.read());

        // Reset is synchronous: asserting it between clock edges must not change the X state yet.
        cycle("synchronous_reset_assertion", sc_dt::SC_LOGIC_0, true);
        cycle("reset_release", sc_dt::SC_LOGIC_0, false);
        for (unsigned i = 0; i < 10; ++i)
            cycle("up_count_and_wrap", sc_dt::SC_LOGIC_0, false);
        for (unsigned i = 0; i < 10; ++i)
            cycle("down_count_and_wrap", sc_dt::SC_LOGIC_1, false);
        cycle("X_direction_takes_up_count_branch", sc_dt::SC_LOGIC_X, false);
        cycle("reset_mid_sequence", sc_dt::SC_LOGIC_1, true);
        cycle("continue_up_after_reset", sc_dt::SC_LOGIC_0, false);

        csv.close();
        if (!csv && !csv_error) {
            csv_error = true;
            ++failures;
            std::cerr << "ERROR: closing cntr_3ud_sc.csv failed\n";
        }
        std::cout << "SUMMARY checks=" << checks << " failures=" << failures
                  << " csv=cntr_3ud_sc.csv vcd=cntr_3ud_sc_wave.vcd\n";
        if (failures) std::cerr << "SUMMARY checks=" << checks << " failures=" << failures << '\n';
        sc_core::sc_stop();
    }

    SC_CTOR(cntr_3ud_sc_stimulus) { SC_THREAD(run); }
};

#endif
