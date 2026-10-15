#ifndef DFF_SYNC_SC_STIMULUS_H
#define DFF_SYNC_SC_STIMULUS_H

#include <systemc>
#include <fstream>
#include <iostream>

SC_MODULE(dff_sync_sc_stimulus) {
    sc_core::sc_out<sc_dt::sc_logic> i_d{"i_d"},clk{"clk"},reset{"reset"};
    sc_core::sc_in<sc_dt::sc_logic> o_q{"o_q"},o_qb{"o_qb"};
    unsigned checks=0,failures=0;
    std::ofstream csv,log;
    sc_dt::sc_logic expected=SC_LOGIC_X;
    void check(const char* name){
        const auto q=o_q.read(),qb=o_qb.read();
        const bool pass=q==expected&&qb==~expected;++checks;if(!pass)++failures;
        const char*st=pass?"PASS":"FAIL";const double t=sc_core::sc_time_stamp().to_seconds()*1e9;
        const char d=i_d.read().to_char(),cl=clk.read().to_char(),r=reset.read().to_char();
        csv<<t<<','<<name<<','<<d<<','<<cl<<','<<r<<','<<expected.to_char()<<','<<q.to_char()<<','
           <<(~expected).to_char()<<','<<qb.to_char()<<','<<st<<'\n';
        log<<t<<','<<name<<','<<d<<','<<cl<<','<<r<<','<<expected.to_char()<<','<<q.to_char()<<','
           <<(~expected).to_char()<<','<<qb.to_char()<<','<<st<<'\n';
        std::cout<<"t="<<sc_core::sc_time_stamp()<<" dff_sync i_d="<<d<<" clk="<<cl<<" reset="<<r
                 <<" expected_o_q="<<expected.to_char()<<" actual_o_q="<<q.to_char()
                 <<" expected_o_qb="<<(~expected).to_char()<<" actual_o_qb="<<qb.to_char()<<' '<<st<<'\n';
        if(!pass)std::cerr<<"FAIL t="<<sc_core::sc_time_stamp()<<" dff_sync test="<<name
                          <<" d="<<d<<" clk="<<cl<<" reset="<<r
                          <<" expected_q="<<expected.to_char()<<" actual_q="<<q.to_char()
                          <<" expected_qb="<<(~expected).to_char()<<" actual_qb="<<qb.to_char()<<'\n';
    }
    void run(){
        csv.open("dff_sync.csv",std::ios::trunc);log.open("simulation.log",std::ios::trunc);
        if(!csv||!log){std::cerr<<"ERROR: cannot create result logs\n";failures=1;sc_core::sc_stop();return;}
        const char*h="time_ns,test_name,i_d,clk,reset,expected_o_q,actual_o_q,expected_o_qb,actual_o_qb,status\n";csv<<h;log<<h;
        i_d.write(SC_LOGIC_0);clk.write(SC_LOGIC_0);reset.write(SC_LOGIC_1);wait(1,sc_core::SC_NS);
        check("reset_between_edges_hold");
        clk.write(SC_LOGIC_1);wait(1,sc_core::SC_NS);expected=SC_LOGIC_0;check("synchronous_reset_posedge");
        clk.write(SC_LOGIC_0);wait(1,sc_core::SC_NS);check("negedge_hold");
        reset.write(SC_LOGIC_0);
        for(unsigned r=0;r<2;++r)for(unsigned d=0;d<2;++d){
            i_d.write(d?SC_LOGIC_1:SC_LOGIC_0);reset.write(r?SC_LOGIC_1:SC_LOGIC_0);
            wait(1,sc_core::SC_NS);check("inputs_between_edges_hold");
            clk.write(SC_LOGIC_1);wait(1,sc_core::SC_NS);
            expected=r?SC_LOGIC_0:(d?SC_LOGIC_1:SC_LOGIC_0);check("sample_posedge");
            clk.write(SC_LOGIC_0);wait(1,sc_core::SC_NS);check("negedge_hold");
        }
        csv.flush();log<<"SUMMARY checks="<<checks<<" failures="<<failures
                       <<" csv=dff_sync.csv vcd=dff_sync_wave.vcd\n";log.flush();
        std::cout<<"SUMMARY checks="<<checks<<" failures="<<failures
                 <<" csv=dff_sync.csv vcd=dff_sync_wave.vcd\n";
        if(!csv||!log){std::cerr<<"ERROR: result log write failed\n";++failures;}sc_core::sc_stop();
    }
    SC_CTOR(dff_sync_sc_stimulus){SC_THREAD(run);}
};

#endif
