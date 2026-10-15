#ifndef CUSTDESIGN_SC_H
#define CUSTDESIGN_SC_H

#include <systemc>

SC_MODULE(srff) {
    sc_core::sc_in<sc_dt::sc_logic> s{"s"}, r{"r"}, clk{"clk"}, reset{"reset"};
    sc_core::sc_out<sc_dt::sc_logic> q{"q"}, qbar{"qbar"};

    SC_CTOR(srff) {
        SC_METHOD(step);
        sensitive << clk.pos();
        dont_initialize();
    }

private:
    void step() {
        const auto old_q = q.read();
        if (reset.read() == sc_dt::SC_LOGIC_1) q.write(sc_dt::SC_LOGIC_0);
        else q.write(s.read() | ((~r.read()) & old_q));
        // qbar is assigned outside the if/else in the RTL and is not reset.
        qbar.write(~old_q);
    }
};

SC_MODULE(custdesign) {
    sc_core::sc_in<sc_dt::sc_logic> x{"x"}, clk{"clk"}, reset{"reset"};
    sc_core::sc_out<sc_dt::sc_logic> y{"y"};

    SC_CTOR(custdesign)
        : sr1("sr1"), sr2("sr2") {
        sr1.s(b); sr1.r(bbar); sr1.clk(clk); sr1.reset(reset); sr1.q(a); sr1.qbar(abar);
        sr2.s(s2); sr2.r(r2); sr2.clk(clk); sr2.reset(reset); sr2.q(b); sr2.qbar(bbar);
        SC_METHOD(combinational);
        sensitive << a << b << x;
    }

private:
public: // Exposed only for VCD state tracing; these are not DUT ports.
    sc_core::sc_signal<sc_dt::sc_logic> a{"a"}, abar{"abar"}, b{"b"}, bbar{"bbar"};
    sc_core::sc_signal<sc_dt::sc_logic> r2{"r2"}, s2{"s2"};

private:
    void combinational() {
        const auto rvalue = a.read() ^ x.read();
        r2.write(rvalue);
        s2.write(~rvalue);
        y.write(rvalue ^ b.read());
    }
    srff sr1, sr2;
};

#endif
