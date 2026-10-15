#ifndef FULL_ADDER_1BIT_SC_STIMULUS_H
#define FULL_ADDER_1BIT_SC_STIMULUS_H

#include <systemc>
#include <fstream>
#include <iostream>
#include "full_adder_1bit_sc.h"

SC_MODULE(full_adder_1bit_sc_stimulus) {
    sc_core::sc_out<sc_dt::sc_logic> i_a{"i_a"};
    sc_core::sc_out<sc_dt::sc_logic> i_b{"i_b"};
    sc_core::sc_out<sc_dt::sc_logic> i_cin{"i_cin"};
    sc_core::sc_in<sc_dt::sc_logic> o_sum{"o_sum"};
    sc_core::sc_in<sc_dt::sc_logic> o_carry{"o_carry"};

    unsigned checks = 0;
    unsigned failures = 0;
    std::ofstream csv;
    std::ofstream log;

    void run() {
        csv.open("full_adder_1bit.csv", std::ios::out | std::ios::trunc);
        log.open("simulation.log", std::ios::out | std::ios::trunc);
        if (!csv || !log) {
            std::cerr << "ERROR: cannot create CSV or simulation.log\n";
            failures = 1;
            sc_core::sc_stop();
            return;
        }
        csv << "time_ns,test_name,i_a,i_b,i_cin,expected_sum,actual_sum,"
               "expected_carry,actual_carry,status\n";
        log << "time_ns,test_name,i_a,i_b,i_cin,expected_sum,actual_sum,"
               "expected_carry,actual_carry,status\n";

        for (unsigned value = 0; value < 8; ++value) {
            const bool a = (value >> 2U) & 1U;
            const bool b = (value >> 1U) & 1U;
            const bool cin = value & 1U;
            i_a.write(a ? SC_LOGIC_1 : SC_LOGIC_0);
            i_b.write(b ? SC_LOGIC_1 : SC_LOGIC_0);
            i_cin.write(cin ? SC_LOGIC_1 : SC_LOGIC_0);
            wait(1, sc_core::SC_NS);

            const unsigned total = static_cast<unsigned>(a) +
                                   static_cast<unsigned>(b) +
                                   static_cast<unsigned>(cin);
            const char expected_sum = (total & 1U) ? '1' : '0';
            const char expected_carry = (total & 2U) ? '1' : '0';
            const char actual_sum = o_sum.read().to_char();
            const char actual_carry = o_carry.read().to_char();
            const bool pass = actual_sum == expected_sum &&
                              actual_carry == expected_carry;
            ++checks;
            if (!pass) {
                ++failures;
                std::cerr << "FAIL t=" << sc_core::sc_time_stamp()
                          << " full_adder_1bit i_a=" << a << " i_b=" << b
                          << " i_cin=" << cin << " expected_sum="
                          << expected_sum << " actual_sum=" << actual_sum
                          << " expected_carry=" << expected_carry
                          << " actual_carry=" << actual_carry << '\n';
            }
            const char* status = pass ? "PASS" : "FAIL";
            const double time_ns = sc_core::sc_time_stamp().to_seconds() * 1.0e9;
            csv << time_ns << ",exhaustive," << a << ',' << b << ',' << cin
                << ',' << expected_sum << ',' << actual_sum << ','
                << expected_carry << ',' << actual_carry << ',' << status << '\n';
            log << time_ns << ",exhaustive," << a << ',' << b << ',' << cin
                << ',' << expected_sum << ',' << actual_sum << ','
                << expected_carry << ',' << actual_carry << ',' << status << '\n';
            std::cout << "t=" << sc_core::sc_time_stamp()
                      << " full_adder_1bit i_a=" << a << " i_b=" << b
                      << " i_cin=" << cin << " expected_sum=" << expected_sum
                      << " actual_sum=" << actual_sum
                      << " expected_carry=" << expected_carry
                      << " actual_carry=" << actual_carry << ' ' << status << '\n';
        }
        csv.flush();
        log << "SUMMARY checks=" << checks << " failures=" << failures
            << " csv=full_adder_1bit.csv vcd=full_adder_1bit_wave.vcd\n";
        log.flush();
        std::cout << "SUMMARY checks=" << checks << " failures=" << failures
                  << " csv=full_adder_1bit.csv vcd=full_adder_1bit_wave.vcd\n";
        if (!csv || !log) {
            std::cerr << "ERROR: failed writing simulation results\n";
            ++failures;
        }
        sc_core::sc_stop();
    }

    SC_CTOR(full_adder_1bit_sc_stimulus) {
        SC_THREAD(run);
    }
};

#endif
