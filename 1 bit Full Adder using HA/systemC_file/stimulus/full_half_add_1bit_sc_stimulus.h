#pragma once

#include <systemc.h>

#include <fstream>

SC_MODULE(full_half_add_1bit_sc_stimulus) {
    sc_out<bool> i_a;
    sc_out<bool> i_b;
    sc_out<bool> i_cin;
    sc_in<bool> o_sum;
    sc_in<bool> o_carry;

    int failures = 0;
    std::ofstream log_file;

    void run() {
        log_file.open("full_half_add_1bit_sc.log");
        if (!log_file) {
            std::cerr << "ERROR: cannot open full_half_add_1bit_sc.log\n";
            ++failures;
            sc_stop();
            return;
        }
        log_file << "time_ns,a,b,cin,expected_sum,expected_carry,sum,carry,result\n";

        for (int input = 0; input < 8; ++input) {
            const bool a = (input >> 2) & 1;
            const bool b = (input >> 1) & 1;
            const bool cin = input & 1;

            i_a.write(a);
            i_b.write(b);
            i_cin.write(cin);
            wait(1, SC_NS);

            const int expected = static_cast<int>(a) + static_cast<int>(b) +
                                 static_cast<int>(cin);
            const bool expected_sum = expected & 1;
            const bool expected_carry = expected >> 1;
            const bool passed = o_sum.read() == expected_sum &&
                                o_carry.read() == expected_carry;
            log_file << sc_time_stamp().to_seconds() * 1.0e9 << ',' << a << ',' << b
                     << ',' << cin << ',' << expected_sum << ',' << expected_carry
                     << ',' << o_sum.read() << ',' << o_carry.read() << ','
                     << (passed ? "PASS" : "FAIL") << '\n';
            if (!passed) {
                std::cerr << "FAIL: a=" << a << " b=" << b << " cin=" << cin
                          << " sum=" << o_sum.read() << " carry=" << o_carry.read()
                          << " expected_sum=" << expected_sum
                          << " expected_carry=" << expected_carry << '\n';
                ++failures;
            }
        }

        log_file << "# SUMMARY: " << (failures == 0 ? "PASS" : "FAIL") << '\n';
        log_file.close();
        sc_stop();
    }

    SC_CTOR(full_half_add_1bit_sc_stimulus) {
        SC_THREAD(run);
    }
};
