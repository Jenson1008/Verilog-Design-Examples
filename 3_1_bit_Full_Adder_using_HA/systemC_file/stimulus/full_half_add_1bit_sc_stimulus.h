#ifndef FULL_HALF_ADD_1BIT_SC_STIMULUS_H
#define FULL_HALF_ADD_1BIT_SC_STIMULUS_H

#include <systemc>
#include <fstream>
#include <iostream>
#include <string>

SC_MODULE(full_half_add_1bit_sc_stimulus) {
    sc_core::sc_out<sc_dt::sc_logic> i_a{"i_a"}, i_b{"i_b"}, i_cin{"i_cin"};
    sc_core::sc_in<sc_dt::sc_logic> o_sum{"o_sum"}, o_carry{"o_carry"};
    sc_core::sc_out<sc_dt::sc_logic> h_a{"h_a"}, h_b{"h_b"};
    sc_core::sc_in<sc_dt::sc_logic> h_sum{"h_sum"}, h_carry{"h_carry"};
    unsigned checks=0,failures=0;
    std::ofstream csv,log;

    void record(const char* name,unsigned a,unsigned b,unsigned cin,unsigned total,
                sc_dt::sc_logic actual_sum,sc_dt::sc_logic actual_carry){
        const char es=(total&1U)?'1':'0',ec=(total&2U)?'1':'0';
        const char as=actual_sum.to_char(),ac=actual_carry.to_char();
        const bool pass=es==as&&ec==ac;++checks;if(!pass)++failures;
        const char*status=pass?"PASS":"FAIL";
        const bool leaf=std::string(name)=="half_adder_leaf";
        const std::string inputs=leaf
            ?"h_a="+std::to_string(a)+";h_b="+std::to_string(b)
            :"i_a="+std::to_string(a)+";i_b="+std::to_string(b)+";i_cin="+std::to_string(cin);
        const double t=sc_core::sc_time_stamp().to_seconds()*1e9;
        csv<<t<<','<<name<<','<<inputs<<','<<es<<','<<as<<','<<ec<<','<<ac<<','<<status<<'\n';
        log<<t<<','<<name<<','<<inputs<<','<<es<<','<<as<<','<<ec<<','<<ac<<','<<status<<'\n';
        std::cout<<"t="<<sc_core::sc_time_stamp()<<' '<<name<<' '<<inputs
                 <<" expected_sum="<<es<<" actual_sum="<<as
                 <<" expected_carry="<<ec<<" actual_carry="<<ac<<' '<<status<<'\n';
        if(!pass)std::cerr<<"FAIL t="<<sc_core::sc_time_stamp()<<' '<<name
                          <<" inputs="<<inputs<<" expected="<<es<<'/'<<ec
                          <<" actual="<<as<<'/'<<ac<<'\n';
    }
    void run(){
        csv.open("full_half_add_1bit.csv",std::ios::trunc);
        log.open("simulation.log",std::ios::trunc);
        if(!csv||!log){std::cerr<<"ERROR: cannot create CSV or simulation.log\n";failures=1;sc_core::sc_stop();return;}
        const char*header="time_ns,test_name,inputs,expected_sum,actual_sum,expected_carry,actual_carry,status\n";
        csv<<header;log<<header;
        for(unsigned a=0;a<2;++a)for(unsigned b=0;b<2;++b){
            h_a.write(a?sc_dt::SC_LOGIC_1:sc_dt::SC_LOGIC_0);
            h_b.write(b?sc_dt::SC_LOGIC_1:sc_dt::SC_LOGIC_0);
            wait(1,sc_core::SC_NS);
            record("half_adder_leaf",a,b,0,a+b,h_sum.read(),h_carry.read());
        }
        for(unsigned a=0;a<2;++a)for(unsigned b=0;b<2;++b)for(unsigned cin=0;cin<2;++cin){
            i_a.write(a?sc_dt::SC_LOGIC_1:sc_dt::SC_LOGIC_0);
            i_b.write(b?sc_dt::SC_LOGIC_1:sc_dt::SC_LOGIC_0);
            i_cin.write(cin?sc_dt::SC_LOGIC_1:sc_dt::SC_LOGIC_0);
            wait(1,sc_core::SC_NS);
            record("full_half_add_1bit",a,b,cin,a+b+cin,o_sum.read(),o_carry.read());
        }
        csv.flush();
        log<<"SUMMARY checks="<<checks<<" failures="<<failures
           <<" csv=full_half_add_1bit.csv vcd=full_half_add_1bit_wave.vcd\n";log.flush();
        std::cout<<"SUMMARY checks="<<checks<<" failures="<<failures
                 <<" csv=full_half_add_1bit.csv vcd=full_half_add_1bit_wave.vcd\n";
        if(!csv||!log){std::cerr<<"ERROR: result log write failed\n";++failures;}
        sc_core::sc_stop();
    }
    SC_CTOR(full_half_add_1bit_sc_stimulus){SC_THREAD(run);}
};

#endif
