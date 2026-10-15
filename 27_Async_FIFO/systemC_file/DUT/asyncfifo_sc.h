#ifndef ASYNCFIFO_SC_H
#define ASYNCFIFO_SC_H

#include <systemc>

SC_MODULE(asyncfifo) {
    sc_core::sc_in<sc_dt::sc_logic> write_clk{"write_clk"}, read_clk{"read_clk"}, reset{"reset"};
    sc_core::sc_in<sc_dt::sc_logic> write_en{"write_en"}, read_en{"read_en"};
    sc_core::sc_in<sc_dt::sc_lv<8>> data_in{"data_in"};
    sc_core::sc_out<sc_dt::sc_logic> mem_full{"mem_full"}, mem_empty{"mem_empty"};
    sc_core::sc_out<sc_dt::sc_lv<8>> out{"out"};

    SC_CTOR(asyncfifo) {
        SC_METHOD(write_process);
        sensitive << write_clk.pos() << reset.neg();
        dont_initialize();
        SC_METHOD(read_process);
        sensitive << read_clk.pos() << reset.neg();
        dont_initialize();
        SC_METHOD(count_process);
        sensitive << write_clk.pos() << read_clk.pos() << reset.neg();
        dont_initialize();
        SC_METHOD(output_events);
        sensitive << write_clk.pos() << read_clk.pos();
        dont_initialize();
        SC_METHOD(flags);
        sensitive << count;
    }

public: // Internal state is exposed only for VCD tracing, not as DUT ports.
    sc_core::sc_signal<sc_dt::sc_lv<8>> mem[8];
    sc_core::sc_signal<sc_dt::sc_lv<3>> write_ptr{"write_ptr"}, read_ptr{"read_ptr"};
    sc_core::sc_signal<sc_dt::sc_lv<4>> count{"count"};

private:
    void flags() {
        const auto c = count.read();
        if (!c.is_01()) { mem_full.write(sc_dt::SC_LOGIC_X); mem_empty.write(sc_dt::SC_LOGIC_X); }
        else {
            mem_full.write(c.to_uint() == 8 ? sc_dt::SC_LOGIC_1 : sc_dt::SC_LOGIC_0);
            mem_empty.write(c.to_uint() == 0 ? sc_dt::SC_LOGIC_1 : sc_dt::SC_LOGIC_0);
        }
    }
    void write_process() {
        if (reset.read() == sc_dt::SC_LOGIC_0) write_ptr.write(0);
        else if (write_en.read() == sc_dt::SC_LOGIC_1 && mem_full.read() == sc_dt::SC_LOGIC_0) {
            const auto ptr = write_ptr.read();
            if (ptr.is_01()) {
                mem[ptr.to_uint()].write(data_in.read());
                write_ptr.write(sc_dt::sc_lv<3>(ptr.to_uint() + 1));
            } else write_ptr.write(sc_dt::sc_lv<3>(sc_dt::SC_LOGIC_X));
        }
    }
    void read_process() {
        if (reset.read() == sc_dt::SC_LOGIC_0) read_ptr.write(0);
        else if (read_en.read() == sc_dt::SC_LOGIC_1 && mem_empty.read() == sc_dt::SC_LOGIC_0) {
            const auto ptr = read_ptr.read();
            read_ptr.write(ptr.is_01() ? sc_dt::sc_lv<3>(ptr.to_uint() + 1) : sc_dt::sc_lv<3>(sc_dt::SC_LOGIC_X));
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
    void output_events() {
        const bool write_event = write_clk.event() && write_clk.read() == sc_dt::SC_LOGIC_1;
        const bool read_event = read_clk.event() && read_clk.read() == sc_dt::SC_LOGIC_1;
        const bool accepted_write = write_event && write_en.read() == sc_dt::SC_LOGIC_1 && mem_full.read() == sc_dt::SC_LOGIC_0;
        const bool accepted_read = read_event && read_en.read() == sc_dt::SC_LOGIC_1 && mem_empty.read() == sc_dt::SC_LOGIC_0;
        if (accepted_write && accepted_read) {
            // The source RTL has competing nonblocking assignments to out
            // from independent clocked processes on coincident active clocks.
            out.write(sc_dt::sc_lv<8>(sc_dt::SC_LOGIC_X));
        } else if (accepted_write) {
            out.write(sc_dt::sc_lv<8>(sc_dt::SC_LOGIC_Z));
        } else if (accepted_read) {
            const auto ptr = read_ptr.read();
            if (ptr.is_01()) out.write(mem[ptr.to_uint()].read());
            else out.write(sc_dt::sc_lv<8>(sc_dt::SC_LOGIC_X));
        }
    }
};

#endif
