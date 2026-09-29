CXX ?= g++
SYSTEMC_HOME ?= /usr
TARGET := out/full_half_add_1bit_sc_tb
SOURCE := full_half_add_1bit_sc_tb.cpp
DUT_HEADER := DUT/full_half_add_1bit_sc.h
STIMULUS_HEADER := stimulus/full_half_add_1bit_sc_stimulus.h
WAVE := out/full_adder_wave.vcd
LOG := out/full_half_add_1bit_sc.log

CPPFLAGS += -I$(SYSTEMC_HOME)/include
CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2
LDFLAGS += -L$(SYSTEMC_HOME)/lib -Wl,-rpath,$(SYSTEMC_HOME)/lib
LDLIBS += -lsystemc

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SOURCE) $(DUT_HEADER) $(STIMULUS_HEADER)
	mkdir -p out
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) "$(SOURCE)" $(LDFLAGS) $(LDLIBS) -o "$@"

run: $(TARGET)
	cd out && ./full_half_add_1bit_sc_tb
	@echo "Log written to $(LOG)"

clean:
	rm -f "$(TARGET)" "$(WAVE)" "$(LOG)"