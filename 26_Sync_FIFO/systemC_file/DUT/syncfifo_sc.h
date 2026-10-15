#ifndef SYNCFIFO_SC_H
#define SYNCFIFO_SC_H

#include <systemc>

SC_MODULE(syncfifo) {
    sc_core::sc_in<sc_dt::sc_logic> clk{"clk"}, reset{"reset"}, write_en{"write_en"}, read_en{"read_en"};
    sc_core::sc_in<sc_dt::sc_lv<8>> data_in{"data_in"};
    sc_core::sc_out<sc_dt::sc_logic> full{"full"}, empty{"empty"};
    sc_core::sc_out<sc_dt::sc_lv<8>> out{"out"};

    SC_CTOR(syncfifo) {
        SC_METHOD(write_process);
        sensitive << clk.pos() << reset.neg();
        dont_initialize();
        SC_METHOD(read_process);
        sensitive << clk.pos() << reset.neg();
        dont_initialize();
        SC_METHOD(count_process);
        sensitive << clk.pos() << reset.neg();
        dont_initialize();
        SC_METHOD(flags);
        sensitive << count;
    }

public: // Internal state is exposed only for VCD tracing, not as DUT ports.
    sc_core::sc_signal<sc_dt::sc_lv<8>> memory[8];
    sc_core::sc_signal<sc_dt::sc_lv<3>> write_pointer{"write_pointer"}, read_pointer{"read_pointer"};
    sc_core::sc_signal<sc_dt::sc_lv<4>> count{"count"};

private:
    void flags() {
        const auto c = count.read();
        if (!c.is_01()) { full.write(sc_dt::SC_LOGIC_X); empty.write(sc_dt::SC_LOGIC_X); }
        else {
            full.write(c.to_uint() == 8 ? sc_dt::SC_LOGIC_1 : sc_dt::SC_LOGIC_0);
            empty.write(c.to_uint() == 0 ? sc_dt::SC_LOGIC_1 : sc_dt::SC_LOGIC_0);
        }
    }
    void write_process() {
        if (reset.read() == sc_dt::SC_LOGIC_0) write_pointer.write(0);
        else if (write_en.read() == sc_dt::SC_LOGIC_1 && full.read() == sc_dt::SC_LOGIC_0) {
            const auto ptr = write_pointer.read();
            if (ptr.is_01()) {
                memory[ptr.to_uint()].write(data_in.read());
                write_pointer.write(sc_dt::sc_lv<3>(ptr.to_uint() + 1));
            } else write_pointer.write(sc_dt::sc_lv<3>(sc_dt::SC_LOGIC_X));
        }
    }
    void read_process() {
        if (reset.read() == sc_dt::SC_LOGIC_0) read_pointer.write(0);
        else if (read_en.read() == sc_dt::SC_LOGIC_1 && empty.read() == sc_dt::SC_LOGIC_0) {
            const auto ptr = read_pointer.read();
            if (ptr.is_01()) {
                out.write(memory[ptr.to_uint()].read());
                read_pointer.write(sc_dt::sc_lv<3>(ptr.to_uint() + 1));
            } else {
                out.write(sc_dt::sc_lv<8>(sc_dt::SC_LOGIC_X));
                read_pointer.write(sc_dt::sc_lv<3>(sc_dt::SC_LOGIC_X));
            }
        }
    }
    void count_process() {
        if (reset.read() == sc_dt::SC_LOGIC_0) count.write(0);
        else if (write_en.read() == sc_dt::SC_LOGIC_1 && read_en.read() == sc_dt::SC_LOGIC_0) {
            const auto c = count.read();
            count.write(c.is_01() ? sc_dt::sc_lv<4>(c.to_uint() + 1) : sc_dt::sc_lv<4>(sc_dt::SC_LOGIC_X));
        } else if (write_en.read() == sc_dt::SC_LOGIC_0 && read_en.read() == sc_dt::SC_LOGIC_1) {
            const auto c = count.read();
            count.write(c.is_01() ? sc_dt::sc_lv<4>(c.to_uint() - 1) : sc_dt::sc_lv<4>(sc_dt::SC_LOGIC_X));
        }
    }
};

#endif
