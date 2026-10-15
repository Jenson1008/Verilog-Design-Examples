CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic
CPPFLAGS ?= -I.
LDLIBS ?= -lsystemc
OUT := out
TARGET := $(OUT)/cntr_3ud_sc_sim
SOURCES := cntr_3ud_sc_tb.cpp DUT/cntr_3ud_sc.h stimulus/cntr_3ud_sc_stimulus.h
.PHONY: all run clean
all: $(TARGET)
$(TARGET): $(SOURCES)
	mkdir -p "$(OUT)"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) cntr_3ud_sc_tb.cpp $(LDLIBS) -o "$@" > "$(OUT)/build.log" 2>&1 || { status=$$?; cat "$(OUT)/build.log"; exit $$status; }
run: $(TARGET)
	bash -o pipefail -c 'cd "$(OUT)" && ./cntr_3ud_sc_sim | tee simulation.log'
clean:
	rm -f "$(TARGET)" "$(OUT)/build.log" "$(OUT)/simulation.log" "$(OUT)/cntr_3ud_sc.csv" "$(OUT)/cntr_3ud_sc_wave.vcd"
