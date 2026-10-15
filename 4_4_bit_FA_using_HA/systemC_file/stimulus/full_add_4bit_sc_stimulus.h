#ifndef FULL_ADD_4BIT_SC_STIMULUS_H
#define FULL_ADD_4BIT_SC_STIMULUS_H

#include <systemc>
#include <fstream>
#include <iostream>

SC_MODULE(full_add_4bit_sc_stimulus) {
    sc_core::sc_out<sc_dt::sc_lv<4>> i_a{"i_a"};
    sc_core::sc_out<sc_dt::sc_lv<4>> i_b{"i_b"};
    sc_core::sc_out<sc_dt::sc_logic> i_cin{"i_cin"};
    sc_core::sc_in<sc_dt::sc_lv<4>> o_sum{"o_sum"};
    sc_core::sc_in<sc_dt::sc_logic> o_carry{"o_carry"};
    sc_core::sc_out<sc_dt::sc_logic> ha_a{"ha_a"}, ha_b{"ha_b"};
    sc_core::sc_in<sc_dt::sc_logic> ha_sum{"ha_sum"}, ha_carry{"ha_carry"};
    sc_core::sc_out<sc_dt::sc_logic> fa_a{"fa_a"}, fa_b{"fa_b"}, fa_cin{"fa_cin"};
    sc_core::sc_in<sc_dt::sc_logic> fa_sum{"fa_sum"}, fa_carry{"fa_carry"};
    unsigned checks = 0, failures = 0;
    std::ofstream csv, log;

    static sc_dt::sc_lv<4> bits4(unsigned v) {
        sc_dt::sc_lv<4> result;
        for (int bit = 0; bit < 4; ++bit)
            result[bit] = ((v >> bit) & 1U) ? SC_LOGIC_1 : SC_LOGIC_0;
        return result;
    }

    void record_leaf(const char* name, unsigned a, unsigned b, unsigned cin,
                     unsigned total, sc_dt::sc_logic actual_sum,
                     sc_dt::sc_logic actual_carry) {
        const char expected_sum=(total&1U)?'1':'0';
        const char expected_carry=(total&2U)?'1':'0';
        const bool pass=actual_sum.to_char()==expected_sum &&
                        actual_carry.to_char()==expected_carry;
        ++checks; if(!pass) ++failures;
        const char* status=pass?"PASS":"FAIL";
        const double t=sc_core::sc_time_stamp().to_seconds()*1.0e9;
        csv<<t<<','<<name<<','<<a<<','<<b<<','<<cin<<','<<expected_sum<<','
           <<actual_sum.to_char()<<','<<expected_carry<<','<<actual_carry.to_char()<<','<<status<<'\n';
        log<<t<<','<<name<<','<<a<<','<<b<<','<<cin<<','<<expected_sum<<','
           <<actual_sum.to_char()<<','<<expected_carry<<','<<actual_carry.to_char()<<','<<status<<'\n';
        std::cout<<"t="<<sc_core::sc_time_stamp()<<' '<<name<<" i_a="<<a
                 <<" i_b="<<b<<" i_cin="<<cin<<" expected_sum="<<expected_sum
                 <<" actual_sum="<<actual_sum.to_char()<<" expected_carry="
                 <<expected_carry<<" actual_carry="<<actual_carry.to_char()<<' '<<status<<'\n';
        if(!pass)std::cerr<<"FAIL t="<<sc_core::sc_time_stamp()<<' '<<name
                          <<" inputs="<<a<<','<<b<<','<<cin<<" expected="
                          <<expected_sum<<'/'<<expected_carry<<" actual="
                          <<actual_sum.to_char()<<'/'<<actual_carry.to_char()<<'\n';
    }

    void run() {
        csv.open("full_add_4bit.csv", std::ios::trunc);
        log.open("simulation.log", std::ios::trunc);
        if (!csv || !log) {
            std::cerr << "ERROR: cannot create CSV or simulation.log\n";
            failures = 1; sc_core::sc_stop(); return;
        }
        csv << "time_ns,test_name,i_a,i_b,i_cin,expected_sum,actual_sum,expected_carry,actual_carry,status\n";
        log << "time_ns,test_name,i_a,i_b,i_cin,expected_sum,actual_sum,expected_carry,actual_carry,status\n";
        // Leaf-stage smoke: exhaust the directly instantiated half-adder and one-bit full-adder models.
        for(unsigned a=0;a<2;++a)for(unsigned b=0;b<2;++b){
            ha_a.write(a?SC_LOGIC_1:SC_LOGIC_0);ha_b.write(b?SC_LOGIC_1:SC_LOGIC_0);
            wait(1,sc_core::SC_NS);
            record_leaf("half_adder_leaf",a,b,0,a+b,ha_sum.read(),ha_carry.read());
        }
        for(unsigned a=0;a<2;++a)for(unsigned b=0;b<2;++b)for(unsigned cin=0;cin<2;++cin){
            fa_a.write(a?SC_LOGIC_1:SC_LOGIC_0);fa_b.write(b?SC_LOGIC_1:SC_LOGIC_0);
            fa_cin.write(cin?SC_LOGIC_1:SC_LOGIC_0);wait(1,sc_core::SC_NS);
            record_leaf("full_half_add_1bit_leaf",a,b,cin,a+b+cin,fa_sum.read(),fa_carry.read());
        }
        for (unsigned a = 0; a < 16; ++a) {
            for (unsigned b = 0; b < 16; ++b) {
                for (unsigned cin = 0; cin < 2; ++cin) {
                    i_a.write(bits4(a)); i_b.write(bits4(b));
                    i_cin.write(cin ? SC_LOGIC_1 : SC_LOGIC_0);
                    wait(1, sc_core::SC_NS);
                    const unsigned total = a + b + cin;
                    const auto expected_sum = bits4(total & 15U);
                    const char expected_carry = (total & 16U) ? '1' : '0';
                    const auto actual_sum = o_sum.read();
                    const char actual_carry = o_carry.read().to_char();
                    const bool pass = actual_sum == expected_sum &&
                                      actual_carry == expected_carry;
                    ++checks; if (!pass) ++failures;
                    const char* status = pass ? "PASS" : "FAIL";
                    const double t = sc_core::sc_time_stamp().to_seconds() * 1.0e9;
                    csv << t << ",exhaustive," << bits4(a) << ',' << bits4(b) << ','
                        << cin << ',' << expected_sum << ',' << actual_sum << ','
                        << expected_carry << ',' << actual_carry << ',' << status << '\n';
                    log << t << ",exhaustive," << bits4(a) << ',' << bits4(b) << ','
                        << cin << ',' << expected_sum << ',' << actual_sum << ','
                        << expected_carry << ',' << actual_carry << ',' << status << '\n';
                    std::cout << "t=" << sc_core::sc_time_stamp() << " full_add_4bit"
                              << " i_a=" << bits4(a) << " i_b=" << bits4(b)
                              << " i_cin=" << cin << " expected_sum=" << expected_sum
                              << " actual_sum=" << actual_sum
                              << " expected_carry=" << expected_carry
                              << " actual_carry=" << actual_carry << ' ' << status << '\n';
                    if (!pass)
                        std::cerr << "FAIL t=" << sc_core::sc_time_stamp()
                                  << " full_add_4bit inputs=" << bits4(a) << ','
                                  << bits4(b) << ',' << cin << " expected="
                                  << expected_sum << '/' << expected_carry
                                  << " actual=" << actual_sum << '/' << actual_carry << '\n';
                }
            }
        }
        csv.flush();
        log << "SUMMARY checks=" << checks << " failures=" << failures
            << " csv=full_add_4bit.csv vcd=full_add_4bit_wave.vcd\n";
        log.flush();
        std::cout << "SUMMARY checks=" << checks << " failures=" << failures
                  << " csv=full_add_4bit.csv vcd=full_add_4bit_wave.vcd\n";
        if (!csv || !log) { std::cerr << "ERROR: failed writing results\n"; ++failures; }
        sc_core::sc_stop();
    }
    SC_CTOR(full_add_4bit_sc_stimulus) { SC_THREAD(run); }
};

#endif
