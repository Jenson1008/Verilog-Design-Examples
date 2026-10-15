#ifndef BIN_GRY_SC_STIMULUS_H
#define BIN_GRY_SC_STIMULUS_H

#include <systemc>
#include <fstream>
#include <iostream>
#include <string>

SC_MODULE(bin_gry_sc_stimulus) {
    sc_core::sc_out<sc_dt::sc_lv<8>> binary{"binary"};
    sc_core::sc_in<sc_dt::sc_lv<8>> gray{"gray"};
    unsigned checks = 0, failures = 0;
    std::ofstream csv;
    void run() {
        csv.open("bin_gry.csv", std::ios::trunc);
        if (!csv) { std::cerr << "ERROR: cannot create bin_gry.csv\n"; failures = 1; sc_core::sc_stop(); return; }
        csv << "time_ns,test_name,binary,expected_oracle,actual_oracle,actual_gray,status\n";
        for (unsigned value = 0; value < 256; ++value) {
            sc_dt::sc_lv<8> input;
            for (int bit = 0; bit < 8; ++bit)
                input[bit] = ((value >> bit) & 1U) ? sc_dt::SC_LOGIC_1 : sc_dt::SC_LOGIC_0;
            binary.write(input); wait(1, sc_core::SC_NS);
            const auto actual = gray.read();
            sc_dt::sc_lv<8> decoded;
            decoded[7] = actual[7];
            for (int bit = 6; bit >= 0; --bit) decoded[bit] = decoded[bit + 1] ^ actual[bit];
            const bool pass = decoded == input; // Independent Gray-to-binary round-trip oracle.
            ++checks; if (!pass) ++failures;
            const char* status = pass ? "PASS" : "FAIL";
            const double t = sc_core::sc_time_stamp().to_seconds() * 1e9;
            const std::string expected = input.to_string(), got = decoded.to_string(), observed = actual.to_string(), in = input.to_string();
            csv << t << ",exhaustive_binary_to_gray," << in << ',' << expected << ',' << got << ',' << observed << ',' << status << '\n'; csv.flush();
            std::cout << "t=" << sc_core::sc_time_stamp() << " test=exhaustive_binary_to_gray binary=" << in
                      << " expected_decoded_binary=" << expected << " actual_decoded_binary=" << got
                      << " actual_gray=" << observed << ' ' << status << '\n';
            if (!pass) std::cerr << "FAIL t=" << sc_core::sc_time_stamp() << " binary=" << in
                                 << " expected_decoded_binary=" << expected << " actual_decoded_binary=" << got
                                 << " actual_gray=" << observed << '\n';
        }
        const sc_dt::sc_lv<8> unknown_vectors[] = {
            sc_dt::sc_lv<8>("0000000X"), sc_dt::sc_lv<8>("00000Z00")
        };
        const sc_dt::sc_lv<8> unknown_expected[] = {
            sc_dt::sc_lv<8>("0000000X"), sc_dt::sc_lv<8>("00000XX0")
        };
        const char* unknown_names[] = {"directed_X_input", "directed_Z_input"};
        for (unsigned i = 0; i < 2; ++i) {
            binary.write(unknown_vectors[i]); wait(1, sc_core::SC_NS);
            const auto actual = gray.read(); const bool pass = actual == unknown_expected[i];
            ++checks; if (!pass) ++failures;
            const char* status = pass ? "PASS" : "FAIL";
            const std::string exp = unknown_expected[i].to_string(), got = actual.to_string(), in = unknown_vectors[i].to_string();
            const double t = sc_core::sc_time_stamp().to_seconds() * 1e9;
            csv << t << ',' << unknown_names[i] << ',' << in << ',' << exp << ',' << got << ',' << got << ',' << status << '\n'; csv.flush();
            std::cout << "t=" << sc_core::sc_time_stamp() << " test=" << unknown_names[i] << " binary=" << in
                      << " expected_gray=" << exp << " actual_gray=" << got << ' ' << status << '\n';
            if (!pass) std::cerr << "FAIL t=" << sc_core::sc_time_stamp() << " binary=" << in
                                 << " expected_gray=" << exp << " actual_gray=" << got << '\n';
        }
        csv.close();
        std::cout << "SUMMARY checks=" << checks << " failures=" << failures << " exhaustive_binary_inputs=256 directed_four_state_vectors=2 csv=bin_gry.csv vcd=bin_gry_wave.vcd\n";
        if (failures) std::cerr << "SUMMARY checks=" << checks << " failures=" << failures << '\n';
        sc_core::sc_stop();
    }
    SC_CTOR(bin_gry_sc_stimulus) { SC_THREAD(run); }
};

#endif
