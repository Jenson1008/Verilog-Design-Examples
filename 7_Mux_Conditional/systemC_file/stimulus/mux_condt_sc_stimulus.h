#ifndef MUX_CONDT_SC_STIMULUS_H
#define MUX_CONDT_SC_STIMULUS_H

#include <systemc>
#include <fstream>
#include <iostream>

SC_MODULE(mux_condt_sc_stimulus) {
    sc_core::sc_out<sc_dt::sc_lv<2>> in0{"in0"},in1{"in1"},in2{"in2"},in3{"in3"},select{"select"};
    sc_core::sc_in<sc_dt::sc_lv<2>> out{"out"};
    unsigned checks=0,failures=0;
    std::ofstream csv,log;
    static sc_dt::sc_lv<2> bits2(unsigned v){sc_dt::sc_lv<2> r;for(int i=0;i<2;++i)r[i]=((v>>i)&1U)?SC_LOGIC_1:SC_LOGIC_0;return r;}
    void run(){
        csv.open("mux_condt.csv",std::ios::trunc);log.open("simulation.log",std::ios::trunc);
        if(!csv||!log){std::cerr<<"ERROR: cannot create result logs\n";failures=1;sc_core::sc_stop();return;}
        const char* header="time_ns,test_name,in0,in1,in2,in3,select,expected_out,actual_out,status\n";csv<<header;log<<header;
        for(unsigned a=0;a<4;++a)for(unsigned b=0;b<4;++b)for(unsigned c=0;c<4;++c)for(unsigned d=0;d<4;++d)for(unsigned s=0;s<4;++s){
            in0.write(bits2(a));in1.write(bits2(b));in2.write(bits2(c));in3.write(bits2(d));select.write(bits2(s));wait(1,sc_core::SC_NS);
            const unsigned data[]={a,b,c,d};const auto expected=bits2(data[s]);const auto actual=out.read();
            const bool pass=expected==actual;++checks;if(!pass)++failures;const char*st=pass?"PASS":"FAIL";
            const double t=sc_core::sc_time_stamp().to_seconds()*1e9;
            csv<<t<<",exhaustive,"<<bits2(a)<<','<<bits2(b)<<','<<bits2(c)<<','<<bits2(d)<<','<<bits2(s)<<','<<expected<<','<<actual<<','<<st<<'\n';
            log<<t<<",exhaustive,"<<bits2(a)<<','<<bits2(b)<<','<<bits2(c)<<','<<bits2(d)<<','<<bits2(s)<<','<<expected<<','<<actual<<','<<st<<'\n';
            std::cout<<"t="<<sc_core::sc_time_stamp()<<" mux_condt in0="<<bits2(a)<<" in1="<<bits2(b)<<" in2="<<bits2(c)<<" in3="<<bits2(d)<<" select="<<bits2(s)<<" expected_out="<<expected<<" actual_out="<<actual<<' '<<st<<'\n';
            if(!pass)std::cerr<<"FAIL t="<<sc_core::sc_time_stamp()<<" mux_condt select="<<bits2(s)<<" expected_out="<<expected<<" actual_out="<<actual<<'\n';
        }
        csv.flush();log<<"SUMMARY checks="<<checks<<" failures="<<failures<<" csv=mux_condt.csv vcd=mux_condt_wave.vcd\n";log.flush();
        std::cout<<"SUMMARY checks="<<checks<<" failures="<<failures<<" csv=mux_condt.csv vcd=mux_condt_wave.vcd\n";
        if(!csv||!log){std::cerr<<"ERROR: log write failed\n";++failures;}sc_core::sc_stop();
    }
    SC_CTOR(mux_condt_sc_stimulus){SC_THREAD(run);}
};

#endif
