#ifndef MUX_ALWAYS_SC_STIMULUS_H
#define MUX_ALWAYS_SC_STIMULUS_H

#include <systemc>
#include <fstream>
#include <iostream>

SC_MODULE(mux_always_sc_stimulus) {
    sc_core::sc_out<sc_dt::sc_logic> in0{"in0"}, in1{"in1"}, select{"select"};
    sc_core::sc_in<sc_dt::sc_logic> out{"out"};
    unsigned checks=0, failures=0;
    std::ofstream csv,log;
    void run() {
        csv.open("mux_always.csv",std::ios::trunc); log.open("simulation.log",std::ios::trunc);
        if(!csv||!log){std::cerr<<"ERROR: cannot create result logs\n";failures=1;sc_core::sc_stop();return;}
        const char* header="time_ns,test_name,in0,in1,select,expected_out,actual_out,status\n";
        csv<<header;log<<header;
        for(unsigned a=0;a<2;++a) for(unsigned b=0;b<2;++b) for(unsigned s=0;s<2;++s) {
            const auto av=a?SC_LOGIC_1:SC_LOGIC_0, bv=b?SC_LOGIC_1:SC_LOGIC_0;
            const auto sv=s?SC_LOGIC_1:SC_LOGIC_0;
            in0.write(av);in1.write(bv);select.write(sv);wait(1,sc_core::SC_NS);
            const char exp=((s?b:a)!=0)?'1':'0';
            const char act=out.read().to_char();const bool pass=exp==act;
            ++checks;if(!pass)++failures;const char* st=pass?"PASS":"FAIL";
            const double t=sc_core::sc_time_stamp().to_seconds()*1e9;
            csv<<t<<",exhaustive,"<<a<<','<<b<<','<<s<<','<<exp<<','<<act<<','<<st<<'\n';
            log<<t<<",exhaustive,"<<a<<','<<b<<','<<s<<','<<exp<<','<<act<<','<<st<<'\n';
            std::cout<<"t="<<sc_core::sc_time_stamp()<<" mux_always in0="<<a<<" in1="<<b
                     <<" select="<<s<<" expected_out="<<exp<<" actual_out="<<act<<' '<<st<<'\n';
            if(!pass)std::cerr<<"FAIL t="<<sc_core::sc_time_stamp()<<" mux_always inputs="
                               <<a<<','<<b<<','<<s<<" expected_out="<<exp<<" actual_out="<<act<<'\n';
        }
        csv.flush();log<<"SUMMARY checks="<<checks<<" failures="<<failures
                       <<" csv=mux_always.csv vcd=mux_always_wave.vcd\n";log.flush();
        std::cout<<"SUMMARY checks="<<checks<<" failures="<<failures
                 <<" csv=mux_always.csv vcd=mux_always_wave.vcd\n";
        if(!csv||!log){std::cerr<<"ERROR: log write failed\n";++failures;}
        sc_core::sc_stop();
    }
    SC_CTOR(mux_always_sc_stimulus){SC_THREAD(run);}
};

#endif
