#ifndef ALU_SC_STIMULUS_H
#define ALU_SC_STIMULUS_H

#include <systemc>
#include <fstream>
#include <iostream>

SC_MODULE(alu_sc_stimulus) {
    sc_core::sc_out<sc_dt::sc_lv<4>> a{"a"},b{"b"};
    sc_core::sc_out<sc_dt::sc_lv<2>> select{"select"};
    sc_core::sc_in<sc_dt::sc_lv<4>> out{"out"};
    sc_core::sc_in<sc_dt::sc_logic> zero{"zero"},carry{"carry"},sign{"sign"},parity{"parity"},overflow{"overflow"};
    unsigned checks=0,failures=0;
    std::ofstream csv,log;
    struct Expected { sc_dt::sc_lv<4> out; sc_dt::sc_logic carry,zero,sign,parity,overflow; };

    static sc_dt::sc_lv<4> bits4(unsigned v){sc_dt::sc_lv<4> r;for(int i=0;i<4;++i)r[i]=((v>>i)&1U)?SC_LOGIC_1:SC_LOGIC_0;return r;}
    static sc_dt::sc_lv<2> bits2(unsigned v){sc_dt::sc_lv<2> r;for(int i=0;i<2;++i)r[i]=((v>>i)&1U)?SC_LOGIC_1:SC_LOGIC_0;return r;}
    static Expected oracle(unsigned x,unsigned y,unsigned op){
        Expected e{sc_dt::sc_lv<4>("0000"),SC_LOGIC_0,SC_LOGIC_0,SC_LOGIC_0,SC_LOGIC_0,SC_LOGIC_0};
        if(op==3&&y==0){
            e.out="XXXX";e.carry=SC_LOGIC_X;e.zero=SC_LOGIC_X;e.sign=SC_LOGIC_X;e.parity=SC_LOGIC_X;
            e.overflow=(((x>>3)&1U)==((y>>3)&1U))?SC_LOGIC_X:SC_LOGIC_0;
            return e;
        }
        unsigned value=op==0?x+y:op==1?((x-y)&31U):op==2?((x*y)&31U):x/y;
        e.carry=(value&16U)?SC_LOGIC_1:SC_LOGIC_0;
        const unsigned low=value&15U;e.out=bits4(low);
        e.zero=low==0?SC_LOGIC_1:SC_LOGIC_0;
        e.sign=(low&8U)?SC_LOGIC_1:SC_LOGIC_0;
        unsigned ones=0;for(unsigned i=0;i<4;++i)ones+=(low>>i)&1U;
        e.parity=(ones%2U==0)?SC_LOGIC_1:SC_LOGIC_0;
        const bool ax=(x&8U)!=0,bx=(y&8U)!=0,ox=(low&8U)!=0;
        const bool ov=(ax&&bx&&!ox)||(!ax&&!bx&&ox);
        e.overflow=ov?SC_LOGIC_1:SC_LOGIC_0;
        return e;
    }
    bool record(const char* name,unsigned x,unsigned y,unsigned op,const Expected&e){
        const auto ao=out.read();const auto ac=carry.read(),az=zero.read(),as=sign.read(),ap=parity.read(),av=overflow.read();
        const bool pass=ao==e.out&&ac==e.carry&&az==e.zero&&as==e.sign&&ap==e.parity&&av==e.overflow;
        ++checks;if(!pass)++failures;const char*st=pass?"PASS":"FAIL";
        const double t=sc_core::sc_time_stamp().to_seconds()*1e9;
        csv<<t<<','<<name<<','<<bits4(x)<<','<<bits4(y)<<','<<bits2(op)<<','<<e.out<<','<<ao<<','
           <<e.carry.to_char()<<','<<ac.to_char()<<','<<e.zero.to_char()<<','<<az.to_char()<<','
           <<e.sign.to_char()<<','<<as.to_char()<<','<<e.parity.to_char()<<','<<ap.to_char()<<','
           <<e.overflow.to_char()<<','<<av.to_char()<<','<<st<<'\n';
        log<<t<<','<<name<<','<<bits4(x)<<','<<bits4(y)<<','<<bits2(op)<<','<<e.out<<','<<ao<<','
           <<e.carry.to_char()<<','<<ac.to_char()<<','<<e.zero.to_char()<<','<<az.to_char()<<','
           <<e.sign.to_char()<<','<<as.to_char()<<','<<e.parity.to_char()<<','<<ap.to_char()<<','
           <<e.overflow.to_char()<<','<<av.to_char()<<','<<st<<'\n';
        std::cout<<"t="<<sc_core::sc_time_stamp()<<" alu a="<<bits4(x)<<" b="<<bits4(y)
                 <<" select="<<bits2(op)<<" expected_out="<<e.out<<" actual_out="<<ao
                 <<" expected_carry="<<e.carry.to_char()<<" actual_carry="<<ac.to_char()
                 <<" expected_zero="<<e.zero.to_char()<<" actual_zero="<<az.to_char()
                 <<" expected_sign="<<e.sign.to_char()<<" actual_sign="<<as.to_char()
                 <<" expected_parity="<<e.parity.to_char()<<" actual_parity="<<ap.to_char()
                 <<" expected_overflow="<<e.overflow.to_char()<<" actual_overflow="<<av.to_char()<<' '<<st<<'\n';
        if(!pass)std::cerr<<"FAIL t="<<sc_core::sc_time_stamp()<<" alu a="<<bits4(x)<<" b="<<bits4(y)
                          <<" select="<<bits2(op)<<" expected_out="<<e.out<<" actual_out="<<ao
                          <<" expected_flags="<<e.carry.to_char()<<e.zero.to_char()<<e.sign.to_char()<<e.parity.to_char()<<e.overflow.to_char()
                          <<" actual_flags="<<ac.to_char()<<az.to_char()<<as.to_char()<<ap.to_char()<<av.to_char()<<'\n';
        return pass;
    }
    void run(){
        csv.open("alu.csv",std::ios::trunc);log.open("simulation.log",std::ios::trunc);
        if(!csv||!log){std::cerr<<"ERROR: cannot create result logs\n";failures=1;sc_core::sc_stop();return;}
        const char*h="time_ns,test_name,a,b,select,expected_out,actual_out,expected_carry,actual_carry,expected_zero,actual_zero,expected_sign,actual_sign,expected_parity,actual_parity,expected_overflow,actual_overflow,status\n";csv<<h;log<<h;
        for(unsigned x=0;x<16;++x)for(unsigned y=0;y<16;++y)for(unsigned op=0;op<4;++op){
            // Force an operand event before each vector, then settle the target operands and opcode together.
            b.write(bits4(y^15U));wait(1,sc_core::SC_NS);
            a.write(bits4(x));b.write(bits4(y));select.write(bits2(op));wait(1,sc_core::SC_NS);
            record("exhaustive",x,y,op,oracle(x,y,op));
        }
        const unsigned x=6,y=3;
        b.write(bits4(12));wait(1,sc_core::SC_NS);a.write(bits4(x));b.write(bits4(y));select.write(bits2(0));wait(1,sc_core::SC_NS);
        record("setup_add",x,y,0,oracle(x,y,0));
        select.write(bits2(1));wait(1,sc_core::SC_NS);
        record("select_only_hold",x,y,0,oracle(x,y,0));
        a.write(bits4(7));wait(1,sc_core::SC_NS);
        record("operand_change_uses_select",7,y,1,oracle(7,y,1));
        csv.flush();log<<"SUMMARY checks="<<checks<<" failures="<<failures<<" csv=alu.csv vcd=alu_wave.vcd\n";log.flush();
        std::cout<<"SUMMARY checks="<<checks<<" failures="<<failures<<" csv=alu.csv vcd=alu_wave.vcd\n";
        if(!csv||!log){std::cerr<<"ERROR: result log write failed\n";++failures;}sc_core::sc_stop();
    }
    SC_CTOR(alu_sc_stimulus){SC_THREAD(run);}
};

#endif
