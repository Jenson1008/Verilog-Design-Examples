CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic
CPPFLAGS ?= -I.
LDLIBS ?= -lsystemc
OUT := out
TARGET := $(OUT)/mealy_ol_sim
SOURCES := mealy_ol_sc_tb.cpp DUT/mealy_ol_sc.h stimulus/mealy_ol_sc_stimulus.h
.PHONY: all run clean
all: $(TARGET)
$(TARGET): $(SOURCES)
	mkdir -p "$(OUT)"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) mealy_ol_sc_tb.cpp $(LDLIBS) -o "$@" > "$(OUT)/build.log" 2>&1 || { status=$$?; cat "$(OUT)/build.log"; exit $$status; }
run: $(TARGET)
	bash -o pipefail -c 'cd "$(OUT)" && ./mealy_ol_sim | tee simulation.log'
clean:
	rm -f "$(TARGET)" "$(OUT)/build.log" "$(OUT)/simulation.log" "$(OUT)/mealy_ol.csv" "$(OUT)/mealy_ol_wave.vcd"
