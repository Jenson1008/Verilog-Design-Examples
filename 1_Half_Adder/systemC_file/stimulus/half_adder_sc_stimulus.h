#ifndef HALF_ADDER_SC_STIMULUS_H
#define HALF_ADDER_SC_STIMULUS_H

#include <systemc>
#include <fstream>
#include <iostream>

SC_MODULE(half_adder_sc_stimulus) {
    sc_core::sc_out<sc_dt::sc_logic> i_a{"i_a"}, i_b{"i_b"};
    sc_core::sc_in<sc_dt::sc_logic> o_sum{"o_sum"}, o_carry{"o_carry"};
    unsigned checks=0, failures=0;
    std::ofstream csv, log;

    void run() {
        csv.open("half_adder.csv",std::ios::trunc);
        log.open("simulation.log",std::ios::trunc);
        if(!csv||!log){std::cerr<<"ERROR: cannot create CSV or simulation.log\n";failures=1;sc_core::sc_stop();return;}
        const char* header="time_ns,test_name,i_a,i_b,expected_sum,actual_sum,expected_carry,actual_carry,status\n";
        csv<<header;log<<header;
        for(unsigned a=0;a<2;++a)for(unsigned b=0;b<2;++b){
            i_a.write(a?sc_dt::SC_LOGIC_1:sc_dt::SC_LOGIC_0);
            i_b.write(b?sc_dt::SC_LOGIC_1:sc_dt::SC_LOGIC_0);
            wait(1,sc_core::SC_NS);
            const char es=(a!=b)?'1':'0', ec=(a&&b)?'1':'0';
            const char as=o_sum.read().to_char(), ac=o_carry.read().to_char();
            const bool pass=es==as&&ec==ac;++checks;if(!pass)++failures;
            const char* status=pass?"PASS":"FAIL";
            const double t=sc_core::sc_time_stamp().to_seconds()*1e9;
            csv<<t<<",exhaustive,"<<a<<','<<b<<','<<es<<','<<as<<','<<ec<<','<<ac<<','<<status<<'\n';
            log<<t<<",exhaustive,"<<a<<','<<b<<','<<es<<','<<as<<','<<ec<<','<<ac<<','<<status<<'\n';
            std::cout<<"t="<<sc_core::sc_time_stamp()<<" half_adder i_a="<<a<<" i_b="<<b
                     <<" expected_o_sum="<<es<<" actual_o_sum="<<as
                     <<" expected_o_carry="<<ec<<" actual_o_carry="<<ac<<' '<<status<<'\n';
            if(!pass)std::cerr<<"FAIL t="<<sc_core::sc_time_stamp()<<" half_adder i_a="<<a<<" i_b="<<b
                              <<" expected_sum="<<es<<" actual_sum="<<as
                              <<" expected_carry="<<ec<<" actual_carry="<<ac<<'\n';
        }
        csv.flush();
        log<<"SUMMARY checks="<<checks<<" failures="<<failures<<" csv=half_adder.csv vcd=half_adder_wave.vcd\n";
        log.flush();
        std::cout<<"SUMMARY checks="<<checks<<" failures="<<failures
                 <<" csv=half_adder.csv vcd=half_adder_wave.vcd\n";
        if(!csv||!log){std::cerr<<"ERROR: result log write failed\n";++failures;}
        sc_core::sc_stop();
    }
    SC_CTOR(half_adder_sc_stimulus){SC_THREAD(run);}
};

#endif
