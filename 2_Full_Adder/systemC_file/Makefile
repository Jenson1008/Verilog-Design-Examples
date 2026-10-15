CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic
CPPFLAGS ?= -IDUT -Istimulus
LDLIBS ?= -lsystemc
OUT := out
TARGET := $(OUT)/full_adder_1bit_sim
SOURCES := full_adder_1bit_sc_tb.cpp DUT/full_adder_1bit_sc.h stimulus/full_adder_1bit_sc_stimulus.h

.PHONY: all run clean
all: $(TARGET)

$(TARGET): $(SOURCES)
	mkdir -p "$(OUT)"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) full_adder_1bit_sc_tb.cpp $(LDLIBS) -o "$@" > "$(OUT)/build.log" 2>&1 || { status=$$?; cat "$(OUT)/build.log"; exit $$status; }

run: $(TARGET)
	cd "$(OUT)" && ./full_adder_1bit_sim

clean:
	rm -f "$(TARGET)" "$(OUT)/build.log" "$(OUT)/simulation.log" "$(OUT)/full_adder_1bit.csv" "$(OUT)/full_adder_1bit_wave.vcd"
